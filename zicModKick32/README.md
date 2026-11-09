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
| 0 | **BODY** | Tune | Decay | Sweep | Sweep Time | Sweep Shape | FM Depth | FM Ratio | FM Snap |
| 1 | **WAVE** | Wave Shape | Skew | Harmonic 2 | Harmonic 3 | Fold | Phase | Resonator | Res Tune |
| 2 | **LAYERS** | Click | Click Decay | Click Tone | Noise | Noise Decay | Noise Tone | Sub | Sub Decay |
| 3 | **MASTER** | Drive | Distortion | Crush | Punch | EQ Low | EQ Mid | EQ High | Volume |

Signal flow: `wavetable(+FM, sweep, resonator) + sub -> distortion -> drive -> punch -> crush -> + click + noise -> EQ -> volume`

## Code split

```
audio/engines/Kick32.h      ENGINE   shared, no UI / OS dependency, runs on the Daisy as is
zicModKick32/core/          MODULE   panel model: pot -> param, defaults, ADC smoothing (Panel::update)
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
| Keys | `space` play/stop, `T` / `Return` trigger, `Esc` quit |
| BPM | wheel on the box, or `-` / `+` |

Pots, BPM and pattern are saved to `emu/kick32_state.txt` on exit and reloaded on start.

## Name

`zicModKick32` is a placeholder (engine `Kick32`, folder, class names). Other ideas that fit the zicMod series:
**zicModThump**, **zicModBoom**, **zicModForge** (it is a kick *forge*: wave + FM + layers + FX), **zicModKick+**.
Renaming = folder + `Kick32` -> new name, nothing else depends on it.
