#pragma once

#include "audioPlugin.h"
#include "mapping.h"

#include "plugins/audio/utils/utils.h"

/*md
## SynthMetalic

Synth engine to generate metalic sounds.
*/

class SynthMetalic : public Mapping {
protected:
    // Sine wave oscillator
    float sineWave(float frequency, float phase)
    {
        return sinf(2.0f * M_PI * frequency * phase);
    }

    float fmModulation(float freq, float phase)
    {
        float fmAmplitude = fmAmp.pct();
        float modulator = fmAmplitude > 0.0f ? sineWave(fmFreq.get(), phase) * fmAmplitude * 10.0f : 0.0f;
        return sineWave(freq + modulator, phase);

        // return sineWave(freq, phase);
    }

    float tanhLookup(float x)
    {
        x = range(x, -1.0f, 1.0f);
        int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1));
        return props.lookupTable->tanh[index];
    }

    float sineLookupInterpolated(float x)
    {
        x -= std::floor(x);
        return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
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
            return input + amount * 3.0f * env * sineWave(noteFreq * 2.0f, phase);
        }
        if (amount < 0.0f) {
            // Might consider using another kind of distortion
            amount = -amount;
            // return input + amount * sinf(input) * 2; // Waveshape <-- this mainly amplify the output
            return input + amount * sineLookupInterpolated(input); // Waveshape LUT
            // return tanhLookup(input * (1.0f + amount * 5.0f)); // Drive
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
    Val& baseFreq = val(100.0f, "BASE_FREQ", { "Base Freq", .min = 10.0, .max = 400.0, .step = 1.0, .unit = "Hz" });
    /*md - `RESONATOR` controls the strength of the resonator. */
    Val& bodyResonance = val(0.8f, "RESONATOR", { "Resonator", .min = 0.00f, .max = 1.5f, .step = 0.01f, .floatingPoint = 2 });
    /*md - `TIMBRE` adjusts the tonal character by shaping the harmonic content. */
    Val& timbre = val(5.0f, "TIMBRE", { "Timbre", .unit = "%" });
    /*md - `BOOST` boost transient or add some distortion. */
    Val& boost = val(0.0, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `TONE_DECAY` adjusts the decay rate of the tonal component. */
    Val& toneDecay = val(0.02f, "TONE_DECAY", { "Tone Decay", .min = 0.005f, .max = 1.0f, .step = 0.005f, .floatingPoint = 2 });
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    /*md - FM_FREQ sets the frequency of the modulator wave. */
    Val& fmFreq = val(50.0f, "FM_FREQ", { "Fm. Freq.", .min = 0.1, .max = 500.0, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });
    /*md - FM_AMP controls the intensity of frequency modulation. */
    Val& fmAmp = val(0.0f, "FM_AMP", { "Fm. Amp.", .step = 0.1, .floatingPoint = 1, .unit = "%" });
    /*md - ENV_MOD intensity of envelope modulation. */
    Val& envMod = val(0.0f, "ENV_MOD", { "Env. Mod.", .unit = "%" });
    /*md - ENV_SHAPE controls the shape of the envelope. */
    // Val& envShape = val(0.5f, "ENV_SHAPE", { "Env. Shape", .min = 1 });
    Val& envShape = val(0.5f, "ENV_SHAPE", { "Env. Shape", .min = 0.1, .max = 5.0, .step = 0.1, .floatingPoint = 1 });

    SynthMetalic(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
        phaseIncrement = 1.0f / props.sampleRate;
    }

    int totalSamples = 0;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    float resonatorState = 0.0f;
    int i = 0;
    float noteFreq = 440.0f;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            // TODO optimize this by precomputing time, see SynthDrum23
            float t = (float)i / totalSamples; // Time normalized to [0, 1]
            float env = 1.0f - t;
            float tone = 0.0f;

            // https://codesandbox.io/p/sandbox/green-platform-tzl4pn?file=%2Fsrc%2Findex.js%3A18%2C13
            float freq = noteFreq;
            if (envMod.pct() > 0.0f) {
                float shape = envShape.get(); // Absolute shape parameter
                float envFactor;

                // Ensure the envelope always decays
                if (shape > 1.0f) {
                    envFactor = pow(1.0f - t, shape); // Power curve decay
                } else if (shape > 0.0f) {
                    envFactor = exp(-t * shape); // Exponential decay
                } else {
                    envFactor = exp(-t); // Default exponential decay if shape is invalid
                }

                freq = freq + freq * envMod.pct() * envFactor;
            }
            // Tonal component with resonance
            // tone = sineWave(freq, phase);
            tone = fmModulation(freq, phase);
            if (bodyResonance.get() > 0.0f) {
                tone = resonator(tone * env, freq * bodyResonance.get(), toneDecay.get(), resonatorState);
            }

            if (timbre.pct() > 0.0f) {
                // Adjust timbre by filtering harmonics dynamically
                tone *= (1.0f - timbre.pct()) + timbre.pct() * sinf(2.0f * M_PI * freq * 0.5f * t);
            }

            float output = applyBoost(tone, env) * env;
            output = applyReverb(output);
            buf[track] = output;

            phase += phaseIncrement;
            i++;
        } else {
            buf[track] = applyReverb(buf[track]);
        }
    }

    uint8_t baseNote = 60;
    uint8_t currentNote = 0;

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        const float sampleRate = props.sampleRate;
        totalSamples = static_cast<int>(sampleRate * (duration.get() / 1000.0f));
        // printf("totalSamples: %d transientSamples: %d\n", totalSamples, transientSamples);
        phase = 0.0f;
        resonatorState = 0.0f;
        noteFreq = baseFreq.get() * powf(2.0f, (note - baseNote) / 12.0f);
        i = 0;
    }
};
