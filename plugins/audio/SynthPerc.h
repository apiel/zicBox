#ifndef _SYNTH_PERC_H_
#define _SYNTH_PERC_H_

#include "audioPlugin.h"
#include "mapping.h"

/*md
## SynthPerc

Synth engine to generate percussion.
*/

class SynthPerc : public Mapping {
protected:
    // Sine wave oscillator
    float sineWave(float frequency, float phase)
    {
        return sinf(2.0f * M_PI * frequency * phase);
    }

    // Exponential decay envelope
    float envelope(float t, float shape)
    {
        return powf(1.0f - t, shape);
    }

    // Resonance simulation for body tone
    float resonator(float input, float freq, float decay, float& state)
    {
        float damped = input * expf(-decay * state);
        state += freq;
        return damped;
    }

public:
    /*md **Values**: */

    /*md - `DURATION` controls the duration of the envelope. */
    Val& duration = val(500.0f, "DURATION", { "Duration", .min = 50.0, .max = 3000.0, .step = 10.0, .unit = "ms" });

    /*md - `BASE_FREQ` sets the base frequency of the percussive tone. */
    Val& baseFreq = val(100.0f, "BASE_FREQ", { "Base Freq", .min = 40.0, .max = 400.0, .step = 1.0, .unit = "Hz" });

    /*md - `OVERTONES` adjusts the number of harmonic overtones. */
    Val& overtones = val(3, "OVERTONES", { "Overtones", .min = 1, .max = 8, .step = 1 });

    /*md - `RES_DECAY` sets the decay rate of the resonator for body tone. */
    Val& resDecay = val(0.01f, "RES_DECAY", { "Resonance Decay", .min = 0.001f, .max = 0.1f, .step = 0.001f });

    /*md - `PUNCH` controls the attack sharpness of the transient. */
    Val& punch = val(0.8f, "PUNCH", { "Punch", .min = 0.5f, .max = 2.0f, .step = 0.1f });

    /*md - `HARMONIC_BALANCE` balances the strength of overtones. */
    Val& harmonicBalance = val(0.5f, "HARMONIC_BALANCE", { "Harmonic Balance", .min = 0.0f, .max = 1.0f, .step = 0.05f });

    /*md - `DECAY_SHAPE` controls the exponential decay curve shape. */
    Val& decayShape = val(2.0f, "DECAY_SHAPE", { "Decay Shape", .min = 1.0f, .max = 5.0f, .step = 0.1f });

    SynthPerc(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    int totalSamples = 0;
    float phase = 0.0f;
    float resonatorState = 0.0f;
    int i = 0;

    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float t = (float)i / totalSamples; // Time normalized to [0, 1]
            float env = envelope(t, decayShape.get());

            float tone = 0.0f;
            for (int h = 1; h <= overtones.get(); ++h) {
                float harmonicFreq = baseFreq.get() * h;
                float harmonicAmp = 1.0f / powf(h, harmonicBalance.get());
                tone += harmonicAmp * sineWave(harmonicFreq, phase);
            }

            // Apply resonance for body simulation
            tone = resonator(tone * env, baseFreq.get(), resDecay.get(), resonatorState);

            // Add punchy attack transient
            float transient = punch.get() * env * sineWave(baseFreq.get() * 2.0f, phase);

            buf[track] = env * (tone + transient);

            phase += 1.0f / props.sampleRate;
            i++;
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        const float sampleRate = props.sampleRate;
        const float durationSec = duration.get() / 1000.0f;
        totalSamples = static_cast<int>(sampleRate * durationSec);
        phase = 0.0f;
        resonatorState = 0.0f;
        i = 0;
    }
};

#endif
