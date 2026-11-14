# zicModKick32_v3

A third take on the 32 pot kick (v1 `zicModKick32` and v2 `zicModKick32_v2` stay as they are):
**BODY > VOWEL > ONE DRIVE > FILTERBANK**, for tekno / hardtek / metal tek / hardcore / tribe.
32 pots, no menu, no switch: every choice is a morph. No second layers (they did not work).

- the **BODY** has 12 pots: amp, pitch drop, a wave morph with harmonics, and four from v1 that shape the wave
  itself: Skew, Sync, Resonator, Fold;
- click + noise are layered into the drive, and a comb tuned to the pitch feeds the drive back into itself;
- the **VOWEL** stage is two formant filters (a e i o u) on the driven sound: a talking kick. It filters what is
  there, it adds nothing;
- the **FILTERBANK** (Sherman idea): LP > HP, both tracking the pitch, with an envelope, audio rate FM of the
  cutoffs by the signal, and a drive after the filter.

```
BODY (wave, skew, sync, resonator, fold, harmonics, drop) + CLICK + NOISE --> [+ FEEDBACK comb] --> DRIVE
   --> VOWEL (2 formants) --> CRUSH --> LP > HP (reso, env, FM) --> FILTER DRIVE --> clip --> OUT
```

## Panel: the signal flow, left to right

```
+-- BODY (4 x 3, orange) -------+  +--- DRIVE -----+  +--- FILTER -------+
| Decay  Shape | Drop  D.Time   |  | Drive   Char  |  | Cutoff   Cutoff 2|
| Curve | Wave  Harm 2  Harm 3  |  | Punch   Fdbk  |  | Reso     Env     |
| Skew  Sync | Reson  Fold      |  | Click   C.Tone|  | Env Dec  FM      |
+-- VOWEL (4 x 1) --------------+  | Noise   Color |  | F.Drive  Crush   |
| Vowel  Mix  Reso  Shift       |  +---------------+  +------------------+
+-------------------------------+
```

Pot index == `Kick32v3::params[]` index (BODY 0..11, VOWEL 12..15, DRIVE 16..23, FILTER 24..31).
The grid position of each pot (4 rows x 8 columns) is in `core/Kick32v3Panel.h` (`potCol` / `potRow`).
The first 8 BODY pots are unchanged from before; the new ones are at the end, so nothing moved.

| # | Pot | What it does |
|---|-----|--------------|
| 0 | **Decay** | 40 ms .. 2 s |
| 1 | **Shape** | amp curve: punchy exponential -> linear -> gated (holds, then rounds off) |
| 2 | **Drop** | pitch drop depth, 0 .. 6 octaves above the note |
| 3 | **Drop Time** | 4 .. 512 ms |
| 4 | **Drop Curve** | laser (holds, then dives) -> linear -> zap (dives at once) |
| 5 | **Wave** | sine -> triangle -> square -> saw (level compensated: flat loudness over the morph) |
| 6 | **Harm 2** | 2nd harmonic, bipolar |
| 7 | **Harm 3** | 3rd harmonic, bipolar |
| 8 | **Skew** | phase warp, bipolar: pulls the wave left or right (sine to a leaning, saw-like shape) |
| 9 | **Sync** | hard sync: a slave oscillator restarted every cycle, its ratio dives with the pitch drop |
| 10 | **Resonator** | a resonant peak following the pitch, saturated: the peak rings on top of the wave |
| 11 | **Fold** | the wave folds over on itself (up to about a fold and a half) |
| 12 | **Vowel** | a -> e -> i -> o -> u: where the two formants sit |
| 13 | **Vowel Mix** | 0 = off. Up: the formants come in, the dry sound steps back |
| 14 | **Vowel Reso** | wide mumble -> narrow whistle |
| 15 | **Vowel Shift** | moves both formants, -1 .. +1 octave (a bigger or smaller mouth) |
| 16 | **Drive** | gain into the one saturation, 1x .. 128x |
| 17 | **Character** | soft (tube) -> hard clip -> wavefold (max 3 folds) -> rectified fuzz (octave up) |
| 18 | **Punch** | the drive has its own envelope (bipolar): + extra drive on the hit, - extra drive in the tail |
| 19 | **Feedback** | a comb tuned to the pitch feeding the drive back into itself (bipolar). + reinforces the pitch, - half harmonics |
| 20 | **Click** | click oscillator on the hit (a short sine burst), into the drive |
| 21 | **Click Tone** | 300 Hz, 8 ms knock -> 6 kHz, 1 ms tick |
| 22 | **Noise** | noise into the drive. Its length follows Decay (shorter when bright) |
| 23 | **Color** | noise filter: LP -> BP -> HP while the cutoff rises (100 Hz .. 10 kHz) |
| 24 | **Cutoff** | filter 1, low pass, 30 Hz .. 20 kHz. Both cutoffs follow the note and the pitch drop |
| 25 | **Cutoff 2** | filter 2, high pass after the low pass, 15 Hz .. 7.7 kHz |
| 26 | **Reso** | both filters, up to a scream. Soft clipped: resonance growls instead of whistling |
| 27 | **Env** | bipolar: + opens both filters on the hit (6 oct), - closes them |
| 28 | **Env Decay** | 5 ms .. 1 s |
| 29 | **FM** | the signal itself modulates both cutoffs at audio rate, up to about 2.6 octaves |
| 30 | **Filter Drive** | saturation after the filter, 1 .. 25x |
| 31 | **Crush** | sample rate reduction, then bits (before the filter) |

