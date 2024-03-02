#ifndef _FILTER_H_
#define _FILTER_H_

#include "audioPlugin.h"

// https://www.martin-finke.de/articles/audio-plugins-013-filter/
// https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html

#include <math.h> // fabs

class EffectFilterData {
protected:
    float inRange(float s)
    {
        if (s < -1.0f) {
            return -1.0f;
        } else if (s > 1.0f) {
            return 1.0f;
        }
        return s;
    }

public:
    float cutoff = 0.0f;
    float feedback = 0.0f;
    float buf = 0.0f;
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
        // inputValue = inRange(inputValue);

        // hp = inRange(inputValue - buf);
        // bp = inRange(buf - lp);
        // buf = inRange(buf + cutoff * (hp + feedback * bp));
        // lp = inRange(lp + cutoff * (buf - lp));

        hp = (inputValue - buf);
        bp = (buf - lp);
        buf = (buf + cutoff * (hp + feedback * bp));
        lp = (lp + cutoff * (buf - lp));

        // if (inputValue > 1.0f || inputValue < -1.0f) {
        //     printf("inputValue = %f buf = %f hp = %f bp = %f lp = %f\n", inputValue, buf, hp, bp, lp);
        // }

        // hp = inputValue;
        // bp = inputValue;
        // lp = inputValue;
    }
};

#endif
