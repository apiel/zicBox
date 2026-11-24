# zicStudioFm4

**A 4-operator FM synthesizer**, adapted from the FM4 of the mwave FM-1 (`/home/alex/Music/fm1-tribe/firmware/src/dsp/fm4.c`):
the 8 algorithms of the Digitone manual (A.3), the four harmonic presets (Sine / Saw / Square / Bell, morphed),
operators C / A / B1 / B2 with ratios and detune, feedback and two MIX outputs, two operator envelopes (A and B,
the manual's B1 / B2 crossfade), the amp envelope, the low-pass filter and the overdrive. One voice.

Engine: `audio/engines/Fm4.h` (header only, float, no UI / OS / audio-driver dependency).

```
A > C, B2 > B1 ... (8 algorithms) --X/Y MIX--> overdrive --> low-pass --> amp --> out
```

## Panel (30 pots, 6 cards)

```
+-- OPERATOR FM (violet, 4x2) ------+  +-- OPERATOR ENVS (amber, 4x2) ----+
| Alg     Ratio C  Ratio A  Ratio B |  | Lev A   Atk A  Dec A  End A     |
| Harm    Detune   Feedback Mix     |  | Lev B   Atk B  Dec B  End B     |
+-----------------------------------+  +---------------------------------+
+-- AMP (green, 4x1) ---------------+  +-- FILTER (cyan, 4x1) -----------+
| Atk    Dec     Sus    Rel         |  | Cutoff  Reso    Drive   Trigger  |
+-----------------------------------+  +---------------------------------+
+-- LFO (pink, 6x1) -------------------------------+  +-- FX (blue, 2x1) -+
| Rate   Depth   Shape   Dest                      |  | Delay   Reverb     |
+--------------------------------------------------+  +-------------------+
```

The **Alg** knob behaves like an encoder with detents (one algorithm per step); the LFO, delay
and reverb sends and the LFO shape / destination knobs do too. The delay (300 ms, damped) and
reverb are fixed defaults; only their sends are exposed here, the studio will own the master FX.

| Pot | What it does |
|-----|--------------|
| **Alg** | the eight algorithms of the Digitone manual, A.3 (C / A / B2 > B1, two X / Y mixes) |
| **Ratio C** | operator C's fixed ratio, 1..16 |
| **Ratio A** | operator A's ratio, 0.25 .. 8 (the classic FM spacing) |
| **Ratio B** | operators B2 and B1: B2 steps through 16 values, then B1 |
| **Harm** | the harmonic morph of the four operators (Sine, Saw, Square, Bell), in and out |
| **Detune** | A and B2 detuned, 0 .. 38 cents |
| **Feedback** | operator A's self-modulation depth, 0 = off |
| **Mix** | the X / Y mix, 0 = X, 100% = Y |
| **Lev A** | A's modulation index (scaled by its envelope) |
| **Atk A** / **Dec A** / **End A** | A's attack (up to 2 s), decay (5 ms .. 2 s), and the level it decays to |
| **Lev B** | the manual's B1 / B2 crossfade (B1 rises to 43, falls to 85, rises; B2 rises from 43) |
| **Atk B** / **Dec B** / **End B** | B's attack / decay / end level |
| **Attack** / **Decay** / **Sustain** / **Release** | the amp envelope, 1 ms .. 2 s / up to 3.5 s / 0 .. 100% / up to 2 s |
| **Cutoff** | the state-variable low-pass, -43 .. +57 st (40 Hz .. 12 kHz) |
| **Reso** | filter resonance, up to a scream |
| **Drive** | overdrive before the filter, 0 = off |
| **Trigger** | Gate holds on the note, Trig runs out by itself |
| **LFO Rate** / **Depth** / **Shape** / **Dest** / **Phase** / **Key** | 0.05 .. 20 Hz, 0 .. 100%, sine/tri/saw/square, pitch/cutoff/amp, 0 .. 360 deg, key-sync on/off |
| **Delay** / **Reverb** | sends into the default delay (300 ms, damped) and reverb |

The knob readouts show the real values (ms / s, %, cents, ratio pairs), not the raw 0..127 pot codes.

## The algorithm visualizer

The card next to the scope draws the routing of the algorithm the **Alg** knob selects: the four
operator nodes (`C`, `A`, `B1`, `B2`) light up with their live output level, the arrows are the FM
modulation paths, and the `X` / `Y` badges mark which operators feed each output. The mix bar at the
bottom follows the **Mix** pot.

## The sequencer

A 32-step piano roll (two pages of 16) drives the synth at 60..220 BPM.

- **PLAY / STOP** (or Space) starts the sequence; the playhead follows the pattern.
- Click an empty cell to place a note; drag a note for pitch, drag its right edge for length.
- Wheel over a note = velocity, Shift+wheel = length, Alt+wheel / Shift+click = slide.
- Right-click a note to delete it, Shift+right-click to toggle the accent.
- **TAB** switches between steps 1-16 and 17-32; **TUNE** transposes the pattern.
- The quick-edit bar above the grid shows the selected step and toggles SLIDE / ACCENT.

## Emulator

```
make            # build + run
make build_zic  # build only
make run        # run it
```

Space / TRIG = audition a note at 60. Click and drag any knob to move it; wheel turns it;
right-click resets it. Six presets: EPIANO, FM BASS, BELL, BRASS, METAL, PAD (keys 1-6). D = randomize.
State (params + sequencer + BPM) autosaves to `~/.config/zicStudioFm4/state.txt`.

`ZIC_AUTOPLAY=1` starts the sequencer on launch; `ZIC_SCREENSHOT=file.png` renders the UI to an image.

## Harness

```
make test   # offline DSP tests (render, release, all 8 algorithms) + the sequencer
```

## Code

- `audio/engines/Fm4.h` — engine (the fm4.c port), plus the live operator levels used by the UI
- `core/Fm4Panel.h` — panel model (blocks, groups, help, value formatting)
- `emu/EmuUi.h` — SFML UI: knobs, algorithm visualizer, piano roll
- `emu/Fm4Sequencer.h` — the 32-step sequencer
- `emu/Persist.h` — state save / load
- `emu/zic.cpp` — ALSA audio loop + SFML window
- `harness/` — offline DSP + sequencer tests
