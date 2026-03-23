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
// Simple radix-2 Cooley-Tukey FFT (in-place, power-of-2 size)
// ================================================================
static void fft(std::vector<std::complex<float>>& x)
{
    int n = (int)x.size();
    if (n <= 1) return;

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }

    // Butterfly stages
    for (int len = 2; len <= n; len <<= 1) {
        float ang = -2.0f * (float)M_PI / len;
        std::complex<float> wlen(cosf(ang), sinf(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<float> w(1.0f, 0.0f);
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
    return 0.5f * (1.0f - cosf(2.0f * (float)M_PI * i / (n - 1)));
}

// ================================================================
// SpectrumAnalyser
// Ring buffer filled by the audio thread (post-EQ samples).
// compute() called by the UI thread to produce log-merged columns.
// ================================================================
static constexpr int FFT_SIZE = 1024; // must be power of 2
static constexpr int NUM_BINS = FFT_SIZE / 2;
static constexpr int SPEC_COLS = 120; // display columns

struct SpectrumAnalyser {
    std::array<float, FFT_SIZE> ring {};
    int writePos = 0;
    std::mutex ringMtx;

    // Output: SPEC_COLS values in [0, 1], ready to draw
    std::array<float, SPEC_COLS> columns {};

    // Push one post-EQ sample — called from audio thread
    inline void push(float s)
    {
        std::lock_guard<std::mutex> lk(ringMtx);
        ring[writePos] = s;
        writePos = (writePos + 1) % FFT_SIZE;
    }

    // Run FFT and fill columns[] — called from UI thread (not hot path)
    void compute(double sampleRate)
    {
        std::vector<std::complex<float>> buf(FFT_SIZE);
        {
            std::lock_guard<std::mutex> lk(ringMtx);
            for (int i = 0; i < FFT_SIZE; i++) {
                int idx = (writePos + i) % FFT_SIZE;
                float w = hannWindow(i, FFT_SIZE);
                buf[i] = { ring[idx] * w, 0.0f };
            }
        }

        fft(buf);

        // Magnitude in dB
        std::array<float, NUM_BINS> mags;
        for (int i = 0; i < NUM_BINS; i++) {
            float mag = std::abs(buf[i]) / (FFT_SIZE / 2);
            mags[i] = 20.0f * log10f(mag + 1e-9f);
        }

        // Merge into log-spaced display columns (20 Hz – 20 kHz)
        float freqPerBin = (float)sampleRate / FFT_SIZE;
        for (int c = 0; c < SPEC_COLS; c++) {
            float t0 = (float)c / SPEC_COLS;
            float t1 = (float)(c + 1) / SPEC_COLS;
            int binLo = std::max(0, std::min(NUM_BINS - 1, (int)(20.0f * powf(1000.0f, t0) / freqPerBin)));
            int binHi = std::max(binLo, std::min(NUM_BINS - 1, (int)(20.0f * powf(1000.0f, t1) / freqPerBin)));

            float peak = -120.0f;
            for (int b = binLo; b <= binHi; b++)
                peak = std::max(peak, mags[b]);

            // Map -80 dB..0 dB -> 0..1
            columns[c] = std::max(0.0f, std::min(1.0f, (peak + 80.0f) / 80.0f));
        }
    }
};

// ================================================================
// EQ band types
// ================================================================
enum EQBandType { EQ_LOW_SHELF,
    EQ_PEAK,
    EQ_HIGH_SHELF };

// ================================================================
// BiquadFilter — Audio EQ Cookbook, Direct Form I
// ================================================================
struct BiquadFilter {
    EQBandType type = EQ_PEAK;
    float freq = 1000.0f;
    float gainDb = 0.0f;
    float Q = 0.707f;

    double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    double x1 = 0, x2 = 0, y1 = 0, y2 = 0;

    void compute(double sampleRate)
    {
        double A = pow(10.0, gainDb / 40.0);
        double w0 = 2.0 * M_PI * freq / sampleRate;
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
        default: { // EQ_PEAK
            double aA = alpha * A, aOA = alpha / A;
            b0 = 1.0 + aA;
            b1 = -2.0 * cosw;
            b2 = 1.0 - aA;
            a0 = 1.0 + aOA;
            a1 = -2.0 * cosw;
            a2 = 1.0 - aOA;
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

    float magnitudeAt(float f, double sampleRate) const
    {
        double w = 2.0 * M_PI * f / sampleRate;
        double cosw = cos(w), sinw = sin(w);
        double cos2w = cos(2.0 * w), sin2w = sin(2.0 * w);
        double nr = b0 + b1 * cosw + b2 * cos2w;
        double ni = -(b1 * sinw + b2 * sin2w);
        double dr = 1.0 + a1 * cosw + a2 * cos2w;
        double di = -(a1 * sinw + a2 * sin2w);
        return (float)(10.0 * log10((nr * nr + ni * ni) / (dr * dr + di * di) + 1e-12));
    }
};

// ================================================================
// 5-band parametric EQ
// ================================================================
struct EQ {
    static constexpr int NUM_BANDS = 5;
    static constexpr float SHELF_Q = 0.707f;
    static constexpr float PEAK_Q = 1.41f;
    static constexpr float GAIN_MIN = -12.0f;
    static constexpr float GAIN_MAX = 12.0f;

    std::array<BiquadFilter, NUM_BANDS> bands;
    const float freqMin[NUM_BANDS] = { 20.f, 80.f, 200.f, 1000.f, 4000.f };
    const float freqMax[NUM_BANDS] = { 200.f, 800.f, 5000.f, 12000.f, 20000.f };

    EQ()
    {
        bands[0] = { EQ_LOW_SHELF, 80.0f, 0.f, SHELF_Q };
        bands[1] = { EQ_PEAK, 250.0f, 0.f, PEAK_Q };
        bands[2] = { EQ_PEAK, 1000.0f, 0.f, PEAK_Q };
        bands[3] = { EQ_PEAK, 4000.0f, 0.f, PEAK_Q };
        bands[4] = { EQ_HIGH_SHELF, 12000.f, 0.f, SHELF_Q };
    }

    void recompute(double sr)
    {
        for (auto& b : bands)
            b.compute(sr);
    }

    inline float process(float in)
    {
        for (auto& b : bands)
            in = b.process(in);
        return in;
    }

    float totalMagnitudeAt(float f, double sr) const
    {
        float t = 0;
        for (const auto& b : bands)
            t += b.magnitudeAt(f, sr);
        return t;
    }

    std::vector<std::pair<float, float>> curvePoints(
        int cx, int cy, int cw, int ch,
        float dbRange, double sr, int n = 200) const
    {
        std::vector<std::pair<float, float>> pts;
        pts.reserve(n);
        for (int i = 0; i < n; i++) {
            float t = (float)i / (n - 1);
            float f = 20.0f * powf(1000.f, t);
            float db = std::max(-dbRange, std::min(dbRange, totalMagnitudeAt(f, sr)));
            pts.push_back({ cx + cw * t, cy + ch / 2.0f - (db / dbRange) * (ch / 2.0f) });
        }
        return pts;
    }

    std::pair<float, float> bandPointPos(
        int bi, int cx, int cy, int cw, int ch, float dbRange) const
    {
        float t = std::max(0.f, std::min(1.f, logf(bands[bi].freq / 20.f) / logf(1000.f)));
        return { cx + cw * t, cy + ch / 2.0f - (bands[bi].gainDb / dbRange) * (ch / 2.0f) };
    }

    void setBandFreqFromX(int bi, float px, int cx, int cw)
    {
        float t = std::max(0.f, std::min(1.f, (px - cx) / (float)cw));
        float f = std::max(freqMin[bi], std::min(freqMax[bi], 20.f * powf(1000.f, t)));
        bands[bi].freq = f;
    }

    void setBandGainFromY(int bi, float py, int cy, int ch, float dbRange)
    {
        float g = std::max(GAIN_MIN, std::min(GAIN_MAX, ((cy + ch / 2.0f) - py) / (ch / 2.0f) * dbRange));
        bands[bi].gainDb = g;
    }
};