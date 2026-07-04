#pragma once

#include "helpers/math.h"
#include "audio/effects/applyReverb.h" 
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

class HorrorEngine : public Engine {
protected:
    MMfilter filter;

    float velocity = 1.0f;
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float hauntPhase = 0.0f;
    float viberPhase = 0.0f;
    
    // Per-note random values [0.0 to 1.0]
    float rndVal = 0.0f;
    float rndDetune = 0.0f;

    float lastOut = 0.0f;
    int holdCounter = 0;

    float *fxBuffer = nullptr;
    int bufferIndex = 0;
    float *fx2Buffer = nullptr;
    int buffer2Index = 0;

public:
    // --- 10 Parameters ---
    
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    // 1. Dread: Controls a slow, dissonant detune between two oscillators
    // Val& dread = val(30.0f, "DREAD", { .label = "Dread (Detune)", .unit = "%" });
    
    // 2. How much the pitch "slips" or drifts when a note starts
    Val& instability = val(40.0f, "SLIP", { .label = "Pitch Slip", .unit = "%" });

    // 3. The "Haunt": A breathing noise floor
    Val& haunt = val(30.0f, "HAUNT", { .label = "Haunt (Noise)", .unit = "%" });

    // 4. Per-note Filter randomness (The "Vowel" shift)
    Val& filterMod = val(50.0f, "FILT_MOD", { .label = "Filt Mod", .unit = "%" });

    // 5. Global Darkness
    Val& cutoff = val(-30.0f, "CUTOFF", { .label = "Darkness", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, valMMfilterCutoff(filter));

    // 6. Global Resonance
    Val& resonance = val(40.0f, "RES", { .label = "Res", .unit = "%" });

    // 7. Lo-fi Redux (Bitcrush/Sample rate)
    Val& redux = val(20.0f, "REDUX", { .label = "Lo-Fi", .unit = "%" });

    // 8. Tension: A very fast, nervous vibrato
    Val& tension = val(15.0f, "TENSION", { .label = "Tension", .unit = "%" });

    // 9. Ghostly Reverb
    Val& reverb = val(50.0f, "REVERB", { .label = "Ghost FX", .unit = "%" });

    // 10. Echo
    Val& delay = val(40.0f, "DELAY", { .label = "Echo", .type = VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    HorrorEngine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer, float* fx2Buffer)
        : Engine(p, c, "Horror")
        , fxBuffer(fxBuffer)
        , fx2Buffer(fx2Buffer)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            float out = applyReverb(buf[track], reverb.pct(), fx2Buffer, buffer2Index);
            buf[track] = fxDelay(out);
            return;
        }

        // 1. Modulations
        // Nervous Vibrato (Tension)
        viberPhase += 8.0f / props.sampleRate; // Fast 8Hz
        if (viberPhase > 1.0f) viberPhase -= 1.0f;
        float tensMod = sinf(viberPhase * 2.0f * M_PI) * (tension.pct() * 0.03f);

        // Breathing Noise (Haunt)
        hauntPhase += 0.5f / props.sampleRate; // Very slow 0.5Hz
        if (hauntPhase > 1.0f) hauntPhase -= 1.0f;
        float breathe = (sinf(hauntPhase * 2.0f * M_PI) + 1.0f) * 0.5f;

        // Pitch Slip (Note On Instability)
        // rndVal decays over time to simulate a tape motor starting up
        float pitchSlip = rndVal * instability.pct() * envAmpVal * 0.1f;

        float freq1 = baseFreq * (1.0f + tensMod + pitchSlip);
        // Dread: Oscillator 2 is dissonantly detuned
        // float detuneAmt = (dread.pct() * 0.05f) + (rndDetune * 0.02f);
        float detuneAmt = 0.05f + (rndDetune * 0.02f);
        float freq2 = freq1 * (1.0f + detuneAmt);

        phase1 = fmodf(phase1 + (freq1 / props.sampleRate), 1.0f);
        phase2 = fmodf(phase2 + (freq2 / props.sampleRate), 1.0f);

        // 2. Oscillators (Dark PWM)
        float osc1 = (phase1 > 0.6f) ? 0.4f : -0.4f;
        float osc2 = (phase2 > 0.4f) ? 0.3f : -0.3f;

        // 3. The Haunt (Filtered Noise Layer)
        float noise = props.lookupTable->getNoise() * haunt.pct() * breathe;
        
        float sig = (osc1 + osc2) + noise;

        // 4. Redux (Sample Rate Reduction)
        if (redux.pct() > 0.0f) {
            int holdSamples = (int)(redux.pct() * 40.0f) + 1;
            if (++holdCounter >= holdSamples) {
                lastOut = sig;
                holdCounter = 0;
            }
            sig = lastOut;
        }

        // 5. Per-Note Filter Mod
        float baseCut = cutoff.get();
        // The filter "closes" slightly as the note decays
        float modCut = baseCut + (rndVal * filterMod.pct() * 0.8f) - ( (1.0f - envAmpVal) * 20.0f );
        filter.setResonance(resonance.pct());
        filter.setCutoff(fmaxf(-1.0f, fminf(1.0f, modCut / 100.0f)));

        sig = filter.process(sig);
        sig *= envAmpVal * velocity;

        // 6. Global FX
        sig = applyReverb(sig, reverb.pct(), fx2Buffer, buffer2Index);
        sig = fxDelay(sig);

        buf[track] = sig;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);

        // Per-note randomization
        rndVal = 1.0f; // Reset slip/mod amount for attack
        rndDetune = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;

        phase1 = 0.0f;
        phase2 = 0.0f;
    }

    float fxDelay(float input) {
        if (std::abs(delay.get()) < 0.1f) return input;
        if (delay.get() > 0.0f) return applyDelay(input, delay.get() * 0.01f, fxBuffer, bufferIndex);
        return applyDelay2(input, delay.get() * -0.01f, fxBuffer, bufferIndex);
    }
};
