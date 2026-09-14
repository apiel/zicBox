# 26.2 zicModSamples

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
- **MIDI Clock Synchronization & Auto-Fallback**:
  - Receives real-time 24 PPQN MIDI Clock (`0xF8`), Start (`0xFA`), and Stop (`0xFC`) on **GPIO 16** via direct 3.3V serial link from `zicModKick`.
  - **Automatic Clock Fallback**: If no external MIDI clock pulse is detected for 500 ms, `zicModSamples` automatically switches back to its internal BPM clock without interrupting audio playback.
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

## Hardware & Pin Layout (LilyGO T-Display-S3)

| Component | Description | Pin / Location |
|-----------|-------------|----------------|
| **Microcontroller** | LilyGO T-Display-S3 (ESP32-S3 Dual Core @ 240MHz) | Core Processor |
| **Internal MIDI Link** | Hardware Serial 1 RX (31,250 baud) | `GPIO 16` (Direct wire from Daisy Seed `D14`) |
| **Audio Output** | Single-pin PDM I2S Audio Output | `GPIO 1` |
| **Display** | ST7789 320x170 Color LCD Display | Pins `4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 38, 39` |
| **Touch Screen** | CST816S Capacitive Touch | SDA: `GPIO 18`, SCL: `GPIO 17`, RST: `GPIO 21` |
| **NeoTrellis Keypad** | Adafruit NeoTrellis 4x4 Keypad | I2C Addr `0x2E` (Shared SDA `18` / SCL `17`) |
| **Power Supply** | Common +5V Rail & Ground | `5V`/`VIN` and `GND` |

---

## Inter-Board Connection Diagram (zicModKick $\rightarrow$ zicModSamples)

```
+------------------------------------+          +-----------------------------------+
|      Daisy Seed (zicModKick)       |          |   LilyGO ESP32 (zicModSamples)    |
|                                    |          |                                   |
|   D6 (UART5 TX)  --> [Front Jack]  |          |                                   |
|   D14 (USART1 TX)---------------------------->| GPIO 16 (Serial1 RX)              |
|   VIN (5V)       <--- 5V Power ---><----------| 5V / VIN                          |
|   GND            <--- Common GND --><---------| GND                               |
+------------------------------------+          +-----------------------------------+
```

* **Direct 3.3V Connection:** No optocoupler or level shifter is needed since both chips share a common ground and run at 3.3V logic.
* **Seamless Sync:** `zicModKick` acts as master clock transmitter; `zicModSamples` locks tempo automatically upon receiving clock bytes, and seamlessly falls back to internal tempo when un-plugged or stopped.

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
