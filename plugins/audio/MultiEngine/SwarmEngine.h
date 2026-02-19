#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "audio/MultiFx.h"

class SwarmEngine : public Engine {
protected:
    MultiFx multiFx;
    float velocity = 1.0f;
    float phases[3] = { 0.0f };
    
    // Filter states
    float v[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; 

public:
    Val& body = val(0.0f, "BODY", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    Val& detune = val(30.0f, "SWARM", { .label = "Swarm", .unit = "%" });

    // We offset cutoff so 0% isn't absolute silence
    Val& cutoff = val(40.0f, "CUTOFF", { .label = "Cutoff", .unit = "%" });
    Val& resonance = val(70.0f, "RESO", { .label = "Resonance", .unit = "%" });

    Val& fmSpeed = val(10.0f, "FM_RATE", { .label = "Mental Rate", .unit = "%" });
    Val& fmDepth = val(0.0f, "FM_DEPTH", { .label = "Mental Depth", .unit = "%" });

    Val& drive = val(20.0f, "DRIVE", { .label = "Distort", .unit = "%" });
    Val& stutter = val(0.0f, "STUTTER", { .label = "Stutter", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX Edit", .unit = "%" });

    SwarmEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Swarm")
        , multiFx(props.sampleRate)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal <= 0.0001f) {
            buf[track] = multiFx.apply(0.0f, fxAmount.pct());
            return;
        }

        // 1. Swarm Oscillators
        float mix = 0.0f;
        float d = detune.pct() * 0.12f;
        float detuneAmts[3] = { 1.0f, 1.0f + d, 1.0f - d * 0.5f };
        
        for (int i = 0; i < 3; i++) {
            phases[i] += (baseFreq * detuneAmts[i]) / props.sampleRate;
            if (phases[i] > 1.0f) phases[i] -= 1.0f;
            mix += (phases[i] * 2.0f - 1.0f);
        }
        mix *= 0.33f;

        // 2. Mental FM (Audio Rate)
        static float fmPh = 0.0f;
        fmPh += (fmSpeed.pct() * 2500.0f + 20.0f) / props.sampleRate;
        if (fmPh > 1.0f) fmPh -= 1.0f;
        float mod = sinf(fmPh * 2.0f * (float)M_PI) * fmDepth.pct() * 0.4f;

        // 3. AGGRESSIVE LADDER FILTER
        // Offset cutoff so 0% is still audible (around 20Hz)
        float cut = CLAMP(0.005f + (cutoff.pct() * 0.95f) + mod, 0.005f, 0.99f);
        
        // Resonance: 4.0 is the point of self-oscillation in a ladder filter
        // We allow it to go slightly above for "Mental" shrieks
        float res = resonance.pct() * 4.2f;

        // Drive: Gain into the filter
        float input = mix * (1.0f + drive.pct() * 10.0f);

        // Resonance Compensation: 
        // Standard filters get quieter when resonance is high. We fix that here.
        input *= (1.0f + res * 0.25f);

        // Main Loop (One-pole stages with non-linear feedback)
        // High-pass the feedback slightly to prevent muddy bottom-end oscillation
        float fb = res * (v[3] - v[2] * 0.5f); 
        float x = input - fb;

        // Stage 1-4 with tanh saturation for "shouting" harmonics
        v[0] += cut * (tanhf(x) - v[0]);
        v[1] += cut * (v[0] - v[1]);
        v[2] += cut * (v[1] - v[2]);
        v[3] += cut * (v[2] - v[3]);

        float out = v[3];

        // 4. Stutter
        if (stutter.pct() > 0.0f) {
            static float gatePhase = 0.0f;
            gatePhase += (2.0f + stutter.pct() * 50.0f) / props.sampleRate;
            if (gatePhase > 1.0f) gatePhase -= 1.0f;
            if (gatePhase > 0.5f) out = 0.0f;
        }

        out = out * envAmpVal * velocity;
        buf[track] = multiFx.apply(out, fxAmount.pct());
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
        // Note: We don't clear filter states here for a smoother "legato" tribe feel
    }
};