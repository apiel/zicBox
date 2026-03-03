#pragma once

#include "helpers/math.h"
#include "audio/effects/applyReverb.h" 
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

class IndustrialEngine : public Engine {
protected:
    MMfilter filter;

    float velocity = 1.0f;
    float phase = 0.0f;
    float viberPhase = 0.0f;
    float shiftLfoPhase = 0.0f; // Slow internal drift
    float modPhase = 0.0f;      // Shifter oscillator phase
    float jitterVal = 0.0f;
    
    // Lo-fi state
    float lastOut = 0.0f;
    int holdCounter = 0;

    float *fxBuffer = nullptr;
    int bufferIndex = 0;

public:
    // --- 10 Parameters ---
    
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    Val& pwm = val(50.0f, "PWM", { .label = "PWM/Shape", .unit = "%" });

    Val& vibrato = val(30.0f, "VIBRATO", { .label = "Vibrato", .unit = "%" });

    Val& jitter = val(20.0f, "JITTER", { .label = "Instability", .unit = "%" });

    Val& grit = val(40.0f, "GRIT", { .label = "Grit (Noise)", .unit = "%" });

    // Frequency Shifter / Ring Mod (Replaces Drive)
    Val& shift = val(0.0f, "SHIFT", { .label = "Freq Shift", .type = VALUE_CENTERED, .min = -100.0f, .max = 100.0f, .unit = "%" });

    Val& redux = val(0.0f, "REDUX", { .label = "Lo-Fi", .unit = "%" });

    Val& cutoff = val(-20.0f, "CUTOFF", { .label = "Filter", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, valMMfilterCutoff(filter));

    Val& resonance = val(40.0f, "RES", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& delay = val(60.0f, "DELAY", { .label = "Delay", .type = VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    // --- Constructor ---
    IndustrialEngine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer)
        : Engine(p, c, "Indust")
        , fxBuffer(fxBuffer)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            buf[track] = fxDelay(buf[track]);
            return;
        }

        // 1. Pitch Instability (Vibrato + Jitter)
        viberPhase += 4.5f / props.sampleRate;
        if (viberPhase > 1.0f) viberPhase -= 1.0f;
        float viberMod = sinf(viberPhase * 2.0f * M_PI) * (vibrato.pct() * 0.02f);

        if (((float)rand() / RAND_MAX) < 0.01f) {
            jitterVal = (((float)rand() / RAND_MAX) - 0.5f) * jitter.pct() * 0.1f;
        }

        float currentFreq = baseFreq * (1.0f + viberMod + jitterVal);
        float phaseInc = currentFreq / props.sampleRate;
        phase += phaseInc;
        if (phase > 1.0f) phase -= 1.0f;

        // 2. Main Oscillator + Noise
        float pulse = (phase > (0.1f + pwm.pct() * 0.8f)) ? 0.5f : -0.5f;
        float noise = props.lookupTable->getNoise() * grit.pct();
        float sig = pulse + noise;

        // 3. Frequency Shifter / Ring Mod Logic
        // Uses a slow 0.2Hz LFO to keep the modulation "swirling"
        shiftLfoPhase += 0.2f / props.sampleRate;
        if (shiftLfoPhase > 1.0f) shiftLfoPhase -= 1.0f;
        float shiftMod = sinf(shiftLfoPhase * 2.0f * M_PI);

        if (std::abs(shift.get()) > 0.1f) {
            // Map param to frequency offset (up to ~200Hz shift) + slow LFO wobble
            float shiftFreq = (shift.get() * 2.0f) + (shiftMod * 5.0f); 
            modPhase += shiftFreq / props.sampleRate;
            if (modPhase > 1.0f) modPhase -= 1.0f;
            
            // Apply Ring Modulation
            sig *= sinf(modPhase * 2.0f * M_PI);
        }

        // 4. Lo-Fi (Sample Rate Reduction)
        if (redux.pct() > 0.0f) {
            int holdSamples = (int)(redux.pct() * 48.0f); 
            if (++holdCounter >= holdSamples) {
                lastOut = sig;
                holdCounter = 0;
            }
            sig = lastOut;
        }

        // 5. Processing Chain
        sig = filter.process(sig);
        sig *= envAmpVal * velocity;

        // 6. Final Delay
        sig = fxDelay(sig);

        buf[track] = sig;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
        // We don't reset modulation phases to keep the drone evolving
    }

    float fxDelay(float input) {
        if (delay.get() > 0.0f) return applyDelay(input, delay.get() * 0.01f, fxBuffer, bufferIndex);
        else if (delay.get() < 0.0f) return applyDelay2(input, delay.get() * -0.01f, fxBuffer, bufferIndex);
        else return input;
    }
};