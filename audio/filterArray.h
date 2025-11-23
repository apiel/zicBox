/** Description:
This code defines an essential building block for digital sound processing: a **Resonant Audio Filter**. It is specifically designed as a generic class capable of handling sound data in batches or arrays.

### Core Function and Purpose

The primary goal of this tool is to sculpt the frequency content of an audio signal. Analogous to an advanced equalizer, the filter modifies the sound by allowing certain frequencies to pass while attenuating others. This technique is fundamental for creating synth sounds, vocal effects, or dynamic sound shaping.

### Key Controls

The character of the filter is governed by two crucial settings:

1.  **Cutoff:** This setting determines the exact frequency point where the filter begins to operate, deciding whether high notes or low notes are primarily affected.
2.  **Resonance:** This controls the sharpness or intensity of the filter’s effect at the cutoff point. High resonance creates a distinct, often dramatic "peak" or ringing sound.

The code internally calculates a necessary "feedback" level based on these two controls to ensure the filter operates smoothly and predictably.

### How the Processing Works

When new sound data is introduced, the filter processes each audio sample and updates its internal memory. Critically, it calculates and stores the results for three simultaneous filter types:

*   **Low Pass:** Allows low frequencies to pass, dampening high notes.
*   **High Pass:** Allows high frequencies to pass, dampening low notes.
*   **Band Pass:** Allows only a specific range of mid-frequencies to pass, cutting off both the highest and lowest notes.

By using internal buffers to store the state of the sound (the previous filtered values), the system ensures that the filtering effect is continuous and smooth, resulting in complex and musical sonic manipulation.

sha: 102bab4291f14c37ff597e91aa763d63ac4e0434632ac1dfffa2c2353c645dc0 
*/
#pragma once

// https://www.martin-finke.de/articles/audio-plugins-013-filter/
// https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html

#include <math.h> // fabs

template <int SIZE>
class EffectFilterArray {
public:
    float cutoff = 0.0f;
    float feedback = 0.0f;
    float buf[SIZE] = { 0.0 };
    float lp[SIZE] = { 0.0 };
    float hp[SIZE] = { 0.0 };
    float bp[SIZE] = { 0.0 };
    float resonance = 0.0f;

    void setCutoff(float _cutoff)
    {
        cutoff = _cutoff;
        setResonance(resonance);
    }

    void setResonance(float _resonance)
    {
        resonance = _resonance;
        if (resonance == 0.0f) {
            feedback = 0.0f;
            return;
        }
        float reso = resonance * 0.99;
        float ratio = 1.0f - cutoff;
        if (ratio <= 0.0f) {
            feedback = 0.0f;
            return;
        }
        feedback = reso + reso / ratio;
    }

    void setSampleData(float inputValue, int index)
    {
        hp[index] = inputValue - buf[index];
        bp[index] = buf[index] - lp[index];
        buf[index] = buf[index] + cutoff * (hp[index] + feedback * bp[index]);
        lp[index] = lp[index] + cutoff * (buf[index] - lp[index]);
    }
};
