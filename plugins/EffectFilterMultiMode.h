#ifndef _EFFECT_FILTER_H_
#define _EFFECT_FILTER_H_

#include "../helpers/range.h"
#include "filter.h"
#include "mapping.h"

class EffectFilterMultiMode : public Mapping<EffectFilterMultiMode> {
protected:
    EffectFilterData hpf;
    EffectFilterData lpf;

public:
    // Cutoff mix
    Val<EffectFilterMultiMode>& mix = val(this, 50.0, "CUTOFF", &EffectFilterMultiMode::setCutoff, { "LPF | HPF", .type = VALUE_CENTERED });
    Val<EffectFilterMultiMode>& resonance = val(this, 0.0, "RESONANCE", &EffectFilterMultiMode::setResonance, { "Resonance", .unit = "%" });

    EffectFilterMultiMode(AudioPlugin::Props& props, char* _name)
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
