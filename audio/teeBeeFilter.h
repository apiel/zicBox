#pragma once

#include <algorithm>
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef TINY
#define TINY 1e-20
#endif

class TeeBeeFilter {
public:
    enum modes {
        FLAT = 0,
        LP_6,
        LP_12,
        LP_18,
        LP_24,
        HP_6,
        HP_12,
        HP_18,
        HP_24,
        BP_12_12,
        BP_6_18,
        BP_18_6,
        BP_6_12,
        BP_12_6,
        BP_6_6,
        TB_303,
        NUM_MODES
    };

    // --- Internal OnePoleFilter Helper ---
    class OnePoleFilter {
    public:
        enum modes { BYPASS = 0,
            LOWPASS,
            HIGHPASS,
            LOWSHELV,
            HIGHSHELV,
            ALLPASS };

        OnePoleFilter()
        {
            shelvingGain = 1.0;
            sampleRate = 44100.0;
            sampleRateRec = 1.0 / 44100.0;
            mode = BYPASS;
            cutoff = 20000.0;
            reset();
        }

        void setSampleRate(double newSampleRate)
        {
            if (newSampleRate > 0.0) sampleRate = newSampleRate;
            sampleRateRec = 1.0 / sampleRate;
            calcCoeffs();
        }

        void setMode(int newMode)
        {
            mode = newMode;
            calcCoeffs();
        }
        void setCutoff(double newCutoff)
        {
            cutoff = (newCutoff > 0.0 && newCutoff <= 20000.0) ? newCutoff : 20000.0;
            calcCoeffs();
        }

        void reset() { x1 = y1 = 0.0; }

        void calcCoeffs()
        {
            double x = std::exp(-2.0 * PI * cutoff * sampleRateRec);
            switch (mode) {
            case LOWPASS:
                b0 = 1 - x;
                b1 = 0.0;
                a1 = x;
                break;
            case HIGHPASS:
                b0 = 0.5 * (1 + x);
                b1 = -0.5 * (1 + x);
                a1 = x;
                break;
            default:
                b0 = 1.0;
                b1 = 0.0;
                a1 = 0.0;
                break;
            }
        }

        inline double getSample(double in)
        {
            y1 = b0 * in + b1 * x1 + a1 * y1 + TINY;
            x1 = in;
            return y1;
        }

    private:
        double x1, y1, b0, b1, a1, cutoff, shelvingGain, sampleRate, sampleRateRec;
        int mode;
    };

    // --- TeeBeeFilter Implementation ---

    TeeBeeFilter()
    {
        cutoff = 1000.0;
        drive = 0.0;
        driveFactor = 1.0;
        resonanceRaw = 0.0;
        resonanceSkewed = 0.0;
        g = 1.0;
        sampleRate = 44100.0;
        twoPiOverSampleRate = 2.0 * PI / sampleRate;

        feedbackHighpass.setMode(OnePoleFilter::HIGHPASS);
        feedbackHighpass.setCutoff(150.0);

        setMode(TB_303);
        reset();
    }

    void setSampleRate(double newSampleRate)
    {
        if (newSampleRate > 0.0) sampleRate = newSampleRate;
        twoPiOverSampleRate = 2.0 * PI / sampleRate;
        feedbackHighpass.setSampleRate(newSampleRate);
        calculateCoefficientsExact();
    }

    void setDrive(double newDrive)
    {
        drive = newDrive;
        driveFactor = std::pow(10.0, drive / 20.0); // dB2amp
    }

    void setMode(int newMode)
    {
        if (newMode >= 0 && newMode < NUM_MODES) {
            mode = newMode;
            c0 = c1 = c2 = c3 = c4 = 0.0;
            switch (mode) {
            case FLAT:
                c0 = 1.0;
                break;
            case LP_6:
                c1 = 1.0;
                break;
            case LP_12:
                c2 = 1.0;
                break;
            case LP_18:
                c3 = 1.0;
                break;
            case LP_24:
                c4 = 1.0;
                break;
            case HP_6:
                c0 = 1.0;
                c1 = -1.0;
                break;
            case HP_12:
                c0 = 1.0;
                c1 = -2.0;
                c2 = 1.0;
                break;
            case HP_18:
                c0 = 1.0;
                c1 = -3.0;
                c2 = 3.0;
                c3 = -1.0;
                break;
            case HP_24:
                c0 = 1.0;
                c1 = -4.0;
                c2 = 6.0;
                c3 = -4.0;
                c4 = 1.0;
                break;
            case BP_12_12:
                c2 = 1.0;
                c3 = -2.0;
                c4 = 1.0;
                break;
            case TB_303:
                break; // Handled in getSample
            default:
                c0 = 1.0;
                break;
            }
        }
        calculateCoefficientsApprox4();
    }

