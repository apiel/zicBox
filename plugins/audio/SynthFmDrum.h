#ifndef _SYNTH_FM_DRUM_H_
#define _SYNTH_FM_DRUM_H_

#include "audioPlugin.h"
#include "mapping.h"

/*md
## SynthFmDrum

Synth engine to make FM drum.
*/

class SynthFmDrum : public Mapping {
protected:
    // Sine wave generator
    float sineWave(float frequency, int sampleIndex) {
        return sinf(2.0f * M_PI * frequency * sampleIndex / props.sampleRate);
    }

    // FM modulation
    float fmModulation(float carrierFreq, float modFreq, float modIndex, int sampleIndex) {
        float modulator = sineWave(modFreq, sampleIndex) * modIndex;
        return sineWave(carrierFreq + modulator, sampleIndex);
    }

    // Envelope generator
    float envelope(int sampleIndex, float attackTime, float decayTime) {
        if (sampleIndex < attackTime) {
            return static_cast<float>(sampleIndex) / attackTime; // Attack phase
        } else if (sampleIndex < attackTime + decayTime) {
            float decayIndex = sampleIndex - attackTime;
            return 1.0f - (decayIndex / decayTime); // Decay phase
        } else {
            return 0.0f; // Silence
        }
    }

    int totalSamples = 0;
    int i = 0;

public:
    /*md **Values**: */
    /*md - CARRIER_FREQ sets the frequency of the carrier wave. */
    Val& carrierFreq = val(200.0f, "CARRIER_FREQ", { "Carrier", .min = 50.0, .max = 2000.0, .step = 10.0, .unit = "Hz" });
    /*md - MOD_FREQ sets the frequency of the modulator wave. */
    Val& modFreq = val(50.0f, "MOD_FREQ", { "Mod Freq", .min = 10.0, .max = 500.0, .step = 1.0, .unit = "Hz" });
    /*md - MOD_INDEX controls the intensity of frequency modulation. */
    Val& modIndex = val(10.0f, "MOD_INDEX", { "Modulation Index", .min = 0.0, .max = 100.0, .step = 0.1 });
    /*md - ATTACK_TIME sets the attack time of the envelope. */
    Val& attackTime = val(0.01f, "ATTACK_TIME", { "Attack", .min = 0.001, .max = 1.0, .step = 0.001, .unit = "s" });
    /*md - DECAY_TIME sets the decay time of the envelope. */
    Val& decayTime = val(0.2f, "DECAY_TIME", { "Decay", .min = 0.01, .max = 2.0, .step = 0.01, .unit = "s" });
    /*md - NOISE_LEVEL adds white noise to the output. */
    Val& noiseLevel = val(0.2f, "NOISE_LEVEL", { "Noise", .min = 0.0, .max = 1.0, .step = 0.01 });
    /*md - OUTPUT_LEVEL adjusts the final output level. */
    Val& outputLevel = val(1.0f, "OUTPUT_LEVEL", { "Output Level", .min = 0.0, .max = 1.0, .step = 0.01 });

    SynthFmDrum(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void sample(float* buf) override {
        if (i < totalSamples) {
            // Generate FM modulated signal
            float fmSignal = fmModulation(carrierFreq.get(), modFreq.get(), modIndex.get(), i);

            // Generate envelope
            float env = envelope(i, attackTime.get() * props.sampleRate, decayTime.get() * props.sampleRate);

            // Add white noise
            float noise = props.lookupTable->getNoise() * noiseLevel.get();

            // Combine FM signal and noise, then scale by envelope and output level
            float output = (fmSignal + noise) * env * outputLevel.get();

            buf[track] = output;
            i++;
        }
    }

    void noteOn(uint8_t note, float _velocity) override {
        const float sampleRate = props.sampleRate;
        totalSamples = static_cast<int>((attackTime.get() + decayTime.get()) * sampleRate);
        i = 0;
    }
};

#endif
