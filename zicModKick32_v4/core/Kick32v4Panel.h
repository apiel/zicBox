#pragma once

/**
 * Front panel model of the zicModKick32_v4: 4 rows x 8 pots.
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
 *   +-- MOTION --------------------+  | Noise  Color |  | F.Drive Vowel   |
 *   | Bend   FM     Ratio Shift    |  +--------------+  +-----------------+
 *   +------------------------------+
 */

#include <cmath>
#include <cstdio>

#include "audio/engines/Kick32v4.h"

namespace Kick32v4Panel {

constexpr int COLS = 8;
constexpr int ROWS = 4;
constexpr int POTS = COLS * ROWS;

static_assert(POTS == Kick32v4::PARAM_COUNT, "one pot per parameter");

// Four blocks: BODY (4 x 3) over MOTION (4 x 1) on the left, DRIVE and FILTER as 2 x 4 columns.
struct Block { const char* name; const char* subtitle; int firstPot, col, row, cols, rows; };
constexpr int BLOCKS = 4;
inline const Block& block(int b)
{
    static const Block blocks[BLOCKS] = {
        { "BODY", "the hit: amp, pitch drop, wave", 0, 0, 0, 4, 3 },
        { "MOTION", "bend, FM, frequency shift", 12, 0, 3, 4, 1 },
        { "DRIVE", "one saturation (2x), click + noise", 16, 4, 0, 2, 4 },
        { "FILTER", "LP > HP tracking the pitch, formants", 24, 6, 0, 2, 4 },
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
        { 12, 1, "BEND" }, { 13, 2, "FM" }, { 15, 1, "SHIFT" },
        { 16, 2, "SATURATE" }, { 18, 2, "PUNCH / FDBK" }, { 20, 2, "CLICK" }, { 22, 2, "NOISE" },
        { 24, 2, "CUTOFFS" }, { 26, 2, "RESO / ENV" }, { 28, 2, "ENV DECAY / FM" }, { 30, 2, "DRIVE / VOWEL" },
    };
    return groups[g];
}

// One line per pot: what it does and its range (shown by the emulator on hover, printed in the docs)
inline const char* help(int pot)
{
    static const char* h[POTS] = {
        "amp decay, 40 ms .. 2 s",
        "amp curve: punchy exponential -> linear -> gated (holds, then rounds off)",
        "pitch drop depth, 0 .. 6 octaves above the note",
        "pitch drop time, 4 .. 512 ms",
        "drop curve: laser (holds, then dives) -> linear -> zap (dives at once)",
        "wave morph: sine -> triangle -> square -> saw, level compensated",
        "2nd harmonic, bipolar",
        "3rd harmonic, bipolar",
        "phase warp, bipolar: pulls the wave left or right, saw-like at the ends",
        "hard sync: a slave oscillator restarted every cycle, its ratio dives with the drop",
        "a resonant peak following the pitch, saturated: it rings on top of the wave",
        "the wave folds over on itself, up to a fold and a half",
        "the tail slides in pitch over the decay: + up to 2 octaves up, - down. 0 = flat",
        "phase modulation of the body by a sine, the index falls over ~45 ms: metallic attack, clean body",
        "FM modulator ratio 0.5x .. 8x: whole numbers are harmonic, in between is metal",
        "frequency shifter after the drive, sweeping down with the drop (1 kHz .. 250 Hz at max): laser -> metal. + up, - down, small = beating",
        "gain into the one saturation, 1x .. 128x (2x oversampled)",
        "saturation character: soft (tube) -> hard clip -> wavefold (2.5 folds) -> rectified fuzz (octave up)",
        "drive envelope, bipolar: + extra drive on the hit, - extra drive in the tail",
        "a comb tuned to the pitch feeding the drive back into itself, bipolar: + reinforces the pitch, - half harmonics",
        "click on the hit, into the drive and after it (so it survives any drive)",
        "click tone: 300 Hz, 8 ms sine knock -> 6 kHz, 1 ms noise tick",
        "noise with the body, its length follows Decay (shorter when bright)",
        "noise filter: LP -> BP -> HP while the cutoff rises, 100 Hz .. 10 kHz",
        "filter 1, low pass, 30 Hz .. 20 kHz. Both cutoffs follow the note, the drop and the bend",
        "filter 2, high pass after the low pass, 15 Hz .. 7.7 kHz",
        "resonance of both filters, up to a scream (soft clipped: it growls)",
        "filter envelope, bipolar: + opens both filters on the hit (6 oct), - closes them",
        "filter envelope decay, 5 ms .. 1 s",
        "audio rate FM of both cutoffs by the signal itself, up to ~2.6 octaves",
        "saturation after the filter, 1 .. 25x",
        "two formant filters on the driven sound: 0 = off, the first fifth brings them in, then a -> e -> i -> o -> u",
    };
    return h[pot];
}

class Panel {
public:
    Kick32v4& engine;

    explicit Panel(Kick32v4& e)
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

} // namespace Kick32v4Panel
