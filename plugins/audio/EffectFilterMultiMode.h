#ifndef _EFFECT_FILTER_H_
#define _EFFECT_FILTER_H_

#include "../../helpers/range.h"
#include "filter.h"
#include "mapping.h"

/*md
## EffectFilterMultiMode

EffectFilterMultiMode plugin is used to apply filter effect on audio buffer.
Cutoff frequency will switch from low pass filter to high pass filter when reaching 50%.
*/
class EffectFilterMultiMode : public Mapping {
protected:
    EffectFilterData hpf;
    EffectFilterData lpf;

public:
    /*md **Values**: */
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& mix = val(50.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED }, [&](auto p) { setCutoff(p.value); });
    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) { setResonance(p.value); });

    EffectFilterMultiMode(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    };

    float sample(float inputValue)
    {
        if (inputValue == 0) {
            return inputValue;
        }

        hpf.setSampleData(inputValue);
        lpf.setSampleData(inputValue);

        return lpf.lp * (1.0 - mix.pct()) + hpf.hp * mix.pct();
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }

    void setCutoff(float value)
    {
        mix.setFloat(value);
        // printf("setCutoff %f >> %f\n", value, mix.pct());
        hpf.setCutoff((0.20 * mix.pct()) + 0.00707);
        lpf.setCutoff(0.85 * mix.pct() + 0.1);
    }

    void setResonance(float value)
    {
        resonance.setFloat(value);
        // printf("setResonance %f >> %f\n", value, resonance.pct());
        lpf.setResonance(resonance.pct());
        hpf.setResonance(resonance.pct());
    };
};

#endif
