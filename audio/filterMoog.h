#pragma once
#include <math.h>

class FilterMoog {
public:
    float cutoff = 0.5f;
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

    // tanh approximation — smooth, stable, good saturation character
    inline float tanh_approx(float x)
    {
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    void calculateCoeffs()
    {
        float t = 1.0f - cutoff;
        p = cutoff + 0.8f * cutoff * t;
        f = p + p - 1.0f;

        // Gentle resonance curve — reaches self-oscillation only near 1.0
        // The 0.85 ceiling keeps it from whistling too aggressively
        q = resonance * 0.85f * (2.0f * p * (1.0f - 0.15f * p * p));
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
        // Saturate only the feedback path — this is the 303 character
        float in = input - tanh_approx(q * b4);

        t1 = b1;
        b1 = (in + b0) * p - b1 * f;

        t2 = b2;
        b2 = (b1 + t1) * p - b2 * f;

        t1 = b3;
        b3 = (b2 + t2) * p - b3 * f;

        b4 = (b3 + t1) * p - b4 * f;

        // Soft output saturation — adds warmth without hard clipping
        b4 = tanh_approx(b4);

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