#pragma once

#include "audioPlugin.h"
#include "mapping.h"

#include "plugins/audio/utils/utils.h"

/*md
## SynthPerc

Synth engine to generate percussion.
*/

class SynthPerc : public Mapping {
protected:
    float whiteNoise()
    {
        return props.lookupTable->getNoise();
    }

    // Sine wave oscillator
    float sineWave(float frequency, float phase)
    {
        return sinf(2.0f * M_PI * frequency * phase);
    }

    float fmModulation(float freq, float phase)
    {
        float fmdAmplitude = fmdAmp.pct();
        float modulator = fmdAmplitude > 0.0f ? sineWave(fmFreq.get(), phase) * fmdAmplitude * 10.0f : 0.0f;
        return sineWave(freq + modulator, phase);

        // return sineWave(freq, phase);
    }

    float tanhLookup(float x)
    {
        x = CLAMP(x, -1.0f, 1.0f);
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

    // Bandpass filter for metallic noise characteristics
    float low = 0.0f, high = 0.0f;
    float bandpassFilter(float input)
    {
        float omega = 2.0f * M_PI * bandFreq.get() / props.sampleRate;
        float alpha = sinf(omega) / (2.0f * bandQ.get());

        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosf(omega);
        float a2 = 1.0f - alpha;

        float b0 = alpha;
        float b1 = 0.0f;
        float b2 = -alpha;

        float out = (b0 / a0) * input + (b1 / a0) * low + (b2 / a0) * high - (a1 / a0) * low - (a2 / a0) * high;
        high = low;
        low = out;
        return out;
    }

    // Low-pass filter for tone brightness control
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float lowPassFilter(float input)
    {
        float rc = 1.0f / (2.0f * M_PI * noiseBrightness.get());
        float dt = 1.0f / props.sampleRate;
        float alpha = dt / (rc + dt);
        float output = alpha * input + (1.0f - alpha) * prevOutput;
        prevInput = input;
        prevOutput = output;
        return output;
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

    /*md - `TRANSIENT_DURATION` set the transient duration. */
    Val& transientDuration = val(10.0f, "TRANSIENT_DURATION", { "Transient", .min = 1.0, .max = 50.0, .step = 1.0, .unit = "ms" });
    /*md - `TRANSIENT_INTENSITY` set the transient intensity. */
    Val& transientIntensity = val(0.0f, "TRANSIENT_INTENSITY", { "Transient", .unit = "%" });

    /*md - `BAND_FREQ` set the band frequency. */
    Val& bandFreq = val(5000.0f, "BAND_FREQ", { "Band Freq.", .min = 2000.0, .max = 9900.0, .step = 100.0, .unit = "Hz" });
    /*md - `BAND_Q` set the band Q. */
    Val& bandQ = val(1.0f, "BAND_Q", { "Band Q", .min = 0.5, .max = 10.0, .step = 0.1, .floatingPoint = 1 });
    /*md - `METALLIC_NOISE_MIX` set the metallic noise mix. */
    Val& metallicNoiseMix = val(50.0f, "METALLIC_NOISE_MIX", { "Noise morph", .unit = "%" });
    /*md - `NOISE_BRIGHTNESS` set the noise brightness. */
    Val& noiseBrightness = val(5000.0f, "NOISE_BRIGHTNESS", { "Noise Brightness", .min = 1000.0, .max = 9900.0, .step = 100.0, .unit = "Hz" });

    /*md - `MIX` set the mix between tone and noise. */
    Val& mix = val(50.0f, "MIX", { "Mix", .step = 0.1, .floatingPoint = 1, .unit = "%" });

    /*md - FM_FREQ sets the frequency of the modulator wave. */
    Val& fmFreq = val(50.0f, "FM_FREQ", { "Fm. Freq.", .min = 0.1, .max = 500.0, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });
    /*md - FM_AMP controls the intensity of frequency modulation. */
    Val& fmdAmp = val(0.0f, "FM_AMP", { "Fm. Amp.", .step = 0.1, .floatingPoint = 1, .unit = "%" });
    /*md - ENV_MOD intensity of envelope modulation. */
    Val& envMod = val(0.0f, "ENV_MOD", { "Env. Mod.", .unit = "%" });
    /*md - ENV_SHAPE controls the shape of the envelope. */
    // Val& envShape = val(0.5f, "ENV_SHAPE", { "Env. Shape", .min = 1 });
    Val& envShape = val(0.5f, "ENV_SHAPE", { "Env. Shape", .min = 0.1, .max = 5.0, .step = 0.1, .floatingPoint = 1 });

    SynthPerc(AudioPlugin::Props& props, AudioPlugin::Config& config)
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
    int transientSamples = 0;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float t = (float)i / totalSamples; // Time normalized to [0, 1]
            float env = 1.0f - t;

            float mixTone = (1.0f - mix.pct());
            float mixNoise = mix.pct();

            float tone = 0.0f;
            float noise = 0.0f;

            // https://codesandbox.io/p/sandbox/green-platform-tzl4pn?file=%2Fsrc%2Findex.js%3A18%2C13
            if (mixTone > 0.0f) {
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
                tone = applyBoost(tone, env);
            }

            if (mixNoise > 0.0f) {
                // Generate raw noise
                float rawNoise = whiteNoise();

                // Apply bandpass filters for metallic noise
                float metallicNoise = bandpassFilter(rawNoise);

                // // Transient component
                // if (i < totalSamples / 10) {
                //     metallicNoise += transientIntensity.get() * whiteNoise();
                // }

                // Transient component
                if (i < transientSamples && transientIntensity.get() > 0.0f) {
                    // metallicNoise += transientIntensity.pct() * 5 * whiteNoise() * (1.0f - ((float)(i) / transientSamples));
                    metallicNoise += transientIntensity.get() * whiteNoise(); // * (1.0f - ((float)(i) / transientSamples));
                    // metallicNoise = CLAMP(metallicNoise, -1.0f, 1.0f);
                }

                noise = (metallicNoiseMix.pct() * metallicNoise) + ((1.0f - metallicNoiseMix.pct()) * rawNoise);

                // // Transient component
                // if (i < transientSamples && transientIntensity.pct() > 0.0f) {
                //     noise += transientIntensity.pct() * 5 * whiteNoise() * (1.0f - ((float)(i) / transientSamples));
                //     noise = CLAMP(noise, -1.0f, 1.0f);
                // }
                noise = lowPassFilter(noise);
            }

            float output = (mix.pct() * noise + (1.0f - mix.pct()) * tone) * env;
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
        transientSamples = static_cast<int>(sampleRate * (transientDuration.get() / 1000.0f));
        // printf("totalSamples: %d transientSamples: %d\n", totalSamples, transientSamples);
        phase = 0.0f;
        resonatorState = 0.0f;
        noteFreq = baseFreq.get() * powf(2.0f, (note - baseNote) / 12.0f);
        i = 0;
    }
};