No Tune (pitch = note from the external sequencer, note 60 = 52 Hz), no velocity, no volume
(fixed level, soft clipper). The filters are gated ~150 ms after the sound ends: a drum module must not drone.

### Starting points

- **Hardcore**: Drive 75, Character 40 (hard clip), Shape 60, Punch -50, Decay 500.
- **Talking kick**: Vowel 40, Vowel Mix 70, Vowel Reso 60, Sync 30, Skew 30.
- **Tribe**: Cutoff 45, Reso 70, Env +60, Cutoff 2 30, Filter Drive 60.
- **Metal tek**: Feedback 70, Drive 60, Character 70 (fold), Fold 60, Resonator 30.
- **Growl**: FM 70, Cutoff 50, Reso 60, Cutoff 2 30.

## Emulator

```bash
cd zicModKick32_v3 && make      # build + run
ZIC_SCREENSHOT=shot.png emu/build/x86/zic             # save a frame and quit
ZIC_SCREENSHOT=shot.png ZIC_DEMO=1 emu/build/x86/zic  # same, with a busy patch and a hit in the scope
```

Each card has a live preview: one cycle of the body wave; the two formants over the spectrum; the drive transfer
curve; the filter response. The HIT scope shows the waveform of the last hit with its pitch and amp curves.

| Where | Action |
|-------|--------|
| Knob | drag up/down (shift = fine), wheel (1 %, shift 0.2 %), double click = default |
| Step | click toggles, drag paints, wheel = note (semitones), shift+wheel = octave, right click = note 0 |
| Keys | `space` trigger (stopped) / hold = mute the kick (playing), `Enter` or `P` play/stop, `Esc` quit |
| BPM / TUNE | wheel on the box, or `-` / `+` (shift = x10 / octave) |

The 64 step sequencer at 180 BPM is for testing only (the module just receives a gate and a pitch CV).
State: `~/.config/zicModKick32_v3/state.txt` (pots stored by key, autosaved every second when something changed).

## Code

```
audio/engines/Kick32v3.h       ENGINE   no UI / OS dependency (Daisy Seed / STM32H7)
zicModKick32_v3/core/          MODULE   panel model: pot -> param, block / group layout, ADC smoothing
zicModKick32_v3/emu/           EMULATOR (gitignored in this repo) SFML + ALSA + 64 step test sequencer
```

Per sample: no std::exp / pow / tan (fast exp2 / sin / tan), pot dependent coefficients every 32 samples.
Memory ~20 KB (a 4096 sample comb for Feedback). Offline: ~0.4 us / sample on x86
(to measure on the H7). Checked offline: no NaN, output held at or below 0.95 over 300 random settings x 4 fast
hits on notes 36..84, silence 3.5 s after a hit with everything at max, and every pot measured
for its effect on the spectrum and envelope.

Firmware (later): `Kick32v3 + Panel`, read the 32 ADC channels into `panel.update(i, raw01)`,
gate input -> `kick.noteOn(note, 1.0f)` with the note from the pitch CV.

## History

- Round 1: layers into one drive, bias, tone, squash, filter track and route. Replaced in round 2.
- Round 2: TAIL voice (a second voice crossfaded in) did not sound good. Replaced by BOUNCE + SUB. Filter FM range halved.
- Round 3: a band picker (range, start, mix). Not liked, removed.
- Round 4: Mad VCA + Sub in the bottom left block. Not what was asked.
- Round 5: BODY takes row 3 (12 pots: Skew, Sync, Resonator, Fold from v1). Row 4 is VOWEL, two formant filters.
  No more second layers.
