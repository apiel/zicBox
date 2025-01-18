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

public:
    /*md **Values**: */

    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 2500.0, .step = 10.0, .unit = "ms" });

    Val& toneFreq = val(200.0f, "TONE_FREQ", { "Tone Freq.", .min = 50.0, .max = 1000.0, .step = 1.0, .unit = "Hz" });

    Val& noiseMix = val(0.7f, "NOISE_MIX", { "Noise Mix" });

    SynthSnare(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    int totalSamples = 0;
    int i = 0;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float decayFactor = 1.0f / totalSamples; // Linear decay

            float env = 1.0f - (i * decayFactor); // Envelope

            float noise = whiteNoise() * env;

            float t = static_cast<float>(i) / props.sampleRate;
            float tone = sinf(2.0f * M_PI * toneFreq.get() * t) * env;

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
        i = 0;
    }
};

// class SynthSnare : public Mapping {
// protected:
//     uint8_t baseNote = 60;

// public:
//     /*md **Values**: */

//     /*md - `DURATION` set the duration of the envelop.*/
//     Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" });

//     SynthSnare(AudioPlugin::Props& props, char* _name)
//         : Mapping(props, _name)
//     {
//         initValues();
//     }

//     void sample(float* buf)
//     {
//         // some logic to generate output
//         // buf[track] = output;
//     }

//     void noteOn(uint8_t note, float _velocity) override
//     {
//     }
// };

#endif
