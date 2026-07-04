/** Description:
This document defines the blueprint for an audio processor known as the Effect Filter Multi-Mode Mix. Its primary function is to apply a dynamic filtering effect to an audio stream, allowing musicians or producers to sculpt the tone and frequency content of the sound.

**Core Mechanism**

The effect works by combining two distinct types of frequency filters: a Low Pass Filter (LPF) and a High Pass Filter (HPF). The LPF allows low-frequency sounds (bass) to pass while blocking high frequencies, resulting in a warmer or muffled tone. Conversely, the HPF allows high-frequency sounds (treble) to pass while blocking low frequencies, resulting in a thinner or brighter sound.

**Key Controls**

1.  **CUTOFF / Mix:** This is the central control that handles two jobs simultaneously. As the user adjusts this setting, it determines the precise frequency where the filtering action begins, and critically, it controls the ratio between the LPF and HPF outputs. For instance, setting the control to 0% results in a pure LPF sound, while 100% results in a pure HPF sound. Settings in the middle blend the two filters seamlessly.
2.  **RESONANCE:** This control adds emphasis or a distinct "peak" to the sound right at the point where the filter is set to cutoff, making the filtering effect more noticeable and often more dramatic.

**Operation**

When an audio signal enters this system, it is processed through both the LPF and HPF simultaneously. The software then calculates the final output by taking a weighted mix of the two resulting signals, based exactly on the percentage set by the `CUTOFF` control. This allows for smooth, continuous transitions between the characteristics of a deep Low Pass filter and a sharp High Pass filter.

sha: 326d7d4bbf0756e8a77aefc4083c53fc3bd59a4f141be4f78e537d1526f458a5 
*/
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
