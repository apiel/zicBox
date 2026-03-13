#pragma once
#include <math.h>

class FilterSVF {
public:
    float cutoff = 0.0f;
    float resonance = 0.0f;
    float feedback = 0.0f;

    // stage 1
    float buf1 = 0.0f;
    float lp1 = 0.0f;
    float bp1 = 0.0f;
    float hp1 = 0.0f;

    // stage 2
    float buf2 = 0.0f;
    float lp2 = 0.0f;
    float bp2 = 0.0f;
    float hp2 = 0.0f;

    void setCutoff(float c)
    {
        cutoff = c;
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

    float process12(float input)
    {
        // resonance feedback from stage 1
        float x = input - feedback * lp1;

        hp1 = x - buf1;
        bp1 = buf1 - lp1;
        buf1 = buf1 + cutoff * hp1;
        lp1 = lp1 + cutoff * (buf1 - lp1);

        return lp1;
    }

    float process24(float input)
    {
        // global resonance feedback
        float x = input - feedback * lp2;

        // stage 1
        hp1 = x - buf1;
        bp1 = buf1 - lp1;
        buf1 = buf1 + cutoff * hp1;
        lp1 = lp1 + cutoff * (buf1 - lp1);

        // stage 2
        hp2 = lp1 - buf2;
        bp2 = buf2 - lp2;
        buf2 = buf2 + cutoff * hp2;
        lp2 = lp2 + cutoff * (buf2 - lp2);

        return lp2;
    }
};