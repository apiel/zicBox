#pragma once

/**
 * Kick32v4 — BODY > MOTION > ONE DRIVE > FILTERBANK. 32 pots, no menu, no switch: every choice is a morph.
 *
 * Built for tekno / hardtek / metal tek / hardcore / tribe. One voice only: everything bends, modulates, folds or
 * filters the same sound. No second layers.
 *
 *   BODY   (12) decay / shape, pitch drop (amount, time, curve), wave morph + harmonics 2 / 3,
 *               skew, hard sync, resonator, fold                                     (unchanged from v3)
 *   MOTION (4)  bend (the tail slides up or down, bipolar), FM depth + ratio (metallic attack),
 *               shift (a frequency shifter that sweeps down with the pitch drop: laser -> metal, bipolar)
 *   DRIVE  (8)  drive, character (soft -> hard -> fold -> fuzz), punch (drive envelope, bipolar),
 *               feedback (comb tuned to the pitch, bipolar), click (level, tone: knock -> noise tick),
 *               noise (level, color). The saturation runs 2x oversampled: less fizz at high gain.
 *   FILTER (8)  cutoff (LP), cutoff 2 (HP), reso, env, env decay, FM, filter drive, vowel (two formants)
 *
 *   BODY (bend + drop pitch, FM, wave, skew, sync, resonator, fold) + CLICK + NOISE --> [+ FEEDBACK comb] --> DRIVE 2x
 *     --> SHIFT --> VOWEL --> LP > HP (reso, env, FM, tracking the pitch) --> FILTER DRIVE --> clip --> OUT
 *
 * Param index == pot index. No volume (fixed level + soft clip), no tune (pitch = note from the sequencer,
 * note 60 = ROOT_HZ), no velocity.
 *
 * No dependency on any UI or desktop API: runs as is on the Daisy Seed / STM32H7.
 * Per sample: no std::exp / pow / tan (fast approximations), pot dependent coefficients at control rate.
 * Memory: a 4096 sample comb (16 KB) + a few hundred bytes of state.
 */

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Kick32v4 : public EngineBase<Kick32v4> {
public:
    static constexpr int PARAM_COUNT = 32;
    static constexpr float ROOT_HZ = 52.0f; // frequency of note 60

protected:
    static constexpr float TWO_PI = 6.28318530718f;
    static constexpr int CTL_RATE = 32; // samples between two control rate updates
    static constexpr int COMB_MAX = 4096; // one period of the lowest pitch (note 36 = 13 Hz) fits

    const float sampleRate;
    const float invSr;

    // ------------------------------------------------------------------ fast math
    static float lerp(float a, float b, float t) { return a + t * (b - a); }
    static float clamp01(float x) { return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x); }

    static float fastExp2(float x)
    {
        x = std::clamp(x, -30.0f, 30.0f);
        float fi = std::floor(x);
        float f = x - fi;
        float p = 1.0f + f * (0.69606564f + f * (0.22449433f + f * 0.07944023f));
        union { float f; int32_t i; } u { p };
        u.i += (int32_t)fi << 23;
        return u.f;
    }

    // sin(2 pi p), any p
    static float sin2pi(float p)
    {
        p -= std::floor(p);
        float x = (p < 0.5f ? p : p - 1.0f) * TWO_PI;
        float y = 1.27323954f * x - 0.40528473f * x * std::abs(x);
        return 0.225f * (y * std::abs(y) - y) + y;
    }

    static float tri(float p) { return p < 0.25f ? 4.0f * p : (p < 0.75f ? 2.0f - 4.0f * p : 4.0f * p - 4.0f); }
    static float fastTan(float x) { float x2 = x * x; return x * (15.0f - x2) / (15.0f - 6.0f * x2); }
    static float tanh(float x) { return Math::fastTanh(x); }
    float gOf(float hz) const { return fastTan((float)M_PI * std::clamp(hz, 15.0f, sampleRate * 0.42f) * invSr); }
    // trigger / control rate only
    float tauCoef(float ms) const { return std::exp(-1.0f / (sampleRate * std::max(ms, 0.05f) * 0.001f)); }
    float decayCoef(float ms) const { return std::exp(-6.9078f / (sampleRate * std::max(ms, 0.1f) * 0.001f)); } // -60 dB after ms

    uint32_t rng = 22222;
    float white()
    {
        rng = rng * 196314165 + 907633389;
        return (float)int32_t(rng) * (1.0f / 2147483648.0f);
    }

    // Zero delay feedback SVF (Cytomic). The band state is soft clipped: full resonance growls, never explodes.
    struct Svf {
        float ic1 = 0, ic2 = 0;
        void tick(float in, float g, float k, float& lp, float& bp, float& hp)
        {
            float a1 = 1.0f / (1.0f + g * (g + k));
            float a2 = g * a1, a3 = g * a2;
            float v3 = in - ic2;
            float v1 = a1 * ic1 + a2 * v3;
            float v2 = ic2 + a2 * ic1 + a3 * v3;
            ic1 = 2.0f * v1 - ic1;
            ic2 = 2.0f * v2 - ic2;
            ic1 = 4.0f * Math::fastTanh(ic1 * 0.25f);
            lp = v2;
            bp = v1;
            hp = in - k * v1 - v2;
        }
    };

    // Hilbert transformer (Olli Niemitalo): two cascades of four allpasses in z^-2, the first path delayed a sample.
    // I and Q are 90 degrees apart from ~30 Hz to ~20 kHz (|I + jQ| flat within 1 %): all a frequency shifter needs.
    struct Hilbert {
        float xa[4][2] = {}, ya[4][2] = {}, xb[4][2] = {}, yb[4][2] = {}; // z^-1, z^-2 per section
        float aDelayed = 0.0f;
        static float section(float in, float c, float* x, float* y)
        {
            float out = c * (in + y[1]) - x[1];
            x[1] = x[0]; x[0] = in;
            y[1] = y[0]; y[0] = out;
            return out;
        }
        void tick(float in, float& I, float& Q)
        {
            static const float A[4] = { 0.47940087f, 0.87621845f, 0.97659759f, 0.99749925f }; // a^2
            static const float B[4] = { 0.16175850f, 0.73302893f, 0.94534970f, 0.99059916f };
            float a = in, b = in;
            for (int i = 0; i < 4; ++i) { a = section(a, A[i], xa[i], ya[i]); b = section(b, B[i], xb[i], yb[i]); }
            I = aDelayed;
            aDelayed = a;
            Q = b;
        }
    };

    // ------------------------------------------------------------------ voice state
    float noteOct = 0.0f;
    float phase = 0.0f, clickPhase = 0.0f;
    int hitPos = 0, ampPos = 0, ampLen = 1, dropPos = 0, dropLen = 1;
    float ampEnv = 0.0f, dropEnv = 0.0f, bendEnv = 0.0f;
    float clickEnv = 0.0f, clickCoef = 0.0f;
    float noiseEnv = 0.0f, noiseCoef = 0.0f;
    float punchEnv = 0.0f, punchCoef = 0.0f;
    float filtEnv = 0.0f, filtCoef = 0.0f;
    float ringGate = 0.0f, ringCoef = 0.0f;
    Svf noiseSvf;

    // body extras: hard sync slave, resonator, FM
    float slavePhase = 0.0f;
    Svf resSvf;
    float modPhase = 0.0f, fmEnv = 0.0f, fmCoef = 0.0f;

    // drive (2x oversampled saturation: the input is interpolated, the output decimated by a 7 tap half band)
    float comb[COMB_MAX] = {};
    int combPos = 0;
    float fbLp = 0.0f;
    float prePrev = 0.0f;
    float osHist[7] = {};
    float satDcIn = 0.0f, satDcOut = 0.0f;

    // shift: Hilbert + quadrature oscillator
    Hilbert hilbert;
    float shiftPhase = 0.0f;

    // vowel: two formant band passes
    Svf vowelSvf[2];

    // filterbank
    Svf f1, f2;
    float dcIn = 0.0f, dcOut = 0.0f;

    // ------------------------------------------------------------------ control rate cache (pot dependent)
    int ctl = 0;
    float cDropOct = 0, cWave = 0, cH2 = 0, cH3 = 0;
    float cSkew = 0.5f, cSync = 0, cRes = 0, cFold = 0, cWaveGain = 1;
    float cBend = 0, cBodyFm = 0, cFmRatio = 1, cShift = 0, cShiftMix = 0;
    float cVowelMix = 0, cVowelK = 0.25f, cVG1 = 0.1f, cVG2 = 0.2f, cVF1 = 700, cVF2 = 1100;
    float cGain = 1, cChar = 0, cCharGain = 1, cPunch = 0, cFeedback = 0;
    float cClick = 0, cClickHz = 300, cClickNoise = 0, cNoise = 0, cNoiseG = 0.1f, cNoiseK = 0.7f, cNoiseLp = 1, cNoiseBp = 0, cNoiseHp = 0;
    float cCut1 = 20000, cCut2 = 15, cK = 1, cEnv = 0, cFm = 0, cFiltDrive = 1, cResoComp = 1;

    void controlRate()
    {
        float d = pct(drop);
        cDropOct = 6.0f * d * std::sqrt(d); // 0 .. 6 oct
        cWave = pct(wave);
        cWaveGain = waveGainOf(cWave);
        cH2 = harm2.value * 0.01f;
        cH3 = harm3.value * 0.01f;

        cSkew = 0.5f + skew.value * 0.0045f; // -100 .. 100 -> 0.05 .. 0.95
        cSync = pct(sync);
        cRes = pct(resonator);
        cFold = pct(fold);

        // MOTION
        float b = bend.value * 0.01f;
        cBend = b * std::abs(b) * 2.0f; // up to +-2 octaves at the end of the decay
        cBodyFm = pct(fmDepth);
        cFmRatio = 0.5f * fastExp2(pct(fmRatio) * 4.0f); // 0.5 .. 8x, in between = metal
        float s = shift.value * 0.01f;
        cShift = s * std::abs(s) * 1000.0f; // +- 1000 Hz while the drop is at its top, a quarter of it on the tail
        cShiftMix = std::min(1.0f, std::abs(s) * 4.0f);

        // VOWEL: two formants (a e i o u). The first fifth of the pot brings them in, the rest moves the vowel.
        static const float F[5][2] = { { 730, 1090 }, { 530, 1840 }, { 270, 2290 }, { 570, 840 }, { 300, 870 } };
        float vp = pct(vowel) * 3.999f;
        int vi = (int)vp;
        float vf = vp - vi;
        cVF1 = lerp(F[vi][0], F[vi + 1][0], vf);
        cVF2 = lerp(F[vi][1], F[vi + 1][1], vf);
        cVG1 = gOf(cVF1);
        cVG2 = gOf(cVF2);
        cVowelMix = std::min(1.0f, pct(vowel) * 5.0f);

        float dv = pct(drive);
        cGain = fastExp2(dv * 7.0f); // 1 .. 128
        cChar = pct(character) * 3.0f;
        cCharGain = charGainOf(pct(character));
        cPunch = punch.value * 0.01f;
        cFeedback = feedback.value * 0.01f;

        cClick = pct(click);
        float ct = pct(clickTone);
        cClickHz = 300.0f * fastExp2(ct * 4.4f); // 300 Hz .. 6.3 kHz
        cClickNoise = ct * ct; // the tick end of the pot is a noise tick
        cNoise = pct(noise);
        float color = pct(noiseColor);
        cNoiseG = gOf(100.0f * fastExp2(color * 6.6f)); // 100 Hz .. 10 kHz
        cNoiseLp = clamp01(1.0f - color * 2.0f);
        cNoiseHp = clamp01(color * 2.0f - 1.0f);
        cNoiseBp = 1.0f - std::abs(color * 2.0f - 1.0f);

        cCut1 = 30.0f * fastExp2(pct(cutoff) * 9.4f); // 30 Hz .. 20 kHz
        cCut2 = 15.0f * fastExp2(pct(cutoff2) * 9.0f); // 15 Hz .. 7.7 kHz
        float r = pct(reso);
        cK = lerp(1.5f, 0.02f, std::sqrt(r));
        cResoComp = 0.6f + 0.4f * std::sqrt(cK / 1.5f);
        cEnv = env.value * 0.01f;
        float fmv = 0.5f * pct(filterFm); // the pot spans the useful half of the range
        cFm = (0.5f * fmv + 0.5f * fmv * fmv) * 7.0f;
        float fd = pct(filterDrive);
        cFiltDrive = 1.0f + fd * fd * 24.0f; // 1 .. 25x after the filter
    }

    // ------------------------------------------------------------------ shapes
    // Shape 0 = punchy exponential, 50 = linear, 100 = gated (holds, then rounds off)
    float ampShape(float t)
    {
        float h = pct(shape);
        if (h < 0.5f) {
            float c = 1.0f - t;
            float c2 = c * c;
            return lerp(c2 * c2, c, h * 2.0f);
        }
        float H = (h - 0.5f) * 2.0f;
        float holdPart = H * 0.85f;
        if (t < holdPart) return 1.0f;
        float s = (t - holdPart) / (1.0f - holdPart);
        return lerp(1.0f - s, 1.0f - s * s, H);
    }

    // Drop envelope 0..1 over Drop Time. Curve 0 = laser (holds, then dives), 50 = linear, 100 = zap (dives at once).
    float dropShape(float t)
    {
        if (t >= 1.0f) return 0.0f;
        float c = 1.0f - t, cv = pct(dropCurve);
        float laser = 1.0f - t * t * t;
        float c2 = c * c;
        float zap = c2 * c2 * c2;
        return cv < 0.5f ? lerp(laser, c, cv * 2.0f) : lerp(c, zap, (cv - 0.5f) * 2.0f);
    }

    // Bend envelope 0..1 over the decay: moves early (while the kick is still loud), then flattens
    static float bendShape(float t) { float c = 1.0f - clamp01(t); return 1.0f - c * c; }

    float dropMsOf() { return 4.0f * fastExp2(pct(dropTime) * 7.0f); } // 4 .. 512 ms

    // The morph sine -> triangle -> square -> saw has a loudness hump (rms 0.59 -> 0.95 -> 0.58): the drive would
    // hit much harder around 65 %. Table of 1 / rms measured every 5 %, scaled to a flat level.
    static float waveGainOf(float x)
    {
        static const float rms[21] = { 0.71f, 0.69f, 0.67f, 0.65f, 0.63f, 0.61f, 0.59f, 0.59f, 0.64f, 0.69f, 0.75f,
            0.82f, 0.88f, 0.95f, 0.92f, 0.85f, 0.79f, 0.73f, 0.67f, 0.62f, 0.58f };
        float f = x * 20.0f;
        int i = std::min(19, (int)f);
        return 0.71f / lerp(rms[i], rms[i + 1], f - i);
    }

    // Character morph level trim (soft -> hard -> fold -> fuzz): measured offline, keeps the loudness flat
    static float charGainOf(float x)
    {
        static const float g[11] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.04f, 1.12f, 1.09f, 1.07f, 1.22f, 1.36f, 1.34f };
        float f = x * 10.0f;
        int i = std::min(9, (int)f);
        return lerp(g[i], g[i + 1], f - i);
    }

    // Body wave: skew (phase warp), morph sine -> triangle -> square -> saw, harmonics 2 / 3 (bipolar, strong), fold.
    float bodyWave(float p)
    {
        p -= std::floor(p);
        if (cSkew != 0.5f) p = p < cSkew ? 0.5f * (p / cSkew) : 0.5f + 0.5f * ((p - cSkew) / (1.0f - cSkew));
        float s = sin2pi(p);
        float x = cWave * 3.0f;
        float w;
        if (x < 1.0f) {
            w = lerp(s, tri(p), x);
        } else {
            float sq = tanh(s * 10.0f);
            if (x < 2.0f) w = lerp(tri(p), sq, x - 1.0f);
            else w = lerp(sq, 2.0f * (p < 0.5f ? p : p - 1.0f), x - 2.0f);
        }
        w *= cWaveGain;
        if (cH2 != 0.0f) w += cH2 * 1.2f * sin2pi(2.0f * p);
        if (cH3 != 0.0f) w += cH3 * 1.2f * sin2pi(3.0f * p);
        if (cFold > 0.0f) { // fold 0 = identity, then the wave folds over on itself
            float folded = sin2pi(0.25f * w * (1.0f + 5.0f * cFold));
            w = lerp(w, folded, std::min(1.0f, cFold * 5.0f));
        }
        return w;
    }

    // The one saturation. x is already gained. Character: soft -> hard -> fold -> fuzz (octave up)
    float saturate(float x, float ch)
    {
        float soft = tanh(x);
        float hard = std::clamp(x, -1.0f, 1.0f);
        if (ch <= 1.0f) return lerp(soft, hard, ch);
        float fold = sin2pi(1.25f * tanh(x * 0.2f)); // 2.5 folds whatever the drive, and a full level plateau when it clips (v3 folded to zero there)
        if (ch <= 2.0f) return lerp(hard, fold, ch - 1.0f);
        float fuzz = 0.5f * (tanh(std::abs(x) * 2.0f) * 2.0f - 1.0f) + 0.5f * hard; // rectified (octave up) + the clip: the fundamental never disappears
        return lerp(fold, fuzz, ch - 2.0f);
    }

    // 2x oversampled saturation: two shaped samples (the midpoint and the sample itself) pushed through a 7 tap
    // half band filter (-1 0 9 16 9 0 -1 / 32), read at the 1x rate. Less aliasing fizz at high gain.
    float saturate2x(float pre)
    {
        float mid = 0.5f * (prePrev + pre);
        prePrev = pre;
        float* h = osHist; // newest first, at the 2x rate: cur, mid, cur-1, mid-1, cur-2, mid-2, cur-3
        h[6] = h[4]; h[5] = h[3]; h[4] = h[2]; h[3] = h[1]; h[2] = h[0];
        h[1] = saturate(mid, cChar);
        h[0] = saturate(pre, cChar);
        return (16.0f * h[3] + 9.0f * (h[2] + h[4]) - (h[0] + h[6])) * (1.0f / 32.0f); // 1.5 samples late
    }

    float combRead(float len)
    {
        float rp = (float)combPos - len;
        if (rp < 0.0f) rp += COMB_MAX;
        int i0 = (int)rp;
        float fr = rp - i0;
        int i1 = i0 + 1;
        if (i1 >= COMB_MAX) i1 = 0;
        return lerp(comb[i0], comb[i1], fr);
    }

