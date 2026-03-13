#pragma once
#include <math.h>

class FilterSVF {
public:
    float cutoff = 0.0f;
    float resonance = 0.0f;
    float feedback = 0.0f;

    struct Data {
        float buf = 0.0f;
        float lp = 0.0f;
        float bp = 0.0f;
        float hp = 0.0f;
    };

    Data data1;
    Data data2;

    // soft saturation (acid character)
    inline float sat(float x)
    {
        return tanhf(x);
    }

    void setCutoff(float c)
    {
        // prevent instability
        cutoff = fminf(c, 0.99f);
        updateFeedback();
    }

    void setResonance(float r)
    {
        resonance = r;
        updateFeedback();
    }

    void updateFeedback()
    {
        if (resonance == 0.0f) {
            feedback = 0.0f;
            return;
        }

        float reso = resonance * 0.99f;
        float ratio = 1.0f - cutoff;

        if (ratio <= 0.0f) {
            feedback = 0.0f;
            return;
        }

        feedback = reso + reso / ratio;
    }

    // single SVF stage (with internal resonance)
    Data& stage(float input, Data& data)
    {
        data.hp = input - data.buf;
        data.bp = data.buf - data.lp;

        // resonance + saturation inside integrator
        float drive = sat(data.hp + feedback * data.bp);

        data.buf += cutoff * drive;
        data.lp += cutoff * (data.buf - data.lp);

        return data;
    }

    // 12 dB / octave
    Data& process12(float input)
    {
        return stage(input, data1);
    }

    // 24 dB / octave
    float processLp24(float input)
    {
        Data& s1 = stage(input, data1);
        return stage(s1.lp, data2).lp;
    }

    Data& stageArray(float input, Data& data)
    {
        data.hp = input - data.buf;
        data.bp = data.buf - data.lp;
        data.buf += cutoff * (data.hp + feedback * data.bp);
        data.lp += cutoff * (data.buf - data.lp);
        return data;
    }

    // 12 dB / octave (array-style cascade)
    Data& processArray12(float input)
    {
        return stageArray(input, data1);
    }

    // 24 dB / octave (array-style cascade)
    float processArrayLp24(float input)
    {
        Data& stage1 = processArray12(input);
        return stageArray(stage1.lp, data2).lp;
    }
};
