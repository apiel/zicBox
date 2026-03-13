#pragma once

#include <algorithm>
#include <cmath>

#ifndef TINY
#define TINY 1e-20
#endif

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

    FilterTB()
    {
        cutoff = 1000.0;
        resonanceSkewed = 0.0;
        sampleRate = 44100.0;

        // Initialize high-pass state
        hp_x1 = hp_y1 = 0.0;

        setMode(FLAT);
        setSampleRate(44100.0);
        reset();
    }

    void setSampleRate(double newSampleRate)
    {
        if (newSampleRate > 0.0) sampleRate = newSampleRate;
        twoPiOverSampleRate = 2.0 * M_PI / sampleRate;

        // Simplified High-pass coefficients (Fixed at 150Hz)
        // Based on the original OnePoleFilter logic
        double x = std::exp(-2.0 * M_PI * 150.0 / sampleRate);
        hp_b0 = 0.5 * (1 + x);
        hp_b1 = -0.5 * (1 + x);
        hp_a1 = x;

        calculateCoefficientsApprox4();
    }

    void setMode(int newMode)
    {
        if (newMode >= 0 && newMode < NUM_MODES) {
            mode = newMode;
            c0 = c1 = c2 = c3 = c4 = 0.0;
            if (mode == LP_6) c1 = 1.0;
            else if (mode == LP_12) c2 = 1.0;
            else if (mode == LP_18) c3 = 1.0;
            else if (mode == LP_24) c4 = 1.0;
            else c0 = 1.0;
        }
        calculateCoefficientsApprox4();
    }

    void reset()
    {
        y1 = y2 = y3 = y4 = 0.0;
        hp_x1 = hp_y1 = 0.0;
    }

    inline void setCutoff(double newPct)
    {
        double newCutoff = std::clamp(newPct * 18000.0 + 200.0, 200.0, 18000.0);
        if (newCutoff != cutoff) {
            cutoff = newCutoff;
            calculateCoefficientsApprox4();
        }
    }

    inline void setResonance(float newResonance)
    {
        resonanceSkewed = (1.0 - std::exp(-3.0 * newResonance)) / (1.0 - std::exp(-3.0));
        calculateCoefficientsApprox4();
    }

    inline double getSample(double in)
    {
        // 1. Process feedback through fixed 150Hz High-pass
        double fb = k * y4;
        double filteredFeedback = hp_b0 * fb + hp_b1 * hp_x1 + hp_a1 * hp_y1 + TINY;
        hp_x1 = fb;
        hp_y1 = filteredFeedback;

        // 2. Ladder Filter Stages
        double y0 = 0.125 * in - filteredFeedback;

        y1 = y0 + a1 * (y0 - y1);
        y2 = y1 + a1 * (y1 - y2);
        y3 = y2 + a1 * (y2 - y3);
        y4 = y3 + a1 * (y3 - y4);

        return 8.0 * (c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3 + c4 * y4);
    }

    void calculateCoefficientsApprox4()
    {
        double wc = twoPiOverSampleRate * cutoff;
        double wc2 = wc * wc;

        // Pole coefficients
        const double pa[] = { -0.9999999999857464, 0.9999999927726119, -0.9999994950291231,
            0.9583192455599817, -0.9164580250284832, 0.8736418933533319,
            -0.8249882473764324, 0.752969164867889, -0.6297350825423579,
            0.4439739664918068, -0.2365036766021623, 0.08168739417977708, -0.01341281325101042 };

        double tmp = wc2 * pa[12] + pa[11] * wc + pa[10];
        for (int i = 9; i >= 1; i -= 2)
            tmp = wc2 * tmp + pa[i] * wc + pa[i - 1];
        a1 = tmp;

        // Resonance coefficients
        const double pr[] = { 4.000000000000113, 3.99999999965004, 1.00000001212423,
            -0.1666668203490468, -0.08333236384240325, 0.00207992115173378,
            0.002784706718370008, -2.022131730719448e-05, -4.554677015609929e-05 };

        double rtmp = wc2 * pr[8] + pr[7] * wc + pr[6];
        rtmp = wc2 * rtmp + pr[5] * wc + pr[4];
        rtmp = wc2 * rtmp + pr[3] * wc + pr[2];
        rtmp = wc2 * rtmp + pr[1] * wc + pr[0];
        k = resonanceSkewed * rtmp;
    }

protected:
    double a1, y1, y2, y3, y4, c0, c1, c2, c3, c4, k, cutoff, resonanceSkewed, sampleRate, twoPiOverSampleRate;

    // Internal High-pass state and coeffs
    double hp_x1, hp_y1, hp_b0, hp_b1, hp_a1;
    int mode;
};