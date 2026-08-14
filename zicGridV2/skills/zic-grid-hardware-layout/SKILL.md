---
name: zic-grid-hardware-layout
description: Architecture standards for zicGridV2 hardware layout, NeoTrellis pad matrices (Dynamic & Global Utility Zone), and 3x4 rotary encoder grid.
---

# zicGridV2 Hardware & Grid Layout Architecture

This skill defines the physical and virtual hardware layout for **zicGridV2**, covering the 48 NeoTrellis RGB pads, the 12 rotary encoders, and their corresponding data structures in C++.

---

## 1. Overview of Layout Boundaries

The zicGridV2 user interface consists of two primary control sections:
- **48 NeoTrellis RGB Pads**: 12 Columns $\times$ 4 Rows (divided into Dynamic Matrix and Global Utility Zone).
- **12 Rotary Encoders**: 4 Columns $\times$ 3 Rows.

```text
+-------------------------------------------------------+-----------------------------+
|               DYNAMIC PAD MATRIX (32 Pads)            |  GLOBAL UTILITY ZONE (16)   |
|            Cols 0..7  x  Rows 0..3 (8x4)               |  Cols 8..11 x Rows 0..3     |
+-------------------------------------------------------+-----------------------------+
|  NeoTrellis 1 (Cols 0..3) | NeoTrellis 2 (Cols 4..7)  |  NeoTrellis 3 (Cols 8..11)  |
|  (Contextual View Content: Sequencer, Notes, Clips)   |  (System Controls & Tracks) |
+-------------------------------------------------------+-----------------------------+

                                ROTARY ENCODERS (12)
                        Cols 0..3  x  Rows 0..2 (4x3 matrix)
                 +--------------+--------------+--------------+--------------+
                 |  Encoder 0   |  Encoder 1   |  Encoder 2   |  Encoder 3   |  Row 0
                 +--------------+--------------+--------------+--------------+
                 |  Encoder 4   |  Encoder 5   |  Encoder 6   |  Encoder 7   |  Row 1
                 +--------------+--------------+--------------+--------------+
                 |  Encoder 8   |  Encoder 9   |  Encoder 10  |  Encoder 11  |  Row 2
                 +--------------+--------------+--------------+--------------+
```

---

## 2. Pad Grid Layout Breakdown

### Dynamic Pad Matrix (NeoTrellis 1 & 2)
- **Columns**: `0` through `7` (8 columns)
- **Rows**: `0` through `3` (4 rows)
- **Total Pads**: 32 pads
- **Hardware Mapping**:
  - **Tile 1 (NeoTrellis 1)**: I2C `0x2E` $\rightarrow$ Columns `0..3`
  - **Tile 2 (NeoTrellis 2)**: I2C `0x30` $\rightarrow$ Columns `4..7`
- **Purpose**: Fully dynamic and contextual to the active `View`.
  - **StepSeqView**: 16 step triggers + 16 pitch/note selector pads.
  - **InstrumentView**: 16 chromatic keyboard notes + 16 synth control/macro pads.
  - **MasterView**: 16 clip slots (4 tracks $\times$ 4 clips) + track select/mute/solo triggers.
  - **MenuView**: Unused / dark matrix.

---

### Global Utility Zone (NeoTrellis 3)
- **Columns**: `8` through `11` (4 columns, referenced as `utilCol = col - 8` in code: `0..3`)
- **Rows**: `0` through `3` (4 rows)
- **Total Pads**: 16 pads
- **Hardware Mapping**:
  - **Tile 3 (NeoTrellis 3)**: I2C `0x2F` $\rightarrow$ Columns `8..11`
- **Desktop Keyboard Mapping & Layout**:

| Row | Desktop Keys | Grid Coordinates | Default Purpose & Label |
|---|---|---|---|
| **Row 0** | `1`, `2`, `3`, `4` | `col=8..11, row=0` | Select Track 1 to Track 4 (`T1`, `T2`, `T3`, `T4`) |
| **Row 1** | `q`, `w`, `e`, `r` | `col=8..11, row=1` | Select Track 5 to Track 8 (`T5`, `T6`, `T7`, `T8`) |
| **Row 2** | `a`, `s`, `d`, `f` | `col=8..11, row=2` | View & Modifier: `a` = Instrument, `s` = StepSeq, `d` = Master, `f` = Shift |
| **Row 3** | `z`, `x`, `c`, `v` | `col=8..11, row=3` | Navigation & Contextual Actions: `z` = Left/Up, `x` = Right/Down, `c` = Oct-/OK, `v` = Oct+/Cancel/Back |

#### Row 3 (`z, x, c, v`) Behavioral Modes:
- **Default / Play Mode**:
  - `z`: Navigation Left / Decrement (`&icon::arrowLeft::filled`)
  - `x`: Navigation Right / Increment (`&icon::arrowRight::filled`)
  - `c`: Octave Down (`Oct-`)
  - `v`: Octave Up (`Oct+`)
- **MenuView Mode** (overrides Row 3):
  - `z`: Up (`&icon::arrowUp::filled`)
  - `x`: Down (`&icon::arrowDown::filled`)
  - `c`: Confirm (`OK`)
  - `v`: Cancel / Back (`Cancel` / `Back`, visible only when applicable)

---

## 3. Rotary Encoder Grid (3 Rows x 4 Columns)

- **Total Encoders**: 12
- **Grid Structure**:
  - **Row 0**: Index `0`, `1`, `2`, `3`
  - **Row 1**: Index `4`, `5`, `6`, `7`
  - **Row 2**: Index `8`, `9`, `10`, `11`
- **Indexing Standards**:
  - **C++ Code**: 0-indexed (`0` through `11`).
  - **Hardware / Config**: 1-indexed (`ENCODER_1` through `ENCODER_12` in `config.json`).
- **Purpose**: Contextual parameter controls mapped by the active view:
  - **InstrumentView**: Filter Cutoff, Resonance, Synth Envelopes, Waveform parameters.
  - **StepSeqView**: Step Note, Velocity, Gate Length, Probability, Ratchet.
  - **MasterView**: Track Volume, Panning, Master Filter, FX Sends.
  - **MenuView**: Single active encoder (Encoder `0`) for option selection.

---

## 4. C++ Implementation & State Structures

### `gridState` Access (`gridState.h`)
```cpp
// Check or update pad state:
PadState& pad = gridState.pads[col][row]; // col: 0..11, row: 0..3

// Configure an encoder:
gridState.setEncoder(encoderIdx, "LABEL", value, minVal, maxVal, stepVal, displayStr, color);

// Encoders cleared when unassigned:
gridState.setEncoder(i, "", 0.0f, 0.0f, 1.0f, 1.0f, nullptr, { 0, 0, 0, 0 });
```

### Event Handling Pipeline
```cpp
// View event handlers:
void handleDynamicPadPress(int col, int row, bool pressed); // col: 0..7
void handleUtilityPadPress(int utilCol, bool pressed);       // utilCol: 0..3 (col 8..11)
void handleEncoder(int encoderId, int delta);                // encoderId: 0..11
```
