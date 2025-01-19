#ifndef _SYNTH_303_H_
#define _SYNTH_303_H_

#include "audioPlugin.h"
#include "mapping.h"

/*md
## Synth303

Synth engine to generate 303 bass.
*/

class Synth303 : public Mapping {
protected:
    // State for the low-pass filter
    float filterLow = 0.0f;
    float filterBand = 0.0f;

    // Low-pass resonant filter
    float resonantFilter(float input, float cutoff, float resonance)
    {
        float f = 2.0f * sinf(M_PI * cutoff / props.sampleRate);
        float q = 1.0f - resonance;

        filterLow += f * filterBand;
        float high = input - filterLow - q * filterBand;
        filterBand += f * high;

        return filterLow;
    }

    // Generate a sawtooth waveform
    float sawWave(float phase)
    {
        return 2.0f * (phase - floor(phase + 0.5f));
    }

    // Generate a square waveform
    float squareWave(float phase)
    {
        return phase < 0.5f ? 1.0f : -1.0f;
    }

public:
    /*md **Values**: */

    Val& waveform = val(0.0f, "WAVEFORM", { "Waveform", .min = 0.0, .max = 1.0, .step = 1.0 }); // 0 = Saw, 1 = Square
    Val& cutoff = val(1000.0f, "CUTOFF", { "Cutoff Frequency", .min = 100.0, .max = 5000.0, .step = 10.0, .unit = "Hz" });
    Val& resonance = val(0.5f, "RESONANCE", { "Resonance", .unit = "%" });
    Val& envMod = val(0.5f, "ENV_MOD", { "Envelope Modulation", .unit = "%" });
    Val& decay = val(300.0f, "DECAY", { "Decay", .min = 50.0, .max = 1000.0, .step = 10.0, .unit = "ms" });
    Val& glide = val(0.0f, "GLIDE", { "Glide", .min = 0.0, .max = 1.0, .step = 0.01 });

    Synth303(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    float phase = 0.0f;
    float currentFreq = 440.0f; // Default frequency
    float amplitude = 0.0f;
    float decayFactor = 0.0f;
    float targetFreq = 440.0f;
    int i = 0;
    void sample(float* buf) override
    {
        if (amplitude != 0.0f) {
            // Apply glide
            currentFreq += (targetFreq - currentFreq) * glide.get();

            // Update phase
            phase += currentFreq / props.sampleRate;
            if (phase >= 1.0f) {
                phase -= 1.0f;
            }

            // Generate waveform
            float rawWave = waveform.get() < 0.5f ? sawWave(phase) : squareWave(phase);

            // Apply filter envelope
            float env = amplitude * envMod.pct();
            float filteredWave = resonantFilter(rawWave + env, cutoff.get(), resonance.pct());

            // Write to buffer
            buf[track] = filteredWave * amplitude;

            // Apply amplitude decay
            amplitude -= decayFactor;
            if (amplitude < 0.0f) {
                amplitude = 0.0f;
            }
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        // Convert MIDI note to frequency
        float freq = 220.0f * powf(2.0f, (note - 69) / 12.0f);

        // Adjust amplitude based on velocity
        amplitude = _velocity;

        // Set target frequency (for glide)
        targetFreq = freq;

        phase = 0.0f;
        currentFreq = 220.0f; // Default frequency

        decayFactor = 1.0f / (props.sampleRate * (decay.get() / 1000.0f));
        amplitude = 1.0f;

        i = 0;
    }
};

#endif
