# 26.3 zicModWave

`zicModWave` is a compact, performance-focused Eurorack/desktop synthesizer module powered by the **`DriftSynth1`** synthesis engine (`audio/engines/DriftSynth1.h`) for the **ESP32-S3 1.47-inch LCD Development Board** (Amazon ASIN `B0H7LZT3ZD`) and **Linux Desktop Simulator**. It features 8 dedicated analog potentiometers, 1 push rotary encoder, real-time MIDI clock/notes synchronization, PDM audio output, and an interactive LCD interface.

---

## Key Features

* **Wavetable & VA Synthesis Engine (`DriftSynth1`):**
  * 4-Waveform Morphing Oscillator: Triangle $\rightarrow$ Sawtooth $\rightarrow$ Square $\rightarrow$ Noise.
  * State Variable Morphing Filter (SVF): Low-Pass $\rightarrow$ Band-Pass $\rightarrow$ High-Pass with cutoff frequency and resonance control.
  * Modulation Matrix with 16 routings: `ENV Cutoff`, `ENV Pitch`, `ENV Wave`, `ENV Crsh/FM`, `LFO Tri Cut`, `LFO Tri Pit`, `LFO Tri Wave`, `LFO Tri Lvl`, `LFO Tri CFM`, `LFO Saw Cut`, `LFO Saw Pit`, `LFO Saw Wave`, `LFO Saw CFM`, `LFO S&H Cut`, `LFO S&H Pit`, `LFO S&H CFM`.
  * Dual Bitcrusher & FM Modulation section.
* **100% Hardware Pin Optimization:**
  * **8 Analog Potentiometers** mapped directly to key synthesis parameters for instant hands-on sound design without menu diving.
  * **1 Push Encoder**: Quadrature rotary channels plus **TX Pin (GPIO 43)** configured as digital input (`INPUT_PULLUP`) for the push switch click.
  * **1 PDM Audio Output Pin**: Single-pin 16-bit 44.1 kHz PDM I2S audio output on **GPIO 1**.
  * **1 MIDI Clock/Notes RX Pin**: Hardware Serial 1 RX on **GPIO 44** (31,250 baud) for direct 3.3V clock sync from `zicModKick` or external clock with automatic 500 ms internal fallback.
* **1.47-Inch Color LCD Interface & Desktop Emulator:**
  * 172x320 resolution display featuring real-time oscillator waveform contour, filter response curve visualization, encoder menu, and 1.5-second pot takeover popups.

---

## Hardware & Bill of Materials (BOM)

| Component | Description | Pin / Location |
|-----------|-------------|----------------|
| **Microcontroller Board** | ESP32-S3 1.47" LCD Board (ASIN `B0H7LZT3ZD`) | Core Processor & Display |
| **Display** | ST7789 172x320 Color LCD Display | MOSI: `45`, SCLK: `40`, CS: `42`, DC: `41`, RST: `39`, BL: `48` |
| **Encoder Push Click (SW)** | Push Switch | **TX Pin (`GPIO 43`)** |
| **Encoder Phase A / B** | Rotary Quadrature Inputs | `GPIO 2` (A), `GPIO 3` (B) |
| **MIDI Input / Sync** | Hardware Serial 1 RX (31,250 baud) | **RX Pin (`GPIO 44`)** |
| **Audio Output** | Single-Pin PDM I2S Audio Output | **Audio Pin (`GPIO 1`)** |
| **Potentiometer 1** | Analog Potentiometer (`Pitch`) | `GPIO 4` (ADC) |
| **Potentiometer 2** | Analog Potentiometer (`Wave`) | `GPIO 5` (ADC) |
| **Potentiometer 3** | Analog Potentiometer (`Cutoff`) | `GPIO 6` (ADC) |
| **Potentiometer 4** | Analog Potentiometer (`Reso`) | `GPIO 7` (ADC) |
| **Potentiometer 5** | Analog Potentiometer (`Release`) | `GPIO 8` (ADC) |
| **Potentiometer 6** | Analog Potentiometer (`Env Amt`) | `GPIO 9` (ADC) |
| **Potentiometer 7** | Analog Potentiometer (`Filt Morph`) | `GPIO 10` (ADC) |
| **Potentiometer 8** | Analog Potentiometer (`Crsh / FM`) | `GPIO 11` (ADC) |

