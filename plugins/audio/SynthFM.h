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
        float frequency = 440.0f; // Might not even need to keep here
        float stepIncrement = 0.0f;
        float index = 0.0f;
    } operators[ZIC_FM_OPS_COUNT];

    uint8_t baseNote = 60;

    // could be use make sample representation for a note duration
    float bufferUi[ZIC_FM_UI];

    // Envelop envelop = Envelop({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.3f, 0.4f }, { 0.0f, 1.0f } });

public:
    /*md **Values**: */
    /*md - `PITCH` set the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", .min = -24, .max = 24 }, [&](auto p) { setPitch(p.value); });

    SynthFM(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name) // clang-format on
        , operators({ { props.sampleRate }, { props.sampleRate }, { props.sampleRate }, { props.sampleRate } })
    {
        initValues();

        // Init sine LUT
        for (int i = 0; i < ZIC_FM_LUT_SIZE; i++) {
            sineLut[i] = sin((float)i / (float)ZIC_FM_LUT_SIZE * 2.0f * M_PI);
        }
    }

    void sample(float* buf)
    {
    }

    void setPitch(float value)
    {
        pitch.setFloat(value);
        // updateUiState++;
    }

    void noteOn(uint8_t note, float _velocity) override
    {
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
