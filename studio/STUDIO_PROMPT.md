# Prompt: Build `zicStudio` — one studio app bringing the three instrument UIs together

## Goal
Create a single desktop studio application (`studio/zicStudio/`) that hosts the three existing
instrument engines/UIs side by side in one window, with one shared transport, one mixer and
one master FX bus, reusing the work already done in:

- `studio/zicStudioKick/`  (Kick32v5 — 32 pots, body/drive/filter)
- `studio/zicStudioAcid/`  (Acid32 — 32 pots, TB-303 style)
- `studio/zicStudioFm4/`   (Fm4 — 32 pots, 4-op FM)

The standalone studios stay as they are (if possible but not a must, we could eventually get rid of them later); the new studio is a separate app that composes them.
Exisiting app can be refactored so it can easily be reused by the main studio app

## Existing building blocks (do not rewrite, reuse)

Engines (header-only, `audio/engines/`, `EngineBase<Derived>` CRTP, `IEngine`):
- `Kick32v5.h` (`PARAM_COUNT = 32`)
- `Acid32.h` (`PARAM_COUNT = 32`)
- `Fm4.h` (`PARAM_COUNT = 32`)

Panels (shared, no UI deps):
- `studio/zicStudioKick/core/Kick32v5Panel.h`
- `studio/zicStudioAcid/core/Acid32Panel.h`
- `studio/zicStudioFm4/core/Fm4Panel.h`

Sequencers:
- `studio/zicStudioKick/emu/Sequencer.h`
- `studio/zicStudioAcid/emu/AcidSequencer.h`
- `studio/zicStudioFm4/emu/Fm4Sequencer.h` (32 steps, piano roll)

there are 2 type of sequencers, one for drum and one for synth, you might put them inside a common folder.

Emulators (SFML + ALSA) to mirror:
- `studio/zicStudioKick/emu/zic.cpp`, `EmuUi.h`, `Persist.h`
- `studio/zicStudioAcid/emu/...`
- `studio/zicStudioFm4/emu/...`

Reference for multi-engine composition: `zicRack/` (8 tracks, engine registry, mixer, master FX, clips).

## What "bringing them together" means
1. **8 instrument tracks** in one project: which can use on of the engine Kick, Acid, Fm4, this must be selectable and more engine to come. Each track owns one engine + its panel + its sequencer.
2. Later on we might want to introduce clip system per track.
3. **One shared transport** (play/stop, BPM, swing maybe) driving all three sequencers in sync.
4. **One mixer**: per-track volume, pan, mute, solo; master volume.
5. **One master FX bus**: the delay/reverb that already live inside Fm4 (and Acid's spatial FX) get promoted to the master bus; the per-instrument "FX send" knobs become sends to the master bus. Right now we should send delay and reverb to master fx bus.
6. **One window** with a tab or stacked layout to switch/edit each instrument's full panel, plus a mixer strip and a transport bar.
   1. the design should be responsive, this is very important. however it should be done in a smart way. For example knob dont really need to resize, unless it is really too small. however, sequencer is often missing space, so i think we should focus on making sequencer responsive, and maybe it could change of position depending of screen size. For example on wide screen, sequencer could be on right side. While on narrow screen it would be under the knob. Concerning the rest of the UI, you must find out what make more sense for responsivness...
   2. no need to match one to one the UI of the prototype, even though the way the knob are organanise seems quiet good, but if you find a better approach feel free to suggest
   3. top bar on the current UI is ugly, it doesnt need to be so big, so again if you find a better way to optimise those button for responsivness, do it.
   4. button to generate patch should be removed, i never requested it
7. **Persistence**: one project file (extend `Persist.h` pattern) saving all three engines' params + sequencers + mixer + BPM + window position.

## First steps for the new session
1. Survey `zicRack/` (multi-track composition) and the three `studio*/emu/zic.cpp` (audio loop + window) to pick the cleanest composition pattern.
2. Decide single-window tabs vs. stacked panels; prototype with Kick only, then add Acid and Fm4.
3. Move Fm4's master FX into a shared `audio/effects/StudioFx.h` and have Fm4 (and Acid) call into it instead of their private copies.
4. Wire shared transport + mixer + persistence.
5. Add screenshot/autoplay verification like the single studios.

## Deliverable for the new session
A runnable `studio/zicStudio/` that opens one window, shows all three instruments (switchable), shares transport/mixer/master FX, persists state, and passes an offline harness.