    void reset()
    {
        feedbackHighpass.reset();
        y1 = y2 = y3 = y4 = 0.0;
    }

    inline void setCutoff(double newPct, bool updateCoefficients = true)
    {
        double newCutoff = newPct * 19800 + 200;
        newCutoff = std::max(200.0, std::min(newCutoff, 20000.0));
        if (newCutoff != cutoff) {
            cutoff = newCutoff;
            if (updateCoefficients) calculateCoefficientsApprox4();
        }
    }

    inline void setResonance(float newResonance, bool updateCoefficients = true)
    {
        resonanceRaw = newResonance;
        resonanceSkewed = (1.0 - std::exp(-3.0 * resonanceRaw)) / (1.0 - std::exp(-3.0));
        if (updateCoefficients) calculateCoefficientsApprox4();
    }

    inline double getSample(double in)
    {
        double y0;
        if (mode == TB_303) {
            y0 = in - feedbackHighpass.getSample(k * y4);
            y1 += 2 * b0 * (y0 - y1 + y2);
            y2 += b0 * (y1 - 2 * y2 + y3);
            y3 += b0 * (y2 - 2 * y3 + y4);
            y4 += b0 * (y3 - 2 * y4);
            return 2 * g * y4;
        }

        y0 = 0.125 * driveFactor * in - feedbackHighpass.getSample(k * y4);
        y1 = y0 + a1 * (y0 - y1);
        y2 = y1 + a1 * (y1 - y2);
        y3 = y2 + a1 * (y2 - y3);
        y4 = y3 + a1 * (y3 - y4);

        return 8.0 * (c0 * y0 + c1 * y1 + c2 * y2 + c3 * y3 + c4 * y4);
    }

    void calculateCoefficientsExact()
    {
        double wc = twoPiOverSampleRate * cutoff;
        double s = std::sin(wc);
        double c = std::cos(wc);
        double t = std::tan(0.25 * (wc - PI));
        double r = resonanceSkewed;

        double a1_fullRes = t / (s - c * t);
        double a1_noRes = -std::exp(-wc);

        a1 = r * a1_fullRes + (1.0 - r) * a1_noRes;
        b0 = 1.0 + a1;

        double gsq = (b0 * b0) / (1.0 + a1 * a1 + 2.0 * a1 * c);
        k = r / (gsq * gsq);

        if (mode == TB_303) k *= (17.0 / 4.0);
    }

    void calculateCoefficientsApprox4()
    {
        double wc = twoPiOverSampleRate * cutoff;
        double wc2 = wc * wc;
        double r = resonanceSkewed;

        const double pa[] = { -0.9999999999857464, 0.9999999927726119, -0.9999994950291231,
            0.9583192455599817, -0.9164580250284832, 0.8736418933533319,
            -0.8249882473764324, 0.752969164867889, -0.6297350825423579,
            0.4439739664918068, -0.2365036766021623, 0.08168739417977708, -0.01341281325101042 };

        double tmp = wc2 * pa[12] + pa[11] * wc + pa[10];
        for (int i = 9; i >= 1; i -= 2) {
            tmp = wc2 * tmp + pa[i] * wc + pa[i - 1];
        }
        a1 = tmp;
        b0 = 1.0 + a1;

        const double pr[] = { 4.000000000000113, 3.99999999965004, 1.00000001212423,
            -0.1666668203490468, -0.08333236384240325, 0.00207992115173378,
            0.002784706718370008, -2.022131730719448e-05, -4.554677015609929e-05 };

        double rtmp = wc2 * pr[8] + pr[7] * wc + pr[6];
        rtmp = wc2 * rtmp + pr[5] * wc + pr[4];
        rtmp = wc2 * rtmp + pr[3] * wc + pr[2];
        rtmp = wc2 * rtmp + pr[1] * wc + pr[0];
        k = r * rtmp;
        g = 1.0;

        if (mode == TB_303) {
            double fx = wc * (1.0 / std::sqrt(2.0)) / (2 * PI);
            b0 = (0.00045522346 + 6.1922189 * fx) / (1.0 + 12.358354 * fx + 4.4156345 * (fx * fx));
            k = fx * (fx * (fx * (fx * (fx * (fx + 7198.6997) - 5837.7917) - 476.47308) + 614.95611) + 213.87126) + 16.998792;
            g = ((k * (1.0 / 17.0) - 1.0) * r + 1.0) * (1.0 + r);
            k = k * r;
        }
    }

protected:
    double b0, a1, y1, y2, y3, y4, c0, c1, c2, c3, c4, k, g, driveFactor, cutoff, drive, resonanceRaw, resonanceSkewed, sampleRate, twoPiOverSampleRate;
    int mode;
    OnePoleFilter feedbackHighpass;
};
