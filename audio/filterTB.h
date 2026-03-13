#pragma once

#include <algorithm>
#include <cmath>

class FilterTB {
public:
    enum modes {
        FLAT = 0,
        LP_6,
        LP_12,
        LP_18,
        LP_24,
        NUM_MODES
    };

    FilterTB(float sampleRate)
    {
        cutoff = 1000.0f;
        resonanceSkewed = 0.0f;

        hp_x1 = hp_y1 = 0.0f;

        setMode(FLAT);
        setSampleRate(sampleRate);
        reset();
    }

    void setSampleRate(float sampleRate)
    {
        twoPiOverSampleRate = 2.0f * static_cast<float>(M_PI) / sampleRate;

        // High-pass coefficients (Fixed at 150Hz)
        float x = std::exp(-2.0f * static_cast<float>(M_PI) * 150.0f / sampleRate);
        hp_b0 = 0.5f * (1.0f + x);
        hp_b1 = -0.5f * (1.0f + x);
        hp_a1 = x;

        calculateCoefficientsApprox4();
    }

    void setMode(int newMode)
    {
        if (newMode >= 0 && newMode < NUM_MODES) {
            mode = newMode;
            c0 = c1 = c2 = c3 = c4 = 0.0f;
            if (mode == LP_6) c1 = 1.0f;
            else if (mode == LP_12) c2 = 1.0f;
            else if (mode == LP_18) c3 = 1.0f;
            else if (mode == LP_24) c4 = 1.0f;
            else c0 = 1.0f;
        }
        calculateCoefficientsApprox4();
    }

    void reset()
    {
        y1 = y2 = y3 = y4 = 0.0f;
        hp_x1 = hp_y1 = 0.0f;
    }

    inline void set(float newCutoff, float newResonance)
    {
        setCutoff(newCutoff, true);
        setResonance(newResonance);
    }

    inline void setCutoff(float newPct, bool skipCoef = false)
    {
        float newCutoff = std::clamp(newPct * 12000.0f + 200.0f, 200.0f, 12200.0f);
        if (std::abs(newCutoff - cutoff) > 0.0001f) {
            cutoff = newCutoff;
            if (!skipCoef) calculateCoefficientsApprox4();
        }
    }

    inline void setResonance(float res)
    {
        res = std::clamp(res, 0.0f, 1.0f);
        resonanceSkewed = (1.0f - std::exp(-3.0f * res)) * 1.0540925533894598f; // Pre-calculated 1/(1-exp(-3))
        calculateCoefficientsApprox4();
    }

    inline float getSample(float in)
    {
        // 1. Feedback High-pass
        float fb = k * y4;
        float filteredFeedback = hp_b0 * fb + hp_b1 * hp_x1 + hp_a1 * hp_y1;
        hp_x1 = fb;
        hp_y1 = filteredFeedback;

        // 2. Ladder Filter Stages
        float y0 = 0.125f * in - filteredFeedback;

        y1 = y0 + a1 * (y0 - y1);
        y2 = y1 + a1 * (y1 - y2);
        y3 = y2 + a1 * (y2 - y3);
        y4 = y3 + a1 * (y3 - y4);

        return 8.0f * (c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3 + c4 * y4);
    }

    void calculateCoefficientsApprox4()
    {
        float wc = twoPiOverSampleRate * cutoff;
        float wc2 = wc * wc;

        // Pole coefficients
        const float pa[] = { -0.9999999999857464f, 0.9999999927726119f, -0.9999994950291231f,
            0.9583192455599817f, -0.9164580250284832f, 0.8736418933533319f,
            -0.8249882473764324f, 0.752969164867889f, -0.6297350825423579f,
            0.4439739664918068f, -0.2365036766021623f, 0.08168739417977708f, -0.01341281325101042f };

        float tmp = wc2 * pa[12] + pa[11] * wc + pa[10];
        for (int i = 9; i >= 1; i -= 2)
            tmp = wc2 * tmp + pa[i] * wc + pa[i - 1];
        a1 = tmp;

        // Resonance coefficients
        const float pr[] = { 4.000000000000113f, 3.99999999965004f, 1.00000001212423f,
            -0.1666668203490468f, -0.08333236384240325f, 0.00207992115173378f,
            0.002784706718370008f, -2.022131730719448e-05f, -4.554677015609929e-05f };

        float rtmp = wc2 * pr[8] + pr[7] * wc + pr[6];
        rtmp = wc2 * rtmp + pr[5] * wc + pr[4];
        rtmp = wc2 * rtmp + pr[3] * wc + pr[2];
        rtmp = wc2 * rtmp + pr[1] * wc + pr[0];
        k = resonanceSkewed * rtmp;
    }

        // y1 += a1 * (y0 - y1);
        // y2 += a1 * (y1 - y2);
        // y3 += a1 * (y2 - y3);
        // y4 += a1 * (y3 - y4);

    // void calculateCoefficientsApprox4()
    // {
    //     float wc = twoPiOverSampleRate * cutoff;

    //     // Simple stable one-pole coefficient
    //     float g = std::tan(wc * 0.5f);
    //     a1 = g / (1.0f + g);

    //     // k max = 4 for Moog ladder (4 = self-oscillation threshold)
    //     k = resonanceSkewed * 4.0f;
    // }

protected:
    float a1, y1, y2, y3, y4, c0, c1, c2, c3, c4, k, cutoff, resonanceSkewed, twoPiOverSampleRate;
    float hp_x1, hp_y1, hp_b0, hp_b1, hp_a1;
    int mode;
};