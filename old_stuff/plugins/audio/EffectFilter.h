/** Description:
This file defines a key audio processing module called `EffectFilter`. Its purpose is to function as a resonant frequency filter, a powerful tool used in audio production to drastically shape or color sound by modifying specific frequency ranges.

**Core Functionality:**
The `EffectFilter` takes an incoming audio signal and runs it through an internal filtering engine. It determines whether to modify the sound based on user settings. If the filter is set to "OFF," the audio passes through untouched; otherwise, the sound is modified according to three main controls:

1.  **Filter Type:** This control selects the fundamental style of the effect:
    *   **OFF:** Bypass the filtering effect.
    *   **LP (Low Pass):** Allows low frequencies to pass while reducing high frequencies.
    *   **HP (High Pass):** Allows high frequencies to pass while reducing low frequencies.
    *   **BP (Band Pass):** Allows only a narrow band of frequencies to pass, cutting off both very low and very high frequencies.

2.  **Cutoff Frequency:** This control sets the target point (often displayed as a percentage) where the filtering action begins or ends. This is the main determinant of the filter's effect on the overall tone.

3.  **Resonance:** This control determines the sharpness or intensity of the filter’s effect precisely at the cutoff frequency. High resonance creates a distinct, often dramatic emphasis on that frequency point.

By manipulating these parameters, the filter can be used to achieve complex sound modifications, from subtle tone shaping to dramatic swept effects.

sha: 57b1b36b2b61b48e3f3b927cea6db5ce2d87cb7332264220f2497ff39df83cca 
*/
#pragma once

#include "audio/filter.h"
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
