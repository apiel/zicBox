#ifndef _EFFECT_FILTER_H_
#define _EFFECT_FILTER_H_

#include "../../helpers/range.h"
#include "filter.h"
#include "mapping.h"

// #include <math.h>

// another version of the same filter but with a small clicking at 0.5

class EffectFilterMultiMode2 : public Mapping<EffectFilterMultiMode2> {
protected:
    EffectFilterData hpf;
    EffectFilterData lpf;

public:
    // Cutoff mix
    Val<EffectFilterMultiMode2>& mix = val(this, 50.0, "CUTOFF", &EffectFilterMultiMode2::setCutoff, { "LPF | HPF", .type = VALUE_CENTERED });
    Val<EffectFilterMultiMode2>& resonance = val(this, 0.0, "RESONANCE", &EffectFilterMultiMode2::setResonance, { "Resonance" });

    EffectFilterMultiMode2(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        setCutoff(50.0);
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

    EffectFilterMultiMode2& setCutoff(float value)
    {
        mix.setFloat(value);

        if (value > 0.5) {
            // 0 to 0.10
            float cutoff = (0.10 * ((mix.pct() - 0.5) * 2)) + 0.00707;
            hpf.setCutoff(cutoff);
            lpf.setCutoff(0.99);
        } else {
            // From 0.95 to 0.1
            float cutoff = 0.85 * (mix.pct() * 2) + 0.1;
            hpf.setCutoff(0.00707);
            lpf.setCutoff(cutoff);
        }

        return *this;
    }

    EffectFilterMultiMode2& setResonance(float value)
    {
        resonance.setFloat(value);
        lpf.setResonance(resonance.pct());
        hpf.setResonance(resonance.pct());

        return *this;
    };
};

#endif
