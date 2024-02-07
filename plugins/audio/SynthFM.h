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
        Val attack;
        Val decay;
        Val sustain;
        Val release;
        Val freq;
        AdsrEnvelop envelop;
        // should it be ratio instead of frequency
        float stepIncrement = 0.0f;
        float index = 0.0f;
    } operators[ZIC_FM_OPS_COUNT] = {
        {
            { 50.0f, "ATTACK_0", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setAttack(p.value, 0); } },
            { 20.0f, "DECAY_0", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setDecay(p.value, 0); } },
            { 0.8f, "SUSTAIN_0", {}, [&](auto p) { setSustain(p.value, 0); } },
            { 50.0f, "RELEASE_0", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setRelease(p.value, 0); } },
            { 440.0f, "FREQUENCY_0", { .min = 20.0, .max = 20000.0, .step = 10 }, [&](auto p) { setFrequency(p.value, 0); } },
        },
        {
            { 50.0f, "ATTACK_1", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setAttack(p.value, 1); } },
            { 20.0f, "DECAY_1", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setDecay(p.value, 1); } },
            { 0.8f, "SUSTAIN_1", {}, [&](auto p) { setSustain(p.value, 1); } },
            { 50.0f, "RELEASE_1", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setRelease(p.value, 1); } },
            { 440.0f, "FREQUENCY_1", { .min = 20.0, .max = 20000.0, .step = 10 }, [&](auto p) { setFrequency(p.value, 1); } },
        },
        {
            { 50.0f, "ATTACK_2", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setAttack(p.value, 2); } },
            { 20.0f, "DECAY_2", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setDecay(p.value, 2); } },
            { 0.8f, "SUSTAIN_2", {}, [&](auto p) { setSustain(p.value, 2); } },
            { 50.0f, "RELEASE_2", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setRelease(p.value, 2); } },
            { 440.0f, "FREQUENCY_2", { .min = 20.0, .max = 20000.0, .step = 10 }, [&](auto p) { setFrequency(p.value, 2); } },
        },
        {
            { 50.0f, "ATTACK_3", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setAttack(p.value, 3); } },
            { 20.0f, "DECAY_3", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setDecay(p.value, 3); } },
            { 0.8f, "SUSTAIN_3", {}, [&](auto p) { setSustain(p.value, 3); } },
            { 50.0f, "RELEASE_3", { .min = 1.0, .max = 5000.0, .step = 10 }, [&](auto p) { setRelease(p.value, 3); } },
            { 440.0f, "FREQUENCY_3", { .min = 20.0, .max = 20000.0, .step = 10 }, [&](auto p) { setFrequency(p.value, 3); } },
        }
    };

    void setFrequency(float value, unsigned int opIndex)
    {
        operators[opIndex].freq.setFloat(value);
        // operators[opIndex].stepIncrement = operators[opIndex].frequency.get() * 2.0f * M_PI / sampleRate;
        operators[opIndex].stepIncrement = ZIC_FM_LUT_SIZE * operators[opIndex].freq.get() / props.sampleRate;
    }

    void setAttack(unsigned int ms, unsigned int opIndex)
    {
        operators[opIndex].attack.setFloat(ms);
        operators[opIndex].envelop.setAttack(operators[opIndex].attack.get());
    }

    void setDecay(unsigned int ms, unsigned int opIndex)
    {
        operators[opIndex].decay.setFloat(ms);
        operators[opIndex].envelop.setDecay(operators[opIndex].decay.get());
    }

    void setSustain(float value, unsigned int opIndex)
    {
        operators[opIndex].sustain.setFloat(value);
        operators[opIndex].envelop.setSustain(operators[opIndex].sustain.pct());
    }

    void setRelease(unsigned int ms, unsigned int opIndex)
    {
        operators[opIndex].release.setFloat(ms);
        operators[opIndex].envelop.setRelease(operators[opIndex].release.get());
    }

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
        : Mapping(props, _name, {
                                    // clang-format off
             &operators[0].attack, &operators[0].decay, &operators[0].sustain, &operators[0].release, &operators[0].freq, 
             &operators[1].attack, &operators[1].decay, &operators[1].sustain, &operators[1].release, &operators[1].freq,
             &operators[2].attack, &operators[2].decay, &operators[2].sustain, &operators[2].release, &operators[2].freq,
             &operators[3].attack, &operators[3].decay, &operators[3].sustain, &operators[3].release, &operators[3].freq,
                                    // clang-format on
                                })
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
                if (mod == 0.0) {
                    op.index += op.stepIncrement;
                } else {
                    float inc = op.stepIncrement + ZIC_FM_LUT_SIZE * op.freq.get() / props.sampleRate; // TODO optimize with precomputing: ZIC_FM_LUT_SIZE * op.freq.get() / props.sampleRate
                    op.index += inc;
                    // printf("inc: %f, mod %f \n", inc, mod);
                    mod = 0.0f;
                }
                while (op.index >= ZIC_FM_LUT_SIZE) {
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
                            if (mod == 0.0f) {
                                mod = s;
                            } else {
                                mod *= s;
                            }
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
            operators[i].stepIncrement = ZIC_FM_LUT_SIZE * operators[i].freq.get() / props.sampleRate;
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
