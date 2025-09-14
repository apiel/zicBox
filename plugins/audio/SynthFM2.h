#pragma once

#include "audioPlugin.h"
#include "helpers/clamp.h"
#include "mapping.h"
#include "utils/AdsrEnvelop.h"

#define ZIC_FM_OPS_COUNT 4

/*md
## SynthFM2

Fm synth engine using 4 operators.

Work in progress
*/
class SynthFM2 : public Mapping {
protected:
    struct FMoperator {
        Val attack;
        Val decay;
        Val sustain;
        Val release;
        Val ratio;
        Val feedback;
        AdsrEnvelop envelop;
        float pitchedFreq = 0.0f;
        float stepIncrement = 0.0f;
        float index = 0.0f;
        float mod = 0.0f;
        float feedbackMod = 0.0f;
    } operators[ZIC_FM_OPS_COUNT] = {
        {
            { 50.0f, "ATTACK_0", { "Attack 1", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setAttack(p.value, 0); } },
            { 20.0f, "DECAY_0", { "Decay 1", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setDecay(p.value, 0); } },
            { 0.8f, "SUSTAIN_0", { "Sustain 1", .unit = "%" }, [&](auto p) { setSustain(p.value, 0); } },
            { 50.0f, "RELEASE_0", { "Release 1", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setRelease(p.value, 0); } },
            { 1.0f, "RATIO_0", { "Ratio 1", .min = 0.01, .max = 10.0, .step = 0.01, .unit = "x" }, [&](auto p) { setRatio(p.value, 0); } },
            { 0.0f, "FEEDBACK_0", { "Feedback 1", .unit = "%" } },
        },
        {
            { 50.0f, "ATTACK_1", { "Attack 2", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setAttack(p.value, 1); } },
            { 20.0f, "DECAY_1", { "Decay 2", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setDecay(p.value, 1); } },
            { 0.8f, "SUSTAIN_1", { "Sustain 2", .unit = "%" }, [&](auto p) { setSustain(p.value, 1); } },
            { 50.0f, "RELEASE_1", { "Release 2", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setRelease(p.value, 1); } },
            { 1.0f, "RATIO_1", { "Ratio 2", .min = 0.01, .max = 10.0, .step = 0.01, .unit = "x" }, [&](auto p) { setRatio(p.value, 1); } },
            { 0.0f, "FEEDBACK_1", { "Feedback 2", .unit = "%" } },
        },
        {
            { 50.0f, "ATTACK_2", { "Attack 3", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setAttack(p.value, 2); } },
            { 20.0f, "DECAY_2", { "Decay 3", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setDecay(p.value, 2); } },
            { 0.8f, "SUSTAIN_2", { "Sustain 3", .unit = "%" }, [&](auto p) { setSustain(p.value, 2); } },
            { 50.0f, "RELEASE_2", { "Release 3", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setRelease(p.value, 2); } },
            { 1.0f, "RATIO_2", { "Ratio 3", .min = 0.01, .max = 10.0, .step = 0.01, .unit = "x" }, [&](auto p) { setRatio(p.value, 2); } },
            { 0.0f, "FEEDBACK_2", { "Feedback 3", .unit = "%" } },
        },
        {
            { 50.0f, "ATTACK_3", { "Attack 4", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setAttack(p.value, 3); } },
            { 20.0f, "DECAY_3", { "Decay 4", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setDecay(p.value, 3); } },
            { 0.8f, "SUSTAIN_3", { "Sustain 4", .unit = "%" }, [&](auto p) { setSustain(p.value, 3); } },
            { 50.0f, "RELEASE_3", { "Release 4", .min = 1.0, .max = 5000.0, .step = 20, .unit = "ms" }, [&](auto p) { setRelease(p.value, 3); } },
            { 1.0f, "RATIO_3", { "Ratio 4", .min = 0.01, .max = 10.0, .step = 0.01, .unit = "x" }, [&](auto p) { setRatio(p.value, 3); } },
            { 0.0f, "FEEDBACK_3", { "Feedback 4", .unit = "%" } },
        }
    };

    void setRatio(float value, unsigned int opIndex)
    {
        operators[opIndex].ratio.setFloat(value);
        updateOperatorFrequency(operators[opIndex]);
    }

    void setAttack(float ms, unsigned int opIndex)
    {
        operators[opIndex].attack.setFloat(ms);
        operators[opIndex].envelop.setAttack(operators[opIndex].attack.get());
    }

    void setDecay(float ms, unsigned int opIndex)
    {
        operators[opIndex].decay.setFloat(ms);
        operators[opIndex].envelop.setDecay(operators[opIndex].decay.get());
    }

    void setSustain(float value, unsigned int opIndex)
    {
        operators[opIndex].sustain.setFloat(value);
        operators[opIndex].envelop.setSustain(operators[opIndex].sustain.pct());
    }

    void setRelease(float ms, unsigned int opIndex)
    {
        operators[opIndex].release.setFloat(ms);
        operators[opIndex].envelop.setRelease(operators[opIndex].release.get());
    }

    uint8_t baseNote = 60;
    float velocity = 1.0f;
    // notePitchRatio is the pitch of the current playing note;
    float notePitchRatio = 1.0f;

