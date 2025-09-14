#pragma once

#include "helpers/clamp.h"
#include "filter.h"
#include "mapping.h"

// #include <math.h>

// another version of the same filter but with a small clicking at 0.5

class EffectFilterMultiMode2 : public Mapping {
protected:
    EffectFilterData hpf;
    EffectFilterData lpf;

public:
    // Cutoff mix
    Val& mix = val(50.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED }, [&](auto p) { setCutoff(p.value); });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance" }, [&](auto p) { setResonance(p.value); });

    EffectFilterMultiMode2(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
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

        if (value > 0.5) {
            // 0 to 0.10
            float cutoff = (0.10 * ((mix.pct() - 0.5) * 2)) + 0.00707;
            hpf.setRawCutoff(cutoff);
            lpf.setRawCutoff(0.99);
        } else {
            // From 0.95 to 0.1
            float cutoff = 0.85 * (mix.pct() * 2) + 0.1;
            hpf.setRawCutoff(0.00707);
            lpf.setRawCutoff(cutoff);
        }
    }

    void setResonance(float value)
    {
        resonance.setFloat(value);
        lpf.setResonance(resonance.pct());
        hpf.setResonance(resonance.pct());
    };
};
