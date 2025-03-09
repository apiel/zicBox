#pragma once

#include "audioPlugin.h"
#include "mapping.h"

/*md
## SynthHiHat

Synth engine to generate HiHat sounds.
*/

class SynthHiHat : public Mapping {
protected:
    float whiteNoise()
    {
        return props.lookupTable->getNoise();
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
        float rc = 1.0f / (2.0f * M_PI * toneBrightness.get());
        float dt = 1.0f / props.sampleRate;
        float alpha = dt / (rc + dt);
        float output = alpha * input + (1.0f - alpha) * prevOutput;
        prevInput = input;
        prevOutput = output;
        return output;
    }

public:
    /*md **Values**: */
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 1000.0, .step = 10.0, .unit = "ms" });
    /*md - `BAND_FREQ` set the band frequency. */
    Val& bandFreq = val(5000.0f, "BAND_FREQ", { "Band Freq.", .min = 2000.0, .max = 9900.0, .step = 100.0, .unit = "Hz" });
    /*md - `BAND_Q` set the band Q. */
    Val& bandQ = val(1.0f, "BAND_Q", { "Band Q", .min = 0.5, .max = 10.0, .step = 0.1, .floatingPoint = 1 });
    /*md - `TRANSIENT_INTENSITY` set the transient intensity. */
    Val& transientIntensity = val(1.0f, "TRANSIENT_INTENSITY", { "Transient", .min = 0.0, .max = 2.0, .step = 0.1, .floatingPoint = 1 });
    /*md - `METALLIC_TONE_MIX` set the metallic tone mix. */
    Val& metallicToneMix = val(50.0f, "METALLIC_TONE_MIX", { "Tone Mix", .unit = "%" });
    /*md - `TONE_BRIGHTNESS` set the tone brightness. */
    Val& toneBrightness = val(5000.0f, "TONE_BRIGHTNESS", { "Tone Brightness", .min = 1000.0, .max = 10000.0, .step = 100.0, .unit = "Hz" });

    SynthHiHat(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    int totalSamples = 0;
    float amplitude = 1.0f;
    float decayFactor = 0.0f;
    int i = 0;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float env = 1.0f - (i * decayFactor); // Envelope

            // Generate raw noise
            float rawNoise = whiteNoise();

            // Apply bandpass filters for metallic noise
            float metallicNoise = bandpassFilter(rawNoise);

            // Transient component
            if (i < totalSamples / 10) {
                metallicNoise += transientIntensity.get() * whiteNoise();
            }

            metallicNoise = (metallicToneMix.pct() * metallicNoise) + ((1.0f - metallicToneMix.pct()) * rawNoise);

            // Apply tone brightness (low-pass filter)
            metallicNoise = lowPassFilter(metallicNoise);

            buf[track] = metallicNoise * env;
            i++;
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        const float sampleRate = props.sampleRate; // Assumes standard sample rate
        float durationSec = duration.get() / 1000.0f;
        totalSamples = static_cast<int>(sampleRate * durationSec);
        decayFactor = 1.0f / totalSamples;

        i = 0;
    }
};
