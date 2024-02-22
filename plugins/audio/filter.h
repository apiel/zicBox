#ifndef _FILTER_H_
#define _FILTER_H_

#include "audioPlugin.h"

// https://www.martin-finke.de/articles/audio-plugins-013-filter/
// https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html

#include <math.h> // fabs

class EffectFilterData {
public:
    float cutoff = 0.0f;
    float feedback = 0.0f;
    float buf1 = 0.0f;
    float lp = 0.0f;
    float hp = 0.0f;
    float bp = 0.0f;
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
        feedback = reso + reso / (1.0 - cutoff);
    }

    void setSampleData(float inputValue)
    {
        hp = inputValue - lp;
        bp = lp - buf1;

        lp = lp + cutoff * (hp + feedback * bp);
        buf1 = buf1 + cutoff * (lp - buf1);
    }
};

#endif
