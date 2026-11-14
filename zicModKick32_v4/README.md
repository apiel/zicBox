# zicModKick32_v4

A fourth take on the 32 pot kick (v1 `zicModKick32`, v2 `zicModKick32_v2` and v3 `zicModKick32_v3` stay as they are):
**BODY > MOTION > ONE DRIVE > FILTERBANK**, for tekno / hardtek / metal tek / hardcore / tribe.
32 pots, no menu, no switch: every choice is a morph. One voice: everything bends, modulates, folds or filters
the same sound, there is no second layer.

```
BODY (bend + drop pitch, FM, wave, skew, sync, resonator, fold) + CLICK + NOISE --> [+ FEEDBACK comb] --> DRIVE (2x)
   --> SHIFT --> VOWEL --> LP > HP (reso, env, FM, tracking the pitch) --> FILTER DRIVE --> clip --> OUT
```

## What changed from v3, and why

The BODY (12 pots), the DRIVE (8 pots) and the FILTER cutoffs / reso / env / FM / drive keep their pots, their
positions and their behaviour. Everything below was decided with the offline harness (`make test`), not by guessing.

| Change | Why |
|--------|-----|
| **Row 4 is MOTION: Bend, FM Depth, FM Ratio, Shift** | the row is now about what moves during the hit |
| **Bend** (new, pot 12) replaces Rectify | v3 had one pitch segment (the drop). Tribe and hardcore want a second one: a tail that slides up (the "wooop" riser) or sinks. Measured: +100 goes 52 -> 202 Hz over the decay, -100 goes down to 24 Hz. Rectify was a duplicate of the Character's fuzz region (both rectify before the clip) |
| **Shift** (new, pot 15) replaces Crush | a frequency shifter after the drive, something v3 could not do at all: the whole distorted spectrum becomes inharmonic. It sweeps down with the drop (1 kHz -> 250 Hz at max), so the attack is a laser and the tail is metal; small amounts are a beating / growl; - shifts down (different flavour, the low sidebands fold back). Half dry / half shifted, level trimmed flat. Crush was a lo-fi effect that mostly added aliasing fizz on a sub heavy kick |
| **Vowel** moves to the FILTER block (pot 31) | it is a filter (two formants). Same algorithm, now trimmed flat (v3 lost 3 dB when it came in) |
| **Click + Noise are also injected after the drive** | measured: fed only into the saturation, click and noise were inaudible above Drive 50 % (the body masks them in the clipper). Now they survive any drive; the pre-drive path is still there so the clipper reacts to the transient |
| **Click Tone** ends in a noise tick | 300 Hz sine knock -> 6 kHz noise tick, more range than a sine at 6 kHz |
| **Drive is 2x oversampled** | the saturation runs on the midpoint and the sample, then a 7 tap half band filter: less aliasing fizz at 128x gain. CPU stays at ~0.4 us / sample |
| **Character fold / fuzz do not die at high drive** | v3's fold went to *zero* when it clipped (sin of 3 pi): with a hard drive the loud part of the hit was silent. Now 2.5 folds with a full level plateau. The fuzz was a rectified square (= DC = silence after the DC blocker) at high gain: now half rectified, half clip. A measured trim table keeps the Character morph within 2 dB |

Rejected ideas (in the spirit of the v3 lessons): no second voice, no sub, no bounce, no band split, no post-drive
tone, no dynamics. A pre-drive EQ was considered for hardcore and left out: Feedback (+) and Resonator already give
a pre-drive resonance, and it would have cost a DRIVE pot.

## Panel: the signal flow, left to right

```
+-- BODY (4 x 3, amber) --------+  +--- DRIVE (red) ---+  +--- FILTER (cyan) ---+
| Decay  Shape | Drop  D.Time   |  | Drive   Char      |  | Cutoff    Cutoff 2  |
| Curve | Wave  Harm 2  Harm 3  |  | Punch   Fdbk      |  | Reso      Env       |
| Skew  Sync | Reson  Fold      |  | Click   C.Tone    |  | Env Dec   FM        |
+-- MOTION (4 x 1, violet) -----+  | Noise   Color     |  | F.Drive   Vowel     |
| Bend | FM Depth  FM Ratio | Shift  +-------------------+  +---------------------+
+-------------------------------+
```

Pot index == `Kick32v4::params[]` index (BODY 0..11, MOTION 12..15, DRIVE 16..23, FILTER 24..31).
The grid position of each pot (4 rows x 8 columns) is in `core/Kick32v4Panel.h` (`potCol` / `potRow`), with a help
line per pot (`help(i)`, shown by the emulator on hover).

