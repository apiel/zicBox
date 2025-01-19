#ifndef _SYNTH_FM_DRUM_H_
#define _SYNTH_FM_DRUM_H_

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/utils.h"

/*md
## SynthFmDrum

Synth engine to make FM drum.
*/

class SynthFmDrum : public Mapping {
protected:
    // Sine wave generator
    float sineWave(float frequency, int sampleIndex)
    {
        return sinf(2.0f * M_PI * frequency * sampleIndex / props.sampleRate);
    }

    // FM modulation
    float fmModulation(float carrierFreq, float modFreq, float modIndex, int sampleIndex)
    {
        float modulator = sineWave(modFreq, sampleIndex) * modIndex;
        return sineWave(carrierFreq + modulator, sampleIndex);
    }

    // Envelope generator
    float envelope(int sampleIndex, float attackTime, float decayTime)
    {
        if (sampleIndex < attackTime) {
            return static_cast<float>(sampleIndex) / attackTime; // Attack phase
        } else if (sampleIndex < attackTime + decayTime) {
            float decayIndex = sampleIndex - attackTime;
            return 1.0f - (decayIndex / decayTime); // Decay phase
        } else {
            return 0.0f; // Silence
        }
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
    float applyDistortion(float input)
    {
        float amount = distortion.pct() * 2 - 1.0f;
        if (amount > 0.0f) {
            return tanhLookup(input * (1.0f + amount * 5.0f));

            // return std::pow(input, 1.0f - amount * 0.8f);
        }
        if (amount < 0.0f) {
            float sineValue = sineLookupInterpolated(input);
            return input + (-amount) * sineValue;
        }
        return input;
    }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;
    void applyReverb(float& signal)
    {
        float reverbAmount = reverb.pct();
        int reverbSamples = static_cast<int>((reverbAmount * 0.5f) * props.sampleRate); // Reverb duration scaled
        float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
        float mix = reverbAmount * 0.5f; // Mix scaled proportionally

        if (reverbSamples > REVERB_BUFFER_SIZE) {
            reverbSamples = REVERB_BUFFER_SIZE; // Cap the reverb duration to buffer size
        }

        float reverbSignal = reverbBuffer[reverbIndex];
        reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;
        reverbIndex = (reverbIndex + 1) % reverbSamples;

        signal = signal * (1.0f - mix) + reverbSignal * mix;
    }

    int totalSamples = 0;
    int i = 0;

public:
    /*md **Values**: */
    /*md - CARRIER_FREQ sets the frequency of the carrier wave. */
    Val& carrierFreq = val(200.0f, "CARRIER_FREQ", { "Carrier Frequency", .min = 10.0, .max = 2000.0, .step = 10.0, .unit = "Hz" });
    /*md - MOD_FREQ sets the frequency of the modulator wave. */
    Val& modFreq = val(50.0f, "MOD_FREQ", { "Modulator Frequency", .min = 10.0, .max = 500.0, .step = 1.0, .unit = "Hz" });
    /*md - MOD_INDEX controls the intensity of frequency modulation. */
    Val& modIndex = val(10.0f, "MOD_INDEX", { "Modulation Index", .min = 0.0, .max = 100.0, .step = 0.1 });
    /*md - ATTACK_TIME sets the attack time of the envelope. */
    Val& attackTime = val(0.01f, "ATTACK_TIME", { "Attack Time", .min = 0.001, .max = 1.0, .step = 0.001, .unit = "s" });
    /*md - DECAY_TIME sets the decay time of the envelope. */
    Val& decayTime = val(0.2f, "DECAY_TIME", { "Decay Time", .min = 0.01, .max = 2.0, .step = 0.01, .unit = "s" });
    /*md - NOISE_LEVEL adds white noise to the output. */
    Val& noiseLevel = val(0.2f, "NOISE_LEVEL", { "Noise Level", .min = 0.0, .max = 1.0, .step = 0.01 });
    /*md - `DISTORTION` to set distortion. */
    Val& distortion = val(0.0, "DISTORTION", { "Distortion", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });

    // /*md - DELAY_TIME sets the delay time in seconds. */
    // Val& delayTime = val(0.1f, "DELAY_TIME", { "Delay Time", .min = 0.0, .max = 1.0, .step = 0.01, .unit = "s" });
    // /*md - DELAY_FEEDBACK controls the amount of signal fed back into the delay. */
    // Val& delayFeedback = val(0.3f, "DELAY_FEEDBACK", { "Delay Feedback", .min = 0.0, .max = 1.0, .step = 0.01 });
    // /*md - DELAY_MIX sets the mix level between dry and wet signals for the delay. */
    // Val& delayMix = val(0.5f, "DELAY_MIX", { "Delay Mix", .min = 0.0, .max = 1.0, .step = 0.01 });
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });

    SynthFmDrum(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void sample(float* buf) override
    {
        if (i < totalSamples) {
            // Adjust carrier frequency based on MIDI note
            float baseFrequency = carrierFreq.get() * powf(2.0f, (currentNote - 60) / 12.0f);

            // Generate FM modulated signal
            float fmSignal = fmModulation(baseFrequency, baseFrequency, modIndex.get(), i);

            // Generate envelope
            float env = envelope(i, attackTime.get() * props.sampleRate, decayTime.get() * props.sampleRate);

            // Add white noise
            // TODO brown noise when negative
            float noise = props.lookupTable->getNoise() * noiseLevel.get();

            // Combine FM signal and noise, then scale by envelope and velocity
            float output = (fmSignal + noise) * env * currentVelocity;

            output = applyDistortion(output);

            applyReverb(output);

            buf[track] = output;
            i++;
        } else {
            applyReverb(buf[track]);
        }
    }

    uint8_t currentNote = 60;
    float currentVelocity = 1.0f;

    void noteOn(uint8_t note, float _velocity) override
    {
        const float sampleRate = props.sampleRate;
        totalSamples = static_cast<int>((attackTime.get() + decayTime.get()) * sampleRate);
        currentNote = note;
        currentVelocity = _velocity;
        i = 0;
    }
};

#endif

// void sample(float* buf) override {
//     if (i < totalSamples) {
//         // Generate FM modulated signal
//         float fmSignal = fmModulation(carrierFreq.get(), modFreq.get(), modIndex.get(), i);

//         // Generate envelope
//         float env = envelope(i, attackTime.get() * props.sampleRate, decayTime.get() * props.sampleRate);

//         // Add white noise
//         float noise = props.lookupTable->getNoise() * noiseLevel.get();

//         // Combine FM signal and noise, then scale by envelope and output level
//         float output = (fmSignal + noise) * env * outputLevel.get();

//         buf[track] = output;
//         i++;
//     }
// }

// void noteOn(uint8_t note, float _velocity) override {
//     const float sampleRate = props.sampleRate;
//     totalSamples = static_cast<int>((attackTime.get() + decayTime.get()) * sampleRate);
//     i = 0;
// }
