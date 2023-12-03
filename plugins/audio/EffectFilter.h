#ifndef _EFFECT_FILTER_H_
#define _EFFECT_FILTER_H_

#include "../../helpers/range.h"
#include "filter.h"
#include "mapping.h"

// #include <math.h>

class EffectFilter : public Mapping {
protected:
    EffectFilterData data;

public:
    // Cutoff mix
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff" }, [&](float value) { setCutoff(value); });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance" }, [&](float value) { setResonance(value); });

    enum Mode {
        OFF,
        LPF,
        HPF,
        BPF,
        MODE_COUNT,
    } mode
        = OFF;
    // TODO how to handle mode in a better way?
    Val& mode_value = val(0.0, "MODE", { "Mode" }, [&](float value) { setMode(value); });

    EffectFilter(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
    }

    float sample(float inputValue)
    {
        if (inputValue == 0 || mode == OFF) {
            return inputValue;
        }

        data.setSampleData(inputValue);

        if (mode == HPF) {
            return data.hp;
        } else if (mode == BPF) {
            return data.bp;
        }
        // LPF
        return data.buf0;
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }

    EffectFilter& setCutoff(float value)
    {
        cutoff.setFloat(value);

        if (mode == LPF) {
            data.setCutoff(0.85 * cutoff.pct() + 0.1);
        } else if (mode == BPF) {
            data.setCutoff(0.85 * cutoff.pct() + 0.1);
        } else { // HPF
            data.setCutoff((0.20 * cutoff.pct()) + 0.00707);
        }

        return *this;
    }

    EffectFilter& setResonance(float value)
    {
        resonance.setFloat(value);
        data.setResonance(resonance.pct());

        return *this;
    };

    EffectFilter& setMode(EffectFilter::Mode _mode)
    {
        mode = _mode;
        setCutoff(cutoff.get());
        return *this;
    };

    EffectFilter& setMode(float value)
    {
        mode_value.setFloat(value);
        mode = (Mode)range((uint8_t)mode_value.get(), 0, (uint8_t)MODE_COUNT);
        return setMode(mode);
    }
};

#endif
