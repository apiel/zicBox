# zicPixMini - Eurorack Groovebox Module

**zicPixMini** is a compact, performance-oriented Eurorack groovebox application designed for custom hardware built around the **Raspberry Pi Zero 2W**. 

Featuring a vertical **240x320 portrait display**, **4 push rotary encoders** mounted at the four corners of the screen, and **6 dedicated navigation buttons** arranged in 2 rows of 3 (`A`, `S`, `D` on row 1; `Z`, `X`, `C` on row 2), `zicPixMini` inherits the aesthetic and parameter design of `zicPixelDrift`.

![Drums View](screenshot_0.png)
![Synth 1 View](screenshot_1.png)
![Synth 2 View](screenshot_4.png)
![Master Mixer View](screenshot_7.png)
![Step Sequencer View](screenshot_9.png)

---

## UI Layout & Sound Engines

1. **Top Parameter Grid (Y = 0..68)**:
   - 4 active encoders arranged as **2 rows of 2 parameters** using `#include "ui/uiParams.h"`.
   - Enc 1 (Top-Left): X=0, Y=0
   - Enc 2 (Top-Right): X=120, Y=0
   - Enc 3 (Bottom-Left): X=0, Y=34
   - Enc 4 (Bottom-Right): X=120, Y=34

2. **View Title & Pagination Dots (Y = 68..84)**:
   - View title centered in part theme color with page indicator dots (`[page / total]`) on the right.

3. **Middle Visualizer Area (Y = 84..260)**:
   - **Drums View (`VIEW_DRUMS`)**: Intentionally kept empty for a clean aesthetic. Parameters cover the 4 drum volumes (Snare Vol, Closed HH Vol, Open HH Vol, Clap Vol).
   - **Synth 1 View**: Render waveform morphing geometry, resonant SVF filter curve, LFO routing icon & dotted line, pitch ribbon.
   - **Synth 2 View**: Render 3D perspective wavetable waterfall mesh, filter response curve, pitch ribbon.
   - **Master View**: 4-channel VU level meters & scope ribbon.
   - **Sequencer View**: 16-step pattern grid across 4 drum lanes & 2 synth tracks.

4. **Bottom Control Bar (Y = 264..320)**:
   - Row 1: `[A] DRM` | `[S] SYN1` | `[D] SYN2`
   - Row 2: `[Z] MST` | `[X] SEQ`  | `[C] PLAY`

---

## Control Mapping

### Navigation Buttons (2 Rows of 3)
- **Button A ('A')**: Drums View (`VIEW_DRUMS` - Snare, CHH, OHH, Clap volumes)
- **Button S ('S')**: Synth 1 View (Cycles Pages 1-3: Osc/Flt, Env/Morph, Mod/Crush)
- **Button D ('D')**: Synth 2 View (Cycles Pages 1-3: WT/Flt, Pitch/Env, Mod/Send)
- **Button Z ('Z')**: Master View (Toggles Page 1: Mixer Volumes / Page 2: Master FX & BPM)
- **Button X ('X')**: Step Sequencer (Multi-lane step pattern editor)
- **Button C ('C')**: Play / Pause transport toggle
- **Press 'P'**: Open Raspberry Pi shutdown confirmation modal

---

## Building & Running

### Desktop Emulation (SFML)
```bash
cd zicPixMini
make build_zic
./build/x86/zicPixMini
```

### Cross-Compiling for Raspberry Pi Zero 2W
```bash
cd zicPixMini
make buildPi
```
