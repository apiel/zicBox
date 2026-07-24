# 25 zicDrop

The **zicDrop** is a generative standalone synthesizer designed for algorithmic drum pattern generation, techno sub-kick crafting, and dynamic live performance. It pairs an algorithmic rhythm & variation generator ("Sequence Brain") with a dedicated sub-kick/rumble synthesis engine and a versatile texture/synth engine.

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

## Hardware Development

The current zicDrop implementation is a proof of concept running as a standalone desktop application. In parallel, a smaller prototype has been developed and tested on the **Electro-Smith Daisy Seed** platform, demonstrating that the core synthesis and sequencing concepts can run successfully on embedded hardware.

The long-term goal is to turn zicDrop into a dedicated standalone hardware instrument based on the Daisy Seed platform. The planned hardware interface will feature **32 dedicated potentiometers**, organized into the same functional sections as the software interface: **Brain, Kick, Synth, and Master**.

The dedicated hardware version is intended to provide a hands-on performance experience, allowing all major synthesis, sequencing, and effect parameters to be controlled directly without the need for a computer. The desktop version will continue to serve as a development and prototyping environment while the embedded version evolves toward a fully self-contained instrument.

To come soon (maybe :p)
