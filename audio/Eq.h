#pragma once

#include <array>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ------------------------------------------------------------------
// Band types
// ------------------------------------------------------------------
enum EQBandType {
    EQ_LOW_SHELF,
    EQ_PEAK,
    EQ_HIGH_SHELF,
};

// ------------------------------------------------------------------
// Single biquad filter band
// Direct Form I, Audio EQ Cookbook coefficients (R. Bristow-Johnson)
// ------------------------------------------------------------------
struct BiquadFilter {
    EQBandType type = EQ_PEAK;
    float freq = 1000.0f; // Hz
    float gainDb = 0.0f; // dB, range [-12, +12]
    float Q = 0.707f; // bandwidth (fixed per band)

    // Coefficients (computed once on param change)
    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;

    // Delay state
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    void compute(double sampleRate)
    {
        double A = pow(10.0, gainDb / 40.0); // sqrt of linear gain
        double w0 = 2.0 * M_PI * freq / sampleRate;
        double cosw = cos(w0);
        double sinw = sin(w0);
        double alpha = sinw / (2.0 * Q);

        double a0;

        switch (type) {
        case EQ_LOW_SHELF: {
            double ap = A + 1, am = A - 1;
            double sqA = 2.0 * sqrt(A) * alpha;
            b0 = A * (ap - am * cosw + sqA);
            b1 = 2.0 * A * (am - ap * cosw);
            b2 = A * (ap - am * cosw - sqA);
            a0 = ap + am * cosw + sqA;
            a1 = -2.0 * (am + ap * cosw);
            a2 = ap + am * cosw - sqA;
            break;
        }
        case EQ_HIGH_SHELF: {
            double ap = A + 1, am = A - 1;
            double sqA = 2.0 * sqrt(A) * alpha;
            b0 = A * (ap + am * cosw + sqA);
            b1 = -2.0 * A * (am + ap * cosw);
            b2 = A * (ap + am * cosw - sqA);
            a0 = ap - am * cosw + sqA;
            a1 = 2.0 * (am - ap * cosw);
            a2 = ap - am * cosw - sqA;
            break;
        }
        case EQ_PEAK:
        default: {
            double alphaA = alpha * A;
            double alphaOA = alpha / A;
            b0 = 1.0 + alphaA;
            b1 = -2.0 * cosw;
            b2 = 1.0 - alphaA;
            a0 = 1.0 + alphaOA;
            a1 = -2.0 * cosw;
            a2 = 1.0 - alphaOA;
            break;
        }
        }

        // Normalise by a0
        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;
    }

    // Call once per sample in the audio thread
    inline float process(float in)
    {
        double out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = in;
        y2 = y1;
        y1 = out;
        return (float)out;
    }

    // Magnitude response at a given frequency (for drawing the curve)
    // Returns dB gain at freq `f` Hz
    float magnitudeAt(float f, double sampleRate) const
    {
        double w = 2.0 * M_PI * f / sampleRate;
        double cosw = cos(w), sinw = sin(w);
        double cos2w = cos(2.0 * w), sin2w = sin(2.0 * w);

        // H(e^jw) numerator/denominator as complex magnitude
        double nr = b0 + b1 * cosw + b2 * cos2w;
        double ni = -(b1 * sinw + b2 * sin2w);
        double dr = 1.0 + a1 * cosw + a2 * cos2w;
        double di = -(a1 * sinw + a2 * sin2w);

        double mag2 = (nr * nr + ni * ni) / (dr * dr + di * di);
        return (float)(10.0 * log10(mag2 + 1e-12));
    }
};

// ------------------------------------------------------------------
// 5-band parametric EQ
//   Band 0: Low shelf   80 Hz
//   Band 1: Peak       250 Hz
//   Band 2: Peak      1000 Hz
//   Band 3: Peak      4000 Hz
//   Band 4: High shelf 12000 Hz
// ------------------------------------------------------------------
struct EQ {
    static constexpr int NUM_BANDS = 5;

    std::array<BiquadFilter, NUM_BANDS> bands;

    // Fixed Q values per band type (simple, not user-editable)
    static constexpr float SHELF_Q = 0.707f;
    static constexpr float PEAK_Q = 1.41f;

    // Gain limits
    static constexpr float GAIN_MIN = -12.0f;
    static constexpr float GAIN_MAX = 12.0f;

