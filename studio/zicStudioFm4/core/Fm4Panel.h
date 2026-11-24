#pragma once

/**
 * Front panel model of the Fm4: 30 pots in 6 cards.
 *
 * Shared by the desktop emulator and (future) firmware, so it stays free of any
 * UI / OS / audio-driver dependency.
 *
 *   hardware : read the ADC channels, call panel.update(i, raw01) from the main loop
 *   emulator : call panel.set(i, norm) from mouse / wheel events
 *
 * Pot i drives engine.params[i] over its full min..max range.
 *
 * The cards read like the signal flow, top to bottom:
 *   OPERATOR FM     (0..7)   algorithm, ratios, harmonics, detune, feedback, mix
 *   OPERATOR ENVS   (8..15)  the A and B operator envelopes
 *   AMP             (16..19) the amp envelope
 *   FILTER          (20..23) the low-pass filter, the drive and the trigger mode
 *   LFO             (24..27) rate, depth, shape and destination
 *   FX              (28..29) the delay and reverb sends
 */

#include <cmath>
#include <cstdio>
#include <cstring>

#include "audio/engines/Fm4.h"

namespace Fm4Panel {

constexpr int COLS = 8;
constexpr int ROWS = 4;
constexpr int POTS = 30;

static_assert(POTS == Fm4::PARAM_COUNT, "one pot per parameter");

/* the engine's ratio tables, for the readouts */
inline constexpr float T16[16] = { 0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };
inline constexpr float T8[8] = { 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f, 12.0f, 16.0f };

struct Block {
    const char* name;
    const char* subtitle;
    int firstPot, col, row, cols, rows;
};

constexpr int BLOCKS = 6;
inline const Block& block(int b)
{
    static const Block blocks[BLOCKS] = {
        { "OPERATOR FM",    "algorithm, ratios, harmonics, detune, feedback, mix", 0,  0, 0, 4, 2 },
        { "OPERATOR ENVS",  "the A and B operator envelopes",                      8,  4, 0, 4, 2 },
        { "AMP",            "the amp envelope",                                    16, 0, 2, 4, 1 },
        { "FILTER",         "low-pass, drive and trigger",                         20, 4, 2, 4, 1 },
        { "LFO",            "rate, depth, shape and destination",                  24, 0, 3, 6, 1 },
        { "FX",             "the delay and reverb sends",                          28, 6, 3, 2, 1 },
    };
    return blocks[b];
}
inline int blockOf(int pot) { return pot < 8 ? 0 : pot < 16 ? 1 : pot < 20 ? 2 : pot < 24 ? 3 : pot < 28 ? 4 : 5; }
inline int potCol(int pot)
{
    const Block& b = block(blockOf(pot));
    return b.col + (pot - b.firstPot) % b.cols;
}
inline int potRow(int pot)
{
    const Block& b = block(blockOf(pot));
    return b.row + (pot - b.firstPot) / b.cols;
}

struct Group { int firstPot, count; const char* name; };
constexpr int GROUPS = 11;
inline const Group& group(int g)
{
    static const Group groups[GROUPS] = {
        // Block 0: OPERATOR FM
        { 0, 4, "ALG & MIX" },
        { 4, 4, "RATIO (C, A, B)" },
        // Block 1: OPERATOR ENVS
        { 8, 2, "LEV A" },        { 10, 2, "ENV A (ATK/DEC/END)" },
        { 12, 2, "LEV B" },       { 14, 2, "ENV B (ATK/DEC/END)" },
        // Block 2: AMP
        { 16, 4, "AMP ENVELOPE" },
        // Block 3: FILTER
        { 20, 2, "LOW-PASS" },
        { 22, 2, "DRIVE & TRIG" },
        // Block 4: LFO
        { 24, 4, "LFO" },
        // Block 5: FX
        { 28, 2, "SENDS" },
    };
    return groups[g];
}

// One line per pot: what it does
inline const char* help(int pot)
{
    static const char* h[POTS] = {
        // Block 0: OPERATOR FM
        "the eight algorithms of the Digitone FM manual, A.3: C > X, A > X, ..., all four in parallel",
        "operator C's fixed ratio, 1..16 (the base of the FM sound)",
        "operator A's ratio to C, 0.25 .. 8 (the classic FM spacing)",
        "operators B2 and B1's ratio: B2 steps through 16 values, then B1",
        "harmonic morph of the four operators: sine, saw, square, bell, morphed in",
        "A and B2 detuned, 0 .. 38 cents",
        "operator A's self-modulation depth (feedback), 0 = off",
        "the mix of the X and Y outputs, 0 = X, 100% = Y",
        // Block 1: OPERATOR ENVS
        "A's modulation index (it is scaled by its envelope)",
        "A's attack time, 0 .. 2 s",
        "A's decay time, 5 ms .. 2 s (to End A)",
        "the level A decays to, 0 .. 100%",
        "the crossfade of B1 and B2 (the manual's graph): B1 rises to 43, falls to 85, rises",
        "B's attack time, 0 .. 2 s",
        "B's decay time, 2.5 ms .. 2 s (to End B)",
        "the level B decays to, 0 .. 100%",
        // Block 2: AMP
        "the amp envelope attack, 0 .. 2 s",
        "the amp envelope decay, 5 ms .. 2 s",
        "the amp envelope sustain level, 0 .. 100%",
        "the amp envelope release, 5 ms .. 2 s",
        // Block 3: FILTER
        "the state-variable low-pass, -43 .. +57 semitones (40 Hz .. 12 kHz)",
        "filter resonance, up to a scream",
        "the overdrive before the filter (0 = off)",
        "trigger mode: Gate holds on the note, Trig runs out by itself",
        // Block 4: LFO
        "the LFO's speed, 0.05 .. 20 Hz",
        "how deep the LFO moves its destination",
        "the LFO's wave: sine, triangle, saw or square",
        "what the LFO moves: pitch (vibrato), cutoff (wobble) or amp (tremolo)",
        // Block 5: FX
        "how much of the voice goes into the default delay (300 ms, damped)",
        "how much of the voice goes into the default reverb",
    };
    return h[pot];
}

class Panel {
public:
    Fm4& engine;
    float defaults[POTS];
    float smooth[POTS];

