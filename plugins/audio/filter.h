#ifndef _FILTER_H_
#define _FILTER_H_

#include "audioPlugin.h"

// https://www.martin-finke.de/articles/audio-plugins-013-filter/
// https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html

#include <math.h> // fabs

class EffectFilterData {
protected:
    float fix(double s)
    {
        if (isnan(s)) {
            return 0.0;
        }
        // if (s < -1.0f) {
        //     return -1.0f;
        // } else if (s > 1.0f) {
        //     return 1.0f;
        // }
        return s;
    }

public:
    float cutoff = 0.0f;
    float feedback = 0.0f;
    double buf = 0.0;
    double lp = 0.0;
    double hp = 0.0;
    double bp = 0.0;
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
        // inputValue = fix(inputValue);
        hp = fix(inputValue - buf);
        bp = fix(buf - lp);
        buf = fix(buf + cutoff * (hp + feedback * bp));
        lp = fix(lp + cutoff * (buf - lp));

        // hp = (inputValue - buf);
        // bp = (buf - lp);
        // buf = (buf + cutoff * (hp + feedback * bp));
        // lp = (lp + cutoff * (buf - lp));

        // printf("inputValue = %f buf = %f hp = %f bp = %f lp = %f\n", inputValue, buf, hp, bp, lp);
    }
};

#endif
