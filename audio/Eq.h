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
        float ang = -2.0f * (float)M_PI / len;
        std::complex<float> wlen(cosf(ang), sinf(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1.f, 0.f);
            for (int j = 0; j < len / 2; j++) {
                std::complex<float> u = x[i + j];
                std::complex<float> v = x[i + j + len / 2] * w;
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
// Ring buffer written by audio thread, compute() called by UI.
// ================================================================
static constexpr int FFT_SIZE = 1024;
static constexpr int NUM_BINS = FFT_SIZE / 2;
static constexpr int SPEC_COLS = 120; // log-merged display columns

struct SpectrumAnalyser {
    std::array<float, FFT_SIZE> ring {};
    int writePos = 0;
    std::mutex ringMtx;

    // Output: SPEC_COLS values in [0,1] — written by compute(), read by renderer
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

        float fpb = (float)sampleRate / FFT_SIZE; // Hz per bin
        for (int c = 0; c < SPEC_COLS; c++) {
            float t0 = (float)c / SPEC_COLS;
            float t1 = (float)(c + 1) / SPEC_COLS;
            int lo = std::max(0, std::min(NUM_BINS - 1, (int)(20.f * powf(1000.f, t0) / fpb)));
            int hi = std::max(lo, std::min(NUM_BINS - 1, (int)(20.f * powf(1000.f, t1) / fpb)));
            float pk = -120.f;
            for (int b = lo; b <= hi; b++)
                pk = std::max(pk, mags[b]);
            // -80 dB -> 0.0,  0 dB -> 1.0
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
        double alpha = sinw / (2.0 * Q);
        double a0;
        switch (type) {
        case EQ_LOW_SHELF: {
            double ap = A + 1, am = A - 1, sqA = 2.0 * sqrt(A) * alpha;
            b0 = A * (ap - am * cosw + sqA);
            b1 = 2.0 * A * (am - ap * cosw);
            b2 = A * (ap - am * cosw - sqA);
            a0 = ap + am * cosw + sqA;
            a1 = -2.0 * (am + ap * cosw);
            a2 = ap + am * cosw - sqA;
            break;
        }
        case EQ_HIGH_SHELF: {
            double ap = A + 1, am = A - 1, sqA = 2.0 * sqrt(A) * alpha;
            b0 = A * (ap + am * cosw + sqA);
            b1 = -2.0 * A * (am + ap * cosw);
            b2 = A * (ap + am * cosw - sqA);
            a0 = ap - am * cosw + sqA;
            a1 = 2.0 * (am - ap * cosw);
            a2 = ap - am * cosw - sqA;
            break;
        }
        default: {
            double aA = alpha * A, aOA = alpha / A;
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
        double w = 2. * M_PI * f / sr, cosw = cos(w), sinw = sin(w);
        double c2 = cos(2. * w), s2 = sin(2. * w);
        double nr = b0 + b1 * cosw + b2 * c2, ni = -(b1 * sinw + b2 * s2);
        double dr = 1. + a1 * cosw + a2 * c2, di = -(a1 * sinw + a2 * s2);
        return (float)(10. * log10((nr * nr + ni * ni) / (dr * dr + di * di) + 1e-12));
    }
};

// ================================================================
// EQ — 3 bands with linked crossover frequencies
//
// Band layout:
//   Band 0: Low shelf   — freq = crossoverLow  (shared with band 1)
//   Band 1: Peak (mid)  — freq = midpoint of [crossoverLow, crossoverHigh]
//   Band 2: High shelf  — freq = crossoverHigh (shared with band 1)
//
// Crossover rules on X-drag:
//   Drag Low  dot X  -> move crossoverLow  (Low shelf + Mid peak left edge)
//   Drag Mid  dot X  -> move both crossovers symmetrically (narrows/widens mid)
//   Drag High dot X  -> move crossoverHigh (High shelf + Mid peak right edge)
//
// All Y-drags are independent gain only.
// ================================================================
struct EQ {
    static constexpr int NUM_BANDS = 3;
    static constexpr float GAIN_MIN = -12.f;
    static constexpr float GAIN_MAX = 12.f;

    // The two crossover frequencies that link the bands
    float crossoverLow = 300.f; // Hz — boundary between Low and Mid
    float crossoverHigh = 4000.f; // Hz — boundary between Mid and High

    // Gain per band (independent)
    float gainDb[NUM_BANDS] = { 0.f, 0.f, 0.f };

    // Internal biquad filters
    BiquadFilter filters[NUM_BANDS];

    static constexpr float FREQ_MIN = 30.f;
    static constexpr float FREQ_MAX = 18000.f;
    static constexpr float CROSS_GAP = 100.f; // min Hz between crossovers

    EQ() { _syncFilters(); }

    // Rebuild filter parameters from crossovers + gains
    void _syncFilters()
    {
        float midFreq = sqrtf(crossoverLow * crossoverHigh); // geometric mean

        filters[0].type = EQ_LOW_SHELF;
        filters[0].freq = crossoverLow;
        filters[0].gainDb = gainDb[0];
        filters[0].Q = 0.707f;

        filters[1].type = EQ_PEAK;
        filters[1].freq = midFreq;
        filters[1].gainDb = gainDb[1];
        filters[1].Q = 1.0f;

        filters[2].type = EQ_HIGH_SHELF;
        filters[2].freq = crossoverHigh;
        filters[2].gainDb = gainDb[2];
        filters[2].Q = 0.707f;
    }

    void recompute(double sr)
    {
        _syncFilters();
        for (auto& f : filters)
            f.compute(sr);
    }

    inline float process(float in)
    {
        for (auto& f : filters)
            in = f.process(in);
        return in;
    }

    // Combined magnitude response at f Hz
    float totalMagnitudeAt(float f, double sr) const
    {
        float t = 0;
        for (const auto& fi : filters)
            t += fi.magnitudeAt(f, sr);
        return t;
    }

    // ---- Coordinate helpers ----------------------------------------

    // Map a frequency to a normalised [0,1] X position (log scale 20–20k)
    static float freqToT(float f)
    {
        return std::max(0.f, std::min(1.f, logf(f / 20.f) / logf(1000.f)));
    }

    // Map normalised X [0,1] to frequency
    static float tToFreq(float t)
    {
        return 20.f * powf(1000.f, std::max(0.f, std::min(1.f, t)));
    }

    // Pixel X -> frequency
    static float pxToFreq(float px, int cx, int cw)
    {
        return tToFreq((px - cx) / (float)cw);
    }

    // Frequency -> pixel X
    static float freqToPx(float f, int cx, int cw)
    {
        return cx + cw * freqToT(f);
    }

    // Gain -> pixel Y
    static float gainToPy(float db, int cy, int ch, float dbRange)
    {
        return cy + ch / 2.f - (db / dbRange) * (ch / 2.f);
    }

    // Pixel Y -> gain
    static float pyToGain(float py, int cy, int ch, float dbRange)
    {
        return std::max(GAIN_MIN, std::min(GAIN_MAX, ((cy + ch / 2.f) - py) / (ch / 2.f) * dbRange));
    }

    // Returns (px, py) for each band's control dot
    std::pair<float, float> dotPos(int band, int cx, int cy, int cw, int ch, float dbRange) const
    {
        float f;
        switch (band) {
        case 0:
            f = crossoverLow;
            break;
        case 2:
            f = crossoverHigh;
            break;
        default:
            f = sqrtf(crossoverLow * crossoverHigh);
            break; // mid: geometric mean
        }
        return { freqToPx(f, cx, cw), gainToPy(gainDb[band], cy, ch, dbRange) };
    }

    // Apply a drag to band `band` given new pixel position (px, py)
    // Handles linked crossover logic
    void applyDrag(int band, float px, float py, int cx, int cy, int cw, int ch, float dbRange, double sr)
    {
        float newFreq = pxToFreq(px, cx, cw);
        gainDb[band] = pyToGain(py, cy, ch, dbRange);

        switch (band) {
        case 0: // Low dot — move crossoverLow, keep crossoverHigh
            crossoverLow = std::max(FREQ_MIN, std::min(crossoverHigh - CROSS_GAP, newFreq));
            break;
        case 2: // High dot — move crossoverHigh, keep crossoverLow
            crossoverHigh = std::max(crossoverLow + CROSS_GAP, std::min(FREQ_MAX, newFreq));
            break;
        case 1: { // Mid dot — move both crossovers keeping the same log-ratio (widen/narrow symmetrically)
            float midFreq = sqrtf(crossoverLow * crossoverHigh);
            float ratio = newFreq / midFreq; // scale factor
            float newLow = crossoverLow * ratio;
            float newHigh = crossoverHigh * ratio;
            // Clamp so we don't go out of range
            if (newLow >= FREQ_MIN && newHigh <= FREQ_MAX) {
                crossoverLow = newLow;
                crossoverHigh = newHigh;
            }
            break;
        }
        }
        recompute(sr);
    }

    // Curve polyline for drawing
    std::vector<std::pair<float, float>> curvePoints(
        int cx, int cy, int cw, int ch, float dbRange, double sr, int n = 250) const
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