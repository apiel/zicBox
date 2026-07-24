# zicDrop

The **zicDrop** is a generative groovebox and standalone synthesizer designed for algorithmic drum pattern generation, techno sub-kick crafting, and dynamic live performance. Powered by a Raspberry Pi Zero 2W (or standard desktop runtime via SFML), it pairs an algorithmic rhythm & variation generator ("Sequence Brain") with a dedicated sub-kick/rumble synthesis engine and a versatile texture/synth engine.

## Key Features

* **Algorithmic Rhythm Generator ("Sequence Brain"):**
  * Controlled syncopated ghost notes and phrase-end rumble rolls.
  * Programmable synth trigger stepping and rate division.
* **Dedicated Techno Kick & Sub-Rumble Engine:**
  * **Kick Body:** Tunable base frequency, decay envelope, pitch sweep depth, sweep length, and VCO morphing.
  * **Transient & Drive:** Adjustable click amount, click decay, and warm overdrive saturation.
  * **Sub-Rumble Tail:** Dedicated techno rumble tail generator with adjustable level and gap timing.
* **Texture & Synth Engine:**
  * Morphable oscillator waveforms (Saw, Square, Noise) with waveshaping and filter morphing.
  * Flexible modulation routing (Cutoff, Pitch, Waveform, Level) with LFO (Triangle, Sawtooth, Sample & Hold) and Envelope depth/speed controls.
  * Built-in FM synthesis morphing and feedback delay effect with mix, time, and feedback controls.
* **Master FX & Output Controls:**
  * Master volume control, master fold distorter/waveshaper, and texture mix balancing.
* **32-Encoder Interface:**
  * Distinct color-coded parameter sections (**Brain**, **Kick**, **Synth**, **Master**) for intuitiveness on hardware encoders or desktop controls.

![screenshot_0](zicDrop/screenshot_0.png)

## Running desktop build locally

To compile and run zicDrop on desktop (Linux with SFML & ALSA installed):

```bash
cd zicDropV2
make
```
