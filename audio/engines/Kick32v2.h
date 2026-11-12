#pragma once

/**
 * Kick32v2 — a layered kick forge: a PUNCH (the body) and a TAIL that takes over. 32 pots, no menu, no switch.
 *
 * Built for tekno / hardtek / metal tek / hardcore / tribe. Hard kicks are built from two layers:
 * a punchy body for the hit and a tonal, distorted tail. Here the tail is a second voice that
 * crossfades in at Start, with its own pitch, wave, unison stack, dirt and formant.
 *
 *   BODY   (16)  amp / FM, pitch drop, waveform design (wave + harmonics + feedback), dirt
 *   TAIL   (8)   takeover amount + start, pitch + glide, shape + stack, dirt + tone (LP <- -> formant)
 *   MASTER (8)   DJ filter (LP <- off -> HP) + reso + env + env decay (cutoff follows the pitch),
 *                spread, rumble (level / length / pump)
 *
 *   BODY osc > DRIVE > TONE > CRUSH --+
 *                                     +-> crossfade at Start > VCA > FILTER -+-> punch/clip > OUT
 *   TAIL osc (stack) > DIRT > TONE ---+                                      +-> RUMBLE -+
 *
 * Param index == pot index. No volume, no tune (pitch = note from the sequencer, note 60 = ROOT_HZ),
 * no velocity. Punch and the output clipper are fixed (always wanted at max).
 *
 * No dependency on any UI or desktop API: runs as is on the Daisy Seed / STM32H7.
 * Per sample: no std::exp / pow / tan (fast approximations); pot dependent coefficients at control rate.
 */

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Kick32v2 : public EngineBase<Kick32v2> {
public:
    static constexpr int PARAM_COUNT = 32;
    static constexpr float ROOT_HZ = 52.0f; // frequency of note 60

protected:
    static constexpr float TWO_PI = 6.28318530718f;

    const float sampleRate;
    const float invSr;

    // ------------------------------------------------------------------ fast math
    static float lerp(float a, float b, float t) { return a + t * (b - a); }

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

    static float fastTan(float x) { float x2 = x * x; return x * (15.0f - x2) / (15.0f - 6.0f * x2); }
    static float tanh(float x) { return Math::fastTanh(x); }
    float gOf(float hz) const { return fastTan((float)M_PI * std::clamp(hz, 15.0f, sampleRate * 0.42f) * invSr); }
    float tauCoef(float ms) const { return std::exp(-1.0f / (sampleRate * std::max(ms, 0.05f) * 0.001f)); }

    uint32_t rng = 22222;
    float white()
    {
        rng = rng * 196314165 + 907633389;
        return (float)int32_t(rng) * (1.0f / 2147483648.0f);
    }

    // Zero delay feedback SVF (Cytomic). Band state soft clipped: full resonance screams, never explodes.
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

    static float tri(float p) { return p < 0.25f ? 4.0f * p : (p < 0.75f ? 2.0f - 4.0f * p : 4.0f * p - 4.0f); }

    // ------------------------------------------------------------------ body state
    float noteOct = 0.0f;
    float phase = 0.0f, modPhase = 0.0f, fbLast = 0.0f;
    int ampPos = 0, ampLen = 1, dropPos = 0, dropLen = 1;
    float ampEnv = 0.0f, dropEnv = 0.0f;
    float fmEnv = 0.0f, fmCoef = 0.0f;
    float atkEnv = 0.0f, atkCoef = 0.0f;
    Svf toneSvf;
    float crushPhase = 0.0f, crushHold = 0.0f;
    float bodyDcIn = 0.0f, bodyDcOut = 0.0f;

    // ------------------------------------------------------------------ tail state
    int tailStartPos = 0;
    float tailPhase[3] = { 0, 0, 0 };
    float tailOct = 0.0f; // current tail pitch (glides)
    float glideCoef = 0.0f;
    bool tailStarted = false;
    Svf tailSvf;
    float filtEnv = 0.0f, filtCoef = 0.0f;

    // ------------------------------------------------------------------ master state
    Svf f1, f2;
    static constexpr int FDN_MAX = 2700;
    float fdn[4][FDN_MAX] = {};
    int fdnLen[4] = { 1117, 1543, 1871, 2251 };
    int fdnPos[4] = { 0, 0, 0, 0 };
    float fdnDamp[4] = { 0, 0, 0, 0 };
    float fdnG[4] = { 0, 0, 0, 0 };
    float rumbleLp1 = 0.0f, rumbleLp2 = 0.0f, rumbleHp = 0.0f, duck = 0.0f;
    float rumbleLpCoef = 0.02f, rumbleHpCoef = 0.005f, duckCoef = 0.99f;
    float dcIn = 0.0f, dcOut = 0.0f;

    int ctl = 0;
    float cRumbleDecay = -1.0f, cPump = -1.0f;

    void controlRate()
    {
        if (rumbleDecay.value != cRumbleDecay) {
            cRumbleDecay = rumbleDecay.value;
            float rt = 0.3f * fastExp2(pct(rumbleDecay) * 4.0f); // 0.3 .. 4.8 s
            for (int i = 0; i < 4; ++i) fdnG[i] = std::pow(10.0f, -3.0f * fdnLen[i] / (sampleRate * rt));
        }
        if (pump.value != cPump) {
            cPump = pump.value;
            duckCoef = tauCoef(40.0f + 260.0f * pct(pump));
        }
    }

    // ------------------------------------------------------------------ body building blocks

    // Hold 0 = punchy exponential, 50 = linear, 100 = gated (stays up, chopped at the end)
    float ampShape(float t)
    {
        float h = pct(hold);
        float H = h * h * 0.85f;
        if (t < H) return 1.0f;
        float s = (t - H) / (1.0f - H);
        float c = 1.0f - s;
        float punchy = c * c * c * c;
        float round = 1.0f - s * s;
        return h < 0.5f ? lerp(punchy, c, h * 2.0f) : lerp(c, round, (h - 0.5f) * 2.0f);
    }

    // Drop envelope 0..1 over Drop Time. Curve 0 = laser (holds then dives), 50 = linear, 100 = zap.
    float dropShape(float t)
    {
        if (t >= 1.0f) return 0.0f;
        float c = 1.0f - t, cv = pct(dropCurve);
        float lazer = 1.0f - t * t * t;
        float c2 = c * c;
        float zap = c2 * c2 * c2;
        return cv < 0.5f ? lerp(lazer, c, cv * 2.0f) : lerp(c, zap, (cv - 0.5f) * 2.0f);
    }

    float dropOct() { float d = pct(drop); return 6.0f * d * std::sqrt(d); } // 0 .. 6 oct
    float dropMs() { return 4.0f * fastExp2(pct(dropTime) * 7.0f); }          // 4 .. 512 ms
    float bendOct(float t) { return -pct(bend) * 2.0f * t; }                    // slow fall, up to -2 oct

    // Body waveform: wave morph sine -> triangle -> square -> saw, then harmonics 2 and 3 (bipolar, strong).
    // Everything starts at 0: no click unless asked for.
    float bodyWave(float p)
    {
        p -= std::floor(p);
        float s = sin2pi(p);
        float x = pct(wave) * 3.0f;
        float w;
        if (x < 1.0f) {
            w = lerp(s, tri(p), x);
        } else if (x < 2.0f) {
            w = lerp(tri(p), tanh(s * 12.0f), x - 1.0f);
        } else {
            float saw = 2.0f * (p < 0.5f ? p : p - 1.0f);
            w = lerp(tanh(s * 12.0f), saw, x - 2.0f);
        }
        float h2 = harm2.value * 0.01f, h3 = harm3.value * 0.01f;
        if (h2 != 0.0f) w += h2 * 1.2f * sin2pi(2.0f * p);
        if (h3 != 0.0f) w += h3 * 1.2f * sin2pi(3.0f * p);
        return w;
    }

    // Drive with a character morph: soft tube -> hard clip -> wavefold -> rectified fuzz (octave up)
    float dirt(float x)
    {
        float dv = pct(drive);
        float g = fastExp2(dv * 7.0f); // 1 .. 128
        float c = pct(character) * 3.0f;
        float gx = g * x;
        float soft = tanh(gx);
        float hard = std::clamp(gx, -1.0f, 1.0f);
        if (c < 1.0f) return lerp(soft, hard, c);
        float folded = sin2pi(x * (0.25f + dv * 2.0f));
        if (c < 2.0f) return lerp(hard, folded, c - 1.0f);
        float fuzz = tanh(std::abs(gx) * 2.0f) * 2.0f - 1.0f;
        return lerp(folded, fuzz, c - 2.0f);
    }

    // ------------------------------------------------------------------ tail
    // Shape morph: sine -> triangle -> saw -> square -> narrow pulse (nasal, hoover like)
    float tailWave(float p)
    {
        p -= std::floor(p);
        float x = pct(tailShape) * 4.0f;
        int i = std::min(3, (int)x);
        float f = x - i;
        auto w = [&](int k) -> float {
            switch (k) {
            case 0: return sin2pi(p);
            case 1: return tri(p);
            case 2: return 2.0f * (p < 0.5f ? p : p - 1.0f);
            case 3: return tanh(sin2pi(p) * 10.0f);
            default: { float q = p + 0.125f; q -= std::floor(q); return q < 0.25f ? 1.0f : -0.33f; }
            }
        };
        float a = w(i);
        return f > 0.001f ? lerp(a, w(i + 1), f) : a;
    }

    float tailStartMsOf() { float x = pct(tailStart); return 5.0f + 295.0f * x * x; } // 5 .. 300 ms

public:
    // UI helpers
    float meterOut = 0.0f;
    float lastLayer = 0.0f, lastOut = 0.0f;

    Param params[PARAM_COUNT];

    // ================================================================ BODY (pots 0..15, a 4 x 4 block)
    // AMP + FM
    Param& decay = addParam({ .key = "decay", .label = "Decay", .unit = "ms", .value = 300.0f, .min = 40.0f, .max = 2000.0f, .step = 10.0f });
    Param& hold = addParam({ .key = "hold", .label = "Hold", .unit = "%", .value = 35.0f });
    Param& fm = addParam({ .key = "fm", .label = "FM", .unit = "%", .value = 0.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "%", .value = 25.0f }); // 0.5 .. 8x, in between = metal
    // PITCH
    Param& drop = addParam({ .key = "drop", .label = "Drop", .unit = "%", .value = 55.0f });
    Param& dropTime = addParam({ .key = "dropTime", .label = "Drop Time", .unit = "%", .value = 42.0f });
    Param& dropCurve = addParam({ .key = "dropCurve", .label = "Drop Curve", .unit = "%", .value = 72.0f });
    Param& bend = addParam({ .key = "bend", .label = "Bend", .unit = "%" }); // slow fall over the whole decay
    // WAVE
    Param& wave = addParam({ .key = "wave", .label = "Wave", .unit = "%" }); // sine -> tri -> square -> saw
    Param& harm2 = addParam({ .key = "harm2", .label = "Harm 2", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& harm3 = addParam({ .key = "harm3", .label = "Harm 3", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& feedback = addParam({ .key = "feedback", .label = "Feedback", .unit = "%" }); // osc modulates its own phase: sine -> saw -> chaos
    // DIRT
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 40.0f });
    Param& character = addParam({ .key = "character", .label = "Character", .unit = "%" }); // soft -> hard -> fold -> fuzz
    Param& tone = addParam({ .key = "tone", .label = "Tone", .unit = "%", .value = 75.0f }); // low pass after the drive, 100 = open
    Param& crush = addParam({ .key = "crush", .label = "Crush", .unit = "%" });

    // ================================================================ TAIL (pots 16..23)
    // VOICE
    Param& tail = addParam({ .key = "tail", .label = "Tail", .unit = "%", .value = 60.0f });             // how much the tail takes over
    Param& tailStart = addParam({ .key = "tailStart", .label = "Start", .unit = "%", .value = 35.0f });  // 5 .. 300 ms
    Param& tailPitch = addParam({ .key = "tailPitch", .label = "Pitch", .unit = "st", .value = 0.0f, .min = -12.0f, .max = 24.0f });
    Param& glide = addParam({ .key = "glide", .label = "Glide", .unit = "%", .value = 20.0f });          // from the body pitch to the tail pitch
    // TIMBRE
    Param& tailShape = addParam({ .key = "tailShape", .label = "Shape", .unit = "%", .value = 50.0f });  // sine -> tri -> saw -> square -> pulse
    Param& stack = addParam({ .key = "stack", .label = "Stack", .unit = "%", .value = 0.0f });           // 3 voice unison detune
    Param& tailDirt = addParam({ .key = "tailDirt", .label = "Dirt", .unit = "%", .value = 50.0f });
    Param& tailTone = addParam({ .key = "tailTone", .label = "Tone", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // LP <- -> formant (screech)

    // ================================================================ MASTER (pots 24..31)
    // FILTER: the cutoff follows the pitch drop (and the note), the env opens it on the hit
    Param& filter = addParam({ .key = "filter", .label = "Filter", .unit = "%", .value = -40.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // DJ: LP <- off -> HP
    Param& reso = addParam({ .key = "reso", .label = "Reso", .unit = "%", .value = 30.0f });
    Param& env = addParam({ .key = "env", .label = "Env", .unit = "%", .value = 45.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // + opens on the hit, - closes
    Param& envDecay = addParam({ .key = "envDecay", .label = "Env Decay", .unit = "%", .value = 35.0f }); // 5 .. 1000 ms
    // SPREAD / RUMBLE
    Param& spread = addParam({ .key = "spread", .label = "Spread", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED }); // - serial HP closing in / + formant peak sliding down
    Param& rumble = addParam({ .key = "rumble", .label = "Rumble", .unit = "%" });
    Param& rumbleDecay = addParam({ .key = "rumbleDecay", .label = "Length", .unit = "%", .value = 45.0f });
    Param& pump = addParam({ .key = "pump", .label = "Pump", .unit = "%", .value = 50.0f });

    Kick32v2(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "Kick32v2", params)
        , sampleRate(sampleRate)
        , invSr(1.0f / sampleRate)
    {
        for (int i = 0; i < 4; ++i) fdnLen[i] = std::min(FDN_MAX - 1, (int)(fdnLen[i] * sampleRate / 44100.0f));
        rumbleHpCoef = 1.0f - std::exp(-TWO_PI * 35.0f / sampleRate);
        rumbleLpCoef = 1.0f - std::exp(-TWO_PI * 170.0f / sampleRate);
        controlRate();
    }

    void trigger() { noteOnImpl(60, 1.0f); }

    void noteOnImpl(uint8_t note, float)
    {
        noteOct = ((float)note - 60.0f) / 12.0f;
        phase = modPhase = 0.0f;
        fbLast = 0.0f;
        ampPos = 0;
        ampLen = std::max(1, (int)(sampleRate * decay.value * 0.001f));
        dropPos = 0;
        dropLen = std::max(1, (int)(sampleRate * dropMs() * 0.001f));
        fmEnv = 1.0f;
        fmCoef = tauCoef(50.0f);
        atkEnv = 1.0f;
        atkCoef = tauCoef(12.0f);
        filtEnv = 1.0f;
        filtCoef = tauCoef(5.0f * fastExp2(pct(envDecay) * 7.6f)); // 5 .. 1000 ms
        crushPhase = 0.0f;

        tailStartPos = (int)(tailStartMsOf() * sampleRate * 0.001f);
        tailStarted = false;
        float gl = pct(glide);
        glideCoef = gl > 0.0f ? tauCoef(400.0f * gl * gl) : 0.0f;
        duck = 1.0f;
        // filters and rumble are not reset: no click over a ringing tail
    }

    void noteOffImpl(uint8_t) { }

    /** UI previews */
    float waveSample(float p) { return bodyWave(p); }
    float drawImpl(float x) { return bodyWave(x); }
    float pitchRatio(float x)
    {
        float ms = decay.value * x * x;
        return fastExp2(dropOct() * dropShape(ms / dropMs()) + bendOct(x * x));
    }
    float tailStartMs() { return tailStartMsOf(); }
    float tailWaveSample(float p) { return tailWave(p); }

    float sampleImpl()
    {
        if (ctl-- <= 0) {
            ctl = 32;
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
        fmEnv *= fmCoef;
        atkEnv *= atkCoef;
        filtEnv *= filtCoef;

        float bodyOct = noteOct + dropOct() * dropEnv + bendOct(ampT);
        float freq = std::min(ROOT_HZ * fastExp2(bodyOct), sampleRate * 0.45f);

        // ---- BODY oscillator
        float body = 0.0f, raw = 0.0f;
        if (ampEnv > 0.0f) {
            float pm = 0.0f;
            float fmA = pct(fm);
            if (fmA > 0.0f) {
                float ratio = 0.5f * fastExp2(pct(fmRatio) * 4.0f);
                modPhase += freq * ratio * invSr;
                modPhase -= std::floor(modPhase);
                pm += fmA * (0.5f + fmA) * 4.0f * (0.3f * ampEnv + 0.7f * fmEnv) * sin2pi(modPhase) * 0.159f;
            }
            float fb = pct(feedback);
            if (fb > 0.0f) pm += fbLast * fb * fb * 0.7f;
            phase += freq * invSr;
            phase -= std::floor(phase);
            raw = bodyWave(phase + pm);
            fbLast = lerp(fbLast, raw, 0.5f);
            body = raw * ampEnv;
        }

        // ---- DIRT
        float x = body + 1e-18f;
        float out = dirt(x);
        bodyDcOut = out - bodyDcIn + 0.997f * bodyDcOut; // fold / fuzz make DC
        bodyDcIn = out;
        out = bodyDcOut;

        float tn = pct(tone);
        if (tn < 0.995f) {
            float lp, bp, hp;
            toneSvf.tick(out, gOf(150.0f * fastExp2(tn * 7.0f)), 1.2f, lp, bp, hp);
            out = lp;
        }

        float cr = pct(crush);
        if (cr > 0.0f) {
            float rc = std::min(1.0f, cr * 2.0f);
            float step = 1.0f + rc * rc * 22.0f;
            crushPhase += 1.0f;
            if (crushPhase >= step) {
                crushPhase -= step;
                crushHold = out;
            }
            out = crushHold;
            if (cr > 0.5f) {
                float levels = fastExp2(lerp(10.0f, 2.0f, (cr - 0.5f) * 2.0f));
                out = std::round(out * levels) / levels;
            }
        }

        // ---- TAIL: a second voice that takes over at Start (equal power crossfade over 20 ms)
        float T = 0.0f;
        float tm = pct(tail);
        if (tm > 0.0f && ampEnv > 0.0f && ampPos >= tailStartPos) {
            float target = noteOct + tailPitch.value / 12.0f;
            if (!tailStarted) { // start where the body is: same pitch, same phase, then glide
                tailStarted = true;
                tailOct = bodyOct;
                tailPhase[0] = tailPhase[1] = tailPhase[2] = phase;
            }
            tailOct = glideCoef > 0.0f ? target + (tailOct - target) * glideCoef : target;
            float tf = std::min(ROOT_HZ * fastExp2(tailOct + bendOct(ampT)), sampleRate * 0.45f);
            float st = pct(stack);
            float det = st * st * 0.03f; // up to +-3 % (~50 cents)
            float w = 0.0f;
            const float mult[3] = { 1.0f, 1.0f + det, 1.0f - det * 1.13f };
            int voices = st > 0.0f ? 3 : 1;
            for (int v = 0; v < voices; ++v) {
                tailPhase[v] += tf * mult[v] * invSr;
                tailPhase[v] -= std::floor(tailPhase[v]);
                w += tailWave(tailPhase[v]) * (v == 0 ? 1.0f : st);
            }
            if (voices > 1) w /= 1.0f + st;
            // own dirt, then tone: LP <- open -> resonant formant on a harmonic of the tail (screech)
            float dg = fastExp2(pct(tailDirt) * 6.0f); // 1 .. 64
            w = tanh(w * dg) * (dg > 4.0f ? 1.0f : 0.6f + 0.1f * dg);
            float tt = tailTone.value * 0.01f;
            if (std::abs(tt) > 0.02f) {
                float lp, bp, hp;
                if (tt < 0.0f) {
                    tailSvf.tick(w, gOf(9000.0f * fastExp2(tt * 6.0f)), 1.0f, lp, bp, hp);
                    w = lp;
                } else {
                    tailSvf.tick(w, gOf(tf * (1.0f + tt * 11.0f)), 0.12f, lp, bp, hp);
                    w = lerp(w, w * 0.4f + bp * 0.9f, std::min(1.0f, tt * 2.0f));
                    w = tanh(w * 1.8f);
                }
            }
            float xf = std::min(1.0f, (float)(ampPos - tailStartPos) / (sampleRate * 0.02f));
            float a = xf * tm;
            out = out * std::sqrt(1.0f - a) + w * std::sqrt(a);
            T = w * std::sqrt(a);
        }

        float post = ampEnv * (2.0f - ampEnv); // post VCA: the shape survives the drive
        out *= post;
        T *= post;

        // ---- FILTER: DJ (LP <- off -> HP), follows the pitch, env on the hit, saturated output
        float fpos = filter.value * 0.01f;
        float ev = env.value * 0.01f * filtEnv;
        float trackOct = noteOct + dropOct() * dropEnv;
        float r = pct(reso);
        float k = lerp(1.6f, 0.015f, std::sqrt(r));
        float lp, bp, hp;
        float y = out;
        bool lpMode = fpos <= 0.0f;
        float fa = std::abs(fpos);
        // LP: 20 kHz .. 40 Hz, HP: 20 Hz .. 4 kHz. Env + opens the filter on the hit, - closes it.
        float fc = lpMode ? 20000.0f * fastExp2(-fa * 9.0f + ev * 7.0f + trackOct) : 20.0f * fastExp2(fa * 7.6f - ev * 7.0f + trackOct);
        fc = std::clamp(fc, 20.0f, sampleRate * 0.42f);
        if (fa > 0.01f || ev != 0.0f) {
            f1.tick(out, gOf(fc), k, lp, bp, hp);
            y = (lpMode ? lp : hp) * (0.45f + 0.55f * std::sqrt(k / 1.6f));
            y = 1.25f * tanh(y * 0.8f); // the filter drives a little: resonance growls instead of whistling
        }
        float sp = spread.value * 0.01f;
        if (sp > 0.0f) { // a formant peak sliding down from the cutoff
            f2.tick(out, gOf(std::min(fc, 2500.0f) * fastExp2(-sp * 4.0f)), lerp(0.15f, 0.04f, r), lp, bp, hp); // 2.5 kHz .. 160 Hz: a talking formant
            y += bp * 0.8f * std::min(1.0f, sp * 3.0f);
        } else if (sp < 0.0f) { // serial high pass closing in: thin, tight, a second resonance
            float fc2 = std::max(20.0f, std::min(fc, 2000.0f) * fastExp2(-5.0f * (1.0f + sp)));
            f2.tick(y, gOf(fc2), std::max(k, 0.3f), lp, bp, hp);
            y = lerp(y, hp * 1.3f, std::min(1.0f, -sp * 4.0f));
        }
        out = y;

        // ---- RUMBLE: send -> FDN -> low pass -> drive -> low cut, ducked by each hit
        float rb = pct(rumble);
        if (rb > 0.0f) {
            float rr[4];
            for (int i = 0; i < 4; ++i) rr[i] = fdn[i][fdnPos[i]];
            float sum = (rr[0] + rr[1] + rr[2] + rr[3]) * 0.5f;
            for (int i = 0; i < 4; ++i) {
                float v = (rr[i] - sum) * fdnG[i];
                fdnDamp[i] += 0.5f * (v - fdnDamp[i]);
                fdn[i][fdnPos[i]] = fdnDamp[i] + out * (i & 1 ? -0.7f : 0.7f) + 1e-18f;
                if (++fdnPos[i] >= fdnLen[i]) fdnPos[i] = 0;
            }
            float wet = (rr[0] - rr[1] + rr[2] - rr[3]) * 0.7f;
            rumbleLp1 += rumbleLpCoef * (wet - rumbleLp1);
            rumbleLp2 += rumbleLpCoef * (rumbleLp1 - rumbleLp2);
            float d = tanh(rumbleLp2 * 1.8f); // warm, not dirty
            rumbleHp += rumbleHpCoef * (d - rumbleHp);
            duck *= duckCoef;
            float depth = 0.3f + 0.7f * pct(pump);
            float g = 1.0f - depth * duck;
            out += (d - rumbleHp) * g * g * rb * 2.4f;
        }

        // ---- punch + clipper (fixed: always wanted at max) + DC
        out *= 1.0f + 0.8f * atkEnv;
        dcOut = out - dcIn + 0.9985f * dcOut;
        dcIn = out;
        float result = 0.95f * tanh(dcOut * 1.5f);
        meterOut = std::max(std::abs(result), meterOut * 0.9985f);
        lastLayer = T;
        lastOut = result;
        return result;
    }
};
