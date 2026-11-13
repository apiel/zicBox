# zicModKick32_v3

A third take on the 32 pot kick (v1 `zicModKick32` and v2 `zicModKick32_v2` stay as they are):
**LAYERS > ONE DRIVE > FILTERBANK**, for tekno / hardtek / metal tek / hardcore / tribe.
32 pots, no menu, no switch: every choice is a morph.

Two hardware ideas, merged:

- the **DROP**: several sources (tone, sub, click, noise) layered into **one** saturation, with a bias;
- the **Sherman**: two resonant filters (serial <-> parallel) that **track the pitch**, with an envelope,
  **audio rate FM** of the cutoffs, and a drive with its own envelope.

```
BODY  (wave, harmonics, drop, metal FM) --+
SUB   (-1 oct, follows the drop)          +--> [+ FEEDBACK comb] --> DRIVE (punch, character, bias) --> TONE --> CRUSH
CLICK (oscillator, own tiny drop)         |                                                                       |
NOISE (color, own decay) -----------------+       OUT <-- clip <-- SQUASH <-- FILTER 1 (LP) =+= FILTER 2 <--------+
                                                                              serial: LP > HP    parallel: LP + BP
```

## Panel: the signal flow, left to right

```
+-------- BODY (4 x 2) --------------------+  +--- DRIVE ---+  +--- FILTER ---+
| AMP: Decay  Shape | DROP: Drop  Drop Time |  | Drive  Char |  | Cutoff  Cut 2|
| DROP: Curve | WAVE: Wave  Harm 2  Harm 3  |  | Bias   Punch|  | Reso    Track|
+-------- LAYERS (4 x 2) ------------------+  | Tone   Crush|  | Env   EnvDec |
| NOISE: Noise Color N.Decay | SUB: Sub     |  | Fdback Squash  | Route    FM  |
| CLICK: Click Tone | METAL: Metal  Ratio   |  +-------------+  +--------------+
+------------------------------------------+
```

Pot index == `Kick32v3::params[]` index (BODY 0..7, LAYERS 8..15, DRIVE 16..23, FILTER 24..31).
The grid position of each pot (4 rows x 8 columns) is in `core/Kick32v3Panel.h` (`potCol` / `potRow`):
columns 0..3 are the sources (BODY on rows 0..1, LAYERS on rows 2..3), columns 4..5 DRIVE, columns 6..7 FILTER.

| # | Pot | What it does |
|---|-----|--------------|
| 0 | **Decay** | 40 ms .. 2 s |
| 1 | **Shape** | amp curve: punchy exponential -> linear -> gated (holds, then rounds off) |
| 2 | **Drop** | pitch drop depth, 0 .. 6 octaves above the note |
| 3 | **Drop Time** | 4 .. 512 ms |
| 4 | **Drop Curve** | laser (holds, then dives) -> linear -> zap (dives at once) |
| 5 | **Wave** | sine -> triangle -> square -> saw |
| 6 | **Harm 2** | 2nd harmonic, bipolar, strong (as in v1) |
| 7 | **Harm 3** | 3rd harmonic, bipolar, strong |
| 8 | **Noise** | noise layer, into the same drive as the body |
| 9 | **Color** | noise filter: LP -> BP -> HP while the cutoff rises (100 Hz .. 10 kHz). Low = rumbling dirt, high = hiss / snap |
| 10 | **Noise Decay** | 3 ms (a snap on the hit) .. 1.5 s (a tail that outlasts the kick) |
| 11 | **Sub** | a sine one octave below the body, following the drop: deeper, and an octave-down growl once driven |
| 12 | **Click** | click oscillator on the hit (a short sine burst with its own one octave drop) |
| 13 | **Click Tone** | 300 Hz, 8 ms knock -> 6 kHz, 1 ms tick |
| 14 | **Metal** | FM of the body by a modulator that decays over ~60 ms |
| 15 | **Metal Ratio** | modulator ratio 0.5x .. 8x, in between = inharmonic / metallic |
| 16 | **Drive** | gain into the one saturation, 1x .. 128x |
| 17 | **Character** | soft (tube) -> hard clip -> wavefold (max 3 folds, stays a tone) -> rectified fuzz (octave up) |
| 18 | **Bias** | asymmetry of the saturation (bipolar): even harmonics, pulse width on a hard clip, shifted fold. Fades with the signal, so no DC in the silence |
| 19 | **Punch** | the drive has its own envelope (bipolar): + extra drive on the hit (crunchy attack, cleaner tail), - extra drive in the tail (clean hit, the tail distorts and swells). Time = 20 % of Decay |
| 20 | **Tone** | low pass after the drive, 150 Hz .. open |
| 21 | **Crush** | sample rate reduction, then bits |
| 22 | **Feedback** | a comb tuned to the pitch (one period, follows the drop) feeding the drive back into itself (bipolar). + reinforces the pitch and rings the drop, - cancels it (half harmonics: hollow, gritty). Dies with the amp envelope |
| 23 | **Squash** | dynamics (bipolar): - compress (the tail comes up, fat and sustained), + expand (gated, dry, only the hit) |
| 24 | **Cutoff** | filter 1, low pass, 30 Hz .. 20 kHz |
| 25 | **Cutoff 2** | filter 2, 15 Hz .. 7.7 kHz: a high pass in serial, a band pass peak in parallel |
| 26 | **Reso** | both filters, up to a scream. The band state is soft clipped and the output saturated: resonance growls instead of whistling |
| 27 | **Track** | both cutoffs follow the note and the pitch drop (0 = static, 100 = 1:1). With Reso: a laser diving with the kick |
| 28 | **Env** | bipolar: + opens both filters on the hit (6 oct), - closes them (the tail opens up) |
| 29 | **Env Decay** | 5 ms .. 1 s |
| 30 | **Route** | 0 = serial (LP then HP: a band), 100 = parallel (LP + a BP formant peak), in between = both |
| 31 | **FM** | the body oscillator modulates both cutoffs at audio rate (up to +-4 oct): growl, ring, talk |

