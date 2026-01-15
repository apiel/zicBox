#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "audio/MultiFx.h"

class ScrapYardEngine : public Engine {
protected:
    MultiFx multiFx;
    MultiFx multiFx2;

    float velocity = 1.0f;
    float phaseMaster = 0.0f;
    float phaseSlave = 0.0f;
    
    // Decimator (Crush) states
    float lastOut = 0.0f;
    int holdCounter = 0;

    // DC Offset filter state
    float dcBlockState = 0.0f;

public:
    // --- 10 Parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    Val& syncFreq = val(2.0f, "SYNC_MULT", { .label = "Sync Freq", .min = 1.0f, .max = 12.0f });
    
    Val& pulseWidth = val(50.0f, "WIDTH", { .label = "Pulse Width", .unit = "%" });
    
    Val& subMix = val(20.0f, "SUB_BEEF", { .label = "Sub Beef", .unit = "%" });

    Val& envToSync = val(60.0f, "ENV_SYNC", { .label = "Env > Sync", .unit = "%" });

    // FIXED: Real Decimation Crush
    Val& crush = val(0.0f, "CRUSH", { .label = "Decimate", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX1 Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX1 Edit", .unit = "%" });
    
    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);
    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 Edit", .unit = "%" });

    ScrapYardEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "ScrapYard")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            float out = buf[track];
            out = multiFx.apply(out, fxAmount.pct());
            buf[track] = out;
            return;
        }

        // 1. Hard Sync Logic
        float masterInc = baseFreq / props.sampleRate;
        phaseMaster += masterInc;
        
        bool syncReset = false;
        if (phaseMaster >= 1.0f) {
            phaseMaster -= 1.0f;
            syncReset = true; 
        }

        // Slave frequency influenced by Sync Knob + Env + Internal Feedback
        float slaveMult = syncFreq.get() + (envToSync.pct() * envAmpVal * 10.0f);
        
        float slaveInc = (baseFreq * slaveMult) / props.sampleRate;
        
        if (syncReset) phaseSlave = 0.0f; 
        phaseSlave += slaveInc;
        if (phaseSlave > 1.0f) phaseSlave -= 1.0f;

        // 2. Waveform Generation
        float threshold = 0.1f + (pulseWidth.pct() * 0.8f);
        float slaveOut = (phaseSlave > threshold) ? 1.0f : -1.0f;
        float subOut = (phaseMaster > 0.5f) ? 1.0f : -1.0f;
        
        float currentOut = (slaveOut * (1.0f - subMix.pct())) + (subOut * subMix.pct());

        // 3. THE FIXED CRUSH (Sample Rate Reduction)
        // This will now definitely be audible!
        if (crush.pct() > 0.0f) {
            int holdSamples = (int)(crush.pct() * 64.0f); // Stay stuck for up to 64 samples
            if (++holdCounter >= holdSamples) {
                lastOut = currentOut;
                holdCounter = 0;
            }
        } else {
            lastOut = currentOut;
        }

        float finalOut = lastOut;

        // 4. DC Block
        dcBlockState = finalOut - dcBlockState * 0.995f; 
        finalOut = finalOut - dcBlockState;

        // 5. Final Gain and FX
        finalOut = finalOut * envAmpVal * velocity;
        finalOut = multiFx.apply(finalOut, fxAmount.pct());
        finalOut = multiFx2.apply(finalOut, fx2Amount.pct());

        buf[track] = finalOut;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
        phaseMaster = 0.0f;
        phaseSlave = 0.0f;
        holdCounter = 0;
        lastOut = 0.0f;
    }
};