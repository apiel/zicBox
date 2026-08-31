# zicPixMini - Eurorack Groovebox Module

**zicPixMini** is a compact, performance-oriented Eurorack groovebox application designed for custom hardware built around the **Raspberry Pi Zero 2W**. 

Featuring a vertical **240x320 portrait display**, **4 push rotary encoders** mounted at the four corners of the screen, and **6 dedicated navigation buttons** arranged in 2 rows of 3, `zicPixMini` delivers immediate hands-on control over drums, synthesis engines, step sequencing, and master FX.

![Drums View 1](screenshot_0.png)
![Synth 1 View](screenshot_2.png)
![Master Mixer View](screenshot_8.png)
![Step Sequencer View](screenshot_10.png)

---

## Sound Engines

1. **Tekno Tribe Drums (`TribeDrums`):**
   - 4-voice multi-element drum engine adapted from `zicTeK`
   - **Snare**: Mental rim & body synthesis with frequency, decay & tone controls
   - **Closed Hi-Hat**: Driving 16th Tekno hi-hat with tightness & decay controls
   - **Open Hi-Hat**: Accentuated cymbal envelope with tightness & decay controls
   - **Clap**: Mental zap & noise burst with clap noise color & transient punch

2. **Lead / Bass Synthesizer (`DriftSynth1`):**
   - Dual-oscillator subtractive & FM synth with 4 morphable waveforms (Tri -> Saw -> Square -> Noise)
   - Resonant state-variable filter with filter morphing (LP -> BP -> HP)
   - Flexible modulation matrix (ENV, LFO Tri, LFO Saw, S&H) targeting Cutoff, Pitch, Morph, or Crush/FM

3. **Wavetable Synthesizer (`DriftWavetable`):**
   - Multi-frame wavetable position morphing with unison & chord modes (Fifth, Octave, Maj7, Min7, Sus4)
   - Resonant filter, attack/release envelope, delay send, and generative drift modulation

4. **Master FX & Multi-Track Sequencer:**
   - 4-channel live mixer (Drums, Synth 1, Synth 2, Master)
   - Real-time **Scatter FX** (Crunch, Drive, Distortion, Acid Glitch)
   - Master soft-clip compressor
   - 16-step multi-lane pattern sequencer covering all 4 drum voices and 2 synth tracks

---

## Hardware Interface & Layout

```
+---------------------------------------------------+
|               ST7789 Display (240x320)            |
|  [Enc 1 (TL)]                         [Enc 2 (TR)]|
|   +---------------------------------+             |
|   |  Top-Left Card  | Top-Right Card |             |
|   +---------------------------------+             |
|   |  Bot-Left Card  | Bot-Right Card |             |
|   +---------------------------------+             |
|  [Enc 3 (BL)]                         [Enc 4 (BR)]|
+---------------------------------------------------+
|  [B1: DRM]      [B2: SYN1]       [B3: SYN2]       |
|  [B4: MST]      [B5: SEQ]        [B6: PLAY/STOP]  |
+---------------------------------------------------+
```

### Display & Encoder Alignment
- **Top-Left Encoder (E1 / P1)**: Controls Top-Left parameter card on screen. Push triggers voice or parameter action.
- **Top-Right Encoder (E2 / P2)**: Controls Top-Right parameter card on screen. Push cycles page or triggers action.
- **Bottom-Left Encoder (E3 / P3)**: Controls Bottom-Left parameter card on screen. Push resets parameters or triggers action.
- **Bottom-Right Encoder (E4 / P4)**: Controls Bottom-Right parameter card on screen. Push toggles mute or trigger.

### Navigation Buttons (2 Rows of 3)
- **Button 1 (B1 / 'Z')**: Drums Page (Toggles Page 1: Snare & Clap / Page 2: Hi-Hats)
- **Button 2 (B2 / 'X')**: Synth 1 Page (Cycles Pages 1-3: Osc/Flt, Env, FX/Mix)
- **Button 3 (B3 / 'C')**: Synth 2 Page (Cycles Pages 1-3: WT/Flt, Env, FX/Mix)
- **Button 4 (B4 / 'V')**: Master Page (Toggles Page 1: Mixer / Page 2: Scatter FX & BPM)
- **Button 5 (B5 / 'B')**: Step Sequencer (Multi-lane step pattern editor)
- **Button 6 (B6 / 'N')**: Play / Pause transport toggle
- **Hold B4 + Press B6 (or 'P')**: Open Raspberry Pi shutdown confirmation modal

---

## Building & Running

### Desktop Emulation (SFML)
```bash
cd zicPixMini
make build_zic
./build/x86/zicPixMini
```
* **Keyboard Controls**:
  - `Z`, `X`, `C`, `V`, `B`, `N`: Navigation Buttons 1..6
  - `1`, `2`, `3`, `4`: Turn Encoders 1..4 (Hold `Shift` for reverse direction)
  - `Q`, `W`, `E`, `R`: Push Encoders 1..4
  - Mouse hover & wheel scroll on corner quadrant cards also turns encoders!

### Cross-Compiling for Raspberry Pi Zero 2W
```bash
cd zicPixMini
make buildPi
```

---

## Custom GPIO Pinout (`config.json`)

To override default GPIO pin assignments, create a `config.json` file (or set `ZIC_CONFIG_PATH`):

```json
{
  "keys": {
    "BTN_1": 20, "BTN_2": 16, "BTN_3": 25,
    "BTN_4": 14, "BTN_5": 12, "BTN_6": 1,
    "PUSH_1": 24, "PUSH_2": 15, "PUSH_3": 7, "PUSH_4": 8
  },
  "encoders": {
    "ENCODER_1": [26, 13],
    "ENCODER_2": [6, 5],
    "ENCODER_3": [0, 9],
    "ENCODER_4": [27, 4]
  }
}
```
