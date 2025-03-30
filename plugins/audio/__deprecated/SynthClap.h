#pragma once

#include "audioPlugin.h"
#include "mapping.h"

/*md
## SynthClap

Synth engine to generate drum clap.
*/
class SynthClap : public Mapping {
protected:
    float whiteNoise()
    {
        return props.lookupTable->getNoise();
    }

    float sineWave(float frequency, int sampleIndex)
    {
        return sinf(2.0f * M_PI * frequency * sampleIndex / props.sampleRate);
    }

    // Multi-stage envelope for clap sound
    float envelope(int sampleIndex)
    {
        float attackTime = 0.005f * props.sampleRate; // Very fast attack (5ms)
        float decayTime = duration.get() * props.sampleRate / 1000.0f; // Decay duration from parameter

        if (sampleIndex < attackTime) {
            return static_cast<float>(sampleIndex) / attackTime; // Linear attack
        } else if (sampleIndex < attackTime + decayTime) {
            float decayIndex = sampleIndex - attackTime;
            return 1.0f - (decayIndex / decayTime); // Linear decay
        } else {
            return 0.0f; // Silence after decay
        }
    }

    // Short envelope specific for sine wave bursts
    float sineEnvelope(int sampleIndex, int offset)
    {
        float durationSamples = 0.01f * props.sampleRate; // 10ms sine burst
        float t = sampleIndex - offset;
        if (t < 0 || t > durationSamples) return 0.0f;
        return 0.5f * (1.0f - (t / durationSamples)); // Short, decaying envelope
    }

    // High-pass filter to shape the clap noise
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float highPassFilter(float input, float cutoff)
    {
        float rc = 1.0f / (2.0f * M_PI * cutoff);
        float dt = 1.0f / props.sampleRate;
        float alpha = rc / (rc + dt);
        float output = alpha * (prevOutput + input - prevInput);
        prevInput = input;
        prevOutput = output;
        return output;
    }

    // Diffusion delay for spatial effect
    float diffusionBuffer[5] = { 0.0f };
    float diffusion(float input)
    {
        float output = 0.0f;
        for (int i = 0; i < 5; i++) {
            output += diffusionBuffer[i];
        }
        output /= 5.0f; // Average

        // Shift diffusion buffer
        for (int i = 4; i > 0; i--) {
            diffusionBuffer[i] = diffusionBuffer[i - 1];
        }
        diffusionBuffer[0] = input;

        return output;
    }

    // Simulate multiple clap transients with random offsets
    static const int maxTransients = 5;
    int transientOffsets[maxTransients];
    float transientAmplitudes[maxTransients];

public:
    /*md **Values**: */
    /*md - DURATION sets the total duration of the clap envelope. */
    Val& duration = val(300.0f, "DURATION", { "Duration", .min = 50.0, .max = 1000.0, .step = 10.0, .unit = "ms" });
    /*md - TONE_CUTOFF sets the base high-pass filter cutoff frequency. */
    Val& toneCutoff = val(2000.0f, "TONE_CUTOFF", { "Tone Cutoff", .min = 500.0f, .max = 5000.0, .step = 50.0, .unit = "Hz" });
    /*md - TRANSIENT_SPREAD adjusts the spread between transients. */
    Val& transientSpread = val(50.0f, "TRANSIENT_SPREAD", { "Transient Spread", .min = 10.0f, .max = 100.0f, .step = 1.0f, .unit = "ms" });
    /*md - SINE_FREQUENCY sets the frequency of the sine wave transient component. */
    Val& sineFrequency = val(800.0f, "SINE_FREQUENCY", { "Sine Frequency", .min = 100.0f, .max = 2000.0f, .step = 10.0f, .unit = "Hz" });
    /*md - SINE_BLEND sets the amplitude of the sine wave transient component. */
    Val& sineBlend = val(50.0f, "SINE_BLEND", { "Sine Blend", .unit = "%" });

    SynthClap(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    int totalSamples = 0;
    int i = 0;

    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float output = 0.0f;

            // Combine transients with random delays
            for (int t = 0; t < maxTransients; ++t) {
                int offset = transientOffsets[t];
                if (i >= offset) {
                    float env = envelope(i - offset) * transientAmplitudes[t];
                    float noise = whiteNoise();
                    noise = highPassFilter(noise, toneCutoff.get() + (t * 500.0f)); // Vary cutoff per transient

                    // Add sine wave for tonal component with short envelope
                    float sine = sineWave(sineFrequency.get() * (1.0f + t * 0.15f), i);
                    float sineEnv = sineEnvelope(i, offset);

                    output += env * noise + sineEnv * sine * sineBlend.pct(); // Blend noise and sine
                }
            }

            // Apply diffusion to create a spatial effect
            output = diffusion(output);

            buf[track] = output;
            i++;
        }
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        const float sampleRate = props.sampleRate;
        float durationSec = duration.get() / 1000.0f;
        totalSamples = static_cast<int>(sampleRate * durationSec);

        // Generate random offsets and amplitudes for transients
        for (int t = 0; t < maxTransients; ++t) {
            transientOffsets[t] = static_cast<int>((transientSpread.get() * sampleRate / 1000.0f) * (t + ((rand() % 10) / 10.0f)));
            transientAmplitudes[t] = 1.0f - (t * 0.2f); // Reduce amplitude for later transients
        }

        i = 0;
    }
};
