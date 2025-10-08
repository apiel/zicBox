# 15 Zic Pixel

<img width="400" alt="image" src="https://github.com/user-attachments/assets/679664b5-7d3b-4592-9c9b-cd8171b85a40" />

The **Pixel Groovebox** is a compact yet powerful digital instrument built around a modular synthesis concept.  
Each of its **8 tracks** can run different sound engines — each designed to emulate, combine, or reinvent classic synthesis and sampling techniques.

---

## 🔸 Track Layout

| Track Type | Count | Description |
|-------------|--------|-------------|
| **Drum Tracks** | 4 | Specialized percussive sound engines for kicks, snares, claps, metallics, and more. |
| **Sample Tracks** | 2 | Multi-engine sample players for one-shots, loops, granular textures, and time-stretching. |
| **Synth Tracks** | 2 | Full-featured synthesis engines for leads, pads, basses, and evolving soundscapes. |

Each track is **independent**, with its own **engine selection**, **envelope**, **mixer parameters**, and **FX routing**.  
You can freely mix and match — for example, use a **Kick Engine** on Drum Track 1, a **Grain Engine** on Sample Track 5, and a **SuperSaw Engine** on Synth Track 8.

---

## 🥁 Drum Engines (`SynthMultiDrum`)

The drum section is designed for fast, expressive, and punchy percussion synthesis.  
Each of the 4 drum tracks can load one of the following **9 engines**:

---

### 1. **Kick Engine**
Deep and focused bass drum generator.
- **Sound:** Sub-heavy sine or clicky thump, tunable for techno, house, or hip-hop.
- **Main controls:** *Pitch*, *Shape*, *Drive*, *Decay*.
- **Tip:** Shorten decay for tight kicks, add drive for distorted punch.

---

### 2. **Clap Engine**
Classic handclap synthesis with layered noise bursts and short reverb tails.
- **Sound:** Synthetic 80s claps to snappy electro hits.
- **Main controls:** *Spread*, *Decay*, *Tone*.
- **Tip:** Increase spread for a “group” clap feel; reduce tone for darker, vintage character.

---

### 3. **Bass Engine**
Percussive bass generator — bridges the gap between kick and melodic bass.
- **Sound:** Fat, short, FM-driven sub-bass hits or plucked analog tones.
- **Main controls:** *Pitch*, *Decay*, *FM amount*.
- **Tip:** Perfect for punchy basslines or “thud” kicks.

---

### 4. **Metalic Drum Engine**
Resonant metallic percussion using frequency modulation and inharmonic overtones.
- **Sound:** Bells, zaps, pipes, or robotic clanks.
- **Main controls:** *Tone*, *Decay*, *Harmonic spread*.
- **Tip:** Automate tone for evolving textures and industrial percussion.

---

### 5. **Percussion Engine**
General-purpose drum synth for mid/high percussive hits.
- **Sound:** Congas, toms, rimshots, and abstract clicks.
- **Main controls:** *Pitch*, *Shape*, *Decay*.
- **Tip:** Great for secondary percussive layers and fills.

---

### 6. **Er1 PCM Engine**
PCM sample-based drum engine inspired by the Korg Electribe ER-1.
- **Sound:** Vintage digital percussion with a lo-fi edge.
- **Main controls:** *Sample pitch*, *Distortion*, *Decay*.
- **Tip:** Combine with FM or metallic engine for hybrid drum tones.

---

### 7. **Volc Engine**
Drum synthesis inspired by Korg Volca modeling.
- **Sound:** Snappy, saturated, quirky percussion.
- **Main controls:** *Drive*, *Tone*, *Decay*.
- **Tip:** Excellent for crunchy snares or distorted kicks.

---

### 8. **FM Engine**
Frequency modulation drum synth.
- **Sound:** Zappy, snappy, or glitchy hits depending on modulator ratio.
- **Main controls:** *FM depth*, *Ratio*, *Envelope amount*.
- **Tip:** For expressive electronic percussion and evolving FM “clicks”.

---

