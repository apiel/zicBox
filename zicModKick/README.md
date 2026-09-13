# 26 ZicModKick

<img src='zicModKick/zicModKick.png?raw=true' width='600'>

The **ZicModKick** is a compact, performance-driven Eurorack modular synthesizer and Kick drum engine powered by the **Electro-Smith Daisy Seed** (ARM Cortex-M7 @ 480MHz). It combines dual high-performance Kick synthesis engines, a 64-step algorithmic pattern sequencer, master 2-band EQ, and a dedicated 10-potentiometer tactile interface for instant hands-on sound design without menu-diving.

## Key Features

* **Dual Synthesis Engines:**
  * **`KickWave`:** Advanced multi-harmonic parametric Kick engine featuring customizable harmonics (Harmonic 2 & 3), wave shaping, wave skewing, wave folding, phase offset, FM depth, drive, resonator, and bitcrushing.
  * **`PotKick`:** Classic punchy drum synth engine featuring body pitch/decay control, FM snap and depth, VCO morphing, sweep depth and shape, wavefolding, bitcrush, and resonator.
* **Hands-on 10-Potentiometer Interface:** 10 dedicated potentiometers mapped directly to key synthesis parameters for immediate tactile control during performance.
* **Master 2-Band EQ:** Global LOW and MID shelf/peaking equalizer (`EQ Low`, `EQ Mid`) active across both synthesis engines.
* **64-Step Sequence Brain:**
  * 8 Predefined Hardcoded Techno Patterns progressing from basic 4-on-the-floor to complex syncopated grooves, pitch turnarounds (+1/+2 semitone pitch variations on turnarounds), industrial rolls, and peak escalations.
  * Real-time **Note Repeat** (stutter roll generator).
  * Shift-key combination shortcuts for body muting and live transposition.
* **MIDI Synchronization:** Integrated hardware MIDI Master Clock output running over UART (pin D6) at 31,250 baud.
* **OLED & Encoder Display Interface:** 64x32 I2C OLED screen driven by a single clickable rotary encoder for deep system and sequencer adjustments.

---

## Hardware & Bill of Materials (BOM)

| Component | Description | Pin / Location |
|-----------|-------------|----------------|
| **Microcontroller** | Electro-Smith Daisy Seed (ARM Cortex-M7) | Core Processor |
| **Display** | SSD1306 64x32 I2C OLED | SDA: `D12`, SCL: `D11` |
| **Rotary Encoder** | Encoder with push button | A: `D8`, B: `D10`, Click: `D9` |
| **Button 1** | Tactile Switch (Shift Key) | `D5` |
| **Button 2** | Tactile Switch (Repeat / Play-Stop) | `D4` |
| **Button 3** | Tactile Switch (Stop Body / Kick Trigger / Mute) | `D3` |
| **MIDI Output** | UART5 TX (31,250 baud) | `D6` |
| **Potentiometer 1** | Analog Potentiometer (FM Depth / Duration) | `A10` |
| **Potentiometer 2** | Analog Potentiometer (Drive) | `A1` |
| **Potentiometer 3** | Analog Potentiometer (Wave Shape / VCO Morph) | `A6` |
| **Potentiometer 4** | Analog Potentiometer (Harmonic 2 / FM Depth) | `A4` |
| **Potentiometer 5** | Analog Potentiometer (Harmonic 3 / FM Snap) | `A11` |
| **Potentiometer 6** | Analog Potentiometer (Wave Skew / Sweep Depth) | `A5` |
| **Potentiometer 7** | Analog Potentiometer (Wave Fold / Sweep Shape) | `A8` |
| **Potentiometer 8** | Analog Potentiometer (Phase Offset / Wavefold) | `A0` |
| **Potentiometer 9** | Analog Potentiometer (Resonator / Bitcrush) | `A3` |
| **Potentiometer 10** | Analog Potentiometer (Bitcrush / Resonator) | `A2` |

---

## Control Layout & Potentiometer Mapping

The 10 physical potentiometers dynamically adapt their target parameters depending on which engine is active:

| Potentiometer | Hardware Pin | `KickWave` Engine | `PotKick` Engine |
|---------------|--------------|-------------------|------------------|
| **Pot 1** | `A10` | FM Depth | Duration |
| **Pot 2** | `A1` | Overdrive / Saturation | Overdrive / Saturation |
| **Pot 3** | `A6` | Wave Shape | VCO Morph |
| **Pot 4** | `A4` | Harmonic 2 Level | FM Depth |
| **Pot 5** | `A11` | Harmonic 3 Level | FM Snap |
| **Pot 6** | `A5` | Wave Skew | Sweep Depth |
| **Pot 7** | `A8` | Wave Fold | Sweep Shape |
| **Pot 8** | `A0` | Phase Offset | Wavefold |
| **Pot 9** | `A3` | Resonator | Bitcrush |
| **Pot 10** | `A2` | Bitcrush | Resonator |

---

## Hardware Navigation & Button Controls

| Control | Action | Function |
|---------|--------|----------|
| **Encoder Rotate** | Scroll | Navigate menu items or adjust value |
| **Encoder Click** | Press | Select / Toggle parameter edit mode |
| **Button 1** | Hold | **Shift** modifier key |
| **Button 2** | Press | Trigger **Note Repeat** (stutter roll) |
| **Button 2** | **Shift + Press** | Toggle Sequencer **PLAY / STOP** |
| **Button 3** | Press | Trigger Kick / Temporary Body Mute |
| **Button 3** | **Shift + Press** | Toggle Persistent Kick Body Mute |

---

## Menu System

Pressing or rotating the encoder accesses the system menu on the 64x32 OLED display:

1. **Engine:** Switch active engine (`0: PotKick`, `1: KickWave`).
2. **BPM:** Adjust sequencer tempo (60 to 240 BPM).
3. **Master Vol:** Adjust master output volume (0% to 100%).
4. **EQ Low:** Adjust Low-shelf master equalizer (-12 dB to +12 dB).
5. **EQ Mid:** Adjust Mid-peaking master equalizer (-12 dB to +12 dB).
6. **Engine-specific Parameters:** Deep parameters not mapped to physical pots (e.g. Pitch, Pitch Envelope, Click, Sub-Rumble).
7. **Pattern:** Select predefined 64-step techno pattern (`1. Basic 4/4`, `2. Subtle Ghost`, `3. Offbeat`, `4. Driving`, `5. Mental`, `6. Rumble`, `7. Peak`, `8. Hard Techno`).
8. **Repeat Rate:** Note repeat stutter speed (1x to 8x).
9. **Transpose:** Transpose sequencer playback (-24 to +24 semitones).
10. **PLAY / STOP:** Toggle playback status.

---

## Build & Compilation

### 1️⃣ Linux Desktop Emulator (SFML & ALSA)

To build and run the standalone desktop simulator with real-time UI display:

```bash
make -C zicModKick/emu
```

Run the compiled executable:

```bash
./zicModKick/emu/zicModKick
```

### 2️⃣ Daisy Seed Hardware Target

Make sure the Daisy toolchain (`arm-none-eabi-gcc` and `libDaisy`) is configured in your environment.

To build the Daisy Seed firmware binary:

```bash
make -C zicModKick/daisy
```

To flash the firmware directly to your Daisy Seed connected via USB in DFU mode:

```bash
cd zicModKick/daisy
make program-dfu
```
