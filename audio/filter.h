/** Description:
This C++ header file defines an essential component for digital audio effects, named `EffectFilterData`. Its primary role is to implement a highly flexible audio filter, similar to the equalization features found on professional mixing consoles or synthesizers.

The code allows users to shape the frequency content of an audio signal by providing three fundamental modes of operation:
1.  **Low-Pass (LP):** Lets lower-pitched sounds pass while blocking high-pitched sounds.
2.  **High-Pass (HP):** Lets higher-pitched sounds pass while blocking low-pitched sounds.
3.  **Band-Pass (BP):** Isolates a specific middle range of frequencies.

The sound is controlled using two main "knobs":
*   **Cutoff:** Determines the specific frequency point where the filtering effect begins.
*   **Resonance:** Dictates the intensity or sharpness of the filter effect at the cutoff point.

**How It Works:**
The system processes the incoming audio signal step-by-step, taking one tiny audio "sample" at a time. To create smooth, time-varying effects, the filter needs a short memory. It uses internal storage areas (buffers) to track previous sample values. This historical data is combined with the current input and the defined cutoff/resonance settings to calculate the filtered output.

Crucially, the system uses internal pointers to instantly switch its calculation routines depending on which filter mode (LP, HP, or BP) is selected, ensuring the correct mathematical process is applied without delay. Additionally, an optional optimization feature allows complex internal settings to be looked up instantly from a pre-calculated table, increasing performance when the user rapidly adjusts the resonance control.

sha: f1837192789a5a5fa5b0ecc92e54ccfb01f337569fbae91337f65952552479ce 
*/
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
