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

> [!NOTE] _deprecated_ in favor of **SynthKick23** that can produce bass with much more possibilities...
> maybe transforming this synth in a virtual 303 would bring value to it but for the moment better to focus on **SynthKick23**
*/
class SynthBass : public Mapping {
protected:
    uint8_t baseNote = 60;

    EffectFilterData filter;

    // could be use make sample representation for a note duration
    float bufferUi[ZIC_BASS_UI];

    // Envelop might need a bit of curve??
    EnvelopRelative envelop = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.3f, 0.4f }, { 0.0f, 1.0f } });

    unsigned int sampleCountDuration = 0;
    unsigned int sampleIndex = 0;
    float sampleValue = 0.0f;
    float stepIncrement = 0.001f;
    float velocity = 1.0f;

    int bufferUiState = -1;
    int updateUiState = 0;

    float stairRatio = 0.0f;

    float noteMult = 1.0f;

public:
    /*md **Values**: */
    /*md - `PITCH` set the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) { setPitch(p.value); });
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" }, [&](auto p) { setDuration(p.value); });
    /*md - `DECAY_LEVEL` set the decay level.*/
    Val& decayLevel = val(0.5f, "DECAY_LEVEL", { "Decay Level", .unit = "%" }, [&](auto p) { setDecayLevel(p.value); });
    /*md - `DECAY_TIME` set the decay time percentage base on the total duration.*/
    Val& decayTime = val(100.0f, "DECAY_TIME", { "Decay Time", .unit = "%" }, [&](auto p) { setDecayTime(p.value); });
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) { setCutoff(p.value); });
    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) { setResonance(p.value); });
    /*md - `STEP_FREQ` set how much the saw waveform ramp up increases on each sample.*/
    Val& stepFreq = val(10.0, "STEP_FREQ", { "Step Frequency", .max = 100 }, [&](auto p) { setStepFreq(p.value); });
    /*md - `STAIRCASE` set how much the saw waveform morph to staircase.*/
    Val& staircase = val(9.0, "STAIRCASE", { "Stairs", .min = -10, .max = 9 }, [&](auto p) { setStaircase(p.value); });
    /*md - `NOISE` set the noise level.*/
    Val& noise = val(0.0, "NOISE", { "Noise", .unit = "%" }, [&](auto p) { setNoise(p.value); });
    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" }, [&](auto p) { setClipping(p.value); });

    SynthBass(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name) // clang-format on
    {
        initValues();
    }

    float sample(EffectFilterData& _filter, float env, float& _sampleValue, float _velocity, float _stepIncrement)
    {
        _sampleValue += _stepIncrement;
        if (_sampleValue >= 1.0) {
            _sampleValue = -1.0;
        }
        _filter.setCutoff(0.85 * cutoff.pct() * env + 0.1);
        float val = _sampleValue;
        if (stairRatio) {
            val = stairRatio * floor(_sampleValue / stairRatio);
            if (staircase.get() < 1.0f) {
                val = val + floor(_sampleValue / stairRatio);
            }
        }
        if (noise.get() > 0.0f) {
            val += 0.01 * props.lookupTable->getNoise() * noise.get();
        }
        _filter.setSampleData(val * _velocity * env);
        float out = _filter.buf0 + _filter.buf0 * clipping.pct() * 8;
        return range(out, -1.0f, 1.0f);
    }

    void sample(float* buf)
    {
        if (sampleIndex < sampleCountDuration) {
            sampleIndex++;
            float time = (float)sampleIndex / (float)sampleCountDuration;
            float env = envelop.next(time);
            buf[track] = sample(filter, env, sampleValue, velocity, stepIncrement * noteMult);
        }
    }

    void setClipping(float value)
    {
        clipping.setFloat(value);
        updateUiState++;
    }

    void setNoise(float value)
    {
        noise.setFloat(value);
        updateUiState++;
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
        updateUiState++;
    }

    void setStepFreq(float value)
    {
        stepFreq.setFloat(value);
        stepIncrement = stepFreq.pct() * 0.002 + 0.0006f;
        updateUiState++;
    }

    void setDecayLevel(float value)
    {
        decayLevel.setFloat(value);
        envelop.data[2].modulation = decayLevel.pct();
        updateUiState++;
    }

    void setDecayTime(float value)
    {
        decayTime.setFloat(value);
        envelop.data[2].time = decayTime.pct();
        updateUiState++;
    }

    void setPitch(float value)
    {
        pitch.setFloat(value);
        // pitchMult = pitch.pct() + 0.5f; // FIXME
        updateUiState++;
    }

    void setDuration(float value)
    {
        duration.setFloat(value);
        bool isOff = sampleCountDuration == sampleIndex;
        sampleCountDuration = duration.get() * (props.sampleRate * 0.001f);
        if (isOff) {
            sampleIndex = sampleCountDuration;
        }
        // printf(">>>>>>>>>>>>>>.... sampleCountDuration: %d (%d)\n", sampleCountDuration, duration.getAsInt());
        updateUiState++;
    }

    void setCutoff(float value)
    {
        cutoff.setFloat(value);
        filter.setCutoff(0.85 * cutoff.pct() + 0.1);
        updateUiState++;
    }

    void setResonance(float value)
    {
        resonance.setFloat(value);
        filter.setResonance(resonance.pct());
        updateUiState++;
    };

    void noteOn(uint8_t note, float _velocity) override
    {
        velocity = _velocity;
        // printf("bass noteOn: %d %f\n", note, velocity);

        sampleIndex = 0;
        sampleValue = 0.0f;
        envelop.reset();

        noteMult = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            return &updateUiState;

        case 1: {
            if (bufferUiState != updateUiState) {
                bufferUiState = updateUiState;
                EffectFilterData _filter;
                _filter.setCutoff(filter.cutoff);
                _filter.setResonance(filter.resonance);
                float _sampleValue = 0.0f;
                unsigned int envIndex = 0;
                // printf("render bass waveform: %d\n", updateUiState);
                for (int i = 0; i < sampleCountDuration; i++) {
                    float time = (float)i / (float)sampleCountDuration;
                    float env = envelop.next(time, envIndex);

                    int bufIndex = i * (float)ZIC_BASS_UI / (float)sampleCountDuration;
                    bufferUi[bufIndex] = sample(_filter, env, _sampleValue, 1.0f, stepIncrement);
                }
            }
            return (void*)&bufferUi;
        }
        case 2:
            return &envelop.data;
        }
        return NULL;
    }
};

#endif