### 9. **String Engine**
A plucked-string resonator engine for metallic or tonal percussion.
- **Sound:** Karplus-Strong plucks, synthetic strings, tuned percussive tones.
- **Main controls:** *Decay*, *Damping*, *Tune*.
- **Tip:** Works beautifully for tuned percussion or melodic percussive leads.

---

## 🎚️ Sample Engines (`SynthMultiSample`)

The sample section gives two dedicated tracks for audio playback and transformation.  
Each can use one of the following **4 sample-based engines**:

---

### 1. **Mono Engine**
Straightforward one-shot or looped sample player.
- **Sound:** Clean playback for drums, vocals, or long samples.
- **Main controls:** *Pitch*, *Start*, *End*, *Loop*.
- **Tip:** Use envelopes or modulation to create dynamic resampling.

---

### 2. **Grain Engine**
Granular sample playback.
- **Sound:** Ambient textures, stretched pads, or glitchy time clouds.
- **Main controls:** *Grain size*, *Density*, *Pitch*, *Randomness*.
- **Tip:** Ideal for atmospheric drones or reinterpreting loops.

---

### 3. **AM Engine**
Amplitude modulation sample player.
- **Sound:** Metallic, tremolo, and ring-mod textures based on the loaded sample.
- **Main controls:** *Mod rate*, *Depth*, *Pitch*.
- **Tip:** Use subtle modulation for shimmer, or go extreme for digital artifacts.

---

### 4. **Stretch Engine**
Time-stretching playback engine.
- **Sound:** Beat-preserving loops or slow-motion soundscapes.
- **Main controls:** *Speed*, *Pitch*, *Formant*, *Grain size*.
- **Tip:** Maintain groove tempo while pitching samples for creative remixes.

---

## 🎹 Synth Engines (`SynthMulti`)

The two synth tracks are designed for melodic content — leads, basses, pads, and evolving textures.  
Each can load one of the following **9 engines**:

### 1. **FM Engine**
Classic frequency modulation synthesis.
- **Sound:** From metallic bells to smooth electric pianos and basses.
- **Main controls:** *FM depth*, *Ratio*, *Feedback*.
- **Tip:** Use low modulation for soft tones, high for aggressive digital edge.

### 2. **Wavetable Engine**
Table-based synthesis for morphing digital timbres.
- **Sound:** Modern evolving synth sounds.
- **Main controls:** *Table position*, *Wave morph*, *Filter*.
- **Tip:** Automate wave position for dynamic movement.

### 3. **Wavetable2 Engine**
Alternate wavetable implementation — smoother or different interpolation curves.
- **Sound:** Creamier or sharper than the standard wavetable.
- **Main controls:** Similar to Wavetable Engine, but with refined morph behavior.

### 4. **Additive Engine**
Sine-based additive synthesis.
- **Sound:** Clean harmonic tones or spectral sweeps.
- **Main controls:** *Harmonic mix*, *Envelope*, *Detune*.
- **Tip:** Excellent for organ, bell, or evolving pad sounds.

### 5. **Additive2 Engine**
Extended additive model with modulation or morphing.
- **Sound:** More dynamic and lively than Additive Engine.
- **Main controls:** *Morph*, *Harmonic weight*, *Spread*.
- **Tip:** Animate morph for evolving spectral motion.

### 6. **SuperSaw Engine**
Layered detuned saw waves à la Roland JP-8000.
- **Sound:** Huge trance leads, pads, and basses.
- **Main controls:** *Detune*, *Mix*, *Filter*, *Spread*.
- **Tip:** Lower detune for bass, higher for lush pads or leads.

### 7. **SpaceShip Engine**
Experimental hybrid synth combining modulation, noise, and reverb-like resonance.
- **Sound:** Sci-fi textures, drones, and evolving effects.
- **Main controls:** *Mod depth*, *Tone*, *Feedback*.
- **Tip:** Great for intros, transitions, and sound design.

### 8. **Bass Engine**
Focused low-end synthesis for melodic or sub-bass content.
- **Sound:** Analog-like punchy basses or deep subs.
- **Main controls:** *Filter*, *Drive*, *Decay*.
- **Tip:** Use short envelopes for percussive bass, or longer for sustained notes.

