#ifndef _SYNTH_HIHAT_H_
#define _SYNTH_HIHAT_H_

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
    float low1 = 0.0f, high1 = 0.0f, low2 = 0.0f, high2 = 0.0f;
    float bandpassFilter(float input, float centerFreq, float qFactor, float& low, float& high)
    {
        float omega = 2.0f * M_PI * centerFreq / props.sampleRate;
        float alpha = sinf(omega) / (2.0f * qFactor);

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

public:
    /*md **Values**: */

    // /*md - `DURATION` set the duration of the envelop.*/
    // Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 2500.0, .step = 10.0, .unit = "ms" });

    // /*md - `TONE_FREQ` set the frequency of the tone. */
    // Val& toneFreq = val(200.0f, "TONE_FREQ", { "Tone Freq.", .min = 50.0, .max = 1000.0, .step = 1.0, .unit = "Hz" });
    // /*md - `HARMONICS_COUNT` set the harmonics count in the tone. */
    // Val& harmonicsCount = val(1, "HARMONICS_COUNT", { "Harmonics Count", .min = 1, .max = 10, .step = 1 });

    // /*md - `PINK_NOISE` set the pink noise amount. */
    // Val& pinkNoiseAmount = val(0.0f, "PINK_NOISE", { "Pink Noise", .unit = "%" });
    // /*md - `NOISE_MIX` set the noise mix. */
    // Val& noiseMix = val(70.0f, "NOISE_MIX", { "Noise Mix", .unit = "%" });

    // /*md - `TRANSIENT_DURATION` set the transient duration. */
    // Val& transientDuration = val(10.0f, "TRANSIENT_DURATION", { "Transient", .min = 1.0, .max = 50.0, .step = 1.0, .unit = "ms" });
    // /*md - `TRANSIENT_INTENSITY` set the transient intensity. */
    // Val& transientIntensity = val(0.0f, "TRANSIENT_INTENSITY", { "Transient", .unit = "%" });

    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 1000.0, .step = 10.0, .unit = "ms" });
    Val& bandFreq = val(5000.0f, "BAND_FREQ", { "Band Freq.", .min = 2000.0, .max = 9900.0, .step = 100.0, .unit = "Hz" });
    Val& bandQ = val(1.0f, "BAND_Q", { "Band Q", .min = 0.5, .max = 10.0, .step = 0.1, .floatingPoint = 1 });
    Val& transientIntensity = val(1.0f, "TRANSIENT_INTENSITY", { "Transient", .min = 0.0, .max = 2.0, .step = 0.1, .floatingPoint = 1 });

    SynthHiHat(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
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
            float metallicNoise = bandpassFilter(rawNoise, bandFreq.get(), bandQ.get(), low1, high1);

            // Transient component
            if (i < totalSamples / 10) {
                metallicNoise += transientIntensity.get() * whiteNoise();
            }

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

#endif
