# 27 zicModDrums

A 4-track performance drum synthesizer and 16-step sequencer designed for the **LilyGO T-Display-S3** (ESP32-S3) and Linux Desktop. Designed to sync seamlessly alongside `zicModKick`.

<img src='emu/screenshot_0.png' width='450'> <img src='emu/screenshot_1.png' width='450'>
<img src='emu/screenshot_2.png' width='450'> <img src='emu/screenshot_3.png' width='450'>

---

## Features

- **4 Multi-Drum Synth Tracks**:
  - **Track 1 (Kick)**: `DrumKick2` (Sub freq, duration, sweep punch, drive, filter tone).
  - **Track 2 (Snare)**: `DrumSnare` (Body decay, ring harmonic, snappy noise, tone).
  - **Track 3 (HiHat)**: `DrumHiHat23` (Inharmonic metallic 6-oscillator bank, open/closed tail).
  - **Track 4 (Perc)**: `DrumPercussion` (FM/tonal bend, harmonics, noise shape, drive).
- **16-Step Pattern Sequencer**:
  - Independent 16-step patterns for each track (`Step pattern[4][16]`).
  - **Smooth Mute / Unmute**: Muting stops new step triggers while allowing active sound release tails to decay down to silence.
  - Algorithmic drum pattern generator.
  - BPM tempo clock (60 to 240 BPM).
- **320x170 Horizontal Display & Touch Gestures**:
  - Slide/Swipe left or right with your finger (or mouse drag on desktop simulator) to switch views:
    1. **Overview & Mute Grid**: Live step playhead progress, track mute buttons, track selection.
    2. **16-Step Sequencer**: 4x4 interactive step grid for toggling steps of the active track.
    3. **Sound Editor**: Real-time parameter sliders & adjust buttons (`-` / `+`) for synthesis tweaking.
    4. **Global Settings**: Play/Pause toggle, BPM adjustment, pattern generator.
- **NeoTrellis Keypad & Keyboard Mappings**:
  - 16 physical buttons (4x4 matrix). On desktop simulator, mapped to PC keyboard keys:
    ```
    1  2  3  4  -> Pads 0,  1,  2,  3
    q  w  e  r  -> Pads 4,  5,  6,  7
    a  s  d  f  -> Pads 8,  9, 10, 11
    z  x  c  v  -> Pads 12, 13, 14, 15
    ```
  - In **Step View**: Pads `0..15` toggle steps `1..16` of the selected track.
  - In **Overview**: Pads `0..3` select track 0..3, Pads `4..7` toggle track mutes.

---

## Desktop Simulator (SFML + ALSA)

### Build & Run

From the repository root:

```bash
make -C zicModDrums
```

Or explicitly build:

```bash
make -C zicModDrums build_zic
```

Run the compiled desktop executable:

```bash
make -C zicModDrums run
```

### Desktop Controls

- **Mouse Drag Left / Right**: Touch swipe gesture between views.
- **Mouse Left Click**: Tap UI buttons, track rows, step pads, sliders.
- **Keyboard Keys `1..4`, `q..r`, `a..f`, `z..v`**: NeoTrellis 4x4 pads simulation.
- **Keyboard `Space`**: Play / Pause toggle.
- **Keyboard `Left` / `Right` Arrow**: Next / Previous view.

---

## LilyGO T-Display-S3 Firmware Setup

### Hardware BOM & Pinout

| Function | ESP32-S3 Pin | Note |
|---|---|---|
| **ST7789 Display** | SPI Pins (11, 12, 10, 13, 1, 38) | Built-in 320x170 TFT |
| **Touch Controller** | SDA: `18`, SCL: `17` | CST816S / FT6336 |
| **I2S Audio Output** | BCK: `5`, WS: `6`, DOUT: `7` | PCM Audio Output / Jack |
| **Adafruit NeoTrellis** | I2C (Address `0x2E`) | 4x4 Keypad |

### Flashing with PlatformIO

1. Connect your **LilyGO T-Display-S3** board via USB-C.
2. Build and upload using `make`:

```bash
make -C zicModDrums up
```

or:

```bash
make -C zicModDrums esp32
```

3. Open serial monitor (optional):

```bash
pio device monitor -d zicModDrums/esp32
```