### 9. **String Engine**
Physical-model string synthesis (Karplus-Strong inspired).
- **Sound:** Plucked, bowed, or synthetic string tones.
- **Main controls:** *Decay*, *Damping*, *Excitation noise*.
- **Tip:** Beautiful for pseudo-acoustic or hybrid string leads.

## ⚡ Summary

| Track | Engine Type | Engine Count | Typical Sounds |
|--------|--------------|---------------|----------------|
| Drum 1–4 | `SynthMultiDrum` | 9 | Kicks, snares, claps, percs, metallics |
| Sample 5–6 | `SynthMultiSample` | 4 | Loops, granular textures, time-stretched samples |
| Synth 7–8 | `SynthMulti` | 9 | Leads, pads, basses, textures |

### 🧠 Creative Notes

- All engines are **morphable** via modulation, morph, or macro parameters.
- Combining engines creates layered and expressive performances — e.g.:
  - Kick Engine + FM Bass Engine = hybrid subdrum
  - Grain Engine + SuperSaw = atmospheric melodic wall
  - Metallic Drum + Wavetable Engine = percussive synth leads

### ✨ The Philosophy

Pixel Groovebox embraces the idea that **each track is a small modular synth**.  

## BOM

<table>
<tr>
    <th>Image</th>
    <th>Qty</th>
    <th>Description</th>
    <th>Price</th>
<tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/rpi0.png?raw=true' width='100'></td>
    <td>1</td>
    <td>Raspberry Pi zero 2w</td>
    <td>23.00€ for 1 piece at <a href="https://www.amazon.de/-/en/Raspberry-Pi-Zero-2-W-multi-coloured/dp/B09KLVX4RT">Amazon</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/encoderPEC12R.png?raw=true' width='100'></td>
    <td>4</td>
    <td>Encoder PEC12R-4025F-N0024</td>
    <td>1.04€ per piece at <a href="https://eu.mouser.com/ProductDetail/Bourns/PEC12R-4025F-N0024?qs=Zq5ylnUbLm4HSBD7%2FFgU%2FA%3D%3D&countryCode=DE&currencyCode=EUR&_gl=1*1nd7s7x*_ga*Nzc0OTY5NDMwLjE2OTg1MDM2NzE.*_ga_15W4STQT4T*MTcwNTk0NTcwNi4xMi4wLjE3MDU5NDU3MDcuNTkuMC4w*_ga_1KQLCYKRX3*MTcwNTk0NTcwNi4yLjAuMTcwNTk0NTcwNy4wLjAuMA..">mouser</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/keysRedragon.png?raw=true' width='100'></td>
    <td>12</td>
    <td>REDRAGON SMD RGB MX Low Profile 5.5 Switch 3Pin</td>
    <td>4.17€ for 30 piece at <a href="https://www.aliexpress.com/item/1005005895774028.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/keycap.png?raw=true' width='100'></td>
    <td>12</td>
    <td>Key Caps for Mx Switch Mechanical Keyboard</td>
    <td>4.66€ for 20 piece at <a href="https://www.aliexpress.com/item/1005008052800345.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/1.9screen.png?raw=true' width='100'></td>
    <td>1</td>
    <td>1.9 inch Screen Color Display SPI 170x320 ST7789</td>
    <td>3.50€ for 1 piece at <a href="https://www.aliexpress.com/item/1005007321299056.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/20x2female.png?raw=true' width='100'></td>
    <td>1</td>
    <td>2x20 Pin Header female</td>
    <td>3.70€ for 10 piece at <a href="https://www.aliexpress.com/item/1005001340091287.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/20x2male.png?raw=true' width='100'></td>
    <td>1</td>
    <td>2x20 Pin Header male</td>
    <td>2.60€ for 10 piece at <a href="https://www.aliexpress.com/item/32848774255.html">Aliexpress</a></td>
</tr>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/dac.png?raw=true' width='100'></td>
    <td>1</td>
    <td>PCM5102A DAC</td>
    <td>2.73€ for 1 piece at <a href="https://www.aliexpress.com/item/1005006198619536.html">Aliexpress</a></td>
