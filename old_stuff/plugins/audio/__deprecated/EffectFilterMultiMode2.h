/** Description:
This C++ header defines an advanced audio processing component called an **Effect Filter Multi-Mode**. Its primary function is to manipulate the frequency content, or tone, of an audio signal, offering complex tonal shifts controlled by simple parameters.

**Core Structure and Mechanism:**
The effect operates internally using two opposing frequency shapers: a Low-Pass Filter (LPF), which lets low tones pass while muting highs, and a High-Pass Filter (HPF), which does the opposite. When audio enters, it is processed simultaneously by both internal filters. The final output is then a carefully calculated blend of these two filtered signals.

**User Controls (Parameters):**
1.  **CUTOFF (Mix):** This is the main control that blends the LPF and HPF outputs. Instead of setting a single frequency, this parameter defines the *mode* of the filter. If the control is set low, the effect emphasizes the Low-Pass Filter, making the sound darker or muffled. If the control is set high, it transitions smoothly to emphasize the High-Pass Filter, making the sound thinner and brighter.
2.  **RESONANCE:** This parameter controls the sharpness or intensity of the filter’s effect. Increasing the resonance adds a distinct, pitched emphasis or "peak" near the point where the filter is actively shaping the sound.

By using the single "Cutoff" control to seamlessly transition between two fundamental filter types, this effect achieves a wide range of sonic manipulation, moving smoothly from a deep, low-end focused sound to a bright, high-end focused sound.

sha: 693ed42c72c0572fed97cd21d7b218eca31e70f85213c796afc30d8e71088904 
*/
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
