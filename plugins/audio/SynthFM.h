#ifndef _SYNTH_FM_H_
#define _SYNTH_FM_H_

#include "../../helpers/range.h"
#include "audioPlugin.h"
#include "mapping.h"
#include "utils/AdsrEnvelop.h"

#define ZIC_FM_UI 1000
#define ZIC_FM_LUT_SIZE 2048
#define ZIC_FM_OPS_COUNT 4

float sineLut[ZIC_FM_LUT_SIZE];

/*md
## SynthFM

Fm synth engine using 4 operators.

Work in progress
*/
class SynthFM : public Mapping {
protected:
    struct FMoperator {
        AdsrEnvelop envelop;
        // should it be ratio instead of frequency
        float frequency = 440.0f; // Might not even need to keep here
        float stepIncrement = 0.0f;
        float index = 0.0f;
    } operators[ZIC_FM_OPS_COUNT];

    uint8_t baseNote = 60;

    // could be use make sample representation for a note duration
    float bufferUi[ZIC_FM_UI];

public:
    uint8_t currentAlgorithm = 0;
    bool algorithm[1][ZIC_FM_OPS_COUNT - 1][ZIC_FM_OPS_COUNT] = {
        { { 0, 0, 1, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 } }
    };

    /*md **Values**: */
    /*md - `PITCH` set the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", .min = -24, .max = 24 }, [&](auto p) { setPitch(p.value); });

    SynthFM(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name) // clang-format on
    {
        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            operators[i].envelop.setSampleRate(props.sampleRate);
        }

        initValues();

        // Init sine LUT
        for (int i = 0; i < ZIC_FM_LUT_SIZE; i++) {
            sineLut[i] = sin((float)i / (float)ZIC_FM_LUT_SIZE * 2.0f * M_PI);
        }
    }

    void sample(float* buf)
    {
        uint8_t outDivider = 0;
        float out = 0.0f;
        float mod = 0.0f;

        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            FMoperator& op = operators[i];
            float env = op.envelop.next();
            if (env > 0.0f) {
                op.index += op.stepIncrement;
                while (op.index >= ZIC_FM_LUT_SIZE)
                {
                    op.index -= ZIC_FM_LUT_SIZE;
                }
                float s = sineLut[(int)op.index] * env;
                if (i == ZIC_FM_OPS_COUNT - 1) {
                    outDivider++;
                    out += s;
                    out = out / outDivider;
                } else {
                    bool isMod = false;
                    for (int j = 0; j < ZIC_FM_OPS_COUNT; j++) {
                        if (algorithm[currentAlgorithm][i][j]) {
                            isMod = true;
                            mod *= s;
                        }
                    }
                    if (!isMod) {
                        outDivider++;
                        out += s;
                    }
                }
            }
        }

        buf[track] = out;
    }

    void setPitch(float value)
    {
        pitch.setFloat(value);
        // updateUiState++;
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            operators[i].envelop.reset();
            operators[i].index = 0.0f;
            operators[i].stepIncrement = ZIC_FM_LUT_SIZE * operators[i].frequency / props.sampleRate;
        }
    }

    void noteOff(uint8_t note, float _velocity) override
    {
        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            operators[i].envelop.release();
        }
    }

    void* data(int id, void* userdata = NULL)
    {
        // switch (id) {
        // case 0:
        //     return &updateUiState;

        // case 1: {
        //     if (bufferUiState != updateUiState) {
        //         bufferUiState = updateUiState;
        //         EffectFilterData _filter;
        //         _filter.setCutoff(filter.cutoff);
        //         _filter.setResonance(filter.resonance);
        //         float _sampleValue = 0.0f;
        //         unsigned int envIndex = 0;
        //         // printf("render bass waveform: %d\n", updateUiState);
        //         for (int i = 0; i < sampleCountDuration; i++) {
        //             float time = (float)i / (float)sampleCountDuration;
        //             float env = envelop.next(time, &envIndex);

        //             int bufIndex = i * (float)ZIC_BASS_UI / (float)sampleCountDuration;
        //             bufferUi[bufIndex] = sample(_filter, env, _sampleValue, 1.0f, stepIncrement);
        //         }
        //     }
        //     return (void*)&bufferUi;
        // }
        // case 2:
        //     return &envelop.data;
        // }
        return NULL;
    }
};

#endif
