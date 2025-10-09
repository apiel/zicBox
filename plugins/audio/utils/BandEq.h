#pragma once

#include <algorithm>
#include <math.h>
#include <cstdint>

class BandEq {
protected:
    uint64_t sampleRate;
    float centerFreq = 1000.0f;
    float rangeHz = 2000.0f;

    struct BQ {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
        float process(float x)
        {
            float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = x;
            y2 = y1;
            y1 = y;
            return y;
        }
    } lowpass, highpass;

    void updateCoeffs()
    {
        // Compute min/max freq from centerFreq and rangeHz
        float minFreq = std::max(20.0f, centerFreq - rangeHz * 0.5f);
        float maxFreq = std::min(sampleRate * 0.5f, centerFreq + rangeHz * 0.5f);

        setHighpass(highpass, minFreq, sampleRate);
        setLowpass(lowpass, maxFreq, sampleRate);
    }

    void setLowpass(BQ& bq, float freq, float fs)
    {
        float w0 = 2.0f * M_PI * freq / fs;
        float cosw0 = cosf(w0);
        float sinw0 = sinf(w0);
        float alpha = sinw0 / (2.0f * 0.707f); // Q=0.707

        float b0n = (1 - cosw0) / 2;
        float b1n = 1 - cosw0;
        float b2n = (1 - cosw0) / 2;
        float a0n = 1 + alpha;
        float a1n = -2 * cosw0;
        float a2n = 1 - alpha;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

    void setHighpass(BQ& bq, float freq, float fs)
    {
        float w0 = 2.0f * M_PI * freq / fs;
        float cosw0 = cosf(w0);
        float sinw0 = sinf(w0);
        float alpha = sinw0 / (2.0f * 0.707f);

        float b0n = (1 + cosw0) / 2;
        float b1n = -(1 + cosw0);
        float b2n = (1 + cosw0) / 2;
        float a0n = 1 + alpha;
        float a1n = -2 * cosw0;
        float a2n = 1 - alpha;

        bq.b0 = b0n / a0n;
        bq.b1 = b1n / a0n;
        bq.b2 = b2n / a0n;
        bq.a1 = a1n / a0n;
        bq.a2 = a2n / a0n;
    }

public:
    BandEq(uint64_t sampleRate) : sampleRate(sampleRate) {
        updateCoeffs();
    }

    void setCenterFreq(float value) {
        centerFreq = value;
        updateCoeffs();
    }

    void setRangeHz(float value) {
        rangeHz = value;
        updateCoeffs();
    }

    float process(float input) {
        float band = highpass.process(input);
        band = lowpass.process(band);
        return band;
    }
};