</tr>
<tr>
    <td></td>
    <td>1</td>
    <td>Resistor 660 ohms</td>
    <td></td>
</tr>
</table>

## PCB

The PCB has been designed using EasyEDA. To access the project, use the following link:

https://easyeda.com/editor#id=5cd71bb61a6c49ae8e3dccd67d79cbfc (wip)

To get files to send to Jlcpcb, click on "Generate PCB Fabrication File (Gerber)"

If you need to modify the PCB, clone the project in your own account.

**Ordering the PCB**
To manufacture the PCB, you need to generate and submit the Gerber files to a PCB fabrication service.

You can order the PCB from JLCPCB by following these steps:

1. Visit JLCPCB.
2. Click on the "Add Gerber File" button and upload the provided Gerber file (.zip).
3. The default settings should work fine, and no changes are necessary.
4. If desired, you can change the PCB color from green to black—this typically does not affect the price.
5. Click "Save to Cart", then proceed to checkout.
   
💰 **Estimated Cost**: ~4€ (with standard shipping, approximately 10 days delivery time).

<table>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pcb.png?raw=true' width='400'></td>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pcb2d.png?raw=true' width='400'></td>
</tr>
</table>

## Assembly

Since PCB is constantly evolving, this tutorial might be outdated.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_00.png?raw=true' width='700'>

Place the pin header of the display onto the board, noting its unusual orientation. The pins must be soldered **only from the top side**, ensuring that they do not protrude through to the underside. This is important because additional components will be mounted on the opposite side, and the space must remain free of obstructions.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_01.png?raw=true' width='700'>

On the back side, you should see the final result, the pins do not come through the board.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_02.png?raw=true'>

Now place the DAC pin header in the same way as you did for the display pin header, and solder it.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_03.png?raw=true' width='700'>

Place the four encoders and solder them in position. Then place the Raspberry Pi female pin header and solder it as well.

Don’t forget to trim the pins step by step as you place and solder each component. This will help keep the board neat and ensure that everything fits together correctly.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_04.png?raw=true' width='700'>

Place the DAC onto the pins and solder it in place. Then solder the jumper on the DAC as [described at the top of this page](https://github.com/apiel/zicBox/wiki/04-Hardware#audio-dac).

<table>
<tr>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_05.png?raw=true' width='400'></td>
    <td><img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_05_bis.png?raw=true' width='400'></td>
</tr>
</table>

Place the display onto the pins and solder it in place. Make sure it is aligned straight and positioned as low as possible, otherwise it may interfere with the keyboard button.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_06.png?raw=true' width='700'>

Finally, solder the Raspberry Pi male pin header onto the Raspberry Pi.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_07.png?raw=true' width='700'>

Once soldered, connect the Raspberry Pi to the board.

<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/assembly/pcb_08.png?raw=true' width='700'>

## User Manual

TODO...

Key layout:

<table>
<tr>
<td>Drum1</td><td>Drum2</td><td>Drum3</td><td>Drum4</td>
</tr>
<tr>
<td>Sample1</td><td>Sample2</td><td>Synth1</td><td>Synth2</td>
</tr>
<tr>
<td>Trigger Note</td><td>Page</td><td>Clips</td><td>Shift</td>
</tr>
</table>

When shift is pressed:
<table>
<tr>
<td>Mute</td><td>Mute</td><td>Mute</td><td>Mute</td>
</tr>
<tr>
<td>Mute</td><td>Mute</td><td>Mute</td><td>Mute</td>
</tr>
<tr>
<td>Play</td><td>Menu</td><td>Reload all</td><td>Shift</td>
</tr>
</table>

Then let assume Drum1 is selected, pressing again Drum1 button will open a sub-menu specific to the track, to go in the sequencer and as well in keyboard mode, ...

---

*Another old prototype of the Pixel*:

<table>
<tr>
    <td>
<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel.png?raw=true' height='220'>
    </td><td>
<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A.png?raw=true' height='220'>
    </td><td>
<img src='https://github.com/apiel/zicBox/blob/main/hardware/ZicPixel/pixel3A_2.png?raw=true' height='220'>
    </td>
</tr>
</table>