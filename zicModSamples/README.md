# zicModSamples

An 8-track PCM sample drum machine and 16-step sequencer designed for the **LilyGO T-Display-S3** (ESP32-S3) and Linux Desktop. Designed to sync seamlessly alongside `zicModKick`.

<img src='emu/screenshot_0.png' width='450'> <img src='emu/screenshot_1.png' width='450'>
<img src='emu/screenshot_2.png' width='450'> <img src='emu/screenshot_3.png' width='450'>

---

## Features

- **8 Sample Playback Tracks**:
  - `Snare`, `Clap`, `HatCl`, `HatOp`, `Perc1`, `Perc2`, `Tom`, `FX`.
  - Zero-latency 16-bit mono 22.05 kHz PCM sample player with pitch shifting (-12..+12 semitones) and volume controls.
- **16-Step Pattern Sequencer**:
  - Independent 16-step patterns for each of the 8 tracks.
  - **Smooth Mute / Unmute**: Muting stops new step triggers while allowing active sound release tails to decay down to silence.
  - Algorithmic drum pattern generator.
  - BPM tempo clock (60 to 240 BPM).
- **320x170 Horizontal Display & Touch Gestures**:
  - Slide/Swipe left or right with your finger (or mouse drag on desktop simulator) to switch views:
    1. **Overview & Mute Grid**: Live step playhead progress, 8-track mute buttons, track selection.
    2. **16-Step Sequencer**: Interactive step grid for toggling steps of the active track.
    3. **Sample Editor**: Real-time sample selection, pitch shifting (-12..+12 ST), and volume controls.
    4. **Global Settings**: Play/Pause toggle, BPM adjustment, pattern generator.
- **NeoTrellis Keypad & Keyboard Mappings**:
  - 16 physical buttons (4x4 matrix). On desktop simulator, mapped to PC keyboard keys:
    ```
    1  2  3  4  -> Pads 0,  1,  2,  3
    q  w  e  r  -> Pads 4,  5,  6,  7
    a  s  d  f  -> Pads 8,  9, 10, 11
    z  x  c  v  -> Pads 12, 13, 14, 15
    ```

---

## Desktop Simulator (SFML + ALSA)

### Build & Run

From the repository root:

```bash
make -C zicModSamples
```

Or explicitly build:

```bash
make -C zicModSamples build_zic
```

Run the compiled desktop executable:

```bash
make -C zicModSamples run
```

---

## LilyGO T-Display-S3 Firmware Setup

### Flashing with PlatformIO

1. Connect your **LilyGO T-Display-S3** board via USB-C.
2. Build and upload using `make`:

```bash
make -C zicModSamples up
```

or:

```bash
make -C zicModSamples esp32
```

3. Convert audio samples:

```bash
make -C zicModSamples samples
```

4. Open serial monitor (optional):

```bash
pio device monitor -d zicModSamples/esp32
```
