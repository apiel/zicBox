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
    Val& toneDecay = val(0.02f, "TONE_DECAY", { "Tone Decay", .min = 0.005f, .max = 1.0f, .step = 0.005f, .floatingPoint = 2 });

    /*md - `BODY_RESONANCE` controls the strength of the resonator. */
    Val& bodyResonance = val(0.8f, "BODY_RESONANCE", { "Resonance", .min = 0.1f, .max = 1.5f, .step = 0.01f, .floatingPoint = 2 });

    /*md - `PUNCH` controls the attack sharpness of the transient. */
    Val& punch = val(0.8f, "PUNCH", { "Punch", .min = 0.5f, .max = 2.0f, .step = 0.1f });

    /*md - `HIT_POSITION` modulates the tonal quality based on hit position. */
    Val& hitPosition = val(0.5f, "HIT_POSITION", { "Hit Position", .min = 0.0f, .max = 1.0f, .step = 0.1f });

    /*md - `DECAY_SHAPE` controls the exponential decay curve shape. */
    Val& decayShape = val(2.0f, "DECAY_SHAPE", { "Decay Shape", .min = 1.0f, .max = 5.0f, .step = 0.1f, .floatingPoint = 1 });

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

            // Tonal component with resonance
            float tone = sineWave(baseFreq.get(), phase);
            tone = resonator(tone * env, baseFreq.get() * bodyResonance.get(), toneDecay.get(), resonatorState);

            // Modulate tonality based on hit position
            float modulatedTone = tone * (1.0f - hitPosition.get()) + tone * hitPosition.get() * 1.5f;

            // Add punchy attack transient
            float transient = punch.get() * env * sineWave(baseFreq.get() * 2.0f, phase);

            buf[track] = env * (modulatedTone + transient);

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
