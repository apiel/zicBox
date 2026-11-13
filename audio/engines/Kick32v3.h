#pragma once

/**
 * Kick32v3 — LAYERS > ONE DRIVE > FILTERBANK. 32 pots, no menu, no switch: every choice is a morph.
 *
 * Built for tekno / hardtek / metal tek / hardcore / tribe. Two ideas borrowed from hardware:
 *   - the DROP: several sources (tone, sub, click, noise) layered into ONE saturation, with a bias
 *   - the Sherman: two resonant filters (serial <-> parallel) that track the pitch, with an envelope,
 *     audio rate FM of the cutoffs, and a drive with its own envelope
 *
 *   BODY    (8)  decay / shape, pitch drop (amount, time, curve), wave morph + harmonics 2 / 3
 *   LAYERS  (8)  noise (level, color, decay), sub (-1 oct), click oscillator (level, tone), metal FM (amount, ratio)
 *   DRIVE   (8)  drive, character (soft -> hard -> fold -> fuzz), bias, punch (drive envelope, bipolar),
 *                tone, crush, feedback (comb tuned to the pitch, bipolar), squash (expand <- -> compress)
 *   FILTER  (8)  cutoff (LP), cutoff 2 (HP serial / BP parallel), reso, track, env, env decay,
 *                route (serial <-> parallel), FM (the body oscillator modulates both cutoffs)
 *
 *   BODY (wave, harmonics, drop, metal FM) --+
 *   SUB  (-1 oct)                            +--> [+ FEEDBACK comb] --> DRIVE (punch, character, bias) --> TONE --> CRUSH
 *   CLICK                                    |                                                                   |
 *   NOISE (color, decay) --------------------+          OUT <-- clip <-- SQUASH <-- FILTER 1 (LP) =+= FILTER 2 <-+
 *                                                                                   serial: LP > HP   parallel: LP + BP
 *
 * Param index == pot index. No volume (fixed level + soft clip), no tune (pitch = note from the sequencer,
 * note 60 = ROOT_HZ), no velocity.
 *
 * No dependency on any UI or desktop API: runs as is on the Daisy Seed / STM32H7.
 * Per sample: no std::exp / pow / tan / log (fast approximations), pot dependent coefficients at control rate.
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

    // log2(x), x > 0, error < 0.01
    static float fastLog2(float x)
    {
        union { float f; uint32_t i; } u { x };
        float e = (float)(int)((u.i >> 23) & 255) - 127.0f;
        u.i = (u.i & 0x7FFFFF) | 0x3F800000; // mantissa in 1..2
        float m = u.f;
        return e + (-1.7417939f + (2.8212026f + (-1.4699568f + (0.44717955f - 0.056570851f * m) * m) * m) * m);
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
    float tauCoef(float ms) const { return std::exp(-1.0f / (sampleRate * std::max(ms, 0.05f) * 0.001f)); } // trigger / control rate only

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
    float phase = 0.0f, subPhase = 0.0f, modPhase = 0.0f, clickPhase = 0.0f;
    int ampPos = 0, ampLen = 1, dropPos = 0, dropLen = 1;
    float ampEnv = 0.0f, dropEnv = 0.0f;
    float metalEnv = 0.0f, metalCoef = 0.0f;
    float clickEnv = 0.0f, clickCoef = 0.0f;
    float noiseEnv = 0.0f, noiseCoef = 0.0f;
    float punchEnv = 0.0f, punchCoef = 0.0f;
    float filtEnv = 0.0f, filtCoef = 0.0f;
    float tailGate = 0.0f, tailCoef = 0.0f;
    Svf noiseSvf;

    // drive
    float comb[COMB_MAX] = {};
    int combPos = 0;
    float fbLp = 0.0f;
    float biasEnv = 0.0f;
    float satDcIn = 0.0f, satDcOut = 0.0f;
    Svf toneSvf;
    float crushPhase = 0.0f, crushHold = 0.0f;

    // filterbank
    Svf f1, f2s, f2p;
    float sqEnv = 0.0f;
    float dcIn = 0.0f, dcOut = 0.0f;

    // ------------------------------------------------------------------ control rate cache (pot dependent)
    int ctl = 0;
    float cDropOct = 0, cWave = 0, cH2 = 0, cH3 = 0;
    float cSub = 0, cClick = 0, cClickHz = 300, cNoise = 0, cNoiseG = 0.1f, cNoiseK = 0.7f, cNoiseLp = 1, cNoiseBp = 0, cNoiseHp = 0;
    float cMetal = 0, cMetalRatio = 1;
    float cGain = 1, cChar = 0, cBias = 0, cPunch = 0, cToneG = 1, cTone = 1, cCrush = 0, cFeedback = 0, cSquash = 0;
    float cCut1 = 20000, cCut2 = 15, cK = 1, cTrack = 0, cEnv = 0, cRoute = 0, cFm = 0, cResoComp = 1;

    void controlRate()
    {
        float d = pct(drop);
        cDropOct = 6.0f * d * std::sqrt(d); // 0 .. 6 oct
        cWave = pct(wave);
        cH2 = harm2.value * 0.01f;
        cH3 = harm3.value * 0.01f;

        cSub = pct(sub);
        cClick = pct(click);
        cClickHz = 300.0f * fastExp2(pct(clickTone) * 4.4f); // 300 Hz .. 6.3 kHz
        cNoise = pct(noise);
        float color = pct(noiseColor);
        cNoiseG = gOf(100.0f * fastExp2(color * 6.6f)); // 100 Hz .. 10 kHz
        cNoiseLp = clamp01(1.0f - color * 2.0f);
        cNoiseHp = clamp01(color * 2.0f - 1.0f);
        cNoiseBp = 1.0f - std::abs(color * 2.0f - 1.0f);
        cMetal = pct(metal);
        cMetalRatio = 0.5f * fastExp2(pct(metalRatio) * 4.0f); // 0.5 .. 8x, in between = metal

        float dv = pct(drive);
        cGain = fastExp2(dv * 7.0f); // 1 .. 128
        cChar = pct(character) * 3.0f;
        cBias = bias.value * 0.01f;
        cPunch = punch.value * 0.01f;
        cTone = pct(tone);
        cToneG = gOf(150.0f * fastExp2(cTone * 7.0f)); // 150 Hz .. 19 kHz
        cCrush = pct(crush);
        cFeedback = feedback.value * 0.01f;
        cSquash = squash.value * 0.01f;

        cCut1 = 30.0f * fastExp2(pct(cutoff) * 9.4f); // 30 Hz .. 20 kHz
        cCut2 = 15.0f * fastExp2(pct(cutoff2) * 9.0f); // 15 Hz .. 7.7 kHz
        float r = pct(reso);
        cK = lerp(1.5f, 0.02f, std::sqrt(r));
        cResoComp = 0.6f + 0.4f * std::sqrt(cK / 1.5f);
        cTrack = pct(track);
        cEnv = env.value * 0.01f;
        cRoute = pct(route);
        float fmv = pct(filterFm);
        cFm = fmv * fmv * 4.0f; // up to +-4 octaves at audio rate
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
        if (cH2 != 0.0f) w += cH2 * 1.2f * sin2pi(2.0f * p);
        if (cH3 != 0.0f) w += cH3 * 1.2f * sin2pi(3.0f * p);
        return w;
    }

    // The one saturation. x is already gained (and biased). Character: soft -> hard -> fold -> fuzz (octave up)
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
    float lastOut = 0.0f, lastBody = 0.0f;

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

    // ================================================================ LAYERS (pots 8..15), all go into the same drive
    Param& noise = addParam({ .key = "noise", .label = "Noise", .unit = "%" });
    Param& noiseColor = addParam({ .key = "noiseColor", .label = "Color", .unit = "%", .value = 50.0f }); // LP -> BP -> HP, cutoff rising
    Param& noiseDecay = addParam({ .key = "noiseDecay", .label = "Noise Decay", .unit = "%", .value = 30.0f }); // 3 ms .. 1.5 s
    Param& sub = addParam({ .key = "sub", .label = "Sub", .unit = "%" }); // sine one octave below the body
    Param& click = addParam({ .key = "click", .label = "Click", .unit = "%", .value = 25.0f }); // click oscillator on the hit
    Param& clickTone = addParam({ .key = "clickTone", .label = "Click Tone", .unit = "%", .value = 50.0f }); // knock -> tick
    Param& metal = addParam({ .key = "metal", .label = "Metal", .unit = "%" }); // FM of the body, decaying
    Param& metalRatio = addParam({ .key = "metalRatio", .label = "Metal Ratio", .unit = "%", .value = 25.0f }); // 0.5 .. 8x

    // ================================================================ DRIVE (pots 16..23)
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 40.0f }); // gain 1 .. 128
    Param& character = addParam({ .key = "character", .label = "Character", .unit = "%", .value = 10.0f }); // soft -> hard -> fold -> fuzz
    Param& bias = addParam({ .key = "bias", .label = "Bias", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // asymmetry
    Param& punch = addParam({ .key = "punch", .label = "Punch", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + dirty hit, - dirty tail
    Param& tone = addParam({ .key = "tone", .label = "Tone", .unit = "%", .value = 80.0f }); // low pass after the drive, 100 = open
    Param& crush = addParam({ .key = "crush", .label = "Crush", .unit = "%" }); // sample rate, then bits
    Param& feedback = addParam({ .key = "feedback", .label = "Feedback", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // comb tuned to the pitch
    Param& squash = addParam({ .key = "squash", .label = "Squash", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // - compress, + expand

    // ================================================================ FILTER (pots 24..31)
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 100.0f }); // filter 1, low pass
    Param& cutoff2 = addParam({ .key = "cutoff2", .label = "Cutoff 2", .unit = "%", .value = 0.0f }); // filter 2: HP (serial) / BP (parallel)
    Param& reso = addParam({ .key = "reso", .label = "Reso", .unit = "%", .value = 25.0f });
    Param& track = addParam({ .key = "track", .label = "Track", .unit = "%", .value = 50.0f }); // cutoffs follow the note + the drop
    Param& env = addParam({ .key = "env", .label = "Env", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + opens on the hit, - closes
    Param& envDecay = addParam({ .key = "envDecay", .label = "Env Decay", .unit = "%", .value = 35.0f }); // 5 .. 1000 ms
    Param& route = addParam({ .key = "route", .label = "Route", .unit = "%", .value = 0.0f }); // serial (LP > HP) <-> parallel (LP + BP)
    Param& filterFm = addParam({ .key = "filterFm", .label = "FM", .unit = "%" }); // the body oscillator modulates both cutoffs

    Kick32v3(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "Kick32v3", params)
        , sampleRate(sampleRate)
        , invSr(1.0f / sampleRate)
    {
        tailCoef = tauCoef(150.0f);
        controlRate();
    }

    void trigger() { noteOnImpl(60, 1.0f); }

    void noteOnImpl(uint8_t note, float)
    {
        controlRate();
        noteOct = ((float)note - 60.0f) / 12.0f;
        phase = subPhase = modPhase = clickPhase = 0.0f;
        ampPos = 0;
        ampLen = std::max(1, (int)(sampleRate * decay.value * 0.001f));
        dropPos = 0;
        dropLen = std::max(1, (int)(sampleRate * dropMsOf() * 0.001f));
        metalEnv = 1.0f;
        metalCoef = tauCoef(60.0f);
        clickEnv = 1.0f;
        clickCoef = tauCoef(1.2f + 7.0f * (1.0f - pct(clickTone))); // tick 1.2 ms .. knock 8 ms
        noiseEnv = 1.0f;
        noiseCoef = tauCoef(3.0f * fastExp2(pct(noiseDecay) * 9.0f) * 0.4f); // 3 ms .. 1.5 s (-60 dB)
        punchEnv = 1.0f;
        punchCoef = tauCoef(std::max(15.0f, decay.value * 0.2f));
        filtEnv = 1.0f;
        filtCoef = tauCoef(5.0f * fastExp2(pct(envDecay) * 7.6f)); // 5 .. 1000 ms
        tailGate = 1.0f;
        crushPhase = 0.0f;
        // filters, comb and followers are not reset: no click over a ringing filter
    }

    void noteOffImpl(uint8_t) { }

    // ------------------------------------------------------------------ UI previews
    float waveSample(float p) { return bodyWave(p); }
    float drawImpl(float x) { return bodyWave(x); }
    float dropMs() { return dropMsOf(); }
    // octaves above the note at `ms` after the hit
    float pitchOctAt(float ms) { return cDropOct * dropShape(ms / dropMsOf()); }
    float ampAt(float ms) { float t = ms / decay.value; return t >= 1.0f ? 0.0f : ampShape(t); }
    // the drive transfer curve for an input -1..1 (static bias)
    float shaperPreview(float x) { return saturate(x * cGain + cBias * 2.0f, cChar); }
    float sr() const { return sampleRate; }
    float filterCutoff1Hz() const { return cCut1; }
    float filterCutoff2Hz() const { return cCut2; }
    float filterK() const { return cK; }
    float filterRoute() const { return cRoute; }
    float filterComp() const { return cResoComp; }
    float clickHz() const { return cClickHz; }
    float noiseWeights(float& lp, float& bp, float& hp) const { lp = cNoiseLp; bp = cNoiseBp; hp = cNoiseHp; return cNoiseG; }
    float noiseMs() { return 3.0f * fastExp2(pct(noiseDecay) * 9.0f); }

    float sampleImpl()
    {
        if (ctl-- <= 0) {
            ctl = CTL_RATE;
            controlRate();
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
        metalEnv *= metalCoef;
        clickEnv *= clickCoef;
        noiseEnv *= noiseCoef;
        punchEnv *= punchCoef;
        filtEnv *= filtCoef;

        float bodyOct = noteOct + cDropOct * dropEnv;
        float freq = std::min(ROOT_HZ * fastExp2(bodyOct), sampleRate * 0.45f);

        // ---- BODY + SUB
        float body = 0.0f, raw = 0.0f;
        if (ampEnv > 0.0f) {
            float pm = 0.0f;
            if (cMetal > 0.0f) {
                modPhase += freq * cMetalRatio * invSr;
                modPhase -= std::floor(modPhase);
                pm = cMetal * (0.5f + cMetal) * 0.6f * (0.3f * ampEnv + 0.7f * metalEnv) * sin2pi(modPhase);
            }
            phase += freq * invSr;
            phase -= std::floor(phase);
            raw = bodyWave(phase + pm);
            if (cSub > 0.0f) {
                subPhase += 0.5f * freq * invSr;
                subPhase -= std::floor(subPhase);
                raw += cSub * 1.1f * sin2pi(subPhase);
            }
            body = raw * ampEnv;
        }
        lastBody = body;

        // ---- CLICK: a short sine burst, one octave of its own drop
        float mix = body;
        if (cClick > 0.0f && clickEnv > 0.001f) {
            clickPhase += cClickHz * (1.0f + clickEnv) * invSr;
            clickPhase -= std::floor(clickPhase);
            mix += cClick * 1.6f * clickEnv * sin2pi(clickPhase);
        }

        // ---- NOISE: white -> SVF (LP -> BP -> HP by Color), its own decay
        float noiseGate = 0.0f;
        if (cNoise > 0.0f && noiseEnv > 0.0005f) {
            float lp, bp, hp;
            noiseSvf.tick(white(), cNoiseG, cNoiseK, lp, bp, hp);
            float n = lp * cNoiseLp + bp * cNoiseK * 1.5f * cNoiseBp + hp * cNoiseHp;
            mix += cNoise * 2.2f * n * noiseEnv;
            noiseGate = noiseEnv * std::min(1.0f, cNoise * 3.0f);
        }

        // ---- FEEDBACK: a comb tuned to the pitch (one period), through the drive. + reinforces the pitch, - half harmonics
        float fbIn = 0.0f;
        if (cFeedback != 0.0f && ampEnv > 0.0f) {
            float len = std::clamp(sampleRate / freq, 2.0f, (float)(COMB_MAX - 2));
            fbLp += 0.3f * (combRead(len) - fbLp);
            fbIn = cFeedback * std::abs(cFeedback) * 1.4f * ampEnv * fbLp;
        }

        // ---- DRIVE: gain (with the punch envelope), bias, character
        float g = cGain;
        if (cPunch > 0.0f) g *= 1.0f + 6.0f * cPunch * punchEnv;
        else if (cPunch < 0.0f) g *= 1.0f - 6.0f * cPunch * (1.0f - punchEnv);
        float x = (mix + fbIn) * g;
        float ax = std::min(std::abs(x), 1.0f);
        biasEnv += (ax > biasEnv ? 0.3f : 0.002f) * (ax - biasEnv); // bias fades with the signal: no DC in the silence
        x += cBias * 2.0f * biasEnv;
        float out = saturate(x + 1e-18f, cChar);
        satDcOut = out - satDcIn + 0.997f * satDcOut; // fold / fuzz / bias make DC
        satDcIn = out;
        out = satDcOut;
        comb[combPos] = out;
        if (++combPos >= COMB_MAX) combPos = 0;

        if (cTone < 0.995f) {
            float lp, bp, hp;
            toneSvf.tick(out, cToneG, 1.2f, lp, bp, hp);
            out = lp;
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

        // post VCA: the amp shape survives the drive (noise may outlast the body)
        float gate = std::max(ampEnv * (2.0f - ampEnv), noiseGate);
        out *= gate;

        // ---- FILTERBANK: two tracking filters, serial (LP > HP) <-> parallel (LP + BP), audio rate FM from the body
        float oct = cTrack * bodyOct + cEnv * 6.0f * filtEnv + cFm * raw * ampEnv;
        float m = fastExp2(oct);
        float g1 = gOf(cCut1 * m), g2 = gOf(cCut2 * m);
        float lp1, bp1, hp1, lp2, bp2, hp2, y;
        f1.tick(out, g1, cK, lp1, bp1, hp1);
        if (cRoute < 0.999f) {
            f2s.tick(lp1, g2, cK, lp2, bp2, hp2);
            y = hp2;
            if (cRoute > 0.001f) {
                float plp, pbp, php;
                f2p.tick(out, g2, cK, plp, pbp, php);
                y = lerp(y, lp1 + pbp, cRoute);
            }
        } else {
            f2p.tick(out, g2, cK, lp2, bp2, hp2);
            y = lp1 + bp2;
        }
        y = 1.25f * tanh(y * cResoComp * 0.8f); // the filter drives a little: resonance growls instead of whistling

        // the ring of the filters dies after the kick: a drum module must not drone
        tailGate = gate > 0.0005f ? 1.0f : tailGate * tailCoef;
        y *= tailGate;

        // ---- SQUASH: - compress (the tail comes up), + expand (gated, dry)
        float ay = std::abs(y);
        sqEnv += (ay > sqEnv ? 0.05f : 0.0004f) * (ay - sqEnv);
        if (cSquash != 0.0f) {
            float e = std::clamp(sqEnv, 0.02f, 1.5f);
            float ex = std::clamp(cSquash * 1.3f * fastLog2(e * 1.6667f), -6.0f, 1.5f);
            y *= fastExp2(ex);
        }

        // ---- output: DC + soft clip (fixed level, never above 0.95). An asymmetric wave (Bias) clips
        // asymmetrically and keeps a little DC: inherent to the design, the analog output is AC coupled.
        dcOut = y - dcIn + 0.9985f * dcOut;
        dcIn = y;
        float result = 0.95f * tanh(dcOut * 1.5f);
        meterOut = std::max(std::abs(result), meterOut * 0.9985f);
        lastOut = result;
        return result;
    }
};