No Tune (pitch = note from the external sequencer, note 60 = 52 Hz), no velocity, no volume
(fixed level, soft clipper). The ring of the filters is gated ~150 ms after the kick ends: a drum module must not drone.

### Starting points

- **Hardcore tail**: Drive 75, Character 40 (hard clip), Tone 60, Shape 60, Sub 40, Punch -50 (the tail distorts), Decay 500.
- **Tribe / Sherman**: Cutoff 45, Reso 70, Env +60, Track 80, Route 50, Cutoff 2 30.
- **Metal tek**: Metal 60, Metal Ratio 55, Feedback +70, Drive 60, Character 70 (fold).
- **Hardtek snap**: Noise 50, Color 70, Noise Decay 45, Click 70, Click Tone 80, Squash +60, Drive 55.
- **Growl**: FM 70, Cutoff 50, Reso 60, Route 100, Cutoff 2 50.

## Emulator

```bash
cd zicModKick32_v3 && make      # build + run
ZIC_SCREENSHOT=shot.png emu/build/x86/zic             # save a frame and quit
ZIC_SCREENSHOT=shot.png ZIC_DEMO=1 emu/build/x86/zic  # same, with a busy patch and a hit in the scope
```

The UI is drawn as the signal flow: BODY + LAYERS (left) > DRIVE > FILTER, with a live preview in each card:
the body wave, the noise color response (+ where the click sits), the **drive transfer curve** and the
**frequency response of the filterbank**. The HIT scope on top shows the waveform of the last hit (min / max
per half millisecond, 600 ms) with the pitch and amp curves and the Drop / Decay markers.

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

Per sample no std::exp / pow / tan / log (fast exp2 / log2 / sin / tan), pot dependent coefficients every
32 samples. Memory ~20 KB (a 4096 sample comb for the Feedback). Offline, every pot at 80 %: ~0.35 us / sample
on x86 (to measure on the H7). Checked offline: no NaN / no output above 0.95 over 300 random settings x 4 fast
hits on notes 36..84, silence 2 s after a hit with Reso / Feedback / compress at max, and every pot measured
for its effect on the spectrum and envelope (all of them do something; the dependent ones with their master on).

Firmware (later): `Kick32v3 + Panel`, read the 32 ADC channels into `panel.update(i, raw01)`,
gate input -> `kick.noteOn(note, 1.0f)` with the note from the pitch CV. Nothing else is needed.
