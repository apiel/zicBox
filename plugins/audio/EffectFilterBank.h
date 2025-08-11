#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/effects/applyClipping.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include "plugins/audio/utils/effects/applySoftClipping.h"
#include "plugins/audio/utils/utils.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

/*md
## EffectFilterBank
*/

class EffectFilterBank : public Mapping {
protected:
    MMfilter filter;

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

    Val& drive = val(0.0f, "DRIVE", { "Drive", .unit = "%" });
    Val& clipAmount = val(0.0f, "CLIP", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // scaled like original plugin
        scaledClipping = p.val.pct() * p.val.pct() * 20.0f;
    });

    Val& feedback = val(0.0f, "FEEDBACK", { "Routing.|Feedback", .unit = "%" });
    Val& mix = val(50.0f, "MIX", { "Mix", .unit = "%" });

    // internal amounts
    float scaledClipping = 0.0f;

    EffectFilterBank(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    // process a single-sample buffer (matching the style of EffectVolumeClipping)
    void sample(float* buf)
    {
        float input = buf[track];

        float filterOut = filter.process(buf[track]);
        float feedbackSignal = filterOut * feedback.pct();

        if (drive.pct() > 0.0f) filterOut = applyDrive(filterOut, drive.pct(), props.lookupTable);
        filterOut = applyClipping(filterOut, scaledClipping);

        float out = input * (1 - mix.pct()) + filterOut * mix.pct();
        out += feedbackSignal;

        buf[track] = out;
    }
};
