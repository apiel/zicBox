#pragma once

#include "utils/filter.h"
#include "helpers/clamp.h"
#include "mapping.h"

// #include <math.h>
#include <string.h>

/*md
## EffectFilter

EffectFilter is a simple resonant filter.
*/
class EffectFilter : public Mapping {
protected:
    EffectFilterData filter;

public:
    /*md **Values**: */
    /*md - `FILTER_TYPE` Select filter type.*/
    enum FilterType {
        FILTER_OFF,
        LP,
        BP,
        HP,
        FILTER_COUNT
    };
    Val& filterType = val(1, "FILTER_TYPE", { "Filter", VALUE_STRING, .max = EffectFilter::FilterType::FILTER_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == EffectFilter::FilterType::FILTER_OFF) {
            p.val.setString("OFF");
        } else if (p.val.get() == EffectFilter::FilterType::LP) {
            p.val.setString("LPF");
            filter.setType(EffectFilterData::Type::LP);
        } else if (p.val.get() == EffectFilter::FilterType::BP) {
            p.val.setString("BPF");
            filter.setType(EffectFilterData::Type::BP);
        } else if (p.val.get() == EffectFilter::FilterType::HP) {
            p.val.setString("HPF");
            filter.setType(EffectFilterData::Type::HP);
        }
        filterCutoff.set(filterCutoff.get());
        filterResonance.set(filterResonance.get());
    });

    /*md - `FILTER_CUTOFF` set the filter cutoff frequency.*/
    Val& filterCutoff = val(0, "FILTER_CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.set(p.val.pct(), filterResonance.pct());
    });

    /*md - `FILTER_RESONANCE` set the filter resonance.*/
    Val& filterResonance = val(0, "FILTER_RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    EffectFilter(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
    }

    float process(float input)
    {
        if (input == 0 || filterType.get() == EffectFilter::FilterType::FILTER_OFF) {
            return input;
        }
        return filter.process(input);
    }

    void sample(float* buf)
    {
        buf[track] = process(buf[track]);
    }
};