public:
    // UI helpers (fast attack, ~15 ms release)
    float meterOut = 0.0f;
    float lastOut = 0.0f;
    float liveFilterOct = 0.0f; // the live offset of both cutoffs (env + FM + pitch), octaves
    float liveShiftHz = 0.0f;
    float liveDriveGain = 1.0f;
    float livePre = 0.0f, livePost = 0.0f; // in / out of the saturation, for the transfer curve dot

    Param params[PARAM_COUNT];

    // ================================================================ BODY (pots 0..11)
    Param& decay = addParam({ .key = "decay", .label = "Decay", .unit = "ms", .value = 350.0f, .min = 40.0f, .max = 2000.0f, .step = 10.0f });
    Param& shape = addParam({ .key = "shape", .label = "Shape", .unit = "%", .value = 30.0f }); // punchy -> linear -> gated
    Param& drop = addParam({ .key = "drop", .label = "Drop", .unit = "%", .value = 55.0f }); // 0 .. 6 oct
    Param& dropTime = addParam({ .key = "dropTime", .label = "Drop Time", .unit = "%", .value = 42.0f }); // 4 .. 512 ms
    Param& dropCurve = addParam({ .key = "dropCurve", .label = "Drop Curve", .unit = "%", .value = 70.0f }); // laser -> linear -> zap
    Param& wave = addParam({ .key = "wave", .label = "Wave", .unit = "%" }); // sine -> tri -> square -> saw
    Param& harm2 = addParam({ .key = "harm2", .label = "Harm 2", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& harm3 = addParam({ .key = "harm3", .label = "Harm 3", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& skew = addParam({ .key = "skew", .label = "Skew", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // phase warp
    Param& sync = addParam({ .key = "sync", .label = "Sync", .unit = "%" }); // hard sync, the slave ratio dives with the drop
    Param& resonator = addParam({ .key = "resonator", .label = "Resonator", .unit = "%" }); // resonant peak following the pitch
    Param& fold = addParam({ .key = "fold", .label = "Fold", .unit = "%" }); // the wave folds over on itself

    // ================================================================ MOTION (pots 12..15)
    Param& bend = addParam({ .key = "bend", .label = "Bend", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // the tail slides up (+) or down (-), up to 2 oct
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%" }); // phase modulation of the body (fast decaying)
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "%", .value = 40.0f }); // 0.5 .. 8x, in between = metal
    Param& shift = addParam({ .key = "shift", .label = "Shift", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // frequency shifter, sweeps with the drop

    // ================================================================ DRIVE (pots 16..23)
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 40.0f }); // gain 1 .. 128
    Param& character = addParam({ .key = "character", .label = "Character", .unit = "%", .value = 10.0f }); // soft -> hard -> fold -> fuzz
    Param& punch = addParam({ .key = "punch", .label = "Punch", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + dirty hit, - dirty tail
    Param& feedback = addParam({ .key = "feedback", .label = "Feedback", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // comb tuned to the pitch
    Param& click = addParam({ .key = "click", .label = "Click", .unit = "%", .value = 25.0f }); // click on the hit
    Param& clickTone = addParam({ .key = "clickTone", .label = "Click Tone", .unit = "%", .value = 50.0f }); // knock -> noise tick
    Param& noise = addParam({ .key = "noise", .label = "Noise", .unit = "%" }); // into the drive with the body, its length follows Decay
    Param& noiseColor = addParam({ .key = "noiseColor", .label = "Color", .unit = "%", .value = 50.0f }); // LP -> BP -> HP, cutoff rising

    // ================================================================ FILTER (pots 24..31): LP > HP, both track the pitch
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 100.0f }); // low pass, 30 Hz .. 20 kHz
    Param& cutoff2 = addParam({ .key = "cutoff2", .label = "Cutoff 2", .unit = "%", .value = 0.0f }); // high pass, 15 Hz .. 7.7 kHz
    Param& reso = addParam({ .key = "reso", .label = "Reso", .unit = "%", .value = 25.0f });
    Param& env = addParam({ .key = "env", .label = "Env", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + opens on the hit, - closes
    Param& envDecay = addParam({ .key = "envDecay", .label = "Env Decay", .unit = "%", .value = 35.0f }); // 5 .. 1000 ms
    Param& filterFm = addParam({ .key = "filterFm", .label = "FM", .unit = "%" }); // the signal modulates both cutoffs
    Param& filterDrive = addParam({ .key = "filterDrive", .label = "Filter Drive", .unit = "%", .value = 15.0f }); // saturation after the filter
    Param& vowel = addParam({ .key = "vowel", .label = "Vowel", .unit = "%" }); // two formants: 0 = off, then a -> e -> i -> o -> u

    Kick32v4(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "Kick32v4", params)
        , sampleRate(sampleRate)
        , invSr(1.0f / sampleRate)
    {
        ringCoef = tauCoef(150.0f);
        controlRate();
    }

    void trigger() { noteOnImpl(60, 1.0f); }

    void noteOnImpl(uint8_t note, float)
    {
        controlRate();
        noteOct = ((float)note - 60.0f) / 12.0f;
        phase = clickPhase = 0.0f;
        hitPos = 0;
        ampPos = 0;
        ampLen = std::max(1, (int)(sampleRate * decay.value * 0.001f));
        dropPos = 0;
        dropLen = std::max(1, (int)(sampleRate * dropMsOf() * 0.001f));
        clickEnv = 1.0f;
        clickCoef = tauCoef(1.2f + 7.0f * (1.0f - pct(clickTone))); // tick 1.2 ms .. knock 8 ms
        noiseEnv = 1.0f;
        noiseCoef = decayCoef((30.0f + 0.25f * decay.value) * (1.25f - 0.75f * pct(noiseColor))); // follows Decay, shorter when bright
        punchEnv = 1.0f;
        punchCoef = tauCoef(std::max(15.0f, decay.value * 0.2f));
        filtEnv = 1.0f;
        filtCoef = tauCoef(5.0f * fastExp2(pct(envDecay) * 7.6f)); // 5 .. 1000 ms
        ringGate = 1.0f;

        slavePhase = 0.0f;
        resSvf = Svf();
        modPhase = 0.0f;
        fmEnv = 1.0f;
        fmCoef = tauCoef(45.0f);
        // filters, comb, Hilbert and followers are not reset: no click over a ringing filter
    }

    void noteOffImpl(uint8_t) { }

    // ------------------------------------------------------------------ UI previews
    float waveSample(float p) { return bodyWave(p); }
    float drawImpl(float x) { return bodyWave(x); }
    float dropMs() { return dropMsOf(); }
    float vowelHz1() const { return cVF1; }
    float vowelHz2() const { return cVF2; }
    float vowelK() const { return cVowelK; }
    float vowelMixValue() const { return cVowelMix; }
    // octaves above the note at `ms` after the hit (drop + bend)
    float pitchOctAt(float ms) { return cDropOct * dropShape(ms / dropMsOf()) + cBend * bendShape(ms / decay.value); }
    float ampAt(float ms) { float t = ms / decay.value; return t >= 1.0f ? 0.0f : ampShape(t); }
    float fmIndexAt(float ms) { return cBodyFm * cBodyFm * 2.0f * std::exp(-ms / 45.0f); }
    float shiftHzAt(float ms) { return cShift * (0.25f + 0.75f * dropShape(ms / dropMsOf())); }
    float bendOct() const { return cBend; }
    // the drive transfer curve for an input -1..1
    float shaperPreview(float x) { return saturate(x * cGain, cChar) * cCharGain; }
    float sr() const { return sampleRate; }
    float filterCutoff1Hz() const { return cCut1; }
    float filterCutoff2Hz() const { return cCut2; }
    float filterK() const { return cK; }
    float filterGain() const { return cResoComp * 0.8f * 1.25f; }
    float filterDriveGain() const { return cFiltDrive; }
    float clickHz() const { return cClickHz; }
    float driveGain() const { return cGain; }
    bool playing() const { return ampPos < ampLen || ringGate > 0.001f; }
    // live envelopes for the UI
    float envAmp() const { return ampEnv; }
    float envFilter() const { return filtEnv; }
    float envFm() const { return fmEnv; }
    float envPunch() const { return punchEnv; }
    float envDrop() const { return dropEnv; }
    float noteOctave() const { return noteOct; }

    float sampleImpl()
    {
        if (ctl-- <= 0) {
            ctl = CTL_RATE;
            controlRate();
        }

        // ---- envelopes
        if (ampPos < ampLen) {
            float ampT = (float)ampPos / ampLen;
            ampEnv = ampShape(ampT);
            bendEnv = bendShape(ampT);
            ampPos++;
        } else {
            ampEnv = 0.0f;
        }
        dropEnv = dropPos < dropLen ? dropShape((float)dropPos++ / dropLen) : 0.0f;
        clickEnv *= clickCoef;
        noiseEnv *= noiseCoef;
        punchEnv *= punchCoef;
        filtEnv *= filtCoef;

        float bodyOct = noteOct + cDropOct * dropEnv + cBend * bendEnv;
        float freq = std::min(ROOT_HZ * fastExp2(bodyOct), sampleRate * 0.45f);

        // ---- BODY: wave, FM, hard sync, resonator
        float mix = 0.0f;
        if (ampEnv > 0.0f) {
            float inc = freq * invSr;
            float ph = phase + inc;
            bool wrapped = ph >= 1.0f;
            phase = ph - std::floor(ph);

            // FM: phase modulation, stable even with a huge index; the index falls fast
            float pm = 0.0f;
            if (cBodyFm > 0.0f) {
                modPhase += freq * cFmRatio * invSr;
                modPhase -= std::floor(modPhase);
                fmEnv *= fmCoef;
                pm = cBodyFm * cBodyFm * 2.0f * fmEnv * sin2pi(modPhase);
            }
            float sig = bodyWave(phase + pm);

            // sync: a slave oscillator restarted every master cycle, its ratio dives with the drop
            if (cSync > 0.0f) {
                if (wrapped) slavePhase = 0.0f;
                slavePhase += inc * (1.0f + 5.0f * cSync * cSync * (0.2f + 0.8f * dropEnv));
                slavePhase -= std::floor(slavePhase);
                float sm = std::min(1.0f, cSync * 3.5f);
                sig = lerp(sig, bodyWave(slavePhase + pm), sm * sm);
            }

            // resonator: a resonant peak following the pitch, saturated (the peak rings, the wave stays)
            if (cRes > 0.0f) {
                float lp, bp, hp;
                float k = lerp(1.6f, 0.05f, cRes);
                resSvf.tick(sig, gOf(std::clamp(freq, 30.0f, 3500.0f)), k, lp, bp, hp);
                float peak = tanh((lp + bp * k * 1.8f) * (1.0f + cRes * 2.5f));
                sig = lerp(sig, peak, std::min(1.0f, cRes * 1.25f));
            }
            mix = sig * ampEnv;
        }

        // ---- CLICK: a short burst, knock (sine, one octave of its own drop) -> tick (noise).
        // Into the drive with the body, and again after it: a heavy drive squashes whatever rides on the body
        // (measured: click and noise were inaudible above Drive 50 % when only fed to the saturation).
        float transient = 0.0f;
        if (cClick > 0.0f && clickEnv > 0.001f) {
            clickPhase += cClickHz * (1.0f + clickEnv) * invSr;
            clickPhase -= std::floor(clickPhase);
            float c = lerp(sin2pi(clickPhase), white() * 0.8f, cClickNoise);
            transient += cClick * 1.6f * clickEnv * c;
        }

        // ---- NOISE: white -> SVF (LP -> BP -> HP by Color)
        float noiseGate = 0.0f;
        if (cNoise > 0.0f && noiseEnv > 0.0005f) {
            float lp, bp, hp;
            noiseSvf.tick(white(), cNoiseG, cNoiseK, lp, bp, hp);
            float n = lp * cNoiseLp + bp * cNoiseK * 1.5f * cNoiseBp + hp * cNoiseHp;
            transient += cNoise * 2.2f * n * noiseEnv;
            noiseGate = noiseEnv * std::min(1.0f, cNoise * 3.0f);
        }
        mix += transient;

        // ---- FEEDBACK: a comb tuned to the pitch (one period), through the drive. + reinforces the pitch, - half harmonics
        float fbIn = 0.0f;
        if (cFeedback != 0.0f && ampEnv > 0.0f) {
            float len = std::clamp(sampleRate / freq, 2.0f, (float)(COMB_MAX - 2));
            fbLp += 0.3f * (combRead(len) - fbLp);
            fbIn = cFeedback * std::abs(cFeedback) * 1.4f * ampEnv * fbLp;
        }

        // ---- DRIVE: gain (with the punch envelope), character, 2x oversampled
        float g = cGain;
        if (cPunch > 0.0f) g *= 1.0f + 6.0f * cPunch * punchEnv;
        else if (cPunch < 0.0f) g *= 1.0f - 6.0f * cPunch * (1.0f - punchEnv);
        liveDriveGain = g;
        float pre = (mix + fbIn) * g + 1e-18f;
        float out = saturate2x(pre) * cCharGain;
        livePre = pre;
        livePost = out;
        satDcOut = out - satDcIn + 0.997f * satDcOut; // fold / fuzz make DC
        satDcIn = out;
        out = satDcOut + tanh(transient * 1.3f) * 0.75f; // the transients again, soft clipped on their own, after the drive
        comb[combPos] = out;
        if (++combPos >= COMB_MAX) combPos = 0;

        // post VCA: the amp shape survives the drive (noise may outlast the body)
        float bodyGate = std::max(ampEnv * (2.0f - ampEnv), noiseGate);
        out *= bodyGate;

        // ---- SHIFT: frequency shifter (Hilbert + quadrature oscillator), half dry / half shifted.
        // The shift in Hz sweeps down with the drop: laser on the hit, metal on the tail, beating at small amounts.
        if (cShiftMix > 0.0f) {
            float hz = cShift * (0.25f + 0.75f * dropEnv);
            liveShiftHz = hz;
            shiftPhase += hz * invSr;
            shiftPhase -= std::floor(shiftPhase);
            float I, Q;
            hilbert.tick(out, I, Q);
            float shifted = I * sin2pi(shiftPhase + 0.25f) + Q * sin2pi(shiftPhase); // + = up
            out = lerp(out, 0.72f * (out + shifted), cShiftMix); // 0.72: the shifted half is uncorrelated, keeps the loudness flat
        } else {
            liveShiftHz = 0.0f;
        }

        // ---- VOWEL: two formant band passes, after the drive (that is where the harmonics are)
        if (cVowelMix > 0.0f) {
            float l1, b1, h1, l2, b2, h2;
            vowelSvf[0].tick(out, cVG1, cVowelK, l1, b1, h1);
            vowelSvf[1].tick(out, cVG2, cVowelK, l2, b2, h2);
            out = out * (1.0f - 0.3f * cVowelMix) + (b1 + b2) * cVowelK * 4.0f * cVowelMix; // trimmed flat: no level drop when the vowel comes in
        }

        // ---- FILTERBANK: LP > HP, both follow the pitch (note + drop + bend), env on the hit, audio rate FM by the signal itself
        float oct = bodyOct + cEnv * 6.0f * filtEnv + cFm * out;
        liveFilterOct = oct;
        float m = fastExp2(oct);
        float lp1, bp1, hp1, lp2, bp2, hp2;
        f1.tick(out, gOf(cCut1 * m), cK, lp1, bp1, hp1);
        f2.tick(lp1, gOf(cCut2 * m), cK, lp2, bp2, hp2);
        float y = 1.25f * tanh(hp2 * cResoComp * 0.8f * cFiltDrive); // filter drive: resonance growls instead of whistling

        // the ring of the filters dies after the kick: a drum module must not drone
        float gate = bodyGate;
        ringGate = gate > 0.0005f ? 1.0f : ringGate * ringCoef;
        y *= ringGate;

        // ---- output: DC + soft clip (fixed level, never above 0.95)
        dcOut = y - dcIn + 0.9985f * dcOut;
        dcIn = y;
        float result = 0.95f * tanh(dcOut * 1.5f);
        meterOut = std::max(std::abs(result), meterOut * 0.9985f);
        lastOut = result;
        return result;
    }
};
