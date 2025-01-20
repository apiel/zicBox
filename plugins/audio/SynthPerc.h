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
        state += freq / props.sampleRate;
        float output = input * expf(-decay * state) * sinf(2.0f * M_PI * freq * state);
        return output;
    }

public:
    /*md **Values**: */

    /*md - `DURATION` controls the duration of the envelope. */
    Val& duration = val(500.0f, "DURATION", { "Duration", .min = 50.0, .max = 3000.0, .step = 10.0, .unit = "ms" });

    /*md - `BASE_FREQ` sets the base frequency of the percussive tone. */
    Val& baseFreq = val(100.0f, "BASE_FREQ", { "Base Freq", .min = 40.0, .max = 400.0, .step = 1.0, .unit = "Hz" });

    /*md - `TONE_DECAY` adjusts the decay rate of the tonal component. */
    Val& toneDecay = val(0.02f, "TONE_DECAY", { "Tone Decay", .min = 0.001f, .max = 0.1f, .step = 0.001f });

    /*md - `BODY_RESONANCE` controls the strength of the resonator. */
    Val& bodyResonance = val(0.8f, "BODY_RESONANCE", { "Resonance", .min = 0.1f, .max = 1.5f, .step = 0.1f });

    /*md - `TIMBRE` adjusts the tonal character by shaping the harmonic content. */
    Val& timbre = val(0.5f, "TIMBRE", { "Timbre", .min = 0.0f, .max = 1.0f, .step = 0.1f });

    /*md - `TRANSIENT_BOOST` amplifies the sharpness and volume of the attack transient. */
    Val& transientBoost = val(1.0f, "TRANSIENT_BOOST", { "Transient Boost", .min = 0.5f, .max = 3.0f, .step = 0.1f });

    /*md - `DISTORTION` adds a nonlinear distortion effect to the tone for aggressive and punchy sounds. */
    Val& distortion = val(0.0f, "DISTORTION", { "Distortion", .min = 0.0f, .max = 1.0f, .step = 0.1f });

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
            float env = envelope(t, 2.0f); // Fixed shape for simpler design

            // Tonal component with resonance
            float tone = sineWave(baseFreq.get(), phase);
            tone = resonator(tone * env, baseFreq.get() * bodyResonance.get(), toneDecay.get(), resonatorState);

            // Adjust timbre by filtering harmonics dynamically
            tone *= (1.0f - timbre.get()) + timbre.get() * sinf(2.0f * M_PI * baseFreq.get() * 0.5f * t);

            // Apply distortion for a more aggressive tone
            tone = tone * (1.0f - distortion.get()) + tanh(tone * distortion.get());

            // Add boosted attack transient
            float transient = transientBoost.get() * env * sineWave(baseFreq.get() * 2.0f, phase);

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
