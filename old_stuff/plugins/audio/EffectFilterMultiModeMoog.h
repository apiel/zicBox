/** Description:
This C++ header defines an audio processing component called `EffectFilterMultiModeMoog`. Its purpose is to digitally simulate a very famous and influential type of sound filter, often associated with Moog synthesizers. This simulation is critical for shaping the tone and character of audio signals.

### Core Functionality

The component acts as a versatile filter, capable of sculpting the frequency content of the sound in multiple ways. It processes incoming audio samples individually, applying complex mathematical stages based on classic analog filter design principles.

### User Controls and Multi-Mode Switching

1.  **CUTOFF:** This is the primary control, acting as a two-in-one switch.
    *   When set from 0% to 50%, the filter operates in **Low-Pass Filter (LPF)** mode, which allows low frequencies (bass) to pass through while reducing higher frequencies.
    *   When set from 50% to 100%, it seamlessly switches to **High-Pass Filter (HPF)** mode, allowing high frequencies to pass while reducing bass.

2.  **RESONANCE:** This control adds feedback into the filter stages. Increasing resonance creates a distinct emphasis or peak right at the cutoff frequency, which is a defining characteristic of the classic Moog sound, often resulting in a sharp, sometimes whining, tone.

### Internal Mechanism

The effect achieves its sound by cycling each incoming audio sample through four mathematical delay and mixing stages. It uses internal variables that model the physics of the original analog circuit, calculating factors like frequency response (`p`, `f`) and the amount of feedback (`q`). A key detail is the inclusion of slight non-linear processing (soft clipping) in the signal path, which mimics how analog circuits naturally distort, adding warmth and richness to the filtered sound.

sha: 614ccb550074a16345cebf4313883dc6ce46a57df937680c79d997b650ad52b2 
*/
#pragma once

#include "helpers/clamp.h"
#include "audio/filter.h"
#include "mapping.h"

// #include <math.h>

// Paul Kellet version of the classic Stilson/Smith "Moog" filter
// https://www.kvraudio.com/forum/viewtopic.php?t=144625

/*md
## EffectFilterMultiModeMoog

EffectFilterMultiModeMoog plugin is used to apply a simulation of the Moog filter on audio buffer.
Cutoff frequency will switch from low pass filter to high pass filter when reaching 50%.
*/
class EffectFilterMultiModeMoog : public Mapping {
protected:
    float cutoff = 0.00;
    float f, p, q = 0.00;
    float b0, b1, b2, b3, b4 = 0.0;
    float t1, t2 = 0.0;

    void calculateVar(float _cutoff, float _resonance)
    {
        q = 1.0f - _cutoff;
        p = _cutoff + 0.8f * _cutoff * q;
        f = p + p - 1.0f;
        // resonance should be from 0.0 to 0.90, higher values is too high
        q = (_resonance * 0.90) * (1.0f + 0.5f * q * (1.0f - q + 5.6f * q * q));
    }

public:
    /*md **Values**: */
    /*md - `CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& mix = val(50.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED }, [&](auto p) { setCutoff(p.value); });
    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance" }, [&](auto p) { setResonance(p.value); });

    EffectFilterMultiModeMoog(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    };

    float fix(float s)
    {
        if (isnan(s)) {
            return 0.0;
        }
        return s;
    }

    float sample(float inputValue)
    {
        inputValue -= q * b4; // feedback
        t1 = b1;
        b1 = fix((inputValue + b0) * p - b1 * f);
        t2 = b2;
        b2 = fix((b1 + t1) * p - b2 * f);
        t1 = b3;
        b3 = fix((b2 + t2) * p - b3 * f);
        b4 = fix((b3 + t1) * p - b4 * f);
        b4 = fix(b4 - b4 * b4 * b4 * 0.166667f); // clipping
        b0 = inputValue;

        // printf("q: %f p: %f b0: %f b1: %f b2: %f b3: %f b4: %f t1: %f t2: %f f: %f\n", q, p, b0, b1, b2, b3, b4, t1, t2, f);


        // Lowpass  output:  b4
        // Highpass output:  in - b4;
        // Bandpass output:  3.0f * (b3 - b4);

        return b4 * (1.0 - mix.pct()) + (inputValue - b4) * mix.pct();
    }

    void sample(float* buf)
    {
        buf[track] = sample(buf[track]);
    }

    EffectFilterMultiModeMoog& setCutoff(float value)
    {
        mix.setFloat(value);
        if (mix.get() > 0.5) {
            cutoff = 1 - mix.pct() + 0.0707;
        } else {
            cutoff = mix.pct() + 0.05; // LPF should not be 0.0
        }
        calculateVar(cutoff, resonance.pct());
        return *this;
    }

    EffectFilterMultiModeMoog& setResonance(float value)
    {
        resonance.setFloat(value);
        calculateVar(cutoff, resonance.pct());
        return *this;
    };
};
