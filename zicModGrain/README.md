# zicModGrain - Granular Synthesizer Module

`zicModGrain` is a performance-focused Eurorack/desktop granular synthesizer module designed for the **ESP32-S3 1.47-inch LCD Development Board** (Amazon ASIN `B0H7LZT3ZD`) and **Linux Desktop Simulator**. It features 8 dedicated analog potentiometers, 1 push rotary encoder, real-time MIDI clock synchronization, PDM audio output, and an interactive LCD interface.

---

## Key Features

* **Granular Synthesis Engine:**
  * Multi-grain playback (up to 16 concurrent grains) powered by `Grains` DSP.
  * Independent control over grain density, grain duration, loop start, loop length, grain detuning, pitch randomization, and grain position delay randomization.
  * Granular detune modes: `Positive`, `Symmetric`, and `Negative`.
  * Playback direction modes: `Forward`, `Backward`, and `Random`.
* **State Variable Morphing Filter:**
  * Low-Pass / High-Pass state-variable morphing filter with cutoff frequency and resonance control.
* **100% Hardware Pin Optimization:**
  * **8 Analog Potentiometers** mapped directly to primary synthesis parameters.
  * **1 Push Encoder**: Quadrature rotary channels plus **TX Pin (GPIO 43)** configured as digital input (`INPUT_PULLUP`) for the push switch click.
  * **1 PDM Audio Output Pin**: Single-pin 16-bit 44.1 kHz PDM I2S audio output on **GPIO 1**.
  * **1 MIDI Clock RX Pin**: Hardware Serial 1 RX on **GPIO 44** (31,250 baud) for direct 3.3V clock sync from `zicModKick` or external clock with automatic 500 ms internal fallback.
* **1.47-Inch Color LCD Interface & Desktop Emulator:**
  * 172x320 resolution display featuring real-time granular waveform display, active grain playhead indicators, encoder menu, and 1.5-second pot takeover popups.

---

## Hardware & Bill of Materials (BOM)

| Component | Description | Pin / Location |
|-----------|-------------|----------------|
| **Microcontroller Board** | ESP32-S3 1.47" LCD Board (ASIN `B0H7LZT3ZD`) | Core Processor & Display |
| **Display** | ST7789 172x320 Color LCD Display | MOSI: `45`, SCLK: `40`, CS: `42`, DC: `41`, RST: `39`, BL: `48` |
| **Encoder Push Click (SW)** | Push Switch | **TX Pin (`GPIO 43`)** |
| **Encoder Phase A / B** | Rotary Quadrature Inputs | `GPIO 2` (A), `GPIO 3` (B) |
| **MIDI Clock Input** | Hardware Serial 1 RX (31,250 baud) | **RX Pin (`GPIO 44`)** |
| **Audio Output** | Single-Pin PDM I2S Audio Output | **Audio Pin (`GPIO 1`)** |
| **Potentiometer 1** | Analog Potentiometer (`Density`) | `GPIO 4` (ADC) |
| **Potentiometer 2** | Analog Potentiometer (`Loop Start`) | `GPIO 5` (ADC) |
| **Potentiometer 3** | Analog Potentiometer (`Loop Len`) | `GPIO 6` (ADC) |
| **Potentiometer 4** | Analog Potentiometer (`Grain Detune`) | `GPIO 7` (ADC) |
| **Potentiometer 5** | Analog Potentiometer (`Grain Delay`) | `GPIO 8` (ADC) |
| **Potentiometer 6** | Analog Potentiometer (`Rand Detune`) | `GPIO 9` (ADC) |
| **Potentiometer 7** | Analog Potentiometer (`Rand Delay`) | `GPIO 10` (ADC) |
| **Potentiometer 8** | Analog Potentiometer (`Cutoff`) | `GPIO 11` (ADC) |

---

## Inter-Board Connection Diagram (zicModKick $\rightarrow$ zicModGrain)

