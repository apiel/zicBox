#pragma once

/**
 * Kick32v3 — BODY + TAIL > ONE DRIVE > FILTERBANK. 32 pots, no menu, no switch: every choice is a morph.
 *
 * Built for tekno / hardtek / metal tek / hardcore / tribe.
 *   - the BODY is the hit: amp shape, pitch drop, wave + harmonics
 *   - BOUNCE: the hit rebounds like a ball (up to 3 ghost hits, fading, accelerating or slowing, stepping in pitch)
 *   - SUB: a clean sine under the driven kick (the DROP idea), with its own pitch, length and drop
 *   - click + noise are layered into the body drive (the DROP idea: several sources, one saturation)
 *   - a comb tuned to the pitch feeds the drive back into itself
 *   - the FILTERBANK (Sherman idea): LP > HP, both tracking the pitch, with an envelope, audio rate FM
 *     of the cutoffs by the signal itself, and a drive after the filter
 *
 *   BODY   (8)  decay / shape, pitch drop (amount, time, curve), wave morph + harmonics 2 / 3
 *   BOUNCE (8)  bounce amount, time, accel (bipolar), pitch step per rebound | sub level, pitch, length, drop
 *   DRIVE  (8)  drive, character (soft -> hard -> fold -> fuzz), punch (drive envelope, bipolar),
 *               feedback (comb tuned to the pitch, bipolar), click (level, tone), noise (level, color)
 *   FILTER (8)  cutoff (LP), cutoff 2 (HP), reso, env, env decay, FM, filter drive, crush
 *
 *   BODY (wave, harmonics, drop; BOUNCE retriggers it) + CLICK + NOISE --> [+ FEEDBACK comb] --> DRIVE (punch, character) --+
 *                                                                                                     +--> CRUSH --> LP > HP (reso, env, FM, track) --> FILTER DRIVE --> clip --> OUT
 *   SUB (clean sine, own pitch / length / drop) ------------------------------------------------------+
 *
 * Param index == pot index. No volume (fixed level + soft clip), no tune (pitch = note from the sequencer,
 * note 60 = ROOT_HZ), no velocity.
 *
 * No dependency on any UI or desktop API: runs as is on the Daisy Seed / STM32H7.
 * Per sample: no std::exp / pow / tan (fast approximations), pot dependent coefficients at control rate.
 * Memory: a 4096 sample comb (16 KB) + a few floats.
 */

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Kick32v3 : public EngineBase<Kick32v3> {
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

    // ------------------------------------------------------------------ voice state
    float noteOct = 0.0f;
    float phase = 0.0f, clickPhase = 0.0f;
    int hitPos = 0, ampPos = 0, ampLen = 1, dropPos = 0, dropLen = 1;
    float ampEnv = 0.0f, dropEnv = 0.0f;
    float clickEnv = 0.0f, clickCoef = 0.0f;
    float noiseEnv = 0.0f, noiseCoef = 0.0f;
    float punchEnv = 0.0f, punchCoef = 0.0f;
    float filtEnv = 0.0f, filtCoef = 0.0f;
    float ringGate = 0.0f, ringCoef = 0.0f;
    Svf noiseSvf;

    // bounce (ghost hits) + sub
    int bounceN = 0;
    float nextHit = 0.0f, hitSpacing = 0.0f, hitGain = 1.0f, bounceOct = 0.0f;
    float subPhase = 0.0f, subEnv = 0.0f, subCoef = 0.0f;

    // drive
    float comb[COMB_MAX] = {};
    int combPos = 0;
    float fbLp = 0.0f;
    float satDcIn = 0.0f, satDcOut = 0.0f;
    float crushPhase = 0.0f, crushHold = 0.0f;

    // filterbank
    Svf f1, f2;
    float dcIn = 0.0f, dcOut = 0.0f;

    // ------------------------------------------------------------------ control rate cache (pot dependent)
    int ctl = 0;
    float cDropOct = 0, cWave = 0, cH2 = 0, cH3 = 0;
    float cBounce = 0, cBounceAccel = 1, cBounceStep = 0, cSub = 0, cSubDrop = 0, cWaveGain = 1;
    float cGain = 1, cChar = 0, cPunch = 0, cFeedback = 0;
    float cClick = 0, cClickHz = 300, cNoise = 0, cNoiseG = 0.1f, cNoiseK = 0.7f, cNoiseLp = 1, cNoiseBp = 0, cNoiseHp = 0;
    float cCut1 = 20000, cCut2 = 15, cK = 1, cEnv = 0, cFm = 0, cFiltDrive = 1, cResoComp = 1, cCrush = 0;

    void controlRate()
    {
        float d = pct(drop);
        cDropOct = 6.0f * d * std::sqrt(d); // 0 .. 6 oct
        cWave = pct(wave);
        cH2 = harm2.value * 0.01f;
        cH3 = harm3.value * 0.01f;

        cBounce = pct(bounce);
        cBounceAccel = fastExp2(bounceAccel.value * 0.01f); // x0.5 .. x2 per rebound
        cBounceStep = bounceStep.value / 12.0f;
        cSub = pct(sub);
        cSubDrop = pct(subDrop);
        cWaveGain = waveGainOf(cWave);

        float dv = pct(drive);
        cGain = fastExp2(dv * 7.0f); // 1 .. 128
        cChar = pct(character) * 3.0f;
        cPunch = punch.value * 0.01f;
        cFeedback = feedback.value * 0.01f;

        cClick = pct(click);
        cClickHz = 300.0f * fastExp2(pct(clickTone) * 4.4f); // 300 Hz .. 6.3 kHz
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
        cFm = (0.5f * fmv + 0.5f * fmv * fmv) * 7.0f; // up to +-7 octaves at audio rate
        float fd = pct(filterDrive);
        cFiltDrive = 1.0f + fd * fd * 24.0f; // 1 .. 25x after the filter
        cCrush = pct(crush);
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

    float dropMsOf() { return 4.0f * fastExp2(pct(dropTime) * 7.0f); } // 4 .. 512 ms
    float bounceMsOf() { return 20.0f * fastExp2(pct(bounceTime) * 3.64f); } // 20 .. 250 ms
    float subMsOf() { return 60.0f * fastExp2(pct(subLength) * 5.06f); } // 60 ms .. 2 s

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

    // Body wave: sine -> triangle -> square -> saw, then harmonics 2 / 3 (bipolar, strong). Everything starts at 0: no click unless asked.
    float bodyWave(float p)
    {
        p -= std::floor(p);
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
        return w;
    }

    // The one saturation. x is already gained. Character: soft -> hard -> fold -> fuzz (octave up)
    float saturate(float x, float ch)
    {
        float soft = tanh(x);
        float hard = std::clamp(x, -1.0f, 1.0f);
        if (ch <= 1.0f) return lerp(soft, hard, ch);
        float fold = sin2pi(1.5f * tanh(x * 0.1667f)); // up to 3 folds whatever the drive: stays a tone
        if (ch <= 2.0f) return lerp(hard, fold, ch - 1.0f);
        float fuzz = tanh(std::abs(x) * 2.0f) * 2.0f - 1.0f;
        return lerp(fold, fuzz, ch - 2.0f);
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
    float lastOut = 0.0f, lastSub = 0.0f;

    Param params[PARAM_COUNT];

    // ================================================================ BODY (pots 0..7)
    Param& decay = addParam({ .key = "decay", .label = "Decay", .unit = "ms", .value = 350.0f, .min = 40.0f, .max = 2000.0f, .step = 10.0f });
    Param& shape = addParam({ .key = "shape", .label = "Shape", .unit = "%", .value = 30.0f }); // punchy -> linear -> gated
    Param& drop = addParam({ .key = "drop", .label = "Drop", .unit = "%", .value = 55.0f }); // 0 .. 6 oct
    Param& dropTime = addParam({ .key = "dropTime", .label = "Drop Time", .unit = "%", .value = 42.0f }); // 4 .. 512 ms
    Param& dropCurve = addParam({ .key = "dropCurve", .label = "Drop Curve", .unit = "%", .value = 70.0f }); // laser -> linear -> zap
    Param& wave = addParam({ .key = "wave", .label = "Wave", .unit = "%" }); // sine -> tri -> square -> saw
    Param& harm2 = addParam({ .key = "harm2", .label = "Harm 2", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& harm3 = addParam({ .key = "harm3", .label = "Harm 3", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });

    // ================================================================ BOUNCE + SUB (pots 8..15)
    Param& bounce = addParam({ .key = "bounce", .label = "Bounce", .unit = "%", .value = 0.0f }); // level of the ghost hits (3 rebounds, each 0.75x the one before)
    Param& bounceTime = addParam({ .key = "bounceTime", .label = "Time", .unit = "%", .value = 35.0f }); // gap before the first rebound, 20 .. 250 ms
    Param& bounceAccel = addParam({ .key = "bounceAccel", .label = "Accel", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // - rebounds speed up like a ball, + slow down
    Param& bounceStep = addParam({ .key = "bounceStep", .label = "Step", .unit = "st", .value = 0.0f, .min = -12.0f, .max = 12.0f, .type = VALUE_CENTERED }); // pitch change per rebound
    Param& sub = addParam({ .key = "sub", .label = "Sub", .unit = "%", .value = 0.0f }); // clean sine under the driven kick
    Param& subPitch = addParam({ .key = "subPitch", .label = "Sub Pitch", .unit = "st", .value = -12.0f, .min = -24.0f, .max = 0.0f }); // from the note
    Param& subLength = addParam({ .key = "subLength", .label = "Sub Length", .unit = "%", .value = 50.0f }); // own decay, 60 ms .. 2 s
    Param& subDrop = addParam({ .key = "subDrop", .label = "Sub Drop", .unit = "%", .value = 50.0f }); // how much of the body pitch drop the sub follows

    // ================================================================ DRIVE (pots 16..23)
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 40.0f }); // gain 1 .. 128
    Param& character = addParam({ .key = "character", .label = "Character", .unit = "%", .value = 10.0f }); // soft -> hard -> fold -> fuzz
    Param& punch = addParam({ .key = "punch", .label = "Punch", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + dirty hit, - dirty tail
    Param& feedback = addParam({ .key = "feedback", .label = "Feedback", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // comb tuned to the pitch
    Param& click = addParam({ .key = "click", .label = "Click", .unit = "%", .value = 25.0f }); // click oscillator on the hit
    Param& clickTone = addParam({ .key = "clickTone", .label = "Click Tone", .unit = "%", .value = 50.0f }); // knock -> tick
    Param& noise = addParam({ .key = "noise", .label = "Noise", .unit = "%" }); // into the drive with the body, its length follows Decay
    Param& noiseColor = addParam({ .key = "noiseColor", .label = "Color", .unit = "%", .value = 50.0f }); // LP -> BP -> HP, cutoff rising, shorter when bright

    // ================================================================ FILTER (pots 24..31): LP > HP, both track the pitch
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 100.0f }); // low pass, 30 Hz .. 20 kHz
    Param& cutoff2 = addParam({ .key = "cutoff2", .label = "Cutoff 2", .unit = "%", .value = 0.0f }); // high pass, 15 Hz .. 7.7 kHz
    Param& reso = addParam({ .key = "reso", .label = "Reso", .unit = "%", .value = 25.0f });
    Param& env = addParam({ .key = "env", .label = "Env", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + opens on the hit, - closes
    Param& envDecay = addParam({ .key = "envDecay", .label = "Env Decay", .unit = "%", .value = 35.0f }); // 5 .. 1000 ms
    Param& filterFm = addParam({ .key = "filterFm", .label = "FM", .unit = "%" }); // the signal modulates both cutoffs, up to 7 oct
    Param& filterDrive = addParam({ .key = "filterDrive", .label = "Filter Drive", .unit = "%", .value = 15.0f }); // saturation after the filter
    Param& crush = addParam({ .key = "crush", .label = "Crush", .unit = "%" }); // sample rate, then bits (before the filter)

    Kick32v3(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "Kick32v3", params)
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
        crushPhase = 0.0f;

        bounceN = 0;
        hitGain = 1.0f;
        bounceOct = 0.0f;
        hitSpacing = bounceMsOf() * sampleRate * 0.001f;
        nextHit = hitSpacing;
        subPhase = 0.0f;
        subEnv = 1.0f;
        subCoef = decayCoef(subMsOf());
        // filters, comb and followers are not reset: no click over a ringing filter
    }

    void noteOffImpl(uint8_t) { }

    // ------------------------------------------------------------------ UI previews
    float waveSample(float p) { return bodyWave(p); }
    float drawImpl(float x) { return bodyWave(x); }
    float dropMs() { return dropMsOf(); }
    // time (ms) and level of rebound n (1..3); n = 0 is the hit
    float bounceAtMs(int n)
    {
        float t = 0.0f, sp = bounceMsOf(), acc = fastExp2(bounceAccel.value * 0.01f);
        for (int i = 0; i < n; ++i) { t += sp; sp *= acc; }
        return t;
    }
    float bounceLevel(int n) { return n == 0 ? 1.0f : pct(bounce) * std::pow(0.75f, (float)(n - 1)); }
    float subMs() { return subMsOf(); }
    // octaves above the note at `ms` after the hit
    float pitchOctAt(float ms) { return cDropOct * dropShape(ms / dropMsOf()); }
    float ampAt(float ms) { float t = ms / decay.value; return t >= 1.0f ? 0.0f : ampShape(t); }
    // the drive transfer curve for an input -1..1
    float shaperPreview(float x) { return saturate(x * cGain, cChar); }
    float sr() const { return sampleRate; }
    float filterCutoff1Hz() const { return cCut1; }
    float filterCutoff2Hz() const { return cCut2; }
    float filterK() const { return cK; }
    float filterGain() const { return cResoComp * 0.8f * 1.25f; }
    float filterDriveGain() const { return cFiltDrive; }
    float clickHz() const { return cClickHz; }

    float sampleImpl()
    {
        if (ctl-- <= 0) {
            ctl = CTL_RATE;
            controlRate();
        }

        // ---- BOUNCE: the hit rebounds (envelopes restart, a little lower and a few semitones away)
        hitPos++;
        if (cBounce > 0.0f && bounceN < 3 && (float)hitPos >= nextHit) {
            bounceN++;
            hitGain = cBounce * (bounceN == 1 ? 1.0f : (bounceN == 2 ? 0.75f : 0.5625f));
            bounceOct = bounceN * cBounceStep;
            hitSpacing *= cBounceAccel;
            nextHit += hitSpacing;
            ampPos = 0;
            dropPos = 0;
            phase = 0.0f;
            clickEnv = noiseEnv = punchEnv = filtEnv = 1.0f;
        }

        // ---- envelopes
        float ampT = 1.0f;
        if (ampPos < ampLen) {
            ampT = (float)ampPos / ampLen;
            ampEnv = ampShape(ampT);
            ampPos++;
        } else {
            ampEnv = 0.0f;
        }
        dropEnv = dropPos < dropLen ? dropShape((float)dropPos++ / dropLen) : 0.0f;
        clickEnv *= clickCoef;
        noiseEnv *= noiseCoef;
        punchEnv *= punchCoef;
        filtEnv *= filtCoef;

        float bodyOct = noteOct + bounceOct + cDropOct * dropEnv;
        float freq = std::min(ROOT_HZ * fastExp2(bodyOct), sampleRate * 0.45f);

        // ---- BODY
        float mix = 0.0f;
        if (ampEnv > 0.0f) {
            phase += freq * invSr;
            phase -= std::floor(phase);
            mix = bodyWave(phase) * ampEnv;
        }

        // ---- CLICK: a short sine burst, one octave of its own drop
        if (cClick > 0.0f && clickEnv > 0.001f) {
            clickPhase += cClickHz * (1.0f + clickEnv) * invSr;
            clickPhase -= std::floor(clickPhase);
            mix += cClick * 1.6f * clickEnv * sin2pi(clickPhase);
        }

        // ---- NOISE: white -> SVF (LP -> BP -> HP by Color)
        float noiseGate = 0.0f;
        if (cNoise > 0.0f && noiseEnv > 0.0005f) {
            float lp, bp, hp;
            noiseSvf.tick(white(), cNoiseG, cNoiseK, lp, bp, hp);
            float n = lp * cNoiseLp + bp * cNoiseK * 1.5f * cNoiseBp + hp * cNoiseHp;
            mix += cNoise * 2.2f * n * noiseEnv;
            noiseGate = noiseEnv * std::min(1.0f, cNoise * 3.0f);
        }

        mix *= hitGain;

        // ---- FEEDBACK: a comb tuned to the pitch (one period), through the drive. + reinforces the pitch, - half harmonics
        float fbIn = 0.0f;
        if (cFeedback != 0.0f && ampEnv > 0.0f) {
            float len = std::clamp(sampleRate / freq, 2.0f, (float)(COMB_MAX - 2));
            fbLp += 0.3f * (combRead(len) - fbLp);
            fbIn = cFeedback * std::abs(cFeedback) * 1.4f * ampEnv * fbLp;
        }

        // ---- DRIVE: gain (with the punch envelope), character
        float g = cGain;
        if (cPunch > 0.0f) g *= 1.0f + 6.0f * cPunch * punchEnv;
        else if (cPunch < 0.0f) g *= 1.0f - 6.0f * cPunch * (1.0f - punchEnv);
        float out = saturate((mix + fbIn) * g + 1e-18f, cChar);
        satDcOut = out - satDcIn + 0.997f * satDcOut; // fold / fuzz make DC
        satDcIn = out;
        out = satDcOut;
        comb[combPos] = out;
        if (++combPos >= COMB_MAX) combPos = 0;

        // post VCA: the amp shape survives the drive (noise may outlast the body)
        float bodyGate = std::max(ampEnv * (2.0f - ampEnv), noiseGate);
        out *= bodyGate * hitGain; // ghost hits are quieter and less driven

        // ---- SUB: a clean sine under the driven kick (after the drive, so it stays clean), its own pitch, length and drop
        float subGate = 0.0f;
        lastSub = 0.0f;
        if (cSub > 0.0f && subEnv > 0.0005f) {
            subEnv *= subCoef;
            float so = noteOct + subPitch.value / 12.0f + cSubDrop * cDropOct * dropEnv;
            subPhase += std::min(ROOT_HZ * fastExp2(so), sampleRate * 0.45f) * invSr;
            subPhase -= std::floor(subPhase);
            float se = subEnv * (2.0f - subEnv);
            lastSub = cSub * 1.3f * sin2pi(subPhase) * se;
            out += lastSub;
            subGate = se * std::min(1.0f, cSub * 3.0f);
        }

        if (cCrush > 0.0f) {
            float rc = std::min(1.0f, cCrush * 2.0f);
            float step = 1.0f + rc * rc * 22.0f;
            crushPhase += 1.0f;
            if (crushPhase >= step) {
                crushPhase -= step;
                crushHold = out;
            }
            out = crushHold;
            if (cCrush > 0.5f) {
                float levels = fastExp2(lerp(10.0f, 2.0f, (cCrush - 0.5f) * 2.0f));
                out = std::round(out * levels) / levels;
            }
        }

        // ---- FILTERBANK: LP > HP, both follow the pitch (note + drop), env on the hit, audio rate FM by the signal itself
        float oct = bodyOct + cEnv * 6.0f * filtEnv + cFm * out;
        float m = fastExp2(oct);
        float lp1, bp1, hp1, lp2, bp2, hp2;
        f1.tick(out, gOf(cCut1 * m), cK, lp1, bp1, hp1);
        f2.tick(lp1, gOf(cCut2 * m), cK, lp2, bp2, hp2);
        float y = 1.25f * tanh(hp2 * cResoComp * 0.8f * cFiltDrive); // filter drive: resonance growls instead of whistling

        // the ring of the filters dies after the kick: a drum module must not drone
        float gate = std::max(bodyGate, subGate);
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
