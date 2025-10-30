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
