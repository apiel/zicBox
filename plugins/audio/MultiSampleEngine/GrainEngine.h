#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"
#include "plugins/audio/mapping.h"
// #include "plugins/audio/utils/AsrEnvelop.h"
#include "plugins/audio/utils/EnvelopRelative.h"

#define MAX_GRAINS 16

class GrainEngine : public SampleEngine {
protected:
    uint64_t grainDuration = 0;
    uint64_t grainDelay = 0;
    float envSteps = 0.00001f;
    float densityDivider = 1.0f;

    // AsrEnvelop env = { &envSteps, &envSteps, NULL };

    struct Grain {
        uint64_t index = 0;
        float position = 0.0f;
        float positionIncrement = 1.0f;
        EnvelopRelative env = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 0.0f }, { 0.0f, 1.0f } });
    } grains[MAX_GRAINS];

    void initGrain(uint8_t densityIndex, uint64_t sampleIndex, float stepIncrement)
    {
        Grain& grain = grains[densityIndex];
        grain.index = 0;
        grain.position = sampleIndex + densityIndex * grainDelay + grainDelay * getRand() * delayRandomize.pct();
        grain.positionIncrement = stepIncrement + stepIncrement * getRand() * pitchRandomize.pct();
        grain.env.reset(grainDuration);
    }

    float getRand()
    {
        return props.lookupTable->getNoise();
    }

public:
    Val& length = val(100.0f, "LENGTH", { "Length", .min = 5.0, .max = 100.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        grainDuration = props.sampleRate * length.get() * 0.001f;
    });

    Val& density = val(10.0f, "DENSITY", { "Density", .min = 1.0, .max = MAX_GRAINS }, [&](auto p) {
        p.val.setFloat(p.value);
        densityDivider = 1.0f / p.val.get();
    });

    Val& densityDelay = val(10.0f, "DENSITY_DELAY", { "Density Delay", .min = 1.0, .max = 1000, .unit = "ms" }, [&](auto p) {
        densityDelay.setFloat(p.value);
        grainDelay = props.sampleRate * densityDelay.get() * 0.001f;
    });

    Val& envelop = val(0.0f, "ENVELOP", { "Envelop", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        for (uint8_t i = 0; i < MAX_GRAINS; i++) {
            grains[i].env.data[1].time = envelop.pct() * 0.5f;
            grains[i].env.data[2].time = 1.0f - envelop.pct() * 0.5f;
            // printf("%f %f\n", grains[i].env.data[0].time, grains[i].env.data[1].time);
        }
    });

    Val& delayRandomize = val(0.0f, "DELAY_RANDOMIZE", { "Delay Rand.", .unit = "%" });

    Val& pitchRandomize = val(0.0f, "PITCH_RANDOMIZE", { "Pitch Rand.", .unit = "%" });

    GrainEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer)
        : SampleEngine(props, config, sampleBuffer, "Grain")
    {
    }

    void sample(float* buf, int index) override
    {
        // Do nothing
    }

    float getSample(int index, float stepIncrement) override
    {
        float out = 0.0f;
        for (uint8_t i = 0; i < density.get(); i++) {
            Grain& grain = grains[i];
            if (grain.index++ < grainDuration) {
                grain.position += grain.positionIncrement;
                if (grain.position >= sampleBuffer.count) {
                    initGrain(i, index, stepIncrement);
                }
            } else {
                initGrain(i, index, stepIncrement);
            }
            out += sampleBuffer.data[(int)grain.position];
        }
        out = out * densityDivider;
        return out;
    }
};
