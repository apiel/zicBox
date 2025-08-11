#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

/*md
## EffectFilteredMultiFx
*/

class EffectFilteredMultiFx : public Mapping {
protected:
    MMfilter filter;
    MultiFx multiFx;

public:
    /*md **Values**: */
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });

    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // printf("setResonance %f >> %f\n", value, resonance.pct());
        filter.setResonance(p.val.pct());
    });

    /*md - `FX_TYPE` select the effect.*/
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    /*md - `FEEDBACK` to set feedback. */
    Val& feedback = val(0.0f, "FEEDBACK", { "Feedback", .unit = "%" });

    /*md - `MIX` set the effect mix vs the original signal.*/
    Val& mix = val(50.0f, "MIX", { "Mix", .unit = "%" });

    // internal amounts
    float scaledClipping = 0.0f;

    EffectFilteredMultiFx(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , multiFx(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    // process a single-sample buffer (matching the style of EffectVolumeClipping)
    void sample(float* buf)
    {
        float input = buf[track];

        float filterOut = filter.process(buf[track]);
        float feedbackSignal = filterOut * feedback.pct();

        filterOut = multiFx.apply(filterOut, fxAmount.pct());

        float out = input * (1 - mix.pct()) + filterOut * mix.pct();
        out += feedbackSignal;

        buf[track] = out;
    }
};
