#pragma once

#include "audio/engines/Acid32.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>

namespace Acid32Panel {

constexpr int COLS = 8;
constexpr int ROWS = 4;
constexpr int POTS = COLS * ROWS; // 32 pots

static_assert(POTS == Acid32::PARAM_COUNT, "one pot per parameter");

struct Block {
    const char* name;
    const char* subtitle;
    int firstPot, col, row, cols, rows;
};

constexpr int BLOCKS = 4;

inline const Block& block(int b)
{
    static const Block blocks[BLOCKS] = {
        { "OSCILLATOR", "morphing wave, unison, sub-oscillator & slide", 0,  0, 0, 4, 2 },
        { "LADDER FILTER", "4-pole 24dB ladder, accent, HPF & modulation", 8,  4, 0, 4, 2 },
        { "ENVELOPES",   "filter decay, punch envelope & amp ADSR",        16, 0, 2, 4, 2 },
        { "SPATIAL FX",  "overdrive, distortion, ping-pong delay & reverb", 24, 4, 2, 4, 2 },
    };
    return blocks[b];
}

inline int blockOf(int pot)
{
    if (pot < 8) return 0;
    if (pot < 16) return 1;
    if (pot < 24) return 2;
    return 3;
}

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

struct Group {
    int firstPot, count;
    const char* name;
};

constexpr int GROUPS = 16;
inline const Group& group(int g)
{
    static const Group groups[GROUPS] = {
        // Block 0: OSCILLATOR
        { 0,  2, "PITCH" },
        { 2,  2, "WAVE & UNISON" },
        { 4,  3, "SUB & DETUNE" },
        { 7,  1, "GLIDE" },

        // Block 1: LADDER FILTER
        { 8,  2, "24dB LADDER" },
        { 10, 2, "MOD & ACCENT" },
        { 12, 1, "TIGHT HPF" },
        { 13, 3, "LFO MODULATION" },

        // Block 2: ENVELOPES
        { 16, 1, "FILTER DECAY" },
        { 17, 2, "PITCH PUNCH" },
        { 19, 1, "ATTACK" },
        { 20, 3, "AMP ADSR" },
        { 23, 1, "PRE-DRIVE" },

        // Block 3: SPATIAL FX
        { 24, 2, "POST DISTORTION" },
        { 26, 2, "PING-PONG DELAY" },
        { 28, 4, "SPACE REVERB" }
    };
    return groups[g];
}

inline const char* help(int pot)
{
    static const char* h[POTS] = {
        // Block 0: OSCILLATOR
        "Tune: pitch offset in semitones (-24 .. +24 st)",
        "Fine Tune: fine pitch centering (-1.0 .. +1.0 st)",
        "Wave Color: morphing wave (Tri -> Saw -> Square -> Folded Wave)",
        "Unison: 1 to 3 detuned oscillator voices",
        "Detune: unison voice spread in cents (0 .. 60 ct)",
        "Sub Level: sub-oscillator level (0 .. 100%)",
        "Sub Octave: sub oscillator octave down (1 or 2 octaves below)",
        "Slide: legato glide time between connected notes (5 .. 500 ms)",

        // Block 1: LADDER FILTER
        "Cutoff: 4-pole ZDF ladder filter cutoff frequency (30 Hz .. 14 kHz)",
        "Resonance: filter feedback peak up to screaming self-oscillation",
        "Env Mod: depth of filter envelope opening on note hit (-100 .. +100%)",
        "Accent: extra filter envelope depth and gain boost on accented notes",
        "HP Filter: 2-pole high-pass filter to clean sub rumble or create screech",
        "LFO Rate: LFO frequency (0.1 .. 25 Hz)",
        "LFO > Cutoff: depth of LFO modulating filter cutoff (-100 .. +100%)",
        "LFO > Wave: depth of LFO modulating wave morph / PWM (-100 .. +100%)",

        // Block 2: ENVELOPES
        "Filter Decay: classic acid 303 filter envelope decay time (20 .. 2500 ms)",
        "Pitch Depth: punchy pitch envelope transient depth (-24 .. +24 st)",
        "Pitch Decay: pitch envelope transient decay time (5 .. 300 ms)",
        "Amp Attack: amp envelope attack time (1 .. 500 ms)",
        "Amp Decay: amp envelope decay time (20 .. 3000 ms)",
        "Amp Sustain: amp envelope sustain level (0 .. 100%)",
        "Amp Release: amp envelope release time (5 .. 2000 ms)",
        "Drive: pre-filter warm non-linear saturation (0 .. 100%)",

        // Block 3: SPATIAL FX
        "Distortion: post-filter multi-mode distortion drive (0 .. 100%)",
        "Dist Type: 0: Soft Tanh, 1: Hard Clip, 2: Wavefold, 3: Fuzz",
        "Delay Time: stereo tape/ping-pong delay time (20 .. 800 ms)",
        "Delay Fdbk: delay feedback regeneration (0 .. 95%)",
        "Delay Mix: delay wet/dry balance (0 .. 100%)",
        "Rev Size: Schroeder reverb room size and decay (10 .. 98%)",
        "Rev Damp: reverb high-frequency damping (0 .. 90%)",
        "Reverb Mix: reverb wet/dry balance (0 .. 100%)"
    };
    return (pot >= 0 && pot < POTS) ? h[pot] : "";
}

class Panel {
public:
    Acid32& engine;
    float defaults[POTS];
    float smooth[POTS];

    explicit Panel(Acid32& e)
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
        const char* k = engine.params[pot].key;
        return !strcmp(k, "tune") || !strcmp(k, "fine") || !strcmp(k, "lfoToCut") ||
               !strcmp(k, "lfoToWave") || !strcmp(k, "pDepth") || !strcmp(k, "envMod");
    }

    void set(int pot, float n)
    {
        n = std::clamp(n, 0.0f, 1.0f);
        Param& p = param(pot);
        p.set(p.min + n * (p.max - p.min));
    }

    void reset(int pot) { set(pot, defaults[pot]); }

    void tweak(int pot, float delta)
    {
        set(pot, norm(pot) + delta);
    }

    void formatValue(int pot, char* buf, size_t size)
    {
        Param& p = param(pot);
        const char* unit = p.unit ? p.unit : "";
        float v = p.value;

        if (!strcmp(p.key, "distType")) {
            const char* names[] = { "Soft", "Hard", "Fold", "Fuzz" };
            int idx = std::clamp((int)v, 0, 3);
            snprintf(buf, size, "%s", names[idx]);
        } else if (!strcmp(p.key, "unison")) {
            snprintf(buf, size, "%d voice%s", (int)v, (int)v > 1 ? "s" : "");
        } else if (!strcmp(p.key, "subOct")) {
            snprintf(buf, size, "-%d oct", (int)v);
        } else if (p.step >= 1.0f) {
            snprintf(buf, size, "%d%s%s", (int)std::lround(v), unit[0] == '%' ? "" : " ", unit);
        } else {
            snprintf(buf, size, "%.1f%s%s", v, unit[0] == '%' ? "" : " ", unit);
        }
    }
};

} // namespace Acid32Panel
