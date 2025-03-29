#pragma once

#include "filter.h"
#include "helpers/range.h"
#include "mapping.h"

/*md
## EffectFilterMultiMode

EffectFilterMultiMode plugin is used to apply filter effect on audio buffer.
Cutoff frequency will switch from low pass filter to high pass filter when reaching 50%.
*/
class EffectFilterMultiMode : public Mapping {
protected:
    EffectFilterData filter;
    float mix = 0.0;

    std::string valueFmt = "%d%% LP|HP %d%%";

public:
    /*md **Values**: */
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0 }, [&](auto p) {
        p.val.setFloat(p.value);
        float amount = p.val.pct() * 2 - 1.0f;

        char strBuf[128];
        if (p.val.get() > 0.0) {
            filter.setType(EffectFilterData::Type::HP);
            filter.setHp(amount, filter.resonance);
            sprintf(strBuf, "HP %d%%", (int)(amount * 100));
            if (p.val.get() < 50.0) {
                mix = p.val.get() * 0.02f;
            }
        } else {
            filter.setType(EffectFilterData::Type::LP);
            filter.setLp(-amount, filter.resonance);
            sprintf(strBuf, "LP %d%%", (int)((-amount) * 100));
            if (p.val.get() > -20.0) {
                mix = -p.val.get() * 0.05f;
            }
            // if (p.val.get() > -10.0) {
            //     mix = -p.val.get() * 0.1f;
            // }
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
        valueFmt = config.json.value("cutoffStringFormat", valueFmt);
    };

    float sample(float inputValue)
    {
        if (inputValue == 0 || cutoff.get() == 0.0) {
            return inputValue;
        }

        if (mix < 1.0) {
            return inputValue * (1.0 - mix) + filter.process(inputValue) * mix;
        }

        return filter.process(inputValue);
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }
};
