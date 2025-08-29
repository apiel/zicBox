#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/mapping.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class MonoEngine : public LoopedEngine {
protected:
    MMfilter filter;
    MultiFx multiFx;
    MultiFx multiFx2;

public:
    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { "FX2 type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });
    Val& fx2Amount = val(0, "FX2_AMOUNT", { "FX2 edit", .unit = "%" });

    MonoEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "Mono")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
    {
    }

    void postProcess(float* buf) override
    {
        float out = buf[track];

        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());

        buf[track] = out;
    }
};
