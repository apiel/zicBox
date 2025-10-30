#pragma once

#include "helpers/clamp.h"
#include "audio/filter.h"
#include "mapping.h"

/*md
## EffectFilterMultiModeMix

EffectFilterMultiModeMix plugin is used to apply filter effect on audio buffer.
Cutoff frequency mixing from low pass filter to high pass filter.
*/
class EffectFilterMultiModeMix : public Mapping {
protected:
    EffectFilterData hpf;
    EffectFilterData lpf;

    std::string valueFmt = "%d%% LP|HP %d%%";

public:
    /*md **Values**: */
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& mix = val(50.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED }, [&](auto p) { setCutoff(p.value); });
    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) { setResonance(p.value); });

    EffectFilterMultiModeMix(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
        valueFmt = config.json.value("cutoffStringFormat", valueFmt);
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
        float mixValue = mix.pct();
        hpf.setRawCutoff((0.20 * mixValue) + 0.00707);
        lpf.setRawCutoff(0.85 * mixValue + 0.1);
        char strBuf[128];
        sprintf(strBuf, valueFmt.c_str(), (int)((1 - mixValue) * 100), (int)(mixValue * 100));
        mix.setString(strBuf);
    }

    void setResonance(float value)
    {
        resonance.setFloat(value);
        // printf("setResonance %f >> %f\n", value, resonance.pct());
        lpf.setResonance(resonance.pct());
        hpf.setResonance(resonance.pct());
    };
};
