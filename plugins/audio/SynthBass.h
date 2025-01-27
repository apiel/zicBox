#ifndef _SYNTH_BASS_H_
#define _SYNTH_BASS_H_

#include "../../helpers/range.h"
#include "audioPlugin.h"
#include "filter.h"
#include "mapping.h"
#include "utils/EnvelopRelative.h"

#define ZIC_BASS_UI 1000

/*md
## SynthBass

Synth engine to generate bass sounds.

*/
class SynthBass : public Mapping {
protected:
    uint8_t baseNote = 60;

    EffectFilterData filter;
    EffectFilterData filter2;

    // Envelop might need a bit of curve??
    EnvelopRelative envelop = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.3f, 0.4f }, { 0.0f, 1.0f } });

    unsigned int sampleCountDuration = 0;
    unsigned int sampleIndex = 0;
    float sampleValue = 0.0f;
    float stepIncrement = 0.001f;
    float velocity = 1.0f;

    float stairRatio = 0.0f;

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

public:
    /*md **Values**: */
    /*md - `PITCH` set the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 });
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" }, [&](auto p) { setDuration(p.value); });
    /*md - `DECAY_LEVEL` set the decay level.*/
    Val& decayLevel = val(0.5f, "DECAY_LEVEL", { "Decay Level", .unit = "%" }, [&](auto p) { setDecayLevel(p.value); });
    /*md - `DECAY_TIME` set the decay time percentage base on the total duration.*/
    Val& decayTime = val(100.0f, "DECAY_TIME", { "Decay Time", .unit = "%" }, [&](auto p) { setDecayTime(p.value); });
    /*md - `CUTOFF` to set cutoff frequency of low pass filter. */
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) { setCutoff(p.value); });
    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) { setResonance(p.value); });
    /*md - `STAIRCASE` set how much the saw waveform morph to staircase.*/
    Val& staircase = val(9.0, "STAIRCASE", { "Stairs", .min = -10, .max = 9 }, [&](auto p) { setStaircase(p.value); });
    /*md - `NOISE` set the noise level.*/
    Val& noise = val(0.0, "NOISE", { "Noise", .unit = "%" });
    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" });
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    /*md - FREQ sets the frequency of the bass. */
    Val& freq = val(30.0f, "FREQ", { "Frequency", .min = 10.0, .max = 200.0, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });

    SynthBass(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name) // clang-format on
    {
        initValues();
    }

    // TODO add waveform morphing
    // TODO add distortion
    // TODO improve filter perf by adding multiple pass... so cutoff and resonance is only calculated once..

    float getWaveform(float time, float& sampleVal, float increment)
    {
        sampleVal += increment;
        if (sampleVal >= 1.0) {
            sampleVal = -1.0;
        }
        float out = sampleVal;
        if (stairRatio) {
            out = stairRatio * floor(sampleVal / stairRatio);
            if (staircase.get() < 1.0f) {
                out = out + floor(sampleVal / stairRatio);
            }
        }
        return out;
    }

    void sample(float* buf)
    {
        if (sampleIndex < sampleCountDuration) {
            sampleIndex++;
            float time = (float)sampleIndex / (float)sampleCountDuration;
            float env = envelop.next(time);
            float out = getWaveform(time, sampleValue, stepIncrement);
            if (noise.get() > 0.0f) {
                out += 0.01 * props.lookupTable->getNoise() * noise.get();
            }
            out = out * velocity * env;
            filter.setCutoff(0.85 * cutoff.pct() * env + 0.1);
            filter.setSampleData(out);
            out = filter.lp;
            filter2.setCutoff(0.85 * cutoff.pct() * env + 0.1);
            filter2.setSampleData(out);
            out = filter2.lp;
            out = range(out + out * clipping.pct() * 8, -1.0f, 1.0f);
            out = applyReverb(out);
            buf[track] = out;
        } else {
            buf[track] = applyReverb(buf[track]);
        }
    }

    void setStaircase(float value)
    {
        staircase.setFloat(value);
        if (staircase.get() < 9.0f) {
            if (staircase.get() > 0.0f) {
                stairRatio = -1.0f / (float)staircase.get();
            } else {
                stairRatio = 1.0f / (float)abs(staircase.get() - 1);
            }
        } else {
            stairRatio = 0;
        }
    }

    void setDecayLevel(float value)
    {
        decayLevel.setFloat(value);
        envelop.data[2].modulation = decayLevel.pct();
    }

    void setDecayTime(float value)
    {
        decayTime.setFloat(value);
        envelop.data[2].time = decayTime.pct();
    }

    void setDuration(float value)
    {
        duration.setFloat(value);
        bool isOff = sampleCountDuration == sampleIndex;
        sampleCountDuration = duration.get() * (props.sampleRate * 0.001f);
        if (isOff) {
            sampleIndex = sampleCountDuration;
        }
    }

    void setCutoff(float value)
    {
        cutoff.setFloat(value);

        float cutoffValue = 0.85 * cutoff.pct() + 0.1;
        filter.setCutoff(cutoffValue);
        filter2.setCutoff(cutoffValue);
    }

    void setResonance(float value)
    {
        resonance.setFloat(value);
        // float res = 0.95 * (1.0 - std::pow(1.0 - resonance.pct(), 3));
        float res = 0.95 * (1.0 - std::pow(1.0 - resonance.pct(), 2));
        filter.setResonance(res);
        filter2.setResonance(res);
    };

    void noteOn(uint8_t note, float _velocity) override
    {
        velocity = _velocity;
        // printf("bass noteOn: %d %f\n", note, velocity);

        sampleIndex = 0;
        sampleValue = 0.0f;
        envelop.reset();

        stepIncrement = (freq.get() / props.sampleRate) * 2.0 * pow(2.0, ((note - baseNote + pitch.get()) / 12.0));
    }
};

#endif
