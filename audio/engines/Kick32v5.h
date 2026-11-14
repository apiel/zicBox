#pragma once

/**
 * Kick32v5 — a big BODY, a heavy DRIVE, a one pot FILTER. 32 pots, no menu, no switch: every choice is a morph.
 *
 * Built for hardtek / mental tribe / hardcore / tekno. One voice only: everything bends, modulates, folds, clips or
 * filters the same sound. No second layers.
 *
 *   BODY   (16) decay (max = infinite) / shape, pitch drop (amount, time, curve), wave morph + harmonics 2 / 3,
 *               skew, hard sync, resonator, fold, harmonic 4, growl (one cycle short, the next one long: a sub octave growl),
 *               grit (noise shakes the phase: a torn, growling wave), steps (the pitch drop falls in steps)
 *   DRIVE  (12) drive, character (soft -> hard -> fold -> fuzz), sputter (a bias the decaying body cannot follow:
 *               the pulse thins along the tail, even harmonics that grow and move),
 *               punch (drive envelope, bipolar), feedback (comb tuned to the pitch, bipolar), crush (sample rate),
 *               bite + bite freq (a resonant peak between a first clip and the drive, following the pitch: the "tok"),
 *               fuzz (octave fuzz: the wave rectified in front of the saturation),
 *               click, tone (knock -> tick -> noise burst), fat (the lows pushed back into a last soft clip)
 *   FILTER (4)  filter (LP <- open -> HP, 24 dB, follows the pitch), reso, env (bipolar), drive (distortion inside the filter)
 *
 *   BODY + CLICK --> [+ FEEDBACK comb] --> gain (PUNCH) --> BITE --> FUZZ --> SPUTTER bias --> DRIVE 2x (CHARACTER)
 *     --> FAT --> CRUSH --> FILTER --> FILTER DRIVE --> soft clip --> OUT
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

class Kick32v5 : public EngineBase<Kick32v5> {
public:
    static constexpr int PARAM_COUNT = 32;
    static constexpr float ROOT_HZ = 52.0f; // frequency of note 60

protected:
    static constexpr float TWO_PI = 6.28318530718f;
    static constexpr int CTL_RATE = 32; // samples between two control rate updates
    static constexpr float SPUTTER_MAKEUP = 1.0f;
    static constexpr float BITE_GAIN = 5.0f;
    static constexpr float GRIT_DEPTH = 2.5f;
    static constexpr float FAT_GAIN = 3.0f;
    static constexpr float FAT_NORM = 1.0f;
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

    // ------------------------------------------------------------------ voice state
    float noteOct = 0.0f;
    float phase = 0.0f, clickPhase = 0.0f;
    int hitPos = 0, ampPos = 0, ampLen = 1, dropPos = 0, dropLen = 1;
    bool infinite = false; // Decay at its maximum: the body never releases (until the next hit)
    float ampEnv = 0.0f, dropEnv = 0.0f;
    float clickEnv = 0.0f, clickCoef = 0.0f;
    float punchEnv = 0.0f, punchCoef = 0.0f;
    float filtEnv = 0.0f, filtCoef = 0.0f;
    float ringGate = 0.0f, ringCoef = 0.0f;

    // body extras: hard sync slave, resonator, grit
    float slavePhase = 0.0f;
    Svf resSvf;
    float gritLp = 0.0f;
    bool growlOdd = false;

    // drive (2x oversampled saturation: the input is interpolated, the output decimated by a 7 tap half band)
    float comb[COMB_MAX] = {};
    int combPos = 0;
    float fbLp = 0.0f;
    Svf biteSvf;
    float prePrev = 0.0f;
    float osHist[7] = {};
    float satDcIn = 0.0f, satDcOut = 0.0f;
    float crushPhase = 0.0f, crushHold = 0.0f;

    float fatLp = 0.0f;

    // filter
    Svf f1, f2;
    float dcIn = 0.0f, dcOut = 0.0f;

    // ------------------------------------------------------------------ control rate cache (pot dependent)
    int ctl = 0;
    float cDropOct = 0, cWave = 0, cH2 = 0, cH3 = 0;
    float cSkew = 0.5f, cSync = 0, cRes = 0, cFold = 0, cWaveGain = 1;
    float cH4 = 0, cGrowl = 0, cGrit = 0, cStepsN = 16, cStepsMix = 0;
    float cGain = 1, cChar = 0, cCharGain = 1, cPunch = 0, cFeedback = 0;
    float cSputter = 0, cSputterGain = 1, cCrush = 0, cCrushStep = 1;
    float cBite = 0, cBiteRatio = 4, cBiteK = 0.2f;
    float cClick = 0, cClickHz = 300, cClickNoise = 0;
    float cFuzz = 0, cFat = 0, cFatNorm = 1;
    float cCut = 20000, cK = 1, cEnv = 0, cResoComp = 1, cFiltDrive = 1;
    bool cHp = false;
    float cWet = 0.0f; // 0 around the centre of the Filter pot (open), 1 from 15 % on either side

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
        cH4 = harm4.value * 0.01f;
        cGrowl = 0.6f * pct(growl); // one cycle up to 1.6x faster, the next one as much slower
        float gr = pct(grit);
        cGrit = gr * gr * GRIT_DEPTH;
        float sp0 = pct(steps);
        cStepsMix = std::min(1.0f, sp0 * 4.0f);
        cStepsN = lerp(9.0f, 1.5f, sp0); // steps over the whole drop: many small ones .. two big ones

        float dv = pct(drive);
        cGain = fastExp2(dv * 7.0f); // 1 .. 128
        cChar = pct(character) * 3.0f;
        cCharGain = charGainOf(pct(character));
        float sp = pct(sputter);
        cSputter = 0.8f * sp * (cSkew > 0.5f ? -1.0f : 1.0f); // against the long side of a skewed wave: no level loss
        cSputterGain = 1.0f + SPUTTER_MAKEUP * sp; // the thin pulse loses rms: measured make-up
        cPunch = punch.value * 0.01f;
        cFeedback = feedback.value * 0.01f;
        cCrush = pct(crush);
        cCrushStep = 1.0f + cCrush * cCrush * 80.0f;
        float bi = pct(bite);
        cBite = bi * bi * BITE_GAIN;
        cBiteRatio = 1.5f * fastExp2(pct(biteFreq) * 3.0f); // 1.5 .. 12 x the pitch
        cBiteK = lerp(0.5f, 0.12f, bi);
        cFuzz = 0.42f * pct(fuzz); // half of it at most: the octave comes in, the fundamental stays
        float ft = pct(fat);
        cFat = ft * FAT_GAIN;
        cFatNorm = 1.0f / (1.0f + ft * FAT_NORM);

        cClick = pct(click);
        float ct = std::min(1.0f, pct(clickTone) * 1.5f); // the first two thirds: knock -> tick. The last third makes the tick longer (a noise burst)
        cClickHz = 300.0f * fastExp2(ct * 4.4f); // 300 Hz .. 6.3 kHz
        cClickNoise = ct * ct; // the tick end of the pot is a noise tick

        // FILTER: one pot. Left of the centre a low pass closing in, right a high pass rising; the centre is open.
        float f = filter.value * 0.01f;
        cHp = f > 0.0f;
        float fa = std::abs(f);
        float fw = clamp01(fa / 0.15f);
        cWet = fw * fw * (3.0f - 2.0f * fw); // the centre is the dry signal: LP and HP both fade into it, no jump when crossing
        float fr = clamp01((fa - 0.04f) / 0.96f);
        cCut = cHp ? 15.0f * fastExp2(fr * 7.0f) : 20000.0f * fastExp2(-fr * 9.4f); // HP 15 Hz .. 1.9 kHz, LP 20 kHz .. 30 Hz
        float r = pct(reso);
        cK = lerp(1.5f, 0.02f, std::sqrt(r));
        cResoComp = 0.6f + 0.4f * std::sqrt(cK / 1.5f);
        cEnv = env.value * 0.01f;
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

    // Steps: the drop envelope quantized, so the pitch falls in steps (a zipper, an arpeggio on a long drop)
    float stepQ(float e) const
    {
        if (cStepsMix <= 0.0f) return e;
        float q = std::floor(e * cStepsN + 0.5f) / cStepsN;
        return lerp(e, q, cStepsMix);
    }

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
        if (cH4 != 0.0f) w += cH4 * 1.2f * sin2pi(4.0f * p);
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
    float liveFilterOct = 0.0f; // the live offset of the cutoff (env + pitch), octaves
    float livePitchOct = 0.0f;
    float liveDriveGain = 1.0f;
    float livePre = 0.0f, livePost = 0.0f; // in / out of the saturation, for the transfer curve dot

    Param params[PARAM_COUNT];

    // ================================================================ BODY (pots 0..15, a 4 x 4 block)
    Param& decay = addParam({ .key = "decay", .label = "Decay", .unit = "ms", .value = 350.0f, .min = 40.0f, .max = 2000.0f, .step = 10.0f }); // max = infinite
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
    Param& harm4 = addParam({ .key = "harm4", .label = "Harm 4", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& growl = addParam({ .key = "growl", .label = "Growl", .unit = "%" }); // period doubling: every other cycle is shorter, the next one longer
    Param& grit = addParam({ .key = "grit", .label = "Grit", .unit = "%" }); // low passed noise shakes the phase
    Param& steps = addParam({ .key = "steps", .label = "Steps", .unit = "%" }); // the pitch drop falls in steps: 9 small -> 2 big

    // ================================================================ DRIVE (pots 16..27, a 3 x 4 block)
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 60.0f }); // gain 1 .. 128
    Param& character = addParam({ .key = "character", .label = "Character", .unit = "%", .value = 15.0f }); // soft -> hard -> fold -> fuzz
    Param& sputter = addParam({ .key = "sputter", .label = "Sputter", .unit = "%", .value = 25.0f }); // bias: the pulse thins along the tail
    Param& punch = addParam({ .key = "punch", .label = "Punch", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + dirty hit, - dirty tail
    Param& feedback = addParam({ .key = "feedback", .label = "Feedback", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // comb tuned to the pitch
    Param& crush = addParam({ .key = "crush", .label = "Crush", .unit = "%" }); // sample and hold, 44 kHz .. ~500 Hz
    Param& bite = addParam({ .key = "bite", .label = "Bite", .unit = "%" }); // resonant peak between a first clip and the drive
    Param& biteFreq = addParam({ .key = "biteFreq", .label = "Bite Freq", .unit = "%", .value = 45.0f }); // 1.5 .. 12 x the pitch, follows the drop
    Param& fuzz = addParam({ .key = "fuzz", .label = "Fuzz", .unit = "%" }); // octave fuzz: full wave rectifier in front of the saturation
    Param& click = addParam({ .key = "click", .label = "Click", .unit = "%", .value = 25.0f }); // click on the hit
    Param& clickTone = addParam({ .key = "clickTone", .label = "Tone", .unit = "%", .value = 35.0f }); // knock -> noise tick -> noise burst
    Param& fat = addParam({ .key = "fat", .label = "Fat", .unit = "%" }); // the lows (around the pitch) pushed back into a last soft clip

    // ================================================================ FILTER (pots 28..31, one column)
    Param& filter = addParam({ .key = "filter", .label = "Filter", .unit = "%", .value = -25.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // LP <- open -> HP, follows the pitch
    Param& reso = addParam({ .key = "reso", .label = "Reso", .unit = "%", .value = 25.0f });
    Param& env = addParam({ .key = "env", .label = "Env", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + opens on the hit, - closes
    Param& filterDrive = addParam({ .key = "filterDrive", .label = "Drive", .unit = "%", .value = 15.0f }); // distortion between the two filter stages, 1 .. 25x

    Kick32v5(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "Kick32v5", params)
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
        growlOdd = false;
        hitPos = 0;
        ampPos = 0;
        infinite = decay.value >= decay.max - 0.5f;
        ampLen = infinite ? 0x7fffffff : std::max(1, (int)(sampleRate * decay.value * 0.001f));
        dropPos = 0;
        dropLen = std::max(1, (int)(sampleRate * dropMsOf() * 0.001f));
        clickEnv = 1.0f;
        float tone = pct(clickTone);
        float burst = clamp01((tone - 0.667f) * 3.0f);
        clickCoef = tauCoef(1.2f + 7.0f * (1.0f - std::min(1.0f, tone * 1.5f)) + burst * burst * 0.2f * decay.value); // knock 8 ms .. tick 1.2 ms .. burst (a fifth of Decay)
        punchEnv = 1.0f;
        punchCoef = tauCoef(std::max(15.0f, decay.value * 0.2f));
        filtEnv = 1.0f;
        filtCoef = tauCoef(std::max(10.0f, 0.25f * decay.value)); // the filter envelope lasts a quarter of Decay
        ringGate = 1.0f;

        slavePhase = 0.0f;
        resSvf = Svf();
        // filters, comb and followers are not reset: no click over a ringing filter
    }

    void noteOffImpl(uint8_t) { }

    // ------------------------------------------------------------------ UI previews
    float waveSample(float p) { return bodyWave(p); }
    float drawImpl(float x) { return bodyWave(x); }
    float dropMs() { return dropMsOf(); }
    bool isInfinite() { return decay.value >= decay.max - 0.5f; }
    float decayMsUi() { return decay.value; }
    // octaves above the note at `ms` after the hit (drop, with its steps)
    float pitchOctAt(float ms) { return cDropOct * stepQ(dropShape(ms / dropMsOf())); }
    float ampAt(float ms) { if (isInfinite()) return 1.0f; float t = ms / decay.value; return t >= 1.0f ? 0.0f : ampShape(t); }
    // the drive transfer curve for an input -1..1 (at full body level, where the sputter bias is the smallest)
    float shaperPreview(float x)
    {
        float pre = x * cGain;
        if (cFuzz > 0.0f) pre = lerp(pre, 2.0f * std::abs(pre) - 1.414f * cGain, cFuzz);
        return saturate(pre + std::clamp(cSputter, -0.88f, 0.88f) * cGain, cChar) * cCharGain * cSputterGain;
    }
    float sr() const { return sampleRate; }
    bool filterIsHp() const { return cHp; }
    float filterCutoffHz() const { return cCut; }
    float filterK() const { return cK; }
    float filterGain() const { return cResoComp * 0.8f * 1.25f; }
    float filterDriveGain() const { return cFiltDrive; }
    float filterWet() const { return cWet; }
    float biteRatio() const { return cBiteRatio; }
    float biteAmount() const { return cBite / BITE_GAIN; }
    float clickHz() const { return cClickHz; }
    float driveGain() const { return cGain; }
    bool playing() const { return infinite || ampPos < ampLen || ringGate > 0.001f; }
    // live envelopes for the UI
    float envAmp() const { return ampEnv; }
    float envFilter() const { return filtEnv; }
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
        if (infinite) {
            ampEnv = 1.0f; // Decay at max: full level until the next hit (Shape is moot there)
            if (ampPos < 0x3fffffff) ampPos++;
        } else if (ampPos < ampLen) {
            ampEnv = ampShape((float)ampPos / ampLen);
            ampPos++;
        } else {
            ampEnv = 0.0f;
        }
        dropEnv = dropPos < dropLen ? dropShape((float)dropPos++ / dropLen) : 0.0f;
        clickEnv *= clickCoef;
        punchEnv *= punchCoef;
        filtEnv *= filtCoef;

        float bodyOct = noteOct + cDropOct * stepQ(dropEnv);
        livePitchOct = bodyOct;
        float freq = std::min(ROOT_HZ * fastExp2(bodyOct), sampleRate * 0.45f);

        // ---- BODY: wave, grit, hard sync, resonator
        float mix = 0.0f;
        if (ampEnv > 0.0f) {
            float inc = freq * invSr;
            // growl: period doubling. One cycle runs faster, the next one slower, the pair keeps its length: the pitch
            // stays, a rough octave below appears with all its half harmonics. It survives any clipper (it moves the zero crossings).
            float ph = phase + (cGrowl > 0.0f ? inc / (growlOdd ? 1.0f + cGrowl : 1.0f - cGrowl) : inc);
            bool wrapped = ph >= 1.0f;
            if (wrapped) growlOdd = !growlOdd;
            phase = ph - std::floor(ph);

            // grit: low passed noise shakes the phase. Every harmonic grows noise sidebands: a torn, growling wave
            float pm = 0.0f;
            if (cGrit > 0.0f) {
                gritLp += 0.015f * (white() - gritLp); // ~100 Hz: a rumble in the phase, not a hiss
                pm = cGrit * gritLp;
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

        // ---- CLICK: a short burst, knock (sine, one octave of its own drop) -> tick (noise) -> noise burst.
        // Into the drive with the body, and again after it: a heavy drive squashes whatever rides on the body.
        float transient = 0.0f;
        if (cClick > 0.0f && clickEnv > 0.001f) {
            clickPhase += cClickHz * (1.0f + clickEnv) * invSr;
            clickPhase -= std::floor(clickPhase);
            float c = lerp(sin2pi(clickPhase), white() * 0.8f, cClickNoise);
            transient = cClick * 1.6f * clickEnv * c;
        }
        mix += transient;

        // ---- FEEDBACK: a comb tuned to the pitch (one period), through the drive. + reinforces the pitch, - half harmonics
        float fbIn = 0.0f;
        if (cFeedback != 0.0f && ampEnv > 0.0f) {
            float len = std::clamp(sampleRate / freq, 2.0f, (float)(COMB_MAX - 2));
            fbLp += 0.3f * (combRead(len) - fbLp);
            fbIn = cFeedback * std::abs(cFeedback) * 1.4f * ampEnv * fbLp;
        }

        // ---- DRIVE: gain (with the punch envelope)
        float g = cGain;
        if (cPunch > 0.0f) g *= 1.0f + 6.0f * cPunch * punchEnv;
        else if (cPunch < 0.0f) g *= 1.0f - 6.0f * cPunch * (1.0f - punchEnv);
        liveDriveGain = g;
        float pre = (mix + fbIn) * g + 1e-18f;

        // ---- BITE: clip > resonant peak > clip. A first clip makes the harmonics, a band pass tuned to a multiple of
        // the pitch rings on them, and the ring goes into the drive on top of the body: the clipper bites at that
        // frequency (the "tok" of a hardcore kick). It follows the pitch, so it dives with the drop.
        if (cBite > 0.0f) {
            float lp, bp, hp;
            biteSvf.tick(tanh(pre * 2.0f), gOf(std::min(freq * cBiteRatio, 9000.0f)), cBiteK, lp, bp, hp);
            pre += cBite * g * ampEnv * bp;
        }

        // ---- FUZZ: octave fuzz. The wave is full wave rectified in front of the saturation (and re-centred on the body
        // level): the clipper sees twice the pitch. Dirty, hollow, screaming with Bite; the fundamental never leaves.
        if (cFuzz > 0.0f) pre = lerp(pre, 2.0f * std::abs(pre) - 1.414f * g * ampEnv, cFuzz);

        // ---- SPUTTER: a fixed bias against a decaying body = the pulse thins out along the tail (50 % -> ~20 % duty):
        // even harmonics that grow and move through the decay, the sputter of a fuzz pushed too far. Capped so
        // the tail never gates out.
        float bias = 0.0f;
        if (cSputter != 0.0f) bias = std::clamp(cSputter, -0.88f * ampEnv, 0.88f * ampEnv) * g;

        // ---- the one saturation: character, 2x oversampled
        float out = saturate2x(pre + bias) * cCharGain * cSputterGain;
        livePre = pre;
        livePost = out;
        satDcOut = out - satDcIn + 0.9985f * satDcOut; // bias / fold / fuzz make DC (10 Hz: the flat tops of a square stay flat)
        satDcIn = out;
        out = satDcOut + tanh(transient * 1.3f) * 1.0f; // the transients again, soft clipped on their own, after the drive
        comb[combPos] = out;
        if (++combPos >= COMB_MAX) combPos = 0;

        // post VCA: the amp shape survives the drive (noise may outlast the body)
        float bodyGate = std::max(ampEnv * (2.0f - ampEnv), std::min(1.0f, std::abs(transient) * 4.0f));
        out *= bodyGate;

        // ---- FAT: the lows around the pitch (a one pole following it) are added back, and the sum goes through a soft
        // clip: the low end swells and takes the room, the buzz above it is pressed down.
        if (cFat > 0.0f) {
            fatLp += std::min(0.5f, 9.42f * freq * invSr) * (out - fatLp); // ~1.5 x the pitch
            out = 1.3f * tanh((out + cFat * fatLp) * cFatNorm);
        }

        // ---- CRUSH: sample and hold, 44 kHz down to ~500 Hz. After a clipper bits do nothing (a square has two
        // levels): the weight is all in the rate, whose aliases fold the harmonics into inharmonic, robotic tones.
        if (cCrush > 0.0f) {
            crushPhase += 1.0f;
            if (crushPhase >= cCrushStep) {
                crushPhase -= cCrushStep;
                crushHold = out;
            }
            out = crushHold;
        }

        // ---- FILTER: one SVF following the pitch (note + drop), env on the hit. LP or HP by the side of the pot.
        float oct = bodyOct + cEnv * 6.0f * filtEnv;
        liveFilterOct = oct;
        // Two 12 dB stages with the drive between them (24 dB in all): the first one has the resonance, the drive makes
        // its ring growl and gives edges back, the second one smooths what the drive added. Around the centre of the
        // pot both stages fade to the dry signal, so the drive stays and LP <-> HP crosses without a jump.
        float gF = gOf(cCut * fastExp2(oct));
        float lp1, bp1, hp1, lp2, bp2, hp2;
        f1.tick(out, gF, cK, lp1, bp1, hp1);
        float x1 = lerp(out, cHp ? hp1 : lp1, cWet);
        float d = 1.25f * tanh(x1 * lerp(1.0f, cResoComp, cWet) * 0.8f * cFiltDrive);
        f2.tick(d, gF, 1.2f, lp2, bp2, hp2);
        float y = lerp(d, cHp ? hp2 : lp2, cWet);

        // the ring of the filter dies after the kick: a drum module must not drone
        float gate = bodyGate;
        ringGate = gate > 0.0005f ? 1.0f : ringGate * ringCoef;
        y *= ringGate;

        // ---- output: DC + soft clip (fixed level, never above 0.95)
        dcOut = y - dcIn + 0.9993f * dcOut;
        dcIn = y;
        float result = 0.95f * tanh(dcOut * 1.5f);
        meterOut = std::max(std::abs(result), meterOut * 0.9985f);
        lastOut = result;
        return result;
    }
};
