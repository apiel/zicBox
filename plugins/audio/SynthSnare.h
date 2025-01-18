#ifndef _SYNTH_SNARE_H_
#define _SYNTH_SNARE_H_

#include "audioPlugin.h"
#include "mapping.h"

/*md
## SynthSnare

Synth engine to generate snare sounds.
*/

class SynthSnare : public Mapping {
protected:
    uint8_t baseNote = 60;

    // A simple random number generator for white noise
    float whiteNoise()
    {
        return (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
    }

    // Pink noise approximation (simple filter-based approach)
    float pinkNoise(float input, float& state)
    {
        state = 0.98f * state + 0.02f * input; // Low-pass filter approximation for pink noise
        return state;
    }

public:
    /*md **Values**: */

    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 2500.0, .step = 10.0, .unit = "ms" });
    Val& toneFreq = val(200.0f, "TONE_FREQ", { "Tone Freq.", .min = 50.0, .max = 1000.0, .step = 1.0, .unit = "Hz" });
    Val& noiseMix = val(70.0f, "NOISE_MIX", { "Noise Mix" });

    // Transient control parameters
    Val& transientDuration = val(10.0f, "TRANSIENT_DURATION", { "Transient", .min = 1.0, .max = 50.0, .step = 1.0, .unit = "ms" });
    Val& transientIntensity = val(0.0f, "TRANSIENT_INTENSITY", { "Transient", .unit = "%" });

    // Noise type parameter (0 = White, 1 = Pink)
    Val& noiseType = val(0, "NOISE_TYPE", { "Noise Type", .min = 0, .max = 1, .step = 1 });

    // Harmonics control
    Val& harmonicsCount = val(1, "HARMONICS_COUNT", { "Harmonics Count", .min = 1, .max = 10, .step = 1 });

    // // Filter parameters
    // Val& filterCutoff = val(5000.0f, "FILTER_CUTOFF", { "Filter Cutoff", .min = 100.0, .max = 20000.0, .step = 100.0, .unit = "Hz" });
    // Val& filterResonance = val(0.7f, "FILTER_RESONANCE", { "Filter Resonance", .min = 0.1, .max = 5.0, .step = 0.1 });

    SynthSnare(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    // float filterState1 = 0.0f; // Additional state for resonant filter
    // float filterState2 = 0.0f;

    float pinkNoiseState = 0.0f;
    int totalSamples = 0;
    int transientSamples = 0;
    int i = 0;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float decayFactor = 1.0f / totalSamples; // Linear decay

            float env = 1.0f - (i * decayFactor); // Envelope

            float noise = whiteNoise() * env;
            // float noise = pinkNoise(whiteNoise(), pinkNoiseState) * env;

            // Transient component
            if (i < transientSamples && transientIntensity.pct() > 0.0f) {
                noise += transientIntensity.pct() * 5 * whiteNoise() * (1.0f - (static_cast<float>(i) / transientSamples));
            }

            // float t = static_cast<float>(i) / props.sampleRate;
            // float tone = sinf(2.0f * M_PI * toneFreq.get() * t) * env;

            // Tonal component with harmonics
            float t = static_cast<float>(i) / props.sampleRate;
            float tone = 0.0f;
            for (int h = 1; h <= harmonicsCount.get(); ++h) {
                tone += (1.0f / h) * sinf(2.0f * M_PI * toneFreq.get() * h * t);
            }
            tone = tone / harmonicsCount.get() * env;

            buf[track] = (noise * noiseMix.pct()) + (tone * (1.0f - noiseMix.pct()));
            i++;
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        // Generate a single snare drum sample buffer
        const float sampleRate = props.sampleRate; // Assumes standard sample rate
        const float durationSec = duration.get() / 1000.0f;
        totalSamples = static_cast<int>(sampleRate * durationSec);
        transientSamples = static_cast<int>(sampleRate * (transientDuration.get() / 1000.0f));
        i = 0;
    }
};

// void sample(float* buf) override
// {
//     const float sampleRate = 44100.0f;
//     const float durationSec = duration.get() / 1000.0f;
//     const int totalSamples = static_cast<int>(sampleRate * durationSec);
//     const int transientSamples = static_cast<int>(sampleRate * (transientDuration.get() / 1000.0f));

//     float amplitude = 1.0f;
//     float decayFactor = 1.0f / totalSamples;
//     float pinkNoiseState = 0.0f;
//     float filterState = 0.0f;

//     for (int i = 0; i < totalSamples; ++i) {
//         float env = 1.0f - (i * decayFactor); // Linear envelope

//         // Noise component
//         float rawNoise = whiteNoise();
//         float noise = (noiseType.get() == 0) ? rawNoise : pinkNoise(rawNoise, pinkNoiseState);
//         noise *= env;

//         // Transient component
//         if (i < transientSamples) {
//             noise += transientIntensity.get() * whiteNoise() * (1.0f - (static_cast<float>(i) / transientSamples));
//         }

//         // Tonal component with harmonics
//         float t = static_cast<float>(i) / sampleRate;
//         float tone = 0.0f;
//         for (int h = 1; h <= harmonicsCount.get(); ++h) {
//             tone += (1.0f / h) * sinf(2.0f * M_PI * toneFreq.get() * h * t);
//         }
//         tone *= harmonicsMix.get() * env;

//         // Filter (simple low-pass filter implementation)
//         float cutoff = filterCutoff.get() / sampleRate;
//         filterState += cutoff * (tone - filterState); // 1-pole filter

//         // Mix components and write to buffer
//         buf[i] = (noise * noiseMix.get()) + (filterState * (1.0f - noiseMix.get()));
//     }
// }

// float cutoff = 2.0f * sinf(M_PI * (filterCutoff.get() / props.sampleRate));
// float q = 1.0f / filterResonance.get(); // Resonance factor
// float input = tone; // Input signal for the filter

// float bandPass = filterState1;
// float lowPass = filterState2 + cutoff * bandPass;
// float highPass = input - lowPass - q * bandPass;

// filterState1 = bandPass + cutoff * highPass;
// filterState2 = lowPass;

#endif
