#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/mapping.h"
#include "plugins/audio/utils/EnvelopRelative.h"
#include "plugins/audio/utils/MultiFx.h"

#define MAX_GRAINS 16

class GrainEngine : public LoopedEngine {
protected:
    MultiFx multiFx;

    uint64_t grainDuration = 0;
    uint64_t grainDelay = 0;
    float envSteps = 0.00001f;
    float densityDivider = 1.0f;

    struct Grain {
        uint64_t index = 0;
        float position = 0.0f;
        float positionIncrement = 1.0f;
    } grains[MAX_GRAINS];

    void initGrain(uint8_t densityIndex, uint64_t sampleIndex, float stepIncrement)
    {
        Grain& grain = grains[densityIndex];
        grain.index = 0;
        grain.position = sampleIndex + densityIndex * grainDelay + grainDelay * getRand() * delayRandomize.pct();
        grain.positionIncrement = stepIncrement + stepIncrement * getRand() * pitchRandomize.pct();
    }

    float getRand()
    {
        return props.lookupTable->getNoise();
    }

    Val& getValExtra()
    {
        return val(4.0f, "DENSITY", { "Density", .min = 1.0, .max = MAX_GRAINS }, [&](auto p) {
            p.val.setFloat(p.value);
            densityDivider = 1.0f / p.val.get();
        });
    }

public:
    Val& length = val(100.0f, "GRAIN_LENGTH", { "Grain Length", .min = 5.0, .max = 100.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        grainDuration = props.sampleRate * length.get() * 0.001f;
    });

    Val& densityDelay = val(100.0f, "GRAIN_DELAY", { "Grain Delay", .min = 1.0, .max = 1000, .unit = "ms" }, [&](auto p) {
        densityDelay.setFloat(p.value);
        grainDelay = props.sampleRate * densityDelay.get() * 0.001f;
    });

    Val& delayRandomize = val(0.0f, "DELAY_RANDOMIZE", { "Delay Rand.", .unit = "%" });

    Val& pitchRandomize = val(0.0f, "PITCH_RANDOMIZE", { "Pitch Rand.", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    struct GetValExtra {
        GrainEngine* engine;
        Val& operator()() { return engine->getValExtra(); }
    };

    GrainEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "Grain", GetValExtra { this })
        , multiFx(props.sampleRate, props.lookupTable)
    {
    }

    void postProcess(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    float getSample(float stepIncrement) override
    {
        float out = 0.0f;
        for (uint8_t i = 0; i < valExtra.get(); i++) {
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

    // Actually one or the other should be enough...
    uint16_t getLoopCountRelease() override { return 0; }
    void postIncrement() override
    {
        if (index >= loopEnd && sustainedNote) {
            index = loopStart;
        }
    }
};