```
+------------------------------------+          +-----------------------------------+
|      Daisy Seed (zicModKick)       |          |     ESP32-S3 (zicModGrain)        |
|                                    |          |                                   |
|   D6 (UART5 TX)  --> [Front Jack]  |          |                                   |
|   D13 (USART1 TX)---------------------------->| RX (GPIO 44)                      |
|   VIN (5V)       <--- 5V Power ---><----------| 5V / VIN                          |
|   GND            <--- Common GND --><---------| GND                               |
+------------------------------------+          +-----------------------------------+
```

* **Seamless Clock Locking:** When connected, `zicModGrain` automatically synchronizes to incoming 24 PPQN MIDI clock pulses. If external clock pulses pause for more than 500 ms, `zicModGrain` seamlessly falls back to its internal tempo.

---

## Parameter Control Division

To maximize hands-on performance control without menu diving, parameters are strictly divided between the 8 potentiometers and the encoder menu. Parameters mapped to physical pots **do not appear in the encoder menu**:

| Control Method | Parameter | Description | Range |
|----------------|-----------|-------------|-------|
| **Pot 1** | `Density` | Active grain count | 1 to 16 grains |
| **Pot 2** | `Loop Start` | Sample loop start position | 0.0% to 100.0% |
| **Pot 3** | `Loop Len` | Loop window length | 0 to 4000 ms |
| **Pot 4** | `Grain Detune` | Grain pitch detune spread | 0.0 to 12.0 st |
| **Pot 5** | `Grain Delay` | Grain duration / delay window | 5 to 500 ms |
| **Pot 6** | `Rand Detune` | Pitch randomization amount | 0% to 100% |
| **Pot 7** | `Rand Delay` | Grain position / timing delay randomization | 0% to 100% |
| **Pot 8** | `Cutoff` | Morphing filter cutoff frequency | -100% to +100% |
| **Encoder Menu 1** | `File` | Select audio sample file | WAV file list |
| **Encoder Menu 2** | `Start` | Crop sample start boundary | 0.0% to 100.0% |
| **Encoder Menu 3** | `End` | Crop sample end boundary | 0.0% to 100.0% |
| **Encoder Menu 4** | `Attack` | Envelope attack time | 0.0% to 100.0% |
| **Encoder Menu 5** | `Release` | Envelope release time | 0.0% to 100.0% |
| **Encoder Menu 6** | `Detune Mode` | Detune distribution mode | `Positive`, `Symmetric`, `Negative` |
| **Encoder Menu 7** | `Direction` | Grain playback direction | `Forward`, `Backward`, `Random` |
| **Encoder Menu 8** | `Resonance` | Filter resonance | 0% to 100% |
| **Encoder Menu 9** | `Transpose` | Global semitone pitch transpose | -24 to +24 st |
| **Encoder Menu 10** | `Volume` | Master output gain | 0% to 100% |
| **Encoder Menu 11** | `BPM` | Internal BPM / MIDI sync status | 40 to 240 BPM |
| **Encoder Menu 12** | `PLAY / STOP` | Audio playback toggle | `PLAYING` / `STOPPED` |

---

## Desktop Simulator Layout

In the Linux desktop simulator, controls are organized into a 3-row modular panel:

```
+-----------------------------------------------------------------------+
|                    1.47-INCH LCD DISPLAY (320x172)                    |
+-----------------------------------------------------------------------+
|  [Pot 1] Density    |  [Pot 2] Loop Start   |  [Pot 3] Loop Len       |
+---------------------+-----------------------+-------------------------+
|  [Pot 4] Grn Detune |  [Pot 5] Grn Delay    |  [Pot 6] Rand Detune    |
+---------------------+-----------------------+-------------------------+
|  [Pot 7] Rand Delay |  [Pot 8] Cutoff       |  [Encoder & Push SW]    |
+-----------------------------------------------------------------------+
```

---

## Build & Compilation

### 1️⃣ Linux Desktop Simulator (SFML & ALSA)

To build the desktop simulator executable:

```bash
make -C zicModGrain build_zic
```

To run the simulator:

```bash
make -C zicModGrain run
```

### 2️⃣ ESP32-S3 Hardware Target (PlatformIO)

To flash the firmware directly to your ESP32-S3 1.47" LCD board via USB-C:

```bash
make -C zicModGrain up
```

or:

```bash
pio run -d zicModGrain/esp32 -t upload
```
