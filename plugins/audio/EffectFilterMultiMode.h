#pragma once

#include "helpers/range.h"
#include "mapping.h"
#include "utils/MMfilter.h"

/*md
## EffectFilterMultiMode

EffectFilterMultiMode plugin is used to apply filter effect on audio buffer.
Cutoff frequency will switch from low pass filter to high pass filter when reaching 50%.
*/
class EffectFilterMultiMode : public Mapping {
protected:
    MMfilter filter;

public:
    /*md **Values**: */
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0 }, [&](auto p) {
        p.val.setFloat(p.value);
        float amount = p.val.pct() * 2 - 1.0f;

        char strBuf[128];
        filter.setCutoff(amount);
        if (amount > 0.0) {
            sprintf(strBuf, "HP %d%%", (int)(amount * 100));
        } else {
            sprintf(strBuf, "LP %d%%", (int)((-amount) * 100));
        }
        p.val.setString(strBuf);
    });

    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // printf("setResonance %f >> %f\n", value, resonance.pct());
        filter.setResonance(p.val.pct());
    });

    EffectFilterMultiMode(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    };

    void sample(float* buf)
    {
        buf[track] = filter.process(buf[track]);
    }
};