| # | Pot | What it does |
|---|-----|--------------|
| 0 | **Decay** | 40 ms .. 2 s |
| 1 | **Shape** | amp curve: punchy exponential -> linear -> gated (holds, then rounds off) |
| 2 | **Drop** | pitch drop depth, 0 .. 6 octaves above the note |
| 3 | **Drop Time** | 4 .. 512 ms |
| 4 | **Drop Curve** | laser (holds, then dives) -> linear -> zap (dives at once) |
| 5 | **Wave** | sine -> triangle -> square -> saw (level compensated) |
| 6 | **Harm 2** | 2nd harmonic, bipolar |
| 7 | **Harm 3** | 3rd harmonic, bipolar |
| 8 | **Skew** | phase warp, bipolar: pulls the wave left or right (sine to a leaning, saw-like shape) |
| 9 | **Sync** | hard sync: a slave oscillator restarted every cycle, its ratio dives with the pitch drop |
| 10 | **Resonator** | a resonant peak following the pitch, saturated: the peak rings on top of the wave |
| 11 | **Fold** | the wave folds over on itself (up to about a fold and a half) |
| 12 | **Bend** | the tail slides in pitch over the decay, bipolar: + up to 2 octaves up, - down. Moves early, while the kick is loud |
| 13 | **FM Depth** | phase modulation of the body by a sine, the index falls over ~45 ms: a metallic attack, then a clean body |
| 14 | **FM Ratio** | modulator ratio 0.5x .. 8x. Whole numbers are harmonic, in between is metal |
| 15 | **Shift** | frequency shifter after the drive, bipolar. Sweeps down with the drop: 1 kHz on the hit -> 250 Hz on the tail at max. + up, - down, small = beating |
| 16 | **Drive** | gain into the one saturation, 1x .. 128x, 2x oversampled |
| 17 | **Character** | soft (tube) -> hard clip -> wavefold (2.5 folds) -> rectified fuzz (octave up) |
| 18 | **Punch** | the drive has its own envelope (bipolar): + extra drive on the hit, - extra drive in the tail |
| 19 | **Feedback** | a comb tuned to the pitch feeding the drive back into itself (bipolar). + reinforces the pitch, - half harmonics |
| 20 | **Click** | click on the hit, into the drive and again after it |
| 21 | **Click Tone** | 300 Hz, 8 ms sine knock -> 6 kHz, 1 ms noise tick |
| 22 | **Noise** | noise with the body (pre and post drive). Its length follows Decay (shorter when bright) |
| 23 | **Color** | noise filter: LP -> BP -> HP while the cutoff rises (100 Hz .. 10 kHz) |
| 24 | **Cutoff** | filter 1, low pass, 30 Hz .. 20 kHz. Both cutoffs follow the note, the drop and the bend |
| 25 | **Cutoff 2** | filter 2, high pass after the low pass, 15 Hz .. 7.7 kHz |
| 26 | **Reso** | both filters, up to a scream. Soft clipped: resonance growls instead of whistling |
| 27 | **Env** | bipolar: + opens both filters on the hit (6 oct), - closes them |
| 28 | **Env Decay** | 5 ms .. 1 s |
| 29 | **FM** | the signal itself modulates both cutoffs at audio rate, up to about 2.6 octaves |
| 30 | **Filter Drive** | saturation after the filter, 1 .. 25x |
| 31 | **Vowel** | two formant filters on the driven sound: 0 = off, the first fifth brings them in, then a -> e -> i -> o -> u |

No Tune (pitch = note from the external sequencer, note 60 = 52 Hz), no velocity, no volume
(fixed level, soft clipper). The filters are gated ~150 ms after the sound ends: a drum module must not drone.

### Starting points (the emulator's presets, keys 1..6)

- **Hardcore**: Drive 75, Character 40, Shape 60, Punch -50, Decay 500, Click 35, Click Tone 30.
- **Tribe**: Cutoff 45, Reso 70, Env +60, Cutoff 2 30, Filter Drive 60, Bend +25, Decay 600, Shape 55.
- **Hardtek**: Decay 300, Drop 60, Wave 30, Harm 2 +40, Feedback +50, Drive 60, Punch +30, Cutoff 70, Env +40, Noise 25.
- **Metal**: FM Depth 45, FM Ratio 58, Shift +45, Feedback +60, Drive 60, Character 70, Fold 40, Resonator 30.
- **Tekno**: Sync 35, Skew +30, Vowel 40, Bend -30, Drive 50, Cutoff 60, Reso 50, Env +50.
- The **dice** (key D) rolls a random kick that stays inside musical ranges.

