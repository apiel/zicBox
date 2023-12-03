#ifndef _EFFECT_FILTER_H_
#define _EFFECT_FILTER_H_

#include "../../helpers/range.h"
#include "filter.h"
#include "mapping.h"

class EffectFilterMultiMode : public Mapping {
protected:
    EffectFilterData hpf;
    EffectFilterData lpf;

public:
    // Cutoff mix
    Val& mix = val(50.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED }, [&](auto p) { setCutoff(p.value); });
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

        return lpf.buf1 * (1.0 - mix.pct()) + hpf.hp * mix.pct();
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }

    EffectFilterMultiMode& setCutoff(float value)
    {
        mix.setFloat(value);
        hpf.setCutoff((0.20 * mix.pct()) + 0.00707);
        lpf.setCutoff(0.85 * mix.pct() + 0.1);

        return *this;
    }

    EffectFilterMultiMode& setResonance(float value)
    {
        resonance.setFloat(value);
        lpf.setResonance(resonance.pct());
        hpf.setResonance(resonance.pct());

        return *this;
    };
};

#endif
