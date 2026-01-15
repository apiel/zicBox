#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "audio/MultiFx.h"

class SwarmEngine : public Engine {
protected:
    MultiFx multiFx;
    float velocity = 1.0f;
    float phases[3] = { 0.0f };
    float filterState = 0.0f;
    float fmPhase = 0.0f;

public:
    // --- 10 Parameters for Total Chaos ---
    Val& body = val(0.0f, "BODY", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    // The "Swarm" - Detunes 3 saws into a massive industrial wall
    Val& detune = val(30.0f, "SWARM", { .label = "Swarm", .unit = "%" });

    // FILTER CHAOS
    Val& cutoff = val(40.0f, "CUTOFF", { .label = "Cutoff", .unit = "%" });
    Val& resonance = val(70.0f, "RESO", { .label = "Acid", .unit = "%" });

    // THE "MENTAL" MODULATOR: FM on the filter cutoff
    Val& fmSpeed = val(10.0f, "FM_RATE", { .label = "Mental Rate", .unit = "%" });
    Val& fmDepth = val(0.0f, "FM_DEPTH", { .label = "Mental Depth", .unit = "%" });

    // DISTORTION
    Val& drive = val(20.0f, "DRIVE", { .label = "Distort", .unit = "%" });
    
    // RHYTHMIC STUTTER (Internal Gate)
    Val& stutter = val(0.0f, "STUTTER", { .label = "Stutter", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX Edit", .unit = "%" });

    SwarmEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Swarm")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            buf[track] = multiFx.apply(buf[track], fxAmount.pct());
            return;
        }

        // 1. Oscillator Swarm (3 Saws)
        float mix = 0.0f;
        float detuneAmts[3] = { 1.0f, 1.0f + detune.pct() * 0.05f, 1.0f - detune.pct() * 0.03f };
        
        for (int i = 0; i < 3; i++) {
            phases[i] += (baseFreq * detuneAmts[i]) / props.sampleRate;
            if (phases[i] > 1.0f) phases[i] -= 1.0f;
            mix += (phases[i] * 2.0f - 1.0f);
        }
        mix *= 0.33f;

        // 2. High-Speed Filter FM (Mental Tek staple)
        fmPhase += (fmSpeed.pct() * 2000.0f) / props.sampleRate; // Up to 2kHz FM
        if (fmPhase > 1.0f) fmPhase -= 1.0f;
        float mod = sinf(fmPhase * 2.0f * M_PI) * fmDepth.pct();

        // 3. Nonlinear Acid Filter (One-pole with feedback)
        float cut = (cutoff.pct() * 0.8f) + (mod * 0.2f);
        if (cut < 0.01f) cut = 0.01f;
        if (cut > 0.99f) cut = 0.99f;

        // Apply Resonance + Drive
        float fb = resonance.pct() * 1.2f;
        float input = mix - (filterState * fb);
        
        // Soft clipping inside the filter loop
        input = tanhf(input * (1.0f + drive.pct() * 4.0f));
        
        filterState += cut * (input - filterState);
        float out = filterState;

        // 4. Stutter (Rhythmic Gate)
        if (stutter.pct() > 0.0f) {
            float gateFreq = 2.0f + stutter.pct() * 30.0f;
            static float gatePhase = 0.0f;
            gatePhase += gateFreq / props.sampleRate;
            if (gatePhase > 1.0f) gatePhase -= 1.0f;
            if (gatePhase > 0.5f) out = 0.0f;
        }

        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
    }
};