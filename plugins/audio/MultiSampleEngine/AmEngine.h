#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

#include <cmath>

class AmEngine : public SampleEngine {
protected:
    MultiFx multiFx;
    MMfilter filter;

    // Parameters
    Val& depth = val(50.0f, "DEPTH", { "Depth", .unit = "%" });
    Val& pitchOffset = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24.0f, .max = 24.0f });
    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) { multiFx.setFxType(p); });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Val& ratio = val(1.0f, "RATIO", { "Ratio", .min = 0.25f, .max = 8.0f, .step = 0.25f, .floatingPoint = 2 });

    float sampleRate;

    float modPhase = 0.0f;
    float lastOutput = 0.0f;

    // Helper: simple sine wave increment
    float modStep = 0.0f;

    inline float midiToFreq(int note) const
    {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

public:
    AmEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer)
        : SampleEngine(props, config, sampleBuffer, "AM")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        sampleRate = props.sampleRate;
    }

    void noteOn(uint8_t note, float velocity, void*) override
    {
        (void)velocity;
        float freq = midiToFreq(note + pitchOffset.get());
        modStep = 2.0f * (float)M_PI * freq * ratio.get() / sampleRate;
        modPhase = 0.0f;
    }

    float getSample(int index, float stepIncrement) override
    {
        if (sampleBuffer.count == 0)
            return 0.0f;

        float modValue = sinf(modPhase);
        float depthFactor = depth.pct();

        float out = 0.0f;

        uint64_t i = (uint64_t)index;
        if (i >= sampleBuffer.count)
            i = sampleBuffer.count - 1;
        out = sampleBuffer.data[i] * (1.0f + modValue * depthFactor) * 0.5f;

        lastOutput = out;

        modPhase += modStep;
        if (modPhase > 2.0f * (float)M_PI)
            modPhase -= 2.0f * (float)M_PI;

        return out;
    }

    void sample(float* buf, int index) override
    {
        float out = buf[track];

        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out;
    }
};
