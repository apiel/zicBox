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

On the emulator panel this routing is drawn as wires around the cards (the idea for the silkscreen of the module): left rail = oscillator -> (+ crunch) -> MASTER, right rails = BODY pitch -> NOISE filter (Noise Track) and NOISE -> mixed in after the crush. Wires get brighter with the amounts set and pulse with the live level.

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
| Ring | ring modulation at an inharmonic ratio: metallic / bell / industrial |
| Bounce | damped pitch wobble (spring), up to +-1 octave, rate follows Decay |
| Pre Tone | tilt BEFORE the distortion: - fat lows get distorted, + harsh highs |
| Low Keep | below 150 Hz skips the distortion: heavy distorted kick with a clean sub |
| Flam | second, softer hit after 5-120 ms (0 = off) |
| Noise Grit | noise only speaks on the waveform peaks of the body: raspy, locked to the pitch |
| Noise Swell | noise fades in (0-100 ms) instead of hitting at once |
| Steps | the pitch dive becomes a staircase (quantised to 0.5 .. 12 semitones) |

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
