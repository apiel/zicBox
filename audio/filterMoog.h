#pragma once
#include <math.h>

class FilterMoog {
public:
    float cutoff = 0.5f; // safe non-zero default
    float resonance = 0.0f;

    float b0 = 0.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float b3 = 0.0f;
    float b4 = 0.0f;

    float t1 = 0.0f;
    float t2 = 0.0f;

    float f = 0.0f;
    float p = 0.0f;
    float q = 0.0f;

    // Proper tanh approximation, stable for all inputs
    inline float clip(float x)
    {
        if (x > 1.0f) return 1.0f;
        if (x < -1.0f) return -1.0f;
        return x * (1.5f - 0.5f * x * x);
    }

    void calculateCoeffs()
    {
        q = 1.0f - cutoff;
        p = cutoff + 0.8f * cutoff * q;
        f = p + p - 1.0f;
        q = resonance * (1.0f + 0.5f * q * (1.0f - q + 5.6f * q * q));
    }

    void setCutoff(float c)
    {
        cutoff = fminf(fmaxf(c, 0.0001f), 0.99f);
        calculateCoeffs();
    }

    void setResonance(float r)
    {
        resonance = fminf(fmaxf(r, 0.0f), 1.0f);
        calculateCoeffs();
    }

    float process(float input)
    {
        float in = clip(input - q * b4); // clip the feedback sum

        t1 = b1;
        b1 = clip((in + b0) * p - b1 * f); // clip each stage

        t2 = b2;
        b2 = clip((b1 + t1) * p - b2 * f);

        t1 = b3;
        b3 = clip((b2 + t2) * p - b3 * f);

        b4 = clip((b3 + t1) * p - b4 * f);

        b0 = in;

        return b4;
    }

    void process(float* buf, int len)
    {
        for (int i = 0; i < len; i++) {
            buf[i] = process(buf[i]);
        }
    }
};