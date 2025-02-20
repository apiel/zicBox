#pragma once

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
        // float modulator = sineWave(carrierFreq, sampleIndex) * modIndex;
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

    int totalSamples = 0;
    int i = 0;

public:
    /*md **Values**: */
    /*md - CARRIER_FREQ sets the frequency of the carrier wave. */
    Val& carrierFreq = val(200.0f, "CARRIER_FREQ", { "Carrier Freq.", .min = 10.0, .max = 2000.0, .step = 10.0, .unit = "Hz" });
    /*md - MOD_FREQ sets the frequency of the modulator wave. */
    Val& modFreq = val(50.0f, "MOD_FREQ", { "Mod. Freq.", .min = 10.0, .max = 500.0, .step = 1.0, .unit = "Hz" });
    /*md - MOD_INDEX controls the intensity of frequency modulation. */
    Val& modIndex = val(10.0f, "MOD_INDEX", { "Mod. Index", .min = 0.0, .max = 100.0, .step = 0.1, .floatingPoint = 1 });
    /*md - ATTACK_TIME sets the attack time of the envelope. */
    Val& attackTime = val(0.01f, "ATTACK_TIME", { "Attack Time", .min = 0.001, .max = 1.0, .step = 0.001, .unit = "s" });
    /*md - DECAY_TIME sets the decay time of the envelope. */
    Val& decayTime = val(0.2f, "DECAY_TIME", { "Decay Time", .min = 0.01, .max = 2.0, .step = 0.01, .unit = "s" });
    /*md - NOISE_LEVEL adds white noise to the output. */
    Val& noiseLevel = val(0.0f, "NOISE_LEVEL", { "Noise", .unit = "%" });
    /*md - `DISTORTION` to set distortion. */
    Val& distortion = val(0.0, "DISTORTION", { "Distortion", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });

    SynthFmDrum(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    float noteFrquency = 220.0f;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            // Generate FM modulated signal
            float fmSignal = fmModulation(noteFrquency, modFreq.get(), modIndex.get(), i);

            // Generate envelope
            float env = envelope(i, attackTime.get() * props.sampleRate, decayTime.get() * props.sampleRate);

            float noise = props.lookupTable->getNoise() * noiseLevel.pct();

            // Combine FM signal and noise, then scale by envelope and velocity
            float output = (fmSignal + noise) * env * currentVelocity;

            output = applyDistortion(output);

            output = applyReverb(output);

            buf[track] = output;
            i++;
        } else {
            buf[track] = applyReverb(buf[track]);
        }
    }

    uint8_t baseNote = 60;
    uint8_t currentNote = 0;
    float currentVelocity = 1.0f;

    void noteOn(uint8_t note, float _velocity) override
    {
        const float sampleRate = props.sampleRate;
        totalSamples = static_cast<int>((attackTime.get() + decayTime.get()) * sampleRate);
        currentNote = note;
        currentVelocity = _velocity;
        // Adjust carrier frequency based on MIDI note
        noteFrquency = carrierFreq.get() * powf(2.0f, (currentNote - baseNote) / 12.0f);
        i = 0;
    }
};
