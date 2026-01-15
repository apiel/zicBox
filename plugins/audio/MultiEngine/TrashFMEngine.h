#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "audio/MultiFx.h"

class TrashFMEngine : public Engine {
protected:
    MultiFx multiFx;
    MultiFx multiFx2;

    float velocity = 1.0f;
    float phaseCarrier = 0.0f;
    float phaseModulator = 0.0f;
    
    // Decimator states
    float lastOut = 0.0f;
    int decimateCounter = 0;

public:
    // --- 10 Parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    // FM Ratio: 1.0 is harmonic, non-integers (like 1.41) are industrial/metallic
    Val& ratio = val(2.0f, "RATIO", { .label = "FM Ratio", .min = 0.5f, .max = 8.0f, .step = 0.01f });
    
    // FM Intensity: The "rubber" and "grit" of the sound
    Val& fmAmount = val(30.0f, "FM_AMT", { .label = "FM Mod", .unit = "%" });
    
    // IMPACTFUL: This cross-modulates the modulator with itself for "Acid" noise
    Val& feedback = val(0.0f, "FEEDBACK", { .label = "Chaos", .unit = "%" });

    // Envelopes
    Val& envToFm = val(50.0f, "ENV_FM", { .label = "Env > FM", .unit = "%" });
    
    // Hard Tek textures: Decimation (Bitcrush/Sample Rate Reduction)
    Val& decimate = val(0.0f, "DECIMATE", { .label = "Decimate", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX1 Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX1 Edit", .unit = "%" });
    
    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);
    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 Edit", .unit = "%" });

    TrashFMEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "TrashFM")
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

        // 1. FM Logic
        float currentFm = fmAmount.pct() + (envToFm.pct() * envAmpVal);
        float modFreq = baseFreq * ratio.get();
        
        // Modulator Phase
        phaseModulator += modFreq / props.sampleRate;
        if (phaseModulator > 1.0f) phaseModulator -= 1.0f;
        
        // Self-feedback for chaotic noise
        float fb = feedback.pct() * 0.5f;
        float modOut = sinf((phaseModulator + (lastOut * fb)) * 2.0f * M_PI);

        // Carrier Phase
        phaseCarrier += baseFreq / props.sampleRate;
        if (phaseCarrier > 1.0f) phaseCarrier -= 1.0f;
        
        // Main FM calculation
        float out = sinf((phaseCarrier + (modOut * currentFm)) * 2.0f * M_PI);

        // 2. DECIMATION (Crucial for Hard Tek/Mental)
        if (decimate.pct() > 0.0f) {
            int factor = (int)(decimate.pct() * 32.0f) + 1;
            if (++decimateCounter >= factor) {
                lastOut = out;
                decimateCounter = 0;
            }
            out = lastOut;
        } else {
            lastOut = out;
        }

        // 3. Final Output
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
        // We don't reset phases here to allow for "drifting" hypnotic textures
    }
};