    // Frequency limits per band (clamp during drag)
    const float freqMin[NUM_BANDS] = { 20.0f, 80.0f, 200.0f, 1000.0f, 4000.0f };
    const float freqMax[NUM_BANDS] = { 200.0f, 800.0f, 5000.0f, 12000.0f, 20000.0f };

    EQ()
    {
        // Band 0 — Low shelf
        bands[0].type = EQ_LOW_SHELF;
        bands[0].freq = 80.0f;
        bands[0].Q = SHELF_Q;

        // Band 1 — Peak
        bands[1].type = EQ_PEAK;
        bands[1].freq = 250.0f;
        bands[1].Q = PEAK_Q;

        // Band 2 — Peak (mid)
        bands[2].type = EQ_PEAK;
        bands[2].freq = 1000.0f;
        bands[2].Q = PEAK_Q;

        // Band 3 — Peak (upper mid)
        bands[3].type = EQ_PEAK;
        bands[3].freq = 4000.0f;
        bands[3].Q = PEAK_Q;

        // Band 4 — High shelf
        bands[4].type = EQ_HIGH_SHELF;
        bands[4].freq = 12000.0f;
        bands[4].Q = SHELF_Q;

        for (auto& b : bands)
            b.gainDb = 0.0f;
    }

    // Call after changing any band parameter to recompute coefficients
    void recompute(double sampleRate)
    {
        for (auto& b : bands)
            b.compute(sampleRate);
    }

    // Process one sample through all bands in series
    inline float process(float in)
    {
        for (auto& b : bands)
            in = b.process(in);
        return in;
    }

    // Combined magnitude response at frequency f (sum of all bands in dB)
    float totalMagnitudeAt(float f, double sampleRate) const
    {
        float total = 0.0f;
        for (const auto& b : bands)
            total += b.magnitudeAt(f, sampleRate);
        return total;
    }

    // Generate a vector of {x_pixel, y_pixel} points for drawing the curve
    // canvasX/Y: top-left of the canvas rect
    // canvasW/H: size of the canvas
    // dbRange: half the dB range shown (e.g. 12 means -12 to +12)
    std::vector<std::pair<float, float>> curvePoints(
        int canvasX, int canvasY, int canvasW, int canvasH,
        float dbRange, double sampleRate, int numPoints = 200) const
    {
        std::vector<std::pair<float, float>> pts;
        pts.reserve(numPoints);

        for (int i = 0; i < numPoints; i++) {
            // Log-spaced from 20 Hz to 20000 Hz
            float t = (float)i / (numPoints - 1);
            float freq = 20.0f * pow(1000.0f, t); // 20 * 1000^t => 20..20000

            float db = totalMagnitudeAt(freq, sampleRate);
            db = std::max(-dbRange, std::min(dbRange, db));

            float px = canvasX + (canvasW * t);
            float py = canvasY + (canvasH / 2.0f) - (db / dbRange) * (canvasH / 2.0f);
            pts.push_back({ px, py });
        }
        return pts;
    }

    // Return the pixel position of a band's control point
    std::pair<float, float> bandPointPos(
        int bandIdx, int canvasX, int canvasY, int canvasW, int canvasH,
        float dbRange) const
    {
        const auto& b = bands[bandIdx];
        float t = log(b.freq / 20.0f) / log(1000.0f); // inverse of freq mapping
        t = std::max(0.0f, std::min(1.0f, t));

        float px = canvasX + canvasW * t;
        float py = canvasY + (canvasH / 2.0f) - (b.gainDb / dbRange) * (canvasH / 2.0f);
        return { px, py };
    }

    // Update a band's frequency from a pixel X position
    void setBandFreqFromX(int bandIdx, float px, int canvasX, int canvasW)
    {
        float t = (px - canvasX) / (float)canvasW;
        t = std::max(0.0f, std::min(1.0f, t));
        float freq = 20.0f * pow(1000.0f, t);
        freq = std::max(freqMin[bandIdx], std::min(freqMax[bandIdx], freq));
        bands[bandIdx].freq = freq;
    }

    // Update a band's gain from a pixel Y position
    void setBandGainFromY(int bandIdx, float py, int canvasY, int canvasH, float dbRange)
    {
        float norm = ((canvasY + canvasH / 2.0f) - py) / (canvasH / 2.0f);
        float gain = norm * dbRange;
        gain = std::max(GAIN_MIN, std::min(GAIN_MAX, gain));
        bands[bandIdx].gainDb = gain;
    }
};