    void updateOperatorFrequency(FMoperator& op)
    {
        op.pitchedFreq = mainFreq.get() * op.ratio.get() * notePitchRatio;
        op.stepIncrement = props.lookupTable->size * op.pitchedFreq / props.sampleRate;
    }

    void triggerOperator(FMoperator& op)
    {
        op.envelop.reset();
        op.index = 0.0f;
        updateOperatorFrequency(op);
    }

    void releaseOperator(FMoperator& op)
    {
        op.envelop.release();
    }

public:
    bool algorithm[12][ZIC_FM_OPS_COUNT - 1][ZIC_FM_OPS_COUNT - 1] = {
        { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } },
        { { 0, 1, 0 }, { 0, 1, 0 }, { 0, 0, 1 } },
        { { 0, 0, 1 }, { 0, 1, 0 }, { 0, 0, 1 } },
        { { 1, 1, 0 }, { 0, 0, 1 }, { 0, 0, 1 } },
        { { 1, 0, 0 }, { 0, 1, 1 }, { 0, 0, 0 } },
        { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 0 } },
        { { 0, 0, 1 }, { 0, 0, 1 }, { 0, 0, 1 } },
        { { 1, 0, 0 }, { 0, 0, 0 }, { 0, 0, 1 } },
        { { 1, 1, 1 }, { 0, 0, 0 }, { 0, 0, 0 } },
        { { 1, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
        { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
        { { 0, 1, 0 }, { 0, 1, 0 }, { 0, 0, 0 } },
    };

    Val& algo = val(1, "ALGO", { "Algorithm", .min = 1, .max = sizeof(algorithm) / sizeof(algorithm[0]) });
    Val& mainFreq = val(440.0f, "FREQUENCY", { "Frequency", .min = 20.0, .max = 5000.0, .step = 10.0, .unit = "Hz" },
        [&](auto p) { 
            mainFreq.setFloat(p.value);
            for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
                updateOperatorFrequency(operators[i]);
            }
         });

    SynthFM2(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config, {
                                    // clang-format off
                    &operators[0].attack, &operators[0].decay, &operators[0].sustain, &operators[0].release, &operators[0].ratio, &operators[0].feedback,
                    &operators[1].attack, &operators[1].decay, &operators[1].sustain, &operators[1].release, &operators[1].ratio, &operators[1].feedback,
                    &operators[2].attack, &operators[2].decay, &operators[2].sustain, &operators[2].release, &operators[2].ratio, &operators[2].feedback,
                    &operators[3].attack, &operators[3].decay, &operators[3].sustain, &operators[3].release, &operators[3].ratio, &operators[3].feedback,
                                    // clang-format on
                                })
    {
        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            operators[i].envelop.setSampleRate(props.sampleRate);
        }

        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        velocity = _velocity;
        notePitchRatio = pow(2.0f, (note - baseNote) / 12.0f);
        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            triggerOperator(operators[i]);
        }
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            releaseOperator(operators[i]);
        }
    }

    void sample(float* buf) override
    {
        uint8_t outDivider = 0;
        float out = 0.0f;

        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            operators[i].mod = 0.0f;
        }

        for (int i = 0; i < ZIC_FM_OPS_COUNT; i++) {
            FMoperator& op = operators[i];
            float env = op.envelop.next();
            if (env > 0.0f) {
                if (op.mod == 0.0 && op.feedbackMod == 0.0) {
                    op.index += op.stepIncrement;
                } else {
                    float freq = op.pitchedFreq + op.pitchedFreq * op.mod + op.pitchedFreq * op.feedbackMod;
                    float inc = op.stepIncrement + props.lookupTable->size * freq / props.sampleRate; // TODO optimize with precomputing: props.lookupTable->size * op.freq.get() / props.sampleRate
                    op.index += inc;
                    // printf("[op %d] freq: %f, inc: %f, mod %f \n", i, freq, inc, mod);
                    // mod = 0.0f;
                }
                while (op.index >= props.lookupTable->size) {
                    op.index -= props.lookupTable->size;
                }
                float s = props.lookupTable->sine[(int)op.index] * env;

                if (op.feedback.get() > 0.0f) {
                    op.feedbackMod = s * op.feedback.pct();
                }

                if (i == ZIC_FM_OPS_COUNT - 1) { // Last operator can only be the carrier
                    outDivider++;
                    out += s;
                    out = out / outDivider;
                } else {
                    bool isMod = false;
                    for (int j = 0; j < ZIC_FM_OPS_COUNT - 1; j++) {
                        if (algorithm[(uint8_t)(algo.get() - 1)][i][j]) {
                            isMod = true;
                            // j + 1 because the first operator doesnt receive modulation
                            if (operators[j + 1].mod == 0.0f) {
                                operators[j + 1].mod = s;
                            } else {
                                operators[j + 1].mod *= s;
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

        buf[track] = out * velocity;
    }

    enum DATA_ID {
        ALGO,
    };

    /*md **Data ID**: */
    uint8_t getDataId(std::string name) override
    {
        /*md - `ALGO` return current algorithm */
        if (name == "ALGO")
            return ALGO;
        return atoi(name.c_str());
    }

    void* data(int id, void* userdata = NULL) override
    {
        switch (id) {
        case ALGO:
            bool(*ret)[3] = algorithm[(uint8_t)(algo.get() - 1)];
            return ret;
        }
        return NULL;
    }
};
