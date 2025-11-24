/** Description:
This code defines the blueprint for a digital sound generator called `SynthSnare`, specialized in creating snare drum sounds. It is an integral part of a larger audio processing system, designed to integrate seamlessly with other plugins and controls.

**How the Code Works:**

The snare sound is synthesized by combining two main elements: a Tonal component (the resonant 'body' or pitch of the drum) and a Noise component (the rattling sound of the snares). When a note is triggered (like hitting the drum), the engine calculates the audio sample by sample. It uses an internal **envelope** to ensure the sound starts loud and rapidly fades out, mimicking the natural decay of a drum hit.

The noise element is refined using a basic digital filter to create "Pink Noise," which is softer and more natural than raw static. The system also applies a sharp, short burst of energy called a **transient** at the very beginning to simulate the initial ‘crack’ or attack of the stick hitting the drum head.

**Customizable Parameters:**

Users have detailed control over the resulting sound:

*   **DURATION:** Sets the overall length of the snare hit (in milliseconds).
*   **TONE\_FREQ & HARMONICS\_COUNT:** Control the pitch and complexity of the resonant, pitched component of the sound.
*   **NOISE\_MIX:** Adjusts the balance between the pitched tone and the rattling noise.
*   **PINK\_NOISE:** Determines how much the raw noise is softened.
*   **TRANSIENT DURATION/INTENSITY:** Allows fine-tuning the initial sharp impact of the drum hit.

This structure allows the synthesizer to create a wide variety of snare effects, from tight, electronic sounds to more realistic, complex acoustic simulations.

sha: 3c32ff64a9d5246c7e175742bdbcf9f3f457ef2c4a4f10ced04a56af37e666eb 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"

/*md
## SynthSnare

Synth engine to generate snare sounds.
*/

class SynthSnare : public Mapping {
protected:
    float whiteNoise()
    {
        return props.lookupTable->getNoise();
    }

    // For Pink noise, make encoder for pink noise amount...
    // Pink noise approximation (simple filter-based approach)
    float pinkNoiseState = 0.0f;
    float pinkNoise(float input)
    {
        // Low-pass filter approximation for pink noise
        pinkNoiseState = pinkNoiseAmount.pct() * pinkNoiseState + (1.0f - pinkNoiseAmount.pct()) * input;
        return pinkNoiseState;
    }

public:
    /*md **Values**: */

    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 2500.0, .step = 10.0, .unit = "ms" });

    /*md - `TONE_FREQ` set the frequency of the tone. */
    Val& toneFreq = val(200.0f, "TONE_FREQ", { "Tone Freq.", .min = 50.0, .max = 1000.0, .step = 1.0, .unit = "Hz" });
    /*md - `HARMONICS_COUNT` set the harmonics count in the tone. */
    Val& harmonicsCount = val(1, "HARMONICS_COUNT", { "Harmonics Count", .min = 1, .max = 10, .step = 1 });

    /*md - `PINK_NOISE` set the pink noise amount. */
    Val& pinkNoiseAmount = val(0.0f, "PINK_NOISE", { "Pink Noise", .unit = "%" });
    /*md - `NOISE_MIX` set the noise mix. */
    Val& noiseMix = val(70.0f, "NOISE_MIX", { "Noise Mix", .unit = "%" });

    /*md - `TRANSIENT_DURATION` set the transient duration. */
    Val& transientDuration = val(10.0f, "TRANSIENT_DURATION", { "Transient", .min = 1.0, .max = 50.0, .step = 1.0, .unit = "ms" });
    /*md - `TRANSIENT_INTENSITY` set the transient intensity. */
    Val& transientIntensity = val(0.0f, "TRANSIENT_INTENSITY", { "Transient", .unit = "%" });

    SynthSnare(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    int totalSamples = 0;
    int transientSamples = 0;
    float decayFactor = 0.0f;
    int i = 0;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float env = 1.0f - (i * decayFactor); // Envelope

            // float noise = whiteNoise() * env;
            float noise = pinkNoise(whiteNoise()) * env;

            // Transient component
            if (i < transientSamples && transientIntensity.pct() > 0.0f) {
                noise += transientIntensity.pct() * 5 * whiteNoise() * (1.0f - ((float)(i) / transientSamples));
            }

            // Tonal component with harmonics
            float t = (float)(i) / props.sampleRate;
            float tone = 0.0f;
            // WARN strangly enough, using sinf is faster than using lookup table
            // After looking at the CPU load on rpi, using sinf is faster than using lookup table...
            for (int h = 1; h <= harmonicsCount.get(); ++h) {
                tone += (1.0f / h) * sinf(2.0f * M_PI * toneFreq.get() * h * t);
            }
            // Using LUT
            // for (int h = 1; h <= harmonicsCount.get(); ++h) {
            //     float phase = fmodf(t * toneFreq.get() * h, 1.0f); // Phase wraps around [0, 1)
            //     tone += (1.0f / h) * props.lookupTable->sine[(int)(phase * props.lookupTable->size)];
            // }
            tone = tone / harmonicsCount.get() * env;

            buf[track] = (noise * noiseMix.pct()) + (tone * (1.0f - noiseMix.pct()));
            i++;
        }
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        // Generate a single snare drum sample buffer
        const float sampleRate = props.sampleRate; // Assumes standard sample rate
        const float durationSec = duration.get() / 1000.0f;
        totalSamples = static_cast<int>(sampleRate * durationSec);
        transientSamples = static_cast<int>(sampleRate * (transientDuration.get() / 1000.0f));
        decayFactor = 1.0f / totalSamples; // Linear decay
        i = 0;
    }
};
