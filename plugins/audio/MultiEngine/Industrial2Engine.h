#pragma once

#include "helpers/math.h"
#include "audio/effects/applyReverb.h" 
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

class Industrial2Engine : public Engine {
protected:
    MMfilter filter;

    float velocity = 1.0f;
    float phase = 0.0f;
    float viberPhase = 0.0f;
    float modPhase = 0.0f;
    
    // Per-note randomized offsets
    float rndCutoff = 0.0f;
    float rndShift = 0.0f;
    float rndPwm = 0.0f;
    float jitterVal = 0.0f;

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

    Val& pwm = val(50.0f, "PWM", { .label = "Shape", .unit = "%" });

    Val& vibrato = val(30.0f, "VIBRATO", { .label = "Vibrato", .unit = "%" });

    // The "Chaos" dial
    Val& modulation = val(40.0f, "MOD_AMT", { .label = "Chaos", .unit = "%" });

    Val& grit = val(40.0f, "GRIT", { .label = "Grit", .unit = "%" });

    Val& shift = val(20.0f, "SHIFT", { .label = "Shift", .unit = "%" });

    Val& redux = val(0.0f, "REDUX", { .label = "Lo-Fi", .unit = "%" });

    // We keep the callback here to ensure the filter is initialized properly
    Val& cutoff = val(-20.0f, "CUTOFF", { .label = "Filter", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, valMMfilterCutoff(filter));

    Val& resonance = val(40.0f, "RES", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& delay = val(60.0f, "DELAY", { .label = "Delay", .type = VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    Industrial2Engine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer)
        : Engine(p, c, "Indst2")
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

        // 1. Pitch & Vibrato
        viberPhase += 4.5f / props.sampleRate;
        if (viberPhase > 1.0f) viberPhase -= 1.0f;
        float viberMod = sinf(viberPhase * 2.0f * M_PI) * (vibrato.pct() * 0.02f);

        if (((float)rand() / RAND_MAX) < 0.01f) {
            jitterVal = (((float)rand() / RAND_MAX) - 0.5f) * 0.05f;
        }

        float currentFreq = baseFreq * (1.0f + viberMod + jitterVal);
        phase = fmodf(phase + (currentFreq / props.sampleRate), 1.0f);

        // 2. Oscillator with Per-Note PWM
        float currentPwm = pwm.pct() + (rndPwm * modulation.pct() * 0.4f);
        currentPwm = fmaxf(0.05f, fminf(0.95f, currentPwm));
        float sig = (phase > currentPwm) ? 0.4f : -0.4f;
        
        sig += props.lookupTable->getNoise() * grit.pct() * 0.5f;

        // 3. Per-Note Frequency Shifter / Ring Mod
        // Map shift to a more audible frequency range
        float currentShiftFreq = (shift.pct() * 150.0f) + (rndShift * modulation.pct() * 300.0f);
        modPhase = fmodf(modPhase + (currentShiftFreq / props.sampleRate), 1.0f);
        sig *= sinf(modPhase * 2.0f * M_PI);

        // 4. Lo-Fi (Redux)
        if (redux.pct() > 0.0f) {
            int holdSamples = (int)(redux.pct() * 64.0f) + 1; 
            if (++holdCounter >= holdSamples) {
                lastOut = sig;
                holdCounter = 0;
            }
            sig = lastOut;
        }

        // 5. Per-Note Filter
        // IMPORTANT: We use the valMMfilterCutoff logic indirectly to avoid dead filter states
        float baseCutoffVal = cutoff.get(); 
        float finalCutoffVal = baseCutoffVal + (rndCutoff * modulation.pct() * 40.0f);
        finalCutoffVal = fmaxf(-100.0f, fminf(100.0f, finalCutoffVal));
        
        // Manual coefficient update for the filter
        filter.setCutoff(finalCutoffVal / 100.0f); 

        sig = filter.process(sig);
        sig *= envAmpVal * velocity;

        // 6. Delay
        sig = fxDelay(sig);

        buf[track] = sig;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);

        // Randomize offsets [-1.0 to 1.0]
        rndCutoff = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        rndShift  = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        rndPwm    = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;

        // Keep phases moving or reset for clicky attack
        phase = 0.0f;
        modPhase = 0.0f;
    }

    float fxDelay(float input) {
        if (delay.get() > 0.1f) return applyDelay(input, delay.get() * 0.01f, fxBuffer, bufferIndex);
        else if (delay.get() < -0.1f) return applyDelay2(input, delay.get() * -0.01f, fxBuffer, bufferIndex);
        return input;
    }
};