## Measured (offline harness, `make test`, x86, 44.1 kHz)

- no NaN, output at or below 0.95 over 300 random settings x 4 fast hits on notes 36..84;
- silence (below -80 dB) 3.5 s after a hit with everything at max, at min and at default;
- every pot measured for its effect on the spectrum (1/3 octave bands), the envelope (10 ms frames) and the attack
  spectrum (first 60 ms), at 0 / 50 / 100 % in three contexts (defaults, a busy hardtek patch, a hardcore patch).
  Mean impact 10.2 dB (v3: 9.8 dB with the same harness, `make test_v3`). The only pot under 1.5 dB is Color, which
  depends on Noise being up (so do FM Ratio and Env Decay on FM Depth and Env: they are measured in the busy patch);
- loudness over the range of every pot in 11 steps: the only hump left is Skew (+5 dB at the centre), which is the
  v3 body as it is: a fully skewed wave is mostly DC, removed by the DC blocker. Character stays within 2 dB,
  Vowel within 1.6 dB, Shift within 0.7 dB;
- pitch track: Drop 100 reads 334 Hz at 15 ms; Bend +100 reads 52 -> 66 -> 117 -> 202 Hz at 15 / 60 / 250 / 600 ms,
  Bend -100 reads 52 -> 45 -> 24 Hz;
- ~0.40 us / sample on x86 (v3: 0.41 us). Memory ~17 KB (a 4096 sample comb + the Hilbert and filter states).

## Emulator

```bash
cd zicModKick32_v4 && make      # build + run
make test                       # offline harness (numbers above), make test_v3 for the reference
ZIC_SCREENSHOT=shot.png emu/build/x86/zic             # save a frame and quit
ZIC_SCREENSHOT=shot.png ZIC_DEMO=1 emu/build/x86/zic  # same, with a busy patch, a hit in the scope and a help bubble
```

- **HIT**: the waveform of the last hit over a time aligned **spectrogram** (20 Hz .. 16 kHz, log), with the pitch the
  body plays (drop + bend) drawn on it, the amp shape, and the DROP / DECAY markers. Next to it a **live spectrum**
  of the output with a slow peak hold.
- Every card has a live preview: the body cycle (over a faint sine), the **motion curves** (pitch, FM index, shift)
  over the decay, the drive transfer curve with the **signal riding on it**, the filterbank response **breathing** with
  its envelope (the resting curve stays faint), the vowel formants when they are in.
- Cards glow with their activity, knobs ease to their value and show a tick at their default.
- Hover a knob: a help bubble says what it does and its range. Presets 1..6, dice (D).

| Where | Action |
|-------|--------|
| Knob | drag up/down (shift = fine), wheel (1 %, shift 0.2 %), double click = default |
| Step | click toggles, drag paints, wheel = note (semitones), shift+wheel = octave, right click = note 0 |
| Keys | `space` trigger (stopped) / hold = mute the kick (playing), `Enter` or `P` play/stop, `1`..`6` presets, `D` dice, `Esc` quit |
| BPM / TUNE | wheel on the box, or `-` / `+` (shift = x10 / octave) |

The 64 step sequencer at 180 BPM is for testing only (the module just receives a gate and a pitch CV).
State: `~/.config/zicModKick32_v4/state.txt` (pots stored by key, autosaved every second when something changed).

## Code

```
audio/engines/Kick32v4.h       ENGINE   no UI / OS dependency (Daisy Seed / STM32H7)
zicModKick32_v4/core/          MODULE   panel model: pot -> param, block / group layout, help lines, ADC smoothing
zicModKick32_v4/emu/           EMULATOR SFML + ALSA + 64 step test sequencer
zicModKick32_v4/harness/       HARNESS  offline checks (NaN, peak, silence, per pot impact, loudness, pitch, timing)
```

Per sample: no std::exp / pow / tan (fast exp2 / sin / tan), pot dependent coefficients every 32 samples.
The frequency shifter is a Hilbert transformer (two cascades of four allpasses, |I + jQ| flat within 1 % from
30 Hz to 16 kHz) and a quadrature oscillator.

Firmware (later): `Kick32v4 + Panel`, read the 32 ADC channels into `panel.update(i, raw01)`,
gate input -> `kick.noteOn(note, 1.0f)` with the note from the pitch CV.
