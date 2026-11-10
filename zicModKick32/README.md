# zicModKick32

One kick engine, **32 pots** (4 rows x 8), no menu. Desktop emulator first, Eurorack module later.

`Kick32` merges the two engines of zicModKick:
the wavetable body of **KickWave** (shape / skew / harmonics / fold / phase)
and the toolbox of **PotKick** (sweep, FM, resonator, wavefolder, crush),
plus click / noise / sub layers and a 3-band EQ.

## Pot map

Pot index == `Kick32::params[]` index, row = index / 8.

| Row | Section | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|-----|---------|---|---|---|---|---|---|---|---|
| 0 | **BODY** | Decay | Curve | Sweep | Bend | FM Depth | FM Ratio | Sync | Feedback |
| 1 | **WAVE** | Wave Shape | Skew | Harmonic 2 | Harmonic 3 | Fold | Fold Env | Rectify | Resonator |
| 2 | **NOISE** | Noise | Noise Type | Noise Color | Noise Reso | Noise Sweep | Noise Decay | Crunch | Noise Track |
| 3 | **MASTER** | Drive | Distortion | Crush | Punch | EQ Low | EQ Mid | EQ High | Volume |

There is no Tune pot: pitch = note (CV on the module, **TUNE** transpose + per step note in the emulator). Note 60 = 52 Hz.

Signal flow: `wavetable(+FM, sync, feedback, sweep, bend, fold env, rectify, resonator) -> distortion -> drive -> punch -> crush -> + noise -> EQ -> volume`

The emulator panel draws this routing as PCB style gold traces (the idea for the silkscreen of the module): the oscillator trace runs through BODY, makes a carriage return into WAVE, goes down the right side and through MASTER to OUT; the pitch trace leaves the PITCH group, goes over the top and comes back into Noise Track; NOISE has a tap mixed in after the crush and a SEND dropped into the main trace before the distortion. Rows are split in captioned groups (AMP, PITCH, FM, OSC, ...). Traces get brighter with the amounts set, and light pulses travel along them with the live level.

- **Sweep**: one pot = start pitch (up to 25x) *and* dive time. **Bend**: slow pitch fall over the whole Decay (up to -1.5 oct).
- **Curve**: amplitude shape, fat -> linear -> punchy.
- **Sync**: hard sync, a slave oscillator restarted every cycle whose ratio dives with the sweep. **Fold Env**: the wave fold follows the amp envelope (+ bright attack, - opens up). **Rectify**: full wave rectifier = octave up.
- **FM**: phase modulation, Depth + Ratio only. **Feedback**: the oscillator modulates its own phase (sine -> saw -> noise-like).
- **NOISE row = one noise voice.** Type (white -> stepped digital grit -> metallic), Color (LP -> BP -> HP, cutoff rising), Reso (plain noise -> pitched ring), Sweep (noise envelope moves the cutoff: + falls, - rises), Decay (3 ms = click .. 1.2 s = tail).
  **Noise** = voice on top of the kick, **Crunch** = voice goes *through* the distortion with the kick (gabber / tekno grit).
- **Noise Track**: the noise filter follows the kick pitch; with Reso up it is a second pitched voice diving along with the kick.

## LAB (playground, to be removed)

8 greyed out pots on the right (params 32..39, `Kick32Panel::LAB_POTS`), only to try ideas and keep the best.

| Pot | Idea |
|-----|------|
| Room | small dark reverb (4 combs + 2 all-passes): space around the kick |
| Pipe | feedback comb tuned to the note, rings 0.1-1 s whatever the pitch: tube / drum shell |
| Vowel | two formant filters morphing a-e-i-o-u: a talking kick |
| Squelch | resonant low-pass that dives with the sweep: acid zap |
| Chop | audio rate gate 4-60 Hz: stutter -> growl |
| Mangle | XORs the bits of the signal: harsh digital destruction |
| Squash | upward compression: pulls every level towards a target, the tail is lifted |
| Lag | the body starts 0-30 ms late: transient first, then the boom |

## Code split

```
audio/engines/Kick32.h      ENGINE   shared, no UI / OS dependency, runs on the Daisy as is
zicModKick32/core/          MODULE   panel model: pot -> param, defaults, ADC smoothing (Panel::update); firmware reads MAIN_POTS only
zicModKick32/emu/           EMULATOR only, never ships on the module
    zic.cpp                   ALSA audio thread + SFML window
    EmuUi.h                   knobs + sequencer drawing and mouse handling
    Sequencer.h               64 steps, 180 BPM, on/off + note
```

Firmware (later): `Kick32 + Panel`, read the 32 ADC channels into `panel.update(i, raw01)`,
gate input -> `kick.noteOn(60, 1.0f)`, CV pitch -> note. Nothing else is needed.

## Emulator

```bash
cd zicModKick32 && make        # build + run
ZIC_SCREENSHOT=shot.png emu/build/x86/zic   # save a frame and quit
```

| Where | Action |
|-------|--------|
| Knob | drag up/down (shift = fine), wheel (1%, shift 0.2%), double click = default |
| Step | click toggles, drag paints, wheel = note (semitones), shift+wheel = octave, right click = note 0 |
| Keys | `space` trigger (stopped) / hold = mute the kick (playing), `Enter` or `P` play/stop, `Esc` quit |
| BPM / TUNE | wheel on the box, or `-` / `+` (shift = x10 / octave) |

Everything (pots, BPM, tune, pattern) is remembered in `~/.config/zicModKick32/state.txt`, whatever the folder you start the emulator from.
Pots are stored by key (`param fmDepth 20`), so adding / removing / reordering params never loses the other settings. Autosaved every second when something changed.
`ZIC_SCREENSHOT=file.png` (never touches the save) and `ZIC_DEMO=1` (fires a kick with the noise routes on, to see the wires glow) are for screenshots.

## Name

`zicModKick32` is a placeholder (engine `Kick32`, folder, class names). Other ideas that fit the zicMod series:
**zicModThump**, **zicModBoom**, **zicModForge** (it is a kick *forge*: wave + FM + layers + FX), **zicModKick+**.
Renaming = folder + `Kick32` -> new name, nothing else depends on it.
