# zicStudio

One desktop window that hosts eight tracks of the zicBox instrument family:

- **Kick** (`Kick32v5`) · **Acid** (`Acid32`) · **Fm4**
- every track can pick its own engine, and keeps its own panel state, sequencer
  and mixer strip
- one shared transport (play/stop, BPM) drives every sequencer in lockstep
- a fixed left sidebar with the track list (volume, mute/solo, engine cog,
  engine menu), master volume, master FX and the transport
- one master FX bus (delay + reverb) fed by the per-track sends
- one project file remembers all of it, including the window position and the
  sequencer placement / split

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

The window is drawn 1:1 in real pixels (no resampling), so text stays crisp at
any size. The sequencer is always stacked below the instrument panel. The only
remembered layout choice is the splitter position:

- the thin **splitter bar** between panel and sequencer resizes the share each
  one gets. Drag it with the mouse; the position is saved in the project.

`Narrow` windows (aspect < 1.4) always stack the sequencer below the panel.

## Controls

**Sidebar**

- click a track row — select it
- drag the volume bar (or scroll over the row) — track volume
- `M` / `S` — mute / solo (when any track is soloed, non-soloed tracks are silent)
- click the **cog** next to a track's engine — engine menu (Kick / Acid / Fm4)
- `PLAY` / `STOP` and the BPM stepper are pinned to the bottom
- `MASTER`, `OUT` and the master FX knobs sit just above the transport
- the CPU meter shows the process usage of the whole app

**Sequencer**

- click a step — toggle it (piano rolls add the note at the clicked row)
- drag across the grid — paint notes with the initial state
- right click — clear the step
- wheel — pitch (semitones); shift+wheel — velocity (piano rolls)
- shift+click an existing piano-roll note — toggle accent
- select a step and use the `NOTE` / `L` / `V` / `SLIDE` / `ACC` strip for
  exact values (shown when the sequencer is wide enough)
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
master FX values, window geometry, split, per-track
engine / volume / pan / mute / solo / sends, every panel parameter and every
sequencer step (note, velocity, accent, slide, length, pattern length and
transpose).

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
