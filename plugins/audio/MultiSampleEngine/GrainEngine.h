#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/mapping.h"
#include "plugins/audio/utils/EnvelopRelative.h"
#include "plugins/audio/utils/Grains.h"
#include "plugins/audio/utils/MultiFx.h"

class GrainEngine : public LoopedEngine {
protected:
    MultiFx multiFx;
    Grains grains;

    float envSteps = 0.00001f;

    Val& getValExtra()
    {
        return val(4.0f, "DENSITY", { "Density", .min = 1.0, .max = MAX_GRAINS }, [&](auto p) {
            p.val.setFloat(p.value);
            grains.setDensity(p.val.get());
        });
    }

    float getDataSample(uint64_t idx)
    {
        return sampleBuffer.data[idx];
    }

public:
    Val& length = val(100.0f, "GRAIN_LENGTH", { "Grain Length", .min = 5.0, .max = 500.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setGrainDuration(props.sampleRate * length.get() * 0.001f);
    });

    Val& densityDelay = val(100.0f, "GRAIN_DELAY", { "Grain Delay", .min = 1.0, .max = 1000, .unit = "ms" }, [&](auto p) {
        densityDelay.setFloat(p.value);
        grains.setGrainDelay(props.sampleRate * p.val.get() * 0.001f);
    });

    Val& detune = val(0.0f, "DETUNE", { "Detune", VALUE_CENTERED, .min = -12.0f, .max = 12.0f, .step = 0.1f, .floatingPoint = 1, .unit = "st" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setDetune(p.val.get());
    });

    Val& randomize = val(0.0f, "RANDOMIZE", { "Randomize", VALUE_CENTERED, .min = -100.0f, .max = 100.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);

        float delay = 0.0f;
        float pitch = 0.0f;
        if (p.val.get() > 0.0f) {
            delay = p.val.get() * 2 * 0.5f;
            p.val.props().unit = "%delay";
        } else if (p.val.get() < 0.0f) {
            pitch = 1 - (p.val.get() * 2 * 0.5f);
            p.val.props().unit = "%pitch";
        } else {
            p.val.props().unit = "none";
        }

        grains.setDelayRandomize(delay);
        grains.setPitchRandomize(pitch);
    });

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
        , grains(props.lookupTable, [this](uint64_t idx) -> float { return getDataSample(idx); })
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
        return grains.getGrainSample(stepIncrement, index, sampleBuffer.count);
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
