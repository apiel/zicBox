#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class StretchEngine : public LoopedEngine {
protected:
    MMfilter filter;
    MultiFx multiFx;

    Val& stretch = val(1.0f, "STRETCH", { "Stretch", .min = 1.0f, .max = 20.0f, .incType = INC_ONE_BY_ONE });
    Val& grainSize = val(100.0f, "GRAIN_SIZE", { "Grain Size", .min = 1.0f, .max = 5000.0f, .incType = INC_MULT });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 },
        [&](auto p) { multiFx.setFxType(p); });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX amount", .unit = "%" });

public:
    StretchEngine(AudioPlugin::Props& props, AudioPlugin::Config& config,
        SampleBuffer& sampleBuffer, float& index, float& stepMultiplier)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "Stretch")
        , multiFx(props.sampleRate, props.lookupTable)
    {
    }

    int stretchLoopCount = 0;
    float grainStart = 0.0f;
    void postIncrement() override
    {
        if (index >= grainStart + grainSize.get()) {
            if (stretchLoopCount < stretch.get() - 1) {
                index = grainStart;
                stretchLoopCount++;
            } else {
                grainStart = index;
                stretchLoopCount = 0;
            }
        }

        if (index >= loopEnd && (sustainedNote || nbOfLoopBeforeRelease > 0)) {
            index = loopStart;
            grainStart = loopStart;
            nbOfLoopBeforeRelease--;
        }
    }

    void postProcess(float* buf) override
    {
        float out = buf[track];
        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void engineNoteOn(uint8_t note, float _velocity) override
    {
        grainStart = indexStart;
        stretchLoopCount = 0;
    }
};