    explicit Panel(Fm4& e)
        : engine(e)
    {
        for (int i = 0; i < POTS; ++i) {
            defaults[i] = norm(i);
            smooth[i] = defaults[i];
        }
    }

    Param& param(int pot) { return engine.params[pot]; }

    float norm(int pot)
    {
        Param& p = param(pot);
        return (p.value - p.min) / (p.max - p.min);
    }

    float defaultNorm(int pot) const { return defaults[pot]; }

    bool isCentered(int pot) const
    {
        (void)pot;
        return false;
    }

    /* a small-range pot behaves like an encoder with detents (alg, trig, LFO shape / dest) */
    bool isDetent(int pot) const
    {
        const Param& p = engine.params[pot];
        return (p.max - p.min) <= 16.0f;
    }

    void set(int pot, float n)
    {
        n = std::clamp(n, 0.0f, 1.0f);
        Param& p = param(pot);
        p.set(p.min + n * (p.max - p.min));
    }

    void setValue(int pot, float v) { param(pot).set(v); }

    void reset(int pot) { set(pot, defaults[pot]); }

    void tweak(int pot, float delta)
    {
        set(pot, norm(pot) + delta);
    }

    void update(int pot, float raw)
    {
        smooth[pot] += 0.2f * (raw - smooth[pot]);
        float d = smooth[pot] - norm(pot);
        if (d > 0.004f || d < -0.004f) set(pot, smooth[pot]);
    }

    void formatValue(int pot, char* buf, size_t size)
    {
        Param& p = param(pot);
        const char* k = p.key;
        float n = norm(pot);
        int v = (int)std::lround(p.value);

        auto ms = [&](float msec) {
            if (msec < 999.5f) snprintf(buf, size, "%.0f ms", msec);
            else snprintf(buf, size, "%.2f s", msec / 1000.0f);
        };
        auto pct = [&]() { snprintf(buf, size, "%.0f%%", n * 100.0f); };

        if (!strcmp(k, "alg")) {
            snprintf(buf, size, "Alg %d", v + 1);
        } else if (!strcmp(k, "ratioC")) {
            snprintf(buf, size, "x%d", v + 1);
        } else if (!strcmp(k, "ratioA")) {
            snprintf(buf, size, "x%.2f", T16[v & 15]);
        } else if (!strcmp(k, "ratioB")) {
            snprintf(buf, size, "%.2f/%.2f", T8[(v >> 4) & 7], T16[v & 15]);
        } else if (!strcmp(k, "harm")) {
            float x;
            const char* pre = "";
            if (v < 64) { x = (float)(64 - v) * (3.0f / 64.0f); pre = "C "; }
            else if (v == 64) { x = 0.0f; }
            else { x = (float)(v - 64) * (3.0f / 63.0f); pre = "A "; }
            const char* nm = x < 0.5f ? "SINE" : x < 1.5f ? "SAW" : x < 2.5f ? "SQR" : "BELL";
            snprintf(buf, size, "%s%s", pre, nm);
        } else if (!strcmp(k, "detune")) {
            snprintf(buf, size, "%.1f ct", p.value * 0.3f);
        } else if (!strcmp(k, "fdbk") || !strcmp(k, "mix")) {
            pct();
        } else if (!strcmp(k, "leva") || !strcmp(k, "leve") || !strcmp(k, "enda") ||
                   !strcmp(k, "endb") || !strcmp(k, "sus") || !strcmp(k, "lfoDepth") ||
                   !strcmp(k, "dlySend") || !strcmp(k, "revSend") || !strcmp(k, "reso") || !strcmp(k, "drive")) {
            pct();
        } else if (!strcmp(k, "atka") || !strcmp(k, "atkb")) {
            ms(std::exp2(n * 10.0f));
        } else if (!strcmp(k, "atk")) {
            ms(std::exp2(n * 11.0f));
        } else if (!strcmp(k, "deca") || !strcmp(k, "decb") || !strcmp(k, "dec")) {
            ms(5.0f * std::exp2(n * 9.0f));
        } else if (!strcmp(k, "rel")) {
            ms(5.0f * std::exp2(n * 9.2f));
        } else if (!strcmp(k, "cut")) {
            snprintf(buf, size, "%+.0f st", -43.0f + 100.0f * n);
        } else if (!strcmp(k, "trig")) {
            snprintf(buf, size, "%s", v ? "Trig" : "Gate");
        } else if (!strcmp(k, "lfoRate")) {
            float hz = 0.05f * std::exp2(n * 8.64f);
            if (hz < 10.0f) snprintf(buf, size, "%.2f Hz", hz);
            else snprintf(buf, size, "%.1f Hz", hz);
        } else if (!strcmp(k, "lfoShape")) {
            static const char* nm[4] = { "Sine", "Tri", "Saw", "Square" };
            snprintf(buf, size, "%s", nm[v < 0 ? 0 : (v > 3 ? 3 : v)]);
        } else if (!strcmp(k, "lfoDest")) {
            static const char* nm[3] = { "Pitch", "Cutoff", "Amp" };
            snprintf(buf, size, "%s", nm[v < 0 ? 0 : (v > 2 ? 2 : v)]);
        } else {
            snprintf(buf, size, "%.2f", p.value);
        }
    }
};

} // namespace Fm4Panel
