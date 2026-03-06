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
    float targetFreq = 100.0f;
    float currentFreq = 100.0f;

    // Per-note randomizer for "Atmospheric" variation
    float rndVal = 0.0f;

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

    // 1. Waveform: Blends between a dirty Saw and a squashed Pulse
    Val& wave = val(0.0f, "WAVE", { .label = "Saw > Square", .unit = "%" });

    // 2. Drive: Turns the acid into an electric guitar
    Val& drive = val(40.0f, "DRIVE", { .label = "Distortion", .unit = "%" });

    // 4. Accent: Adds a "snap" to the filter based on velocity
    Val& accent = val(50.0f, "ACCENT", { .label = "Accent", .unit = "%" });

    // 5. Global Cutoff (Your FilterArray)
    Val& cutoff = val(40.0f, "CUTOFF", { .label = "Cutoff", .unit = "%" });

    // 6. Resonance (The "Scream" factor)
    Val& resonance = val(80.0f, "RES", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        float res = 0.98f * (1.0f - std::pow(1.0f - p.val.pct(), 2.0f));
        filter.setResonance(res);
    });

    // 7. Env Mod: How much the envelope opens the filter
    Val& envMod = val(60.0f, "ENV_MOD", { .label = "Env Mod", .unit = "%" });

    // 8. Chaos: Per-note randomized texture offset
    Val& chaos = val(20.0f, "CHAOS", { .label = "Chaos", .unit = "%" });

    // 9. Reverb: For that atmospheric space
    Val& reverb = val(30.0f, "REVERB", { .label = "Reverb", .unit = "%" });

    // 10. Delay: Essential for the "tribe" feedback loop
    Val& delay = val(60.0f, "DELAY", { .label = "Delay", .type = VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    // 3. Slide: How slow the pitch follows the melody (Key for Mental Tek)
    Val& slide = val(50.0f, "SLIDE", { .label = "Slide/Inertia", .unit = "%" });

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

        // 1. Slide Logic (Portamento)
        targetFreq = baseFreq;
        float slideTime = 1.0f - (slide.pct() * 0.99f);
        currentFreq += (targetFreq - currentFreq) * (slideTime * 0.01f);

        phase = fmodf(phase + (currentFreq / props.sampleRate), 1.0f);

        // 2. Oscillator Generation
        float saw = (phase * 2.0f) - 1.0f;
        float square = (phase > 0.5f) ? 0.5f : -0.5f;
        float sig = saw * (1.0f - wave.pct()) + square * wave.pct();

        // 3. Electric Guitar Style Distortion (Asymmetric Hard Clip)
        if (drive.get() > 0.0f) {
            float gain = 1.0f + (drive.pct() * 15.0f);
            sig *= gain;
            // Asymmetric clipping adds "warm" even harmonics
            if (sig > 0.8f) sig = 0.8f;
            else if (sig < -1.0f) sig = -1.0f;
        }

        // 4. Multi-Stage Filter (Your implementation)
        // Combine Cutoff + EnvMod + Velocity Accent + Per-note Chaos
        float snap = (velocity * accent.pct());
        float mod = (envAmpVal * envMod.pct()) + (rndVal * chaos.pct() * 0.2f);
        float cutVal = 0.85f * (cutoff.pct() + mod + snap) + 0.1f;

        filter.setCutoff(fmaxf(0.01f, fminf(0.99f, cutVal)));
        filter.setSampleData(sig, 0);
        filter.setSampleData(filter.lp[0], 1);
        sig = filter.lp[1];

        sig *= envAmpVal * velocity;

        // 5. Global FX
        sig = applyReverb(sig, reverb.pct(), fxBuffer2, bufferIndex2);
        sig = fxDelay(sig);

        buf[track] = sig;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get() - 12, note);

        // For first note, jump to freq. For subsequent, slide from previous.
        if (envelopAmp.get() <= 0.05f) currentFreq = baseFreq;

        rndVal = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
    }

    float fxDelay(float input)
    {
        if (std::abs(delay.get()) < 0.1f) return input;
        if (delay.get() > 0.0f) return applyDelay(input, delay.get() * 0.01f, fxBuffer, bufferIndex);
        return applyDelay2(input, delay.get() * -0.01f, fxBuffer, bufferIndex);
    }
};