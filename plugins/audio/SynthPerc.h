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

    float applyBoost(float input, float env)
    {
        float amount = boost.pct() * 2 - 1.0f;
        if (amount > 0.0f) {
            return input + amount * 3.0f * env * sineWave(baseFreq.get() * 2.0f, phase);
        }
        if (amount < 0.0f) {
            float distortion = -amount;
            return input * (1.0f - distortion) + tanh(input * distortion);
        }
        return input;
    }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;
    float applyReverb(float signal)
    {
        float reverbAmount = reverb.pct();
        if (reverbAmount == 0.0f) {
            return signal;
        }
        int reverbSamples = static_cast<int>((reverbAmount * 0.5f) * props.sampleRate); // Reverb duration scaled
        float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
        float mix = reverbAmount * 0.5f; // Mix scaled proportionally

        if (reverbSamples > REVERB_BUFFER_SIZE) {
            reverbSamples = REVERB_BUFFER_SIZE; // Cap the reverb duration to buffer size
        }

        float reverbSignal = reverbBuffer[reverbIndex];
        reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;
        reverbIndex = (reverbIndex + 1) % reverbSamples;

        return signal * (1.0f - mix) + reverbSignal * mix;
    }

public:
    /*md **Values**: */

    /*md - `DURATION` controls the duration of the envelope. */
    Val& duration = val(500.0f, "DURATION", { "Duration", .min = 50.0, .max = 3000.0, .step = 10.0, .unit = "ms" });
    /*md - `BASE_FREQ` sets the base frequency of the percussive tone. */
    Val& baseFreq = val(100.0f, "BASE_FREQ", { "Base Freq", .min = 40.0, .max = 400.0, .step = 1.0, .unit = "Hz" });
    /*md - `RESONATOR` controls the strength of the resonator. */
    Val& bodyResonance = val(0.8f, "RESONATOR", { "resonator", .min = 0.01f, .max = 1.5f, .step = 0.01f, .floatingPoint = 2 });
    /*md - `TIMBRE` adjusts the tonal character by shaping the harmonic content. */
    Val& timbre = val(5.0f, "TIMBRE", { "Timbre", .unit = "%" });
    /*md - `BOOST` boost transient or add some distortion. */
    Val& boost = val(0.0, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });

    // TODO tone decay and reverb doesnt work well together, so we could put them on the same parameter...
    /*md - `TONE_DECAY` adjusts the decay rate of the tonal component. */
    Val& toneDecay = val(0.02f, "TONE_DECAY", { "Tone Decay", .min = 0.005f, .max = 1.0f, .step = 0.005f, .floatingPoint = 2 });
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });

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

            if (timbre.pct() > 0.0f) {
                // Adjust timbre by filtering harmonics dynamically
                tone *= (1.0f - timbre.pct()) + timbre.pct() * sinf(2.0f * M_PI * baseFreq.get() * 0.5f * t);
            }
            float output = tone * env;
            output = applyReverb(output);
            output = applyBoost(tone, env);
            buf[track] = output;

            phase += 1.0f / props.sampleRate;
            i++;
        } else {
            buf[track] = applyReverb(buf[track]);
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
