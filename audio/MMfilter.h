/** Description:
This blueprint defines a structure for an advanced digital audio processor called `MMfilter`. It is designed to function as a sophisticated frequency filter, commonly used in signal processing, synthesizers, or effects units to shape the sound texture.

**Core Functionality:**

The filter manages two primary controls: Cutoff and Resonance.

1.  **Cutoff Control:** This is the master frequency setting.
    *   Positive values activate a High-Pass filter (which removes low frequencies, allowing only high frequencies to pass).
    *   Negative values activate a Low-Pass filter (which removes high frequencies, allowing only low frequencies to pass).
2.  **Resonance:** This determines the sharpness or intensity boost applied right at the designated cutoff frequency.

**How it Works:**

The system takes an audio input and processes it through an external filtering engine. The `MMfilter` then performs a crucial step: **Automatic Blending (Mix)**.

Based on the intensity of the cutoff setting, the class automatically calculates how much of the original, untouched signal should be combined with the modified, filtered signal. If the filtering effect is very subtle (cutoff near zero), the system maintains a gentle blend. If the effect is strong, it uses a higher percentage of the filtered sound.

The final output is the result of this dynamically calculated mix, offering a smooth and controlled way to apply frequency modifications to an audio stream.

sha: 72a4efe07aa95a4d986df9aad52327942b39c1c3ae093082eb7d00982488aa25 
*/
#pragma once

#include "filter.h"

class MMfilter {
protected:
    EffectFilterData filter;
    float mix = 0.0;
    float cutoffAmount = 0.0;

public:
    void setResonance(float amount) { 
        filter.resonance = amount; 
        setCutoff(cutoffAmount);
    }

    void setCutoff(float amount)
    {
        cutoffAmount = amount;
        if (amount > 0.0) {
            filter.setType(EffectFilterData::Type::HP);
            filter.setHp(amount, filter.resonance);
            if (amount < 0.5) {
                mix = amount * 2;
            }
        } else {
            filter.setType(EffectFilterData::Type::LP);
            filter.setLp(-amount, filter.resonance);
            if (amount > -0.20) {
                mix = -amount * 5;
            }
            // if (amount > -0.10) {
            //     mix = -amount * 10;
            // }
        }
    }

    float process(float inputValue)
    {
        if (inputValue == 0 || mix == 0.0) {
            return inputValue;
        }

        if (mix < 1.0) {
            return inputValue * (1.0 - mix) + filter.process(inputValue) * mix;
        }

        return filter.process(inputValue);
    }
};
