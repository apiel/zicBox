#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

#include <cmath>

class AmEngine : public LoopedEngine {
protected:
    MultiFx multiFx;
    MMfilter filter;

    uint8_t playedNote = 0;

    // Parameters
    Val& depth = val(50.0f, "DEPTH", { "Depth", .unit = "%" });
    Val& pitchOffset = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24.0f, .max = 24.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        setModStep();
    });
    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) { multiFx.setFxType(p); });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Val& ratio = val(1.0f, "RATIO", { "Ratio", .min = 0.25f, .max = 8.0f, .step = 0.25f, .floatingPoint = 2 }, [&](auto p) {
        p.val.setFloat(p.value);
        setModStep();
    });

    float sampleRate;

    float modPhase = 0.0f;

    // Helper: simple sine wave increment
    float modStep = 0.0f;

    inline float midiToFreq(int note) const
    {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

    void setModStep()
    {
        float freq = midiToFreq(playedNote + pitchOffset.get());
        modStep = 2.0f * (float)M_PI * freq * ratio.get() / sampleRate;
    }

public:
    AmEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "AM")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        sampleRate = props.sampleRate;
    }

    void engineNoteOn(uint8_t note, float velocity) override
    {
        playedNote = note;
        setModStep();
        modPhase = 0.0f;
    }

    float getSample(float stepIncrement) override
    {
        if (sampleBuffer.count == 0)
            return 0.0f;

        float modValue = sinf(modPhase);
        float depthFactor = depth.pct();

        modPhase += modStep;
        if (modPhase > 2.0f * (float)M_PI)
            modPhase -= 2.0f * (float)M_PI;

        return sampleBuffer.data[(int)index] * (1.0f + modValue * depthFactor) * 0.5f;
    }

    void postProcess(float* buf) override
    {
        float out = buf[track];

        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out;
    }
};
