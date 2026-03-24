#pragma once

#include <array>
#include <cmath>
#include <complex>
#include <mutex>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ================================================================
// Radix-2 Cooley-Tukey FFT (in-place, power-of-2 only)
// ================================================================
static void fft(std::vector<std::complex<float>>& x)
{
    int n = (int)x.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        float ang = -2.f * (float)M_PI / len;
        std::complex<float> wlen(cosf(ang), sinf(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1.f, 0.f);
            for (int j = 0; j < len / 2; j++) {
                auto u = x[i + j], v = x[i + j + len / 2] * w;
                x[i + j] = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

static inline float hannWindow(int i, int n)
{
    return 0.5f * (1.f - cosf(2.f * (float)M_PI * i / (n - 1)));
}

// ================================================================
// SpectrumAnalyser
// Ring buffer written by audio thread (post-EQ).
// compute() called by UI thread every frame.
// ================================================================
static constexpr int FFT_SIZE = 1024;
static constexpr int NUM_BINS = FFT_SIZE / 2;
static constexpr int SPEC_COLS = 128; // log-merged display columns

struct SpectrumAnalyser {
    std::array<float, FFT_SIZE> ring {};
    int writePos = 0;
    std::mutex ringMtx;

    // [0,1] per column — written by compute(), read by pixel renderer
    std::array<float, SPEC_COLS> columns {};

    inline void push(float s)
    {
        std::lock_guard<std::mutex> lk(ringMtx);
        ring[writePos] = s;
        writePos = (writePos + 1) % FFT_SIZE;
    }

    void compute(double sampleRate)
    {
        std::vector<std::complex<float>> buf(FFT_SIZE);
        {
            std::lock_guard<std::mutex> lk(ringMtx);
            for (int i = 0; i < FFT_SIZE; i++) {
                int idx = (writePos + i) % FFT_SIZE;
                buf[i] = { ring[idx] * hannWindow(i, FFT_SIZE), 0.f };
            }
        }
        fft(buf);

        std::array<float, NUM_BINS> mags;
        for (int i = 0; i < NUM_BINS; i++) {
            float m = std::abs(buf[i]) / (FFT_SIZE / 2);
            mags[i] = 20.f * log10f(m + 1e-9f);
        }

        float fpb = (float)sampleRate / FFT_SIZE;
        for (int c = 0; c < SPEC_COLS; c++) {
            float t0 = (float)c / SPEC_COLS;
            float t1 = (float)(c + 1) / SPEC_COLS;
            int lo = std::max(0, std::min(NUM_BINS - 1, (int)(20.f * powf(1000.f, t0) / fpb)));
            int hi = std::max(lo, std::min(NUM_BINS - 1, (int)(20.f * powf(1000.f, t1) / fpb)));
            float pk = -120.f;
            for (int b = lo; b <= hi; b++)
                pk = std::max(pk, mags[b]);
            columns[c] = std::max(0.f, std::min(1.f, (pk + 80.f) / 80.f));
        }
    }
};

// ================================================================
// BiquadFilter — Audio EQ Cookbook, Direct Form I
// ================================================================
enum EQBandType { EQ_LOW_SHELF,
    EQ_PEAK,
    EQ_HIGH_SHELF };

struct BiquadFilter {
    EQBandType type = EQ_PEAK;
    float freq = 1000.f;
    float gainDb = 0.f;
    float Q = 0.707f;

    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    void compute(double sr)
    {
        double A = pow(10.0, gainDb / 40.0);
        double w0 = 2.0 * M_PI * freq / sr;
        double cosw = cos(w0), sinw = sin(w0);
        double alp = sinw / (2.0 * Q);
        double a0;
        switch (type) {
        case EQ_LOW_SHELF: {
            double ap = A + 1, am = A - 1, sq = 2.0 * sqrt(A) * alp;
            b0 = A * (ap - am * cosw + sq);
            b1 = 2. * A * (am - ap * cosw);
            b2 = A * (ap - am * cosw - sq);
            a0 = ap + am * cosw + sq;
            a1 = -2. * (am + ap * cosw);
            a2 = ap + am * cosw - sq;
            break;
        }
        case EQ_HIGH_SHELF: {
            double ap = A + 1, am = A - 1, sq = 2.0 * sqrt(A) * alp;
            b0 = A * (ap + am * cosw + sq);
            b1 = -2. * A * (am + ap * cosw);
            b2 = A * (ap + am * cosw - sq);
            a0 = ap - am * cosw + sq;
            a1 = 2. * (am - ap * cosw);
            a2 = ap - am * cosw - sq;
            break;
        }
        default: {
            double aA = alp * A, aOA = alp / A;
            b0 = 1. + aA;
            b1 = -2. * cosw;
            b2 = 1. - aA;
            a0 = 1. + aOA;
            a1 = -2. * cosw;
            a2 = 1. - aOA;
            break;
        }
        }
        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;
    }

    inline float process(float in)
    {
        double out = b0 * in + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1;
        x1 = in;
        y2 = y1;
        y1 = out;
        return (float)out;
    }

    float magnitudeAt(float f, double sr) const
    {
        double w = 2. * M_PI * f / sr, cw = cos(w), sw = sin(w);
        double c2 = cos(2. * w), s2 = sin(2. * w);
        double nr = b0 + b1 * cw + b2 * c2, ni = -(b1 * sw + b2 * s2);
        double dr = 1. + a1 * cw + a2 * c2, di = -(a1 * sw + a2 * s2);
        return (float)(10. * log10((nr * nr + ni * ni) / (dr * dr + di * di) + 1e-12));
    }
};

// ================================================================
// EQ — 3 bands with linked crossover frequencies
//
//   Band 0: Low shelf  — centred at crossoverLow
//   Band 1: Peak       — centred at geometric mean of crossovers
//   Band 2: High shelf — centred at crossoverHigh
//
// X-drag rules:
//   Low  dot → moves crossoverLow  only
//   Mid  dot → translates both crossovers (preserving log-ratio)
//   High dot → moves crossoverHigh only
// ================================================================
struct EQ {
    static constexpr int NUM_BANDS = 3;
    static constexpr float GAIN_MIN = -12.f;
    static constexpr float GAIN_MAX = 12.f;
    static constexpr float FREQ_MIN = 30.f;
    static constexpr float FREQ_MAX = 18000.f;
    static constexpr float CROSS_GAP = 80.f; // min Hz between crossovers

    float crossoverLow = 300.f;
    float crossoverHigh = 4000.f;
    float gainDb[NUM_BANDS] = { 0.f, 0.f, 0.f };

    BiquadFilter filters[NUM_BANDS];

    EQ() { _sync(); }

    void _sync()
    {
        float mid = sqrtf(crossoverLow * crossoverHigh);
        filters[0] = { EQ_LOW_SHELF, crossoverLow, gainDb[0], 0.707f };
        filters[1] = { EQ_PEAK, mid, gainDb[1], 1.0f };
        filters[2] = { EQ_HIGH_SHELF, crossoverHigh, gainDb[2], 0.707f };
    }

    void recompute(double sr)
    {
        _sync();
        for (auto& f : filters)
            f.compute(sr);
    }

    inline float process(float in)
    {
        for (auto& f : filters)
            in = f.process(in);
        return in;
    }

    float totalMagnitudeAt(float f, double sr) const
    {
        float t = 0;
        for (const auto& fi : filters)
            t += fi.magnitudeAt(f, sr);
        return t;
    }

    // ---- Coordinate helpers (static, used by both draw and mouse) ----

    static float freqToT(float f)
    {
        return std::max(0.f, std::min(1.f, logf(f / 20.f) / logf(1000.f)));
    }
    static float tToFreq(float t)
    {
        return 20.f * powf(1000.f, std::max(0.f, std::min(1.f, t)));
    }
    static float freqToPx(float f, float cx, float cw)
    {
        return cx + cw * freqToT(f);
    }
    static float pxToFreq(float px, float cx, float cw)
    {
        return tToFreq((px - cx) / cw);
    }
    static float gainToPy(float db, float cy, float ch, float dbRange)
    {
        return cy + ch / 2.f - (db / dbRange) * (ch / 2.f);
    }
    static float pyToGain(float py, float cy, float ch, float dbRange)
    {
        return std::max(GAIN_MIN, std::min(GAIN_MAX, ((cy + ch / 2.f) - py) / (ch / 2.f) * dbRange));
    }

    // Dot pixel position for band b
    std::pair<float, float> dotPos(int b, float cx, float cy, float cw, float ch, float dbRange) const
    {
        float f;
        if (b == 0) f = crossoverLow;
        else if (b == 2) f = crossoverHigh;
        else f = sqrtf(crossoverLow * crossoverHigh);
        return { freqToPx(f, cx, cw), gainToPy(gainDb[b], cy, ch, dbRange) };
    }

    // Apply drag — handles linked crossover logic then recomputes
    void applyDrag(int b, float px, float py,
        float cx, float cy, float cw, float ch, float dbRange, double sr)
    {
        gainDb[b] = pyToGain(py, cy, ch, dbRange);
        float newF = pxToFreq(px, cx, cw);

        if (b == 0) {
            crossoverLow = std::max(FREQ_MIN, std::min(crossoverHigh - CROSS_GAP, newF));
        } else if (b == 2) {
            crossoverHigh = std::max(crossoverLow + CROSS_GAP, std::min(FREQ_MAX, newF));
        } else { // mid: translate both preserving log-ratio
            float mid = sqrtf(crossoverLow * crossoverHigh);
            float ratio = newF / mid;
            float nLow = crossoverLow * ratio;
            float nHigh = crossoverHigh * ratio;
            if (nLow >= FREQ_MIN && nHigh <= FREQ_MAX) {
                crossoverLow = nLow;
                crossoverHigh = nHigh;
            }
        }
        recompute(sr);
    }

    // Polyline for drawing the combined response curve
    std::vector<std::pair<float, float>> curvePoints(
        float cx, float cy, float cw, float ch, float dbRange, double sr, int n = 250) const
    {
        std::vector<std::pair<float, float>> pts;
        pts.reserve(n);
        for (int i = 0; i < n; i++) {
            float t = (float)i / (n - 1);
            float f = 20.f * powf(1000.f, t);
            float db = std::max(-dbRange, std::min(dbRange, totalMagnitudeAt(f, sr)));
            pts.push_back({ cx + cw * t, cy + ch / 2.f - (db / dbRange) * (ch / 2.f) });
        }
        return pts;
    }
};