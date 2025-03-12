#pragma once

// https://www.martin-finke.de/articles/audio-plugins-013-filter/
// https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html

#include <functional>
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

    EffectFilterData()
    {
        setType(LP);
    }

    typedef void (EffectFilterData::*SetFnPtr)(float, float);
    SetFnPtr setFn;

    typedef float (EffectFilterData::*ProcessFnPtr)(float);
    ProcessFnPtr processFn;

    float getLp(float _cutoff) { return 0.90 - (0.90 * _cutoff) + 0.1; }
    float getHp(float _cutoff) { return (0.20 * _cutoff) + 0.00707; }
    float getBp(float _cutoff) { return 0.95 * _cutoff + 0.1; }
    void setLp(float _cutoff, float _resonance) { setRawCutoff(getLp(_cutoff), _resonance); }
    void setHp(float _cutoff, float _resonance) { setRawCutoff(getHp(_cutoff), _resonance); }
    void setBp(float _cutoff, float _resonance) { setRawCutoff(getBp(_cutoff), _resonance); }

    enum Type {
        LP,
        HP,
        BP
    };

    void setType(Type type)
    {
        if (type == LP) {
            setFn = &EffectFilterData::setLp;
            processFn = &EffectFilterData::processLp;
        } else if (type == HP) {
            setFn = &EffectFilterData::setHp;
            processFn = &EffectFilterData::processHp;
        } else if (type == BP) {
            setFn = &EffectFilterData::setBp;
            processFn = &EffectFilterData::processBp;
        }
    }

    float process(float inputValue)
    {
        return (this->*processFn)(inputValue);
    }

    void set(float _cutoff, float _resonance)
    {
        (this->*setFn)(_cutoff, _resonance);
    }

    void setRawCutoff(float _cutoff)
    {
        cutoff = _cutoff;
        feedback = getFeedback();
    }

    void setRawCutoff(float _cutoff, float _resonance)
    {
        cutoff = _cutoff;
        setResonance(_resonance);
    }

    void setResonance(float _resonance)
    {
        resonance = _resonance;
        feedback = getFeedback();
    }

    float getFeedback() {
        if (resonance == 0.0f) {
            return 0.0;
        }
        if (cutoff >= 1.0f) {
            return 0.0;
        }
        float ratio = 1.0f - cutoff;
        float reso = resonance * 0.99;
        return reso + reso / ratio;
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
