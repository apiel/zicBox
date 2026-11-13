# zicModKick32_v3

A third take on the 32 pot kick (v1 `zicModKick32` and v2 `zicModKick32_v2` stay as they are):
**BODY + TAIL > ONE DRIVE > FILTERBANK**, for tekno / hardtek / metal tek / hardcore / tribe.
32 pots, no menu, no switch: every choice is a morph.

- the **BODY** is the hit: amp shape, pitch drop, wave + strong harmonics;
- the **TAIL** is a second voice that takes over at Start, with its own pitch, glide, shape, dirt, tone and length;
- click + noise are layered into the body drive (the DROP idea: several sources, **one** saturation), and a
  comb tuned to the pitch feeds the drive back into itself;
- the **FILTERBANK** (Sherman idea): LP > HP, both tracking the pitch (note + drop), with an envelope,
  **audio rate FM** of the cutoffs by the signal itself, and a drive after the filter.

```
BODY (wave, harmonics, drop) + CLICK + NOISE --> [+ FEEDBACK comb] --> DRIVE (punch, character) --+
                                                                                                  +--> CRUSH --> LP > HP (reso, env, FM, pitch tracking) --> FILTER DRIVE --> clip --> OUT
TAIL (shape) --> DIRT --> TONE -------------------------- crossfade at Start ----------------------+
```

## Panel: the signal flow, left to right

```
+-------- BODY (4 x 2) --------------------+  +--- DRIVE -----+  +--- FILTER -------+
| AMP: Decay  Shape | DROP: Drop  Drop Time |  | Drive   Char  |  | Cutoff   Cutoff 2|
| DROP: Curve | WAVE: Wave  Harm 2  Harm 3  |  | Punch   Fdback|  | Reso     Env     |
+-------- TAIL (4 x 2) --------------------+  | Click   C.Tone|  | Env Dec  FM      |
| TAKEOVER: Tail  Start | PITCH: Pitch Glide|  | Noise   Color |  | F.Drive  Crush   |
| TIMBRE: Shape  Dirt | TONE: Tone | Length |  +---------------+  +------------------+
+------------------------------------------+
```

Pot index == `Kick32v3::params[]` index (BODY 0..7, TAIL 8..15, DRIVE 16..23, FILTER 24..31).
The grid position of each pot (4 rows x 8 columns) is in `core/Kick32v3Panel.h` (`potCol` / `potRow`):
columns 0..3 are the two voices (BODY on rows 0..1, TAIL on rows 2..3), columns 4..5 DRIVE, columns 6..7 FILTER.

| # | Pot | What it does |
|---|-----|--------------|
| 0 | **Decay** | 40 ms .. 2 s |
| 1 | **Shape** | amp curve: punchy exponential -> linear -> gated (holds, then rounds off) |
| 2 | **Drop** | pitch drop depth, 0 .. 6 octaves above the note |
| 3 | **Drop Time** | 4 .. 512 ms |
| 4 | **Drop Curve** | laser (holds, then dives) -> linear -> zap (dives at once) |
| 5 | **Wave** | sine -> triangle -> square -> saw |
| 6 | **Harm 2** | 2nd harmonic, bipolar, strong |
| 7 | **Harm 3** | 3rd harmonic, bipolar, strong |
| 8 | **Tail** | how much the tail takes over the body (equal power crossfade over 20 ms) |
| 9 | **Start** | when, 5 .. 300 ms after the hit |
| 10 | **Pitch** | -24 .. +12 st from the note: a tuned tail |
| 11 | **Glide** | the tail starts at the body pitch and phase, then slides to its pitch (0 .. 400 ms) |
| 12 | **Shape** | sine -> triangle -> saw -> square -> narrow pulse (nasal, hoover like) |
| 13 | **Dirt** | its own drive: 0..50 gain 1 .. 8x (tanh), 50..100 the wave folds on top (metallic) |
| 14 | **Tone** | bipolar: - low pass closing in (9 kHz .. 140 Hz), + resonant formant on a harmonic of the tail (screech / talk) |
| 15 | **Length** | the tail's own decay, 50 ms .. 3 s, from Start (the body keeps its Decay) |
| 16 | **Drive** | gain into the one saturation, 1x .. 128x |
| 17 | **Character** | soft (tube) -> hard clip -> wavefold (max 3 folds, stays a tone) -> rectified fuzz (octave up) |
| 18 | **Punch** | the drive has its own envelope (bipolar): + extra drive on the hit (crunchy attack, cleaner tail), - extra drive in the tail (clean hit, the tail distorts and swells). Time = 20 % of Decay |
| 19 | **Feedback** | a comb tuned to the pitch (one period, follows the drop) feeding the drive back into itself (bipolar). + reinforces the pitch and rings the drop, - cancels it (half harmonics: hollow, gritty). Dies with the body |
| 20 | **Click** | click oscillator on the hit (a short sine burst with its own one octave drop), into the drive |
| 21 | **Click Tone** | 300 Hz, 8 ms knock -> 6 kHz, 1 ms tick |
| 22 | **Noise** | noise layer into the drive. Its length follows Decay (about a quarter of it), shorter when bright |
| 23 | **Color** | noise filter: LP -> BP -> HP while the cutoff rises (100 Hz .. 10 kHz). Low = rumbling dirt, high = snap |
| 24 | **Cutoff** | filter 1, low pass, 30 Hz .. 20 kHz. Both cutoffs follow the note and the pitch drop |
| 25 | **Cutoff 2** | filter 2, high pass after the low pass, 15 Hz .. 7.7 kHz |
| 26 | **Reso** | both filters, up to a scream. The band state is soft clipped: resonance growls instead of whistling |
| 27 | **Env** | bipolar: + opens both filters on the hit (6 oct), - closes them (the tail opens up) |
| 28 | **Env Decay** | 5 ms .. 1 s |
| 29 | **FM** | the signal itself (body and tail, after the drive) modulates both cutoffs at audio rate, up to +-7 oct: growl, ring, talk |
| 30 | **Filter Drive** | saturation after the filter, 1 .. 25x: the resonance saturates (Sherman style) |
| 31 | **Crush** | sample rate reduction, then bits (before the filter, which smooths the aliasing) |

