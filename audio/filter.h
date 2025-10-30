#pragma once

// https://www.martin-finke.de/articles/audio-plugins-013-filter/
// https://www.musicdsp.org/en/latest/Filters/29-resonant-filter.html

#include <functional>
#include <math.h> // fabs

// #define FILTER_USE_LUT

class EffectFilterData {
protected:
    float fix(float s)
    {
        if (isnan(s)) {
            return 0.0;
        }
        return s;
    }

#ifdef FILTER_USE_LUT
    float feedbackLut[101 * 101];
#endif

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
#ifdef FILTER_USE_LUT
        for (int c = 0; c <= 100; c++) {
            for (int r = 0; r <= 100; r++) {
                feedbackLut[c * 100 + r] = getFeedback(c / 100.0f, r / 100.0f);
            }
        }
#endif
    }

    typedef void (EffectFilterData::*SetFnPtr)(float, float);
    SetFnPtr setFn;

    typedef float (EffectFilterData::*ProcessFnPtr)(float);
    ProcessFnPtr processFn;

    float getLp(float _cutoff) { return 0.90 - (0.90 * _cutoff) + 0.1; }
    float getHp(float _cutoff) { return (0.80 * _cutoff) + 0.00707; }
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

    void setRawResonance(float _resonance)
    {
        resonance = _resonance;
        feedback = getFeedback();
    }

    void setResonance(float _resonance)
    {
        resonance = _resonance;
#ifdef FILTER_USE_LUT
        feedback = getLutFeedback();
#else
        feedback = getFeedback();
#endif
    }

#ifdef FILTER_USE_LUT
    float getLutFeedback()
    {
        int c = cutoff * 100;
        int r = resonance * 100;
        return feedbackLut[c * 100 + r];
    }
#endif

    float getFeedback()
    {
        return getFeedback(cutoff, resonance);
    }

    float getFeedback(float _cutoff, float _resonance)
    {
        if (_resonance == 0.0f) {
            return 0.0;
        }
        if (_cutoff >= 1.0f) {
            return 0.0;
        }
        float ratio = 1.0f - _cutoff;
        float reso = _resonance * 0.99;
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
        setSampleData(inputValue, buf, hp, bp, lp);
    }

    void setSampleData(float inputValue, float& _buf, float& _hp, float& _bp, float& _lp)
    {
        _hp = inputValue - _buf;
        _bp = _buf - _lp;
        _buf = _buf + cutoff * (_hp + feedback * _bp);
        _lp = _lp + cutoff * (_buf - _lp);
    }

    float processLp(float inputValue)
    {
        setSampleData(inputValue, buf, hp, bp, lp);
        return lp;
    }

    float processHp(float inputValue)
    {
        setSampleData(inputValue, buf, hp, bp, lp);
        return hp;
    }

    float processBp(float inputValue)
    {
        setSampleData(inputValue, buf, hp, bp, lp);
        return bp;
    }
};
