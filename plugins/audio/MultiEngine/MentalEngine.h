#pragma once

#include "audio/effects/applyReverb.h"
#include "audio/filterArray.h"
#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"

class MentalEngine : public Engine {
protected:
    EffectFilterArray<2> filter;

    float velocity = 1.0f;
    float phase = 0.0f;
    float lfoPhase = 0.0f;

    float* fxBuffer = nullptr;
    int bufferIndex = 0;
    float* fxBuffer2 = nullptr;
    int bufferIndex2 = 0;

public:
    // --- 10 Parameters ---

    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get() - 12);
    });

    // 1. Waveform: Dirty Saw to Square
    Val& wave = val(0.0f, "WAVE", { .label = "Saw > Square", .unit = "%" });

    // 2. Distortion: High-gain stage mimicking an overdriven amp
    Val& drive = val(60.0f, "DRIVE", { .label = "Gain/Heat", .unit = "%" });

    // 3. LFO Rate: Speed of the bubbling filter modulation
    Val& lfoRate = val(20.0f, "LFO_RATE", { .label = "LFO Speed", .unit = "%" });

    // 4. LFO Depth: How much the LFO "stretches" the filter
    Val& lfoDepth = val(30.0f, "LFO_MOD", { .label = "LFO Depth", .unit = "%" });

    // 5. Global Cutoff
    Val& cutoff = val(40.0f, "CUTOFF", { .label = "Cutoff", .unit = "%" });

    // 6. Resonance: The 303-style scream
    Val& resonance = val(80.0f, "RES", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // Exponential resonance mapping for better "squeal"
        float res = 0.98f * (1.0f - std::pow(1.0f - p.val.pct(), 3.0f));
        filter.setResonance(res);
    });

    // 7. Env Mod: Filter Envelope depth
    Val& envMod = val(50.0f, "ENV_MOD", { .label = "Env Mod", .unit = "%" });

    // 8. Accent: Velocity-to-Cutoff sensitivity
    Val& accent = val(40.0f, "ACCENT", { .label = "Accent", .unit = "%" });

    // 9. Reverb
    Val& reverb = val(30.0f, "REVERB", { .label = "Reverb", .unit = "%" });

    // 10. Delay
    Val& delay = val(60.0f, "DELAY", { .label = "Delay", .type = VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    MentalEngine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer, float* fxBuffer2)
        : Engine(p, c, "Mental")
        , fxBuffer(fxBuffer)
        , fxBuffer2(fxBuffer2)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            float out = applyReverb(buf[track], reverb.pct(), fxBuffer2, bufferIndex2);
            buf[track] = fxDelay(out);
            return;
        }

        // 1. Oscillator & LFO
        phase = fmodf(phase + (baseFreq / props.sampleRate), 1.0f);

        // LFO (Triangular for smoother "liquid" movement)
        float lfoFreq = (lfoRate.pct() * 8.0f) + 0.1f;
        lfoPhase = fmodf(lfoPhase + (lfoFreq / props.sampleRate), 1.0f);
        float lfoTri = (lfoPhase < 0.5f) ? (lfoPhase * 4.0f - 1.0f) : (3.0f - lfoPhase * 4.0f);
        float lfoMod = lfoTri * lfoDepth.pct();

        float saw = (phase * 2.0f) - 1.0f;
        float square = (phase > 0.5f) ? 0.6f : -0.6f;
        float sig = saw * (1.0f - wave.pct()) + square * wave.pct();

        // 2. High-Gain Electric Guitar Distortion
        if (drive.get() > 0.0f) {
            float gain = 1.0f + (drive.pct() * 25.0f);
            sig *= gain;

            // Stage 1: Soft-Clipping (tanh-like curve for harmonics)
            // Stage 2: Asymmetric shaping (offsetting zero-crossing)
            float offset = 0.15f * drive.pct();
            sig += offset;

            if (sig > 1.0f) sig = 0.85f; // "Hard" sag
            else if (sig < -1.0f) sig = -0.95f;
            else sig = sig * (1.5f - 0.5f * sig * sig); // Soft saturate

            sig -= offset * 0.8f; // Recover DC
        }

        // 3. Filter Processing
        float snap = (velocity * accent.pct());
        float env = (envAmpVal * envMod.pct());
        float cutVal = 0.85f * (cutoff.pct() + env + snap + (lfoMod * 40.0f)) + 0.1f;

        filter.setCutoff(fmaxf(0.01f, fminf(0.99f, cutVal)));
        filter.setSampleData(sig, 0);
        filter.setSampleData(filter.lp[0], 1);
        sig = filter.lp[1];

        sig *= envAmpVal * velocity;

        // 4. Global FX
        sig = applyReverb(sig, reverb.pct(), fxBuffer2, bufferIndex2);
        sig = fxDelay(sig);

        buf[track] = sig;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get() - 12, note);
        // Phases are kept free-running for a more "liquid" analog feel
    }

    float fxDelay(float input)
    {
        if (std::abs(delay.get()) < 0.1f) return input;
        if (delay.get() > 0.0f) return applyDelay(input, delay.get() * 0.01f, fxBuffer, bufferIndex);
        return applyDelay2(input, delay.get() * -0.01f, fxBuffer, bufferIndex);
    }
};