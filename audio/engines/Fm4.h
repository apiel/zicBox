#pragma once

/**
 * Fm4 — a mono 4-operator FM synthesizer, ported from /home/alex/Music/fm1-tribe/firmware/src/dsp/fm4.c
 * (the FM4 synth of the mwave FM-1, from the Elektron Digitone FM appendix A.3 of the Digitone manual).
 *
 * Desktop adaptation: the firmware used fixed point and a hand-written math library (fastmath.h);
 * here everything is float / std math, with the same structure:
 *
 *   Operators: 0 = C (a carrier, or a modulator at a fixed depth, no envelope), 1 = A, 2 = B1, 3 = B2
 *   (A and B2 are detuned by DTUN; HARM positive reaches A and B1, negative reaches C).
 *   A and B1, B2 are scaled by their envelopes. One voice: a new note restarts the operators, the
 *   envelopes and the phases; a note over a sounding one (slide) only changes the pitch.
 *
 * Signal flow: 8 algorithms feed two outputs X and Y (ALG_X / ALG_Y), mixed with Mix;
 * harmonic partials of the four presets (Harm); operator A's self-modulation (Feedback);
 * two operator envelopes A and B (each an attack and a decay to an end level; B drives B1 and B2),
 * each scaled by Lev A / Lev B; then a low-pass filter, then an overdrive, then the amp envelope.
 *
 * Desktop additions: an LFO (pitch / cutoff / amp), and two FX sends (Delay, Reverb) into a fixed
 * default delay + reverb. The sends are channel-strip controls; the studio will own the master FX later.
 */

#include <cmath>
#include <cstring>

#include "audio/engines/EngineBase.h"

class Fm4 : public EngineBase<Fm4> {
public:
    static constexpr int PARAM_COUNT = 30;
    Param params[PARAM_COUNT];

    // ------------------------------------------------------------------ Pots 0..7: OPERATOR FM
    Param& alg    = addParam({ .key = "alg",    .label = "Alg",      .value = 0.0f,   .min = 0.0f,   .max = 7.0f,   .step = 1.0f });
    Param& ratioC = addParam({ .key = "ratioC", .label = "Ratio C",  .value = 0.0f,   .min = 0.0f,   .max = 15.0f,  .step = 1.0f });
    Param& ratioA = addParam({ .key = "ratioA", .label = "Ratio A",  .value = 3.0f,   .min = 0.0f,   .max = 15.0f,  .step = 1.0f });
    Param& ratioB = addParam({ .key = "ratioB", .label = "Ratio B",  .value = 39.0f,  .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& harm   = addParam({ .key = "harm",   .label = "Harm",     .value = 64.0f,  .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& detune = addParam({ .key = "detune", .label = "Detune",   .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& fdbk   = addParam({ .key = "fdbk",   .label = "Feedback", .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& mix    = addParam({ .key = "mix",    .label = "Mix",      .value = 64.0f,  .min = 0.0f,   .max = 127.0f, .step = 1.0f });

    // ------------------------------------------------------------------ Pots 8..15: OPERATOR ENVELOPES
    Param& leva   = addParam({ .key = "leva",   .label = "Lev A",    .unit = "%",    .value = 100.0f, .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& atka   = addParam({ .key = "atka",   .label = "Atk A",    .unit = "ms",   .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& deca   = addParam({ .key = "deca",   .label = "Dec A",    .unit = "ms",   .value = 60.0f,  .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& enda   = addParam({ .key = "enda",   .label = "End A",    .unit = "%",    .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& leve   = addParam({ .key = "leve",   .label = "Lev B",    .unit = "%",    .value = 64.0f,  .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& atkb   = addParam({ .key = "atkb",   .label = "Atk B",    .unit = "ms",   .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& decb   = addParam({ .key = "decb",   .label = "Dec B",    .unit = "ms",   .value = 50.0f,  .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& endb   = addParam({ .key = "endb",   .label = "End B",    .unit = "%",    .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });

    // ------------------------------------------------------------------ Pots 16..23: AMP & FILTER
    Param& atk    = addParam({ .key = "atk",    .label = "Attack",   .unit = "ms",   .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& dec    = addParam({ .key = "dec",    .label = "Decay",    .unit = "ms",   .value = 64.0f,  .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& sus    = addParam({ .key = "sus",    .label = "Sustain",  .unit = "%",    .value = 100.0f, .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& rel    = addParam({ .key = "rel",    .label = "Release",  .unit = "ms",   .value = 50.0f,  .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& cut    = addParam({ .key = "cut",    .label = "Cutoff",   .unit = "st",   .value = 110.0f, .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& reso   = addParam({ .key = "reso",   .label = "Reso",     .unit = "%",    .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& drive  = addParam({ .key = "drive",  .label = "Drive",    .unit = "%",    .value = 0.0f,   .min = 0.0f,   .max = 127.0f, .step = 1.0f });
    Param& trig   = addParam({ .key = "trig",   .label = "Trigger",  .value = 0.0f,   .min = 0.0f,   .max = 1.0f,   .step = 1.0f });

    // ------------------------------------------------------------------ Pots 24..27: LFO
    Param& lfoRate  = addParam({ .key = "lfoRate",  .label = "LFO Rate",  .unit = "Hz", .value = 64.0f, .min = 0.0f, .max = 127.0f, .step = 1.0f });
    Param& lfoDepth = addParam({ .key = "lfoDepth", .label = "LFO Depth", .unit = "%",  .value = 0.0f,  .min = 0.0f, .max = 127.0f, .step = 1.0f });
    Param& lfoShape = addParam({ .key = "lfoShape", .label = "LFO Shape", .value = 0.0f, .min = 0.0f, .max = 3.0f,   .step = 1.0f });
    Param& lfoDest  = addParam({ .key = "lfoDest",  .label = "LFO Dest",  .value = 0.0f, .min = 0.0f, .max = 2.0f,   .step = 1.0f });

    // ------------------------------------------------------------------ Pots 28..29: FX sends
    Param& dlySend  = addParam({ .key = "dlySend",  .label = "Delay",     .unit = "%",  .value = 0.0f,  .min = 0.0f, .max = 127.0f, .step = 1.0f });
    Param& revSend  = addParam({ .key = "revSend",  .label = "Reverb",    .unit = "%",  .value = 0.0f,  .min = 0.0f, .max = 127.0f, .step = 1.0f });

    const float sampleRate;

    /* UI: the operators' live levels, the two envelopes' levels, the LFO and the FX tails */
    static constexpr int NUM_OPS = 4;
    float liveOp[NUM_OPS] = {};
    float liveEnvA = 0.0f, liveEnvB = 0.0f;
    float liveAmp = 0.0f;
    float liveLfo = 0.0f, liveLfoPhase = 0.0f;
    float liveDelay = 0.0f, liveReverb = 0.0f;
    int liveStageA = 0, liveStageB = 0, liveStageAmp = 3;   /* 0 attack, 1 decay, 2 release, 3 idle */

    using EngineBase::noteOn;
    using EngineBase::noteOff;

    explicit Fm4(const float sampleRate = 44100.0f)
        : EngineBase(Synth, "Fm4", params)
        , sampleRate(sampleRate)
    {
        init();
        refreshAll();
        resetState();
    }

    /* fm4_init: the voice starts idle, silent */
    void resetState()
    {
        gate = 0;
        idle = 1;
        ampStage = 0;
        cc = 0;
        target_st = 0.0f;
        amp = 0.0f;
        ostage[0] = ostage[1] = 0;
        oenv[0] = oenv[1] = 0.0f;
        for (int j = 0; j < OPS; ++j) {
            inc[j] = ph[j] = s[j] = 0.0f;
        }
        ic1 = ic2 = 0.0f;
        g = a1 = a2 = a3 = 0.0f;
        liveEnvA = liveEnvB = liveAmp = 0.0f;
        for (int j = 0; j < NUM_OPS; ++j) liveOp[j] = 0.0f;
        liveLfo = 0.0f;
        liveDelay = liveReverb = 0.0f;
        liveStageA = liveStageB = 0;
        liveStageAmp = 3;
    }

    void noteOnImpl(uint8_t note, float velocity)
    {
        noteOn(note, velocity, false);
    }

    void noteOn(uint8_t note, float velocity, bool slide)
    {
        if (note > 127) note = 127;
        bool legato = !idle && gate && slide;             /* over a sounding note: the pitch changes, the envelopes go on */
        target_st = (float)(note - 69);
        if (!legato) {                                    /* a new note: the envelopes and the phases start again */
            ostage[0] = ostage[1] = 0;
            oenv[0] = oenv[1] = 0.0f;
            ampStage = 0;
            for (int j = 0; j < OPS; ++j) ph[j] = 0.0f;
            if (idle) {
                ic1 = ic2 = 0.0f;
                for (int j = 0; j < OPS; ++j) s[j] = 0.0f;
            }
        }
        cc = 0;                                           /* the pitch and the filter are set on the next sample */
        gate = 1;
        idle = 0;
    }

    void noteOffImpl(uint8_t)
    {
        noteOff();
    }
    void noteOff()
    {
        gate = 0;
        ampStage = 2;
    }

    float sampleImpl()
    {
        refreshChanged();
        return processFx(render());
    }

    /* the current partials of an operator, for the UI's waveform previews */
    float wavePreview(int op, float phase) const
    {
        if (op < 0 || op >= OPS) return 0.0f;
        return partial(op, phase);
    }

    // ------------------------------------------------------------------ derived state (from the pots)

public:
    static constexpr int OPS = 4;

    /* the algorithms (ALG_MOD[a][j]: the operators that modulate operator j in algorithm a + 1; the bit k is operator k) */
    static constexpr uint8_t ALG_MOD[8][OPS] = {
        {2, 0, 10, 0},    /* 1: A > C, B2 > B1, A > B1 */
        {2, 0, 8, 0},     /* 2: A > C, B2 > B1 */
        {2, 0, 8, 2},     /* 3: A > C, A > B2 > B1 */
        {2, 4, 8, 0},     /* 4: B2 > B1 > A > C */
        {8, 0, 2, 0},     /* 5: A > B1, B2 > C */
        {0, 0, 8, 2},     /* 6: A > B2 > B1 */
        {2, 0, 0, 0},     /* 7: A > C, B1 and B2 alone */
        {0, 0, 0, 0},     /* 8: all four in parallel */
    };
    static constexpr uint8_t ALG_X[8] = {1, 1, 1, 1, 1, 1, 1, 3};   /* the carriers of output X */
    static constexpr uint8_t ALG_Y[8] = {4, 4, 4, 4, 4, 4, 12, 12}; /* the carriers of output Y */

protected:
    static constexpr int PARTIALS = 4;
    static constexpr int CTRL = 8;                        /* control rate: the operators' pitch and the filter's coefficients */
    static constexpr float PI2 = 6.28318530717958647692f;

    /* the ratio tables: T16 for A and for B2, T8 for B1 (the B pair's two steps) */
    static constexpr float T16[16] = {0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    static constexpr float T8[8] = {0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 12.0f, 16.0f};

    /* HARM: the partials of the four presets (the additive waveforms), morphed between */
    static constexpr float PRESET[4][PARTIALS] = {
        {1.0f, 0.0f, 0.0f, 0.0f},            /* sine */
        {1.0f, 0.5f, 0.33f, 0.25f},          /* saw */
        {1.0f, 0.0f, 0.33f, 0.0f},           /* square: the odd partials */
        {1.0f, 0.25f, 0.5f, 0.2f},           /* bell */
    };

    int algV;
    uint8_t ord[OPS];
    uint8_t mods[OPS][OPS];
    uint8_t nmods[OPS];
    float ratio[OPS];
    float dt_factor;
    float idx[OPS];
    float part[OPS][PARTIALS];
    uint8_t nparts[OPS];
    float fb, mixV;
    float endl[2], atk_step[2], dec_c[2];
    float amp_att_step, amp_dec_c, sustain, rel_c;
    int trigV;
    float drive_g, drive_norm;
    int drive_on;
    float cut_st, reso_k;

    /* LFO */
    float lfoInc = 0.0f, lfoVal = 0.0f, lfoDepthV = 0.0f, lfoPhase = 0.0f;
    int lfoShapeV = 0, lfoDestV = 0;

    /* FX sends */
    float dlyMix = 0.0f, revMix = 0.0f;

    /* voice state */
    int gate, idle, ampStage, cc;
    int ostage[2];
    float target_st, amp;
    float inc[OPS], ph[OPS], s[OPS];
    float oenv[2];
    float g, a1, a2, a3;
    float ic1, ic2;

    float lastVal[PARAM_COUNT];

    float decayMul(float seconds, float stepS) { return std::exp(-stepS / seconds); }

    /* the LFO's four shapes, phase p in 0 .. 1, output -1 .. 1 */
    static float lfoShapeValue(int shape, float p)
    {
        switch (shape) {
        case 1: return 1.0f - 4.0f * std::fabs(p - 0.5f);          /* triangle */
        case 2: return 2.0f * p - 1.0f;                             /* saw */
        case 3: return p < 0.5f ? 1.0f : -1.0f;                     /* square */
        default: return std::sin(PI2 * p);                          /* sine */
        }
    }

    /* applyParam: fm4_set() — one pot change, from the firmware */
    void applyParam(int i)
    {
        int v = (int)std::lround(params[i].value);
        float n = (float)v * (1.0f / 127.0f);
        switch (i) {
        case 0: algV = v; buildOrder(); break;
        case 1: ratio[0] = (float)(v + 1); break;
        case 2: ratio[1] = T16[v]; break;
        case 3: ratio[2] = T8[v >> 4]; ratio[3] = T16[v & 15]; break;
        case 4: setHarm(v); break;
        case 5: dt_factor = std::exp2(v * 0.3f * (1.0f / 1200.0f)); break;
        case 6: fb = n * n; break;
        case 7: mixV = n; break;
        case 8: idx[1] = 1.5f * n * n; break;
        case 9: atk_step[0] = 1.0f / (0.001f * std::exp2(n * 10.0f) * sampleRate); break;
        case 10: dec_c[0] = decayMul(0.005f * std::exp2(n * 9.0f), 1.0f / sampleRate); break;
        case 11: endl[0] = n; break;
        case 12: {                                      /* the manual's graph: B1 rises to 43, falls to 85, rises; B2 rises from 43 to 85 */
            float b1, b2;
            if (v <= 43) b1 = (float)v / 43.0f;
            else if (v <= 85) b1 = (85.0f - (float)v) / 42.0f;
            else b1 = ((float)v - 85.0f) / 42.0f;
            b2 = v < 43 ? 0.0f : v >= 85 ? 1.0f : ((float)v - 43.0f) / 42.0f;
            idx[2] = 1.5f * b1;
            idx[3] = 1.5f * b2;
            break;
        }
        case 13: atk_step[1] = 1.0f / (0.001f * std::exp2(n * 10.0f) * sampleRate); break;
        case 14: dec_c[1] = decayMul(0.005f * std::exp2(n * 9.0f), 1.0f / sampleRate); break;
        case 15: endl[1] = n; break;
        case 16: amp_att_step = 1.0f / (0.001f * std::exp2(n * 11.0f) * sampleRate); break;
        case 17: amp_dec_c = decayMul(0.005f * std::exp2(n * 9.0f), 1.0f / sampleRate); break;
        case 18: sustain = n; break;
        case 19: rel_c = decayMul(0.005f * std::exp2(n * 9.2f), 1.0f / sampleRate); break;
        case 20: cut_st = -43.0f + 100.0f * n; break;
        case 21: reso_k = 2.0f - 1.9f * n; break;
        case 22:
            drive_on = v > 0;
            drive_g = 1.0f + 7.0f * n * n;
            drive_norm = 1.0f / std::tanh(drive_g);
            break;
        case 23: trigV = v; break;
        case 24: lfoInc = (0.05f * std::exp2(n * 8.64f)) / sampleRate; break;
        case 25: lfoDepthV = n; break;
        case 26: lfoShapeV = v; break;
        case 27: lfoDestV = v; break;
        case 28: dlyMix = n; break;
        case 29: revMix = n; break;
        }
    }

    void refreshAll()
    {
        for (int i = 0; i < PARAM_COUNT; ++i) {
            lastVal[i] = params[i].value;
            applyParam(i);
        }
    }

    void refreshChanged()
    {
        for (int i = 0; i < PARAM_COUNT; ++i) {
            if (params[i].value != lastVal[i]) {
                applyParam(i);
                lastVal[i] = params[i].value;
            }
        }
    }

    /* the order the operators are computed in: each after the operators that modulate it (a cycle keeps the listed order) */
    void buildOrder()
    {
        const uint8_t* mod = ALG_MOD[algV];
        int done[OPS] = {0, 0, 0, 0}, n = 0, pass, j, k, ok;
        for (pass = 0; pass < OPS; pass++) {
            for (j = 0; j < OPS; j++) {
                if (done[j]) continue;
                ok = 1;
                for (k = 0; k < OPS; k++)
                    if (((mod[j] >> k) & 1u) && !done[k]) ok = 0;
                if (ok) {
                    ord[n++] = (uint8_t)j;
                    done[j] = 1;
                }
            }
        }
        for (j = 0; j < OPS; j++)
            if (!done[j]) {
                ord[n++] = (uint8_t)j;
                done[j] = 1;
            }
        for (j = 0; j < OPS; j++) {                       /* the modulators of each operator, lowest first */
            nmods[j] = 0;
            for (k = 0; k < OPS; k++)
                if ((mod[j] >> k) & 1u) mods[j][nmods[j]++] = (uint8_t)k;
        }
    }

    /* HARM: the preset amplitudes, morphed by x in 0 .. 3 (sine, saw, square, bell), normalised to unit energy */
    void morph(float* w, float x)
    {
        int a = (int)x, p;
        float f = x - (float)a, e = 0.0f;
        if (a >= 3) {
            a = 2;
            f = 1.0f;
        }
        for (p = 0; p < PARTIALS; p++) {
            w[p] = PRESET[a][p] * (1.0f - f) + PRESET[a + 1][p] * f;
            e += w[p] * w[p];
        }
        e = std::sqrt(e);
        for (p = 0; p < PARTIALS; p++)
            w[p] = e > 0.0f ? w[p] / e : 0.0f;
    }

    /* HARM: below 64, C takes the morph; above, A and B1 do; B2 stays a sine. */
    void setHarm(int v)
    {
        int d = v - 64, j, p, last;
        float w[PARTIALS];
        for (j = 0; j < OPS; j++) {
            morph(w, (j == 0 ? (d < 0 ? (float)(-d) * (3.0f / 64.0f) : (float)d * (3.0f / 63.0f)) : 0.0f));
            last = 0;
            for (p = 0; p < PARTIALS; p++) {
                part[j][p] = w[p];
                if (w[p] != 0.0f) last = p;
            }
            nparts[j] = (uint8_t)(last + 1);
        }
    }

    /* a control step: the operators' increments and the filter's coefficients for the next CTRL samples */
    void control()
    {
        float base, cut, fc, x, sn, cs, g, k, v;
        float vib = (lfoDestV == 0) ? lfoVal * lfoDepthV * 2.0f : 0.0f;   /* vibrato, up to +-2 st */
        base = 440.0f * std::exp2((target_st + vib) * (1.0f / 12.0f)) * (1.0f / sampleRate);
        v = base * ratio[0];                              inc[0] = v > 0.45f ? 0.45f : v;
        v = base * ratio[1] * dt_factor;                  inc[1] = v > 0.45f ? 0.45f : v;
        v = base * ratio[2];                              inc[2] = v > 0.45f ? 0.45f : v;
        v = base * ratio[3] * dt_factor;                  inc[3] = v > 0.45f ? 0.45f : v;

        cut = cut_st + ((lfoDestV == 1) ? lfoVal * lfoDepthV * 36.0f : 0.0f);   /* filter wobble, up to +-36 st */
        cut = cut > 60.0f ? 60.0f : (cut < -60.0f ? -60.0f : cut);
        fc = 440.0f * std::exp2(cut * (1.0f / 12.0f));
        x = fc * (0.5f / sampleRate);                     /* pi fc / fs, in turns of 2 pi */
        if (x > 0.2f) x = 0.2f;
        sn = std::sin(PI2 * x);
        cs = std::sin(PI2 * (x + 0.25f));
        g = sn / cs;                                      /* tan(pi fc / fs) */
        k = reso_k;
        this->g = g;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    /* one operator's raw output at phase x (turns): its partials, the first nparts[j] of them */
    float partial(int j, float x) const
    {
        float s = 0.0f;
        for (int p = 0; p < nparts[j]; ++p)
            if (part[j][p] != 0.0f)
                s += part[j][p] * std::sin(PI2 * (float)(p + 1) * x);
        return s;
    }

    /* an envelope step: A (k = 0) or B (k = 1). Attack to 1, then a decay to the end level; for gate the decay
     * waits for the key to be let go, and holds at 1 meanwhile */
    void envStep(int k)
    {
        float* e = &oenv[k];
        if (ostage[k] == 0) {
            *e += atk_step[k];
            if (*e >= 1.0f) {
                *e = 1.0f;
                ostage[k] = 1;
            }
            return;
        }
        if (ostage[k] == 1) {
            if (trigV)
                *e = endl[k] + (*e - endl[k]) * dec_c[k];
            else if (!gate)
                ostage[k] = 2;
            return;
        }
        *e = endl[k] + (*e - endl[k]) * dec_c[k];
    }

    float render()
    {
        /* the LFO runs free, even while the voice is idle, so the UI can animate it */
        lfoPhase += lfoInc;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
        lfoVal = lfoShapeValue(lfoShapeV, lfoPhase);
        liveLfo = lfoVal;
        liveLfoPhase = lfoPhase;

        if (idle) {
            liveEnvA = liveEnvB = liveAmp = 0.0f;
            for (int j = 0; j < OPS; ++j) liveOp[j] = 0.0f;
            liveStageAmp = 3;
            return 0.0f;
        }
        float a[OPS];
        if (cc == 0) {
            control();
            cc = CTRL;
        }
        cc--;

        envStep(0);
        envStep(1);
        liveEnvA = oenv[0];
        liveEnvB = oenv[1];
        a[0] = 1.0f;                                      /* C: no envelope */
        a[1] = oenv[0];
        a[2] = oenv[1];
        a[3] = oenv[1];

        float xs = 0.0f, ys = 0.0f, pm, y, u, v3, v1, v2, lp;
        unsigned xm = ALG_X[algV], ym = ALG_Y[algV];
        for (int k = 0; k < OPS; ++k) {                   /* in the order that makes each modulator ready first */
            int j = ord[k];
            pm = 0.0f;
            for (int q = 0; q < nmods[j]; ++q) {
                int m = mods[j][q];
                pm += idx[m] * a[m] * s[m];
            }
            if (j == 1)
                pm += fb * s[1];                          /* A's feedback: its own last output */
            ph[j] += inc[j];
            if (ph[j] >= 1.0f) ph[j] -= 1.0f;
            s[j] = partial(j, ph[j] + pm);
            y = s[j] * a[j];
            liveOp[j] = std::fabs(y);
            if ((xm >> j) & 1u) xs += y;
            if ((ym >> j) & 1u) ys += y;
        }

        u = 0.5f * ((1.0f - mixV) * xs + mixV * ys);
        if (drive_on)
            u = std::tanh(u * drive_g) * drive_norm;

        /* the state-variable low-pass (zero-delay), two integrators */
        v3 = u - ic2;
        v1 = a1 * ic1 + a2 * v3;
        v2 = ic2 + a2 * ic1 + a3 * v3;
        ic1 = std::clamp(2.0f * v1 - ic1, -8.0f, 8.0f);
        ic2 = std::clamp(2.0f * v2 - ic2, -8.0f, 8.0f);
        lp = v2;

        /* the amp envelope: attack, decay to the sustain, release */
        amp = this->amp;
        if (ampStage == 0) {
            amp += amp_att_step;
            if (amp >= 1.0f) {
                amp = 1.0f;
                ampStage = 1;
            }
        } else if (ampStage == 1) {
            amp = sustain + (amp - sustain) * amp_dec_c;
        } else {
            amp *= rel_c;
        }
        this->amp = amp;

        float out = 0.5f * amp * std::tanh(lp);
        if (lfoDestV == 2)                                /* tremolo */
            out *= 1.0f - lfoDepthV * 0.5f * (1.0f - lfoVal);
        liveAmp = amp;
        liveStageA = ostage[0];
        liveStageB = ostage[1];
        liveStageAmp = ampStage;
        if (ampStage == 2 && amp < 1e-4f) {
            idle = 1;
            amp = 0.0f;
            liveAmp = 0.0f;
            liveStageAmp = 3;
        }
        return out;
    }

    /* ------------------------------------------------------------------ default FX (the two sends feed these)
     * A fixed 300 ms damped delay and a small Schroeder reverb. The sends are the channel strip's;
     * the studio will replace these with its master FX bus later.
     */
    static constexpr int MAX_DELAY = 48000;              /* ~1 s */
    float delayBuf[MAX_DELAY] = {};
    int delayWritePos = 0;
    float delayLp = 0.0f;

    static constexpr int COMB1_L = 1557, COMB2_L = 1617, COMB3_L = 1491, COMB4_L = 1422;
    static constexpr int AP1_L = 225, AP2_L = 556;
    float combBuf1[COMB1_L] = {}, combBuf2[COMB2_L] = {}, combBuf3[COMB3_L] = {}, combBuf4[COMB4_L] = {};
    float apBuf1[AP1_L] = {}, apBuf2[AP2_L] = {};
    int cPos1 = 0, cPos2 = 0, cPos3 = 0, cPos4 = 0, apPos1 = 0, apPos2 = 0;
    float combFilter1 = 0.0f, combFilter2 = 0.0f, combFilter3 = 0.0f, combFilter4 = 0.0f;

    float processFx(float in)
    {
        /* 1. the delay: 300 ms, a damped feedback */
        int dLen = (int)(0.30f * sampleRate);
        if (dLen < 1) dLen = 1;
        if (dLen >= MAX_DELAY) dLen = MAX_DELAY - 1;
        int readPos = delayWritePos - dLen;
        if (readPos < 0) readPos += MAX_DELAY;
        float dOut = delayBuf[readPos];
        delayLp += 0.30f * (dOut - delayLp);
        delayBuf[delayWritePos] = in + delayLp * 0.42f;
        delayWritePos = (delayWritePos + 1) % MAX_DELAY;

        /* 2. the reverb: 4 parallel combs + 2 all-passes */
        const float revFeedback = 0.82f, damp = 0.35f;
        float cOut1 = combBuf1[cPos1];
        combFilter1 = cOut1 * (1.0f - damp) + combFilter1 * damp;
        combBuf1[cPos1] = in + combFilter1 * revFeedback;
        cPos1 = (cPos1 + 1) % COMB1_L;

        float cOut2 = combBuf2[cPos2];
        combFilter2 = cOut2 * (1.0f - damp) + combFilter2 * damp;
        combBuf2[cPos2] = in + combFilter2 * revFeedback;
        cPos2 = (cPos2 + 1) % COMB2_L;

        float cOut3 = combBuf3[cPos3];
        combFilter3 = cOut3 * (1.0f - damp) + combFilter3 * damp;
        combBuf3[cPos3] = in + combFilter3 * revFeedback;
        cPos3 = (cPos3 + 1) % COMB3_L;

        float cOut4 = combBuf4[cPos4];
        combFilter4 = cOut4 * (1.0f - damp) + combFilter4 * damp;
        combBuf4[cPos4] = in + combFilter4 * revFeedback;
        cPos4 = (cPos4 + 1) % COMB4_L;

        float combSum = (cOut1 + cOut2 + cOut3 + cOut4) * 0.25f;

        float apOut1 = -combSum + apBuf1[apPos1];
        apBuf1[apPos1] = combSum + apBuf1[apPos1] * 0.5f;
        apPos1 = (apPos1 + 1) % AP1_L;

        float apOut2 = -apOut1 + apBuf2[apPos2];
        apBuf2[apPos2] = apOut1 + apBuf2[apPos2] * 0.5f;
        apPos2 = (apPos2 + 1) % AP2_L;

        liveDelay = dOut * dlyMix;
        liveReverb = apOut2 * revMix;
        return in + dOut * dlyMix * 0.55f + apOut2 * revMix * 0.55f;
    }
};
