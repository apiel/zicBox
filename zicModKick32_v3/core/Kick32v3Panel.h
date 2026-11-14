#pragma once

/**
 * Front panel model of the zicModKick32_v3: 4 rows x 8 pots.
 *
 * Shared by the desktop emulator and the (future) Daisy firmware, so it must stay
 * free of any UI / OS / audio-driver dependency.
 *
 *   hardware : read the 32 ADC channels, call panel.update(i, raw01) from the main loop
 *   emulator : call panel.set(i, norm) from mouse / wheel events
 *
 * Pot i drives engine.params[i] over its full min..max range.
 *
 * The panel reads like the signal flow, left to right:
 *
 *   +-- BODY (12 pots) ------------+  +-- DRIVE -----+  +-- FILTER -------+
 *   | Decay  Shape  Drop  D.Time   |  | Drive  Char  |  | Cutoff  Cut 2   |
 *   | Curve  Wave   Harm2 Harm3    |  | Punch  Fdbk  |  | Reso    Env     |
 *   | Skew   Sync   Reson Fold     |  | Click  Tone  |  | EnvDec  FM      |
 *   +-- TIMBRE ---------------------+  | Noise  Color |  | F.Drive Crush   |
 *   | Vowel  FM     Ratio Rectify  |  +--------------+  +-----------------+
 *   +------------------------------+
 */

#include <cmath>
#include <cstdio>

#include "audio/engines/Kick32v3.h"

namespace Kick32v3Panel {

constexpr int COLS = 8;
constexpr int ROWS = 4;
constexpr int POTS = COLS * ROWS;

static_assert(POTS == Kick32v3::PARAM_COUNT, "one pot per parameter");

// Four blocks: BODY (4 x 3) over TIMBRE (4 x 1) on the left, DRIVE and FILTER as 2 x 4 columns.
struct Block { const char* name; const char* subtitle; int firstPot, col, row, cols, rows; };
constexpr int BLOCKS = 4;
inline const Block& block(int b)
{
    static const Block blocks[BLOCKS] = {
        { "BODY", "the hit: amp, pitch drop, wave", 0, 0, 0, 4, 3 },
        { "TIMBRE", "formants, FM, rectifier", 12, 0, 3, 4, 1 },
        { "DRIVE", "one saturation, click + noise layered in", 16, 4, 0, 2, 4 },
        { "FILTER", "LP > HP, tracking the pitch", 24, 6, 0, 2, 4 },
    };
    return blocks[b];
}
inline int blockOf(int pot) { return pot < 12 ? 0 : (pot < 16 ? 1 : (pot < 24 ? 2 : 3)); }
inline int potCol(int pot) { const Block& b = block(blockOf(pot)); return b.col + (pot - b.firstPot) % b.cols; }
inline int potRow(int pot) { const Block& b = block(blockOf(pot)); return b.row + (pot - b.firstPot) / b.cols; }

// Captions: a group of consecutive pots on one line
struct Group { int firstPot, count; const char* name; };
constexpr int GROUPS = 17;
inline const Group& group(int g)
{
    static const Group groups[GROUPS] = {
        { 0, 2, "AMP" }, { 2, 2, "PITCH DROP" },
        { 4, 1, "DROP" }, { 5, 3, "WAVE" },
        { 8, 2, "PHASE" }, { 10, 2, "TIMBRE" },
        { 12, 1, "VOWEL" }, { 13, 2, "FM" }, { 15, 1, "RECT" },
        { 16, 2, "SATURATE" }, { 18, 2, "MAD" }, { 20, 2, "CLICK" }, { 22, 2, "NOISE" },
        { 24, 2, "CUTOFFS" }, { 26, 2, "RESO / ENV" }, { 28, 2, "ENV DECAY / FM" }, { 30, 2, "OUT" },
    };
    return groups[g];
}

class Panel {
public:
    Kick32v3& engine;

    explicit Panel(Kick32v3& e)
        : engine(e)
    {
        for (int i = 0; i < POTS; ++i) {
            defaults[i] = norm(i);
            smooth[i] = defaults[i];
        }
    }

    Param& param(int pot) { return engine.params[pot]; }

    // Current parameter position as 0..1
    float norm(int pot)
    {
        Param& p = param(pot);
        return (p.value - p.min) / (p.max - p.min);
    }

    float defaultNorm(int pot) const { return defaults[pot]; }

    // Absolute set, 0..1 (emulator)
    void set(int pot, float n)
    {
        n = n < 0.0f ? 0.0f : (n > 1.0f ? 1.0f : n);
        Param& p = param(pot);
        p.set(p.min + n * (p.max - p.min));
    }

    void reset(int pot) { set(pot, defaults[pot]); }

    // Raw ADC reading 0..1 (hardware): one-pole smoothing + dead band against jitter
    void update(int pot, float raw)
    {
        smooth[pot] += 0.2f * (raw - smooth[pot]);
        float d = smooth[pot] - norm(pot);
        if (d > 0.004f || d < -0.004f) set(pot, smooth[pot]);
    }

    // "350 ms", "-35%" ... for display
    void formatValue(int pot, char* buf, size_t size)
    {
        Param& p = param(pot);
        const char* unit = p.unit ? p.unit : "";
        float v = p.value;
        if (p.step >= 1.0f || (p.max - p.min) >= 100.0f) snprintf(buf, size, "%d%s%s", (int)std::lround(v), unit[0] == '%' ? "" : " ", unit);
        else snprintf(buf, size, "%.1f %s", v, unit);
    }

    bool isCentered(int pot) { return param(pot).min < 0.0f && param(pot).max > 0.0f && std::abs(param(pot).min + param(pot).max) < 0.001f; }

private:
    float defaults[POTS];
    float smooth[POTS];
};

} // namespace Kick32v3Panel
