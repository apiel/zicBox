# zicStudio

One desktop window that hosts eight tracks of the zicBox instrument family:

- **Kick** (`Kick32v5`) · **Acid** (`Acid32`) · **Fm4**
- every track can pick its own engine, and keeps its own panel state, sequencer
  and mixer strip
- one shared transport (play/stop, BPM) drives every sequencer in lockstep
- one mixer with fader, pan, mute/solo and delay/reverb sends per track
- one master FX bus (delay + reverb) fed by the mixer sends
- one project file remembers all of it, including the window position

The instrument panels are the real `EmuUi` front panels from the standalone
studios, reused through namespaced wrappers in `emu/ui/` so all three engines
can coexist in one process without colliding `Theme` classes.

## Build and run

```sh
make -C studio/zicStudio build_zic
make -C studio/zicStudio run
```

The app needs ALSA and SFML. Audio goes to the default ALSA device. To run the
UI without a sound device:

```sh
ZIC_SILENT=1 ./studio/zicStudio/emu/build/x86/zic
```

## Layout

- **Wide window** (aspect ≥ 1.4): full instrument panel on the left; track
  header, mixer, master FX and sequencer stacked on the right.
- **Narrow window** (aspect < 1.4): panel on top, mixer / master FX / sequencer
  below it.
- Drag the window to any size; the layout and all hit boxes rescale with it.

## Controls

**Transport / tracks**

- `Space`, `P` or `Return` — play/stop
- `1`–`8` — select track
- `-` / `+` in the BPM box — change tempo (shift = ±10)

**Mixer**

- click a strip to select its track
- drag the fader / pan / `DLY` / `REV` bars; mouse wheel also works
- `M` / `S` — mute / solo (when any track is soloed, non-soloed tracks are silent)

**Sequencer**

- click a step — toggle it (piano rolls add the note at the clicked row)
- right click — clear the step
- wheel — pitch (semitones); shift+wheel — velocity (piano rolls)
- shift+click an existing piano-roll note — toggle accent
- `-` / `+` under `TR` — transpose (shift = ±12, wheel over the box works too)
- `16` / `32` — pattern length for Acid and Fm4
- `CLEAR` — clear the whole pattern

**Instrument panel**

- the panel's own knobs, presets and keyboard shortcuts work exactly as in the
  standalone studio
- the panel only receives input for the selected track

## Project file

`$XDG_CONFIG_HOME/zicStudio/state.txt` (or `~/.config/zicStudio/state.txt`).
Saved every second while running and on exit. It stores BPM, master volume,
master FX values, window geometry, per-track engine / volume / pan / mute /
solo / sends, every panel parameter and every sequencer step
(note, velocity, accent, slide, length, pattern length and transpose).

The loader is backward compatible with the first `zicStudio` state files.

## Verification

```sh
make -C studio/zicStudio test          # offline DSP + sequencer + FX harness
make -C studio/zicStudio screenshots   # renders 3 engines + narrow layout headlessly
```

Screenshots are written to `studio/zicStudio/emu/build/screenshots/` and the
target fails if a capture is empty. The render target runs with `ZIC_SILENT=1`
so it does not need an audio device.

Other environment variables:

- `ZIC_SCREENSHOT=/path.png` — render one frame and exit
- `ZIC_DEMO=1` — start playing with the factory patterns (used with screenshots)
- `ZIC_TRACK=N` — pre-select track N
- `ZIC_NARROW=1` — force the stacked narrow layout
