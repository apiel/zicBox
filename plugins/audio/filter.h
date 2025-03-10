#pragma once

// https://www.martin-finke.de/articles/audio-plugins-013-filter/
// https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html

#include <math.h> // fabs

class EffectFilterData {
protected:
    float fix(float s)
    {
        if (isnan(s)) {
            return 0.0;
        }
        return s;
    }

public:
    float cutoff = 0.0f;
    float feedback = 0.0f;
    float buf = 0.0;
    float lp = 0.0;
    float hp = 0.0;
    float bp = 0.0;
    float resonance = 0.0f;

    void setCutoff(float _cutoff)
    {
        cutoff = _cutoff;
        setResonance(resonance);
    }

    void setLpCutoff(float _cutoff)
    {
        // setCutoff((0.85 - (0.85 * _cutoff)) + 0.1);
        setCutoff((0.90 - (0.90 * _cutoff)) + 0.1);
    }

    void setHpCutoff(float _cutoff)
    {
        setCutoff((0.20 * _cutoff) + 0.00707);
    }

    void setBpCutoff(float _cutoff)
    {
        // setCutoff(0.85 * _cutoff + 0.1);
        setCutoff(0.95 * _cutoff + 0.1);
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

    // void setSampleData(float inputValue)
    // {
    //     hp = fix(inputValue - buf);
    //     bp = fix(buf - lp);
    //     buf = fix(buf + cutoff * (hp + feedback * bp));
    //     lp = fix(lp + cutoff * (buf - lp));
    // }

    void setSampleData(float inputValue)
    {
        hp = inputValue - buf;
        bp = buf - lp;
        buf = buf + cutoff * (hp + feedback * bp);
        lp = lp + cutoff * (buf - lp);
    }

    float processLp(float inputValue)
    {
        setSampleData(inputValue);
        return lp;
    }

    float processHp(float inputValue)
    {
        setSampleData(inputValue);
        return hp;
    }

    float processBp(float inputValue)
    {
        setSampleData(inputValue);
        return bp;
    }
};
