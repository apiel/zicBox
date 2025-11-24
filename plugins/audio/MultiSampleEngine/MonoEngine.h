/** Description:
This C++ header defines the blueprint for an advanced audio processing module called `MonoEngine`. It is designed to handle a single track of audio, building upon a fundamental system specialized in looping and sample playback.

The core purpose of the `MonoEngine` is to shape and modify the sound through a defined sequence of steps. Internally, it is equipped with three main sound modifiers: a primary audio **Filter** (used for tone shaping, like adjusting bass or treble) and two independent **Multi-Effects units**.

Crucially, this blueprint defines several controls that an external user interface or host software can manipulate:

1.  **Filter Controls:** Allow adjustment of the filter's characteristics, specifically the "Cutoff" (determining the frequency range affected) and "Resonance" (controlling the intensity of the filter effect).
2.  **Dual Effects Controls (FX1 and FX2):** Provides two separate sets of controls. For both the first and the second effects unit, the user can select the "Type" of effect (e.g., delay or chorus) and control the "Amount" or depth of the processing.

When the audio signal passes through the engine, it is processed sequentially: the signal first goes through the dedicated Filter, then through the first Multi-Effect unit, and finally through the second Multi-Effect unit before reaching the output. This structure allows for powerful, layered sound creation and modification.

sha: cbce8bf2155a052199c8bb6786660a5ddacf756cbf8780fe38ee385af3acf013 
*/
#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/mapping.h"
#include "audio/MMfilter.h"
#include "audio/MultiFx.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

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

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { "FX2 type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);
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