No Tune (pitch = note from the external sequencer, note 60 = 52 Hz), no velocity, no volume
(fixed level, soft clipper). The ring of the filters is gated ~150 ms after the sound ends: a drum module must not drone.

### Starting points

- **Hardcore**: Drive 75, Character 40 (hard clip), Shape 60, Punch -50, Decay 500, Tail 60, Pitch -7, Dirt 70.
- **Tribe / Sherman**: Cutoff 45, Reso 70, Env +60, Cutoff 2 30, Filter Drive 60.
- **Metal tek**: Feedback +70, Drive 60, Character 70 (fold), Tail 50, Shape 90 (pulse), Tone +60 (formant).
- **Hardtek snap**: Noise 50, Color 70, Click 70, Click Tone 80, Drive 55.
- **Growl**: FM 70, Cutoff 50, Reso 60, Cutoff 2 30.

## Emulator

```bash
cd zicModKick32_v3 && make      # build + run
ZIC_SCREENSHOT=shot.png emu/build/x86/zic             # save a frame and quit
ZIC_SCREENSHOT=shot.png ZIC_DEMO=1 emu/build/x86/zic  # same, with a busy patch and a hit in the scope
```

The UI is drawn as the signal flow: BODY + TAIL (left) > DRIVE > FILTER, with a live preview in each card:
the body wave, the tail wave + the handover (body giving way to the tail at Start, with Length), the
**drive transfer curve** and the **frequency response of the filterbank**. The HIT scope on top shows the
waveform of the last hit (min / max per half millisecond, 600 ms, the tail part in green) with the pitch
and amp curves and the Drop / Tail / Decay markers.

| Where | Action |
|-------|--------|
| Knob | drag up/down (shift = fine), wheel (1 %, shift 0.2 %), double click = default |
| Step | click toggles, drag paints, wheel = note (semitones), shift+wheel = octave, right click = note 0 |
| Keys | `space` trigger (stopped) / hold = mute the kick (playing), `Enter` or `P` play/stop, `Esc` quit |
| BPM / TUNE | wheel on the box, or `-` / `+` (shift = x10 / octave) |

The 64 step sequencer at 180 BPM is for testing only (the module just receives a gate and a pitch CV).
State: `~/.config/zicModKick32_v3/state.txt` (pots stored by key, autosaved every second when something changed;
`ZIC_SCREENSHOT` never touches it).

## Code

```
audio/engines/Kick32v3.h       ENGINE   no UI / OS dependency (Daisy Seed / STM32H7)
zicModKick32_v3/core/          MODULE   panel model: pot -> param, block / group layout, ADC smoothing
zicModKick32_v3/emu/           EMULATOR SFML + ALSA + 64 step test sequencer
```

Per sample no std::exp / pow / tan (fast exp2 / sin / tan), pot dependent coefficients every 32 samples.
Memory ~20 KB (a 4096 sample comb for the Feedback). Offline, every pot at 80 %: ~0.35 us / sample on x86
(to measure on the H7). Checked offline: no NaN / no output above 0.95 over 300 random settings x 4 fast hits
on notes 36..84, silence 3.5 s after a hit with Reso / Feedback / FM / Length at max, and every pot measured
for its effect on the spectrum and envelope (the dependent ones with their master on).

Firmware (later): `Kick32v3 + Panel`, read the 32 ADC channels into `panel.update(i, raw01)`,
gate input -> `kick.noteOn(note, 1.0f)` with the note from the pitch CV. Nothing else is needed.

## History

- Round 1 (layers: sub / click / noise / metal FM into one drive, bias, tone, squash, filter track + route):
  BODY, saturate, punch, feedback and the filter kept; the layer block, bias, tone, squash, track and route
  replaced by the TAIL voice (the v2 concept, simplified: Length instead of Stack), click + noise moved into
  DRIVE, FM made much stronger, Filter Drive added.
