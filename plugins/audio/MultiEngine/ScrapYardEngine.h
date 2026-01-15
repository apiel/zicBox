#pragma once

#include "audio/MultiFx.h"
#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"

class ScrapYardEngine : public Engine {
protected:
    MultiFx multiFx;
    MultiFx multiFx2;

    float velocity = 1.0f;
    float phaseMaster = 0.0f;
    float phaseSlave = 0.0f;

    // DC Offset filter state
    float dcBlockState = 0.0f;

public:
    // --- 10 Parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    // The core of the "Screech" - Slave frequency multiplier
    Val& syncFreq = val(2.0f, "SYNC_MULT", { .label = "Sync Freq", .min = 1.0f, .max = 12.0f });

    // PWM of the slave oscillator
    Val& pulseWidth = val(50.0f, "WIDTH", { .label = "Pulse Width", .unit = "%" });

    // IMPACTFUL: This cross-fades between the Sync Screech and a brutal "Sub" square
    Val& subMix = val(20.0f, "SUB_BEEF", { .label = "Sub Beef", .unit = "%" });

    // Envelope influence on the Sync Freq (The "Pew" or "Scream" sweep)
    Val& envToSync = val(60.0f, "ENV_SYNC", { .label = "Env > Sync", .unit = "%" });

    // BIT-CRUSH: Essential for that lo-fi industrial mental vibe
    Val& crush = val(0.0f, "CRUSH", { .label = "Crush", .unit = "%" });

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
            syncReset = true; // Signal the slave to reset
        }

        // Slave frequency influenced by knob + envelope
        float slaveMult = syncFreq.get() + (envToSync.pct() * envAmpVal * 10.0f);
        float slaveInc = (baseFreq * slaveMult) / props.sampleRate;

        if (syncReset) phaseSlave = 0.0f; // HARD SYNC
        phaseSlave += slaveInc;
        if (phaseSlave > 1.0f) phaseSlave -= 1.0f;

        // 2. Waveform Generation (Pulse)
        float threshold = 0.1f + (pulseWidth.pct() * 0.8f);
        float slaveOut = (phaseSlave > threshold) ? 1.0f : -1.0f;

        // Add Sub Oscillator (Square wave at master frequency)
        float subOut = (phaseMaster > 0.5f) ? 1.0f : -1.0f;

        float out = (slaveOut * (1.0f - subMix.pct())) + (subOut * subMix.pct());

        // 3. BIT CRUSH (In-engine for maximum aggression)
        if (crush.pct() > 0.0f) {
            float levels = 2.0f + (1.0f - crush.pct()) * 16.0f;
            out = roundf(out * levels) / levels;
        }

        // 4. DC Block (Sync creates a lot of DC offset which can kill speakers)
        dcBlockState = out - dcBlockState * 0.995f;
        out = out - dcBlockState;

        // 5. Final Gain and FX
        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());

        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
        phaseMaster = 0.0f;
        phaseSlave = 0.0f;
    }
};