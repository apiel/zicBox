#pragma once

/**
 * Front panel model of the zicModKick32_v2: 4 rows x 8 pots.
 *
 * Shared by the desktop emulator and the (future) Daisy firmware, so it must stay
 * free of any UI / OS / audio-driver dependency.
 *
 *   hardware : read the 32 ADC channels, call panel.update(i, raw01) from the main loop
 *   emulator : call panel.set(i, norm) from mouse / wheel events
 *
 * Pot i drives engine.params[i] over its full min..max range.
 */

#include <cstdio>
#include <cmath>

#include "audio/engines/Kick32v2.h"

namespace Kick32v2Panel {

constexpr int COLS = 8;
constexpr int ROWS = 4;
constexpr int POTS = COLS * ROWS;

static_assert(POTS == Kick32v2::PARAM_COUNT, "one pot per parameter");

// The panel is not 4 identical rows: three modules side by side.
//   BODY   pots  0..15  a 4 x 4 block on the left
//   TAIL   pots 16..23  4 x 2, top right
//   MASTER pots 24..31  4 x 2, bottom right
struct Block { const char* name; int firstPot, col, row, cols, rows; };
inline const Block& block(int b)
{
    static const Block blocks[3] = { { "BODY", 0, 0, 0, 4, 4 }, { "TAIL", 16, 4, 0, 4, 2 }, { "MASTER", 24, 4, 2, 4, 2 } };
    return blocks[b];
}
constexpr int BLOCKS = 3;
inline int blockOf(int pot) { return pot < 16 ? 0 : (pot < 24 ? 1 : 2); }
inline int potCol(int pot) { const Block& b = block(blockOf(pot)); return b.col + (pot - b.firstPot) % 4; }
inline int potRow(int pot) { const Block& b = block(blockOf(pot)); return b.row + (pot - b.firstPot) / 4; }

// One caption per line of 4 pots
inline const char* groupName(int pot)
{
    static const char* names[8] = { "AMP / FM", "PITCH", "WAVE", "DIRT", "VOICE", "TIMBRE", "FILTER", "SPREAD / RUMBLE" };
    return names[pot / 4];
}

class Panel {
public:
    Kick32v2& engine;

    explicit Panel(Kick32v2& e)
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

    // "52 Hz", "-3.5 dB" ... for display
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

} // namespace Kick32v2Panel
