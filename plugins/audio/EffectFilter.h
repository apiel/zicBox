#ifndef _EFFECT_FILTER_H_
#define _EFFECT_FILTER_H_

#include "../../helpers/range.h"
#include "filter.h"
#include "mapping.h"

// #include <math.h>
#include <string.h>

/*md
## EffectFilter

EffectFilter is a simple resonant filter.
*/
class EffectFilter : public Mapping {
protected:
    EffectFilterData data;

public:
    /*md **Values**: */
    /*md - `CUTOFF` set cutoff. */
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) { setCutoff(p.value); });
    /*md - `RESONANCE` set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) { setResonance(p.value); });

    enum Mode {
        OFF,
        LPF,
        HPF,
        BPF,
        MODE_COUNT,
    } mode
        = OFF;
    /*md - `MODE` set filter mode. */
    Val& mode_value = val(0.0, "MODE", { "Mode", VALUE_STRING, .max = MODE_COUNT - 1, }, [&](auto p) { setMode(p.value); });

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
        return data.lp;
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }

    std::string getMode(enum Mode _mode)
    {
        switch (_mode) {
        case OFF:
            return "OFF";
        case LPF:
            return "LPF";
        case HPF:
            return "HPF";
        case BPF:
            return "BPF";
        default:
            return "UNKNOWN";
        }
    }

    EffectFilter& setCutoff(float value)
    {
        cutoff.setFloat(value);
        cutoff.setString(getMode(mode));

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
        mode_value.setFloat(_mode);
        mode = mode_value.get();
        mode_value.setString(getMode(mode));
        setCutoff(cutoff.get());
        return *this;
    };

    EffectFilter& setMode(float value)
    {
        mode_value.setFloat(value);
        mode = (Mode)range((uint8_t)mode_value.get(), 0, (uint8_t)MODE_COUNT);
        mode_value.setString(getMode(mode));
        return setMode(mode);
    }
};

#endif