---

## Inter-Board Connection Diagram (zicModKick $\rightarrow$ zicModWave)

```
+------------------------------------+          +-----------------------------------+
|      Daisy Seed (zicModKick)       |          |     ESP32-S3 (zicModWave)         |
|                                    |          |                                   |
|   D6 (UART5 TX)  --> [Front Jack]  |          |                                   |
|   D13 (USART1 TX)---------------------------->| RX (GPIO 44)                      |
|   VIN (5V)       <--- 5V Power ---><----------| 5V / VIN                          |
|   GND            <--- Common GND --><---------| GND                               |
+------------------------------------+          +-----------------------------------+
```

---

## Parameter Control Division

Parameters mapped to the 8 physical potentiometers **do not appear in the encoder menu**:

| Control Method | Parameter | Description | Range |
|----------------|-----------|-------------|-------|
| **Pot 1** | `Pitch` | Oscillator pitch semitones | 24 to 72 (C1..C5) |
| **Pot 2** | `Wave` | Oscillator waveform morphing | Tri $\rightarrow$ Saw $\rightarrow$ Sq $\rightarrow$ Noise |
| **Pot 3** | `Cutoff` | Filter cutoff frequency | 20 Hz to 20 kHz |
| **Pot 4** | `Reso` | Filter resonance amount | 0% to 95% |
| **Pot 5** | `Release` | Amp & filter envelope release time | 10 to 2000 ms |
| **Pot 6** | `Env Amt` | Filter envelope modulation amount | 0% to 100% |
| **Pot 7** | `Filt Morph` | Filter type morphing | Low-Pass $\rightarrow$ Band-Pass $\rightarrow$ High-Pass |
| **Pot 8** | `Crsh / FM` | Bitcrusher [-100%..0%] & FM depth [0%..+100%] | -100% to +100% |
| **Encoder Menu 1** | `Mod Type` | Modulation matrix routing | 16 Matrix Routings |
| **Encoder Menu 2** | `Mod Depth` | Modulation depth | -100% to +100% |
| **Encoder Menu 3** | `Mod Speed` | LFO modulation speed | 0% to 100% |
| **Encoder Menu 4** | `Dly Send` | Delay FX send level | 0% to 100% |
| **Encoder Menu 5** | `Volume` | Master output gain | 0% to 100% |
| **Encoder Menu 6** | `BPM` | Internal BPM / MIDI sync status | 40 to 240 BPM |
| **Encoder Menu 7** | `PLAY / STOP` | Audio playback toggle | `PLAYING` / `STOPPED` |

---

## Desktop Simulator Layout

```
+-----------------------------------------------------------------------+
|                    1.47-INCH LCD DISPLAY (320x172)                    |
+-----------------------------------------------------------------------+
|  [Pot 1] Pitch      |  [Pot 2] Wave         |  [Pot 3] Cutoff         |
+---------------------+-----------------------+-------------------------+
|  [Pot 4] Reso       |  [Pot 5] Release      |  [Pot 6] Env Amt        |
+---------------------+-----------------------+-------------------------+
|  [Pot 7] Filt Morph |  [Pot 8] Crsh / FM    |  [Encoder & Push SW]    |
+-----------------------------------------------------------------------+
```

---

## Build & Compilation

### 1️⃣ Linux Desktop Simulator (SFML & ALSA)

To build the desktop simulator executable:

```bash
make -C zicModWave build_zic
```

To run the simulator:

```bash
make -C zicModWave run
```

### 2️⃣ ESP32-S3 Hardware Target (PlatformIO)

To flash the firmware directly to your ESP32-S3 1.47" LCD board via USB-C:

```bash
make -C zicModWave up
```

or:

```bash
pio run -d zicModWave/esp32 -t upload
```
