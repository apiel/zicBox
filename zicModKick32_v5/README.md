# zicModKick32_v5

**A big BODY > a heavy DRIVE > a one-pot FILTER**, for hardtek / mental tribe / hardcore / tekno.
One voice, 32 pots (4 x 8), no menu, no switch. Engine: `audio/engines/Kick32v5.h` (header only, no UI / OS dependency).

```
BODY + CLICK + NOISE --> [+ FEEDBACK comb] --> gain (PUNCH) --> BITE --> SPUTTER bias --> DRIVE 2x (CHARACTER)
   --> CRUSH --> SHIFT --> FILTER (LP <> HP) --> soft clip --> OUT
```

## Panel

```
+-- BODY (amber, 4 x 4) --------+  +-- DRIVE (red, 3 x 4) ----+  +- FILTER -+
| Decay  Shape | Drop  D.Time   |  | Drive   Char    Sputter  |  | Filter   |
| Curve | Wave   Harm 2  Harm 3 |  | Punch   Fdbk    Crush    |  | Reso     |
| Skew   Sync  | Reson   Fold   |  | Bite    B.Freq | Shift   |  | Env      |
| Bend | FM     Ratio | WaveEnv |  | Click   Tone    Noise    |  | EnvDecay |
+-------------------------------+  +--------------------------+  +----------+
```

Pot index == `Kick32v5::params[]` index: BODY 0..15, DRIVE 16..27, FILTER 28..31 (each block row by row).
The first three BODY rows are the v4 / v3 body, same positions. Row 4 keeps Bend, FM Depth, FM Ratio where they were.

| # | Pot | What it does |
|---|-----|--------------|
| 0 | **Decay** | 40 ms .. 2 s. At the very maximum: infinite, the body holds until the next hit |
| 1 | **Shape** | amp curve: punchy exponential -> linear -> gated |
| 2 | **Drop** | pitch drop depth, 0 .. 6 octaves |
| 3 | **Drop Time** | 4 .. 512 ms |
| 4 | **Drop Curve** | laser (holds, then dives) -> linear -> zap |
| 5 | **Wave** | sine -> triangle -> square -> saw, level compensated |
| 6 | **Harm 2** | 2nd harmonic, bipolar |
| 7 | **Harm 3** | 3rd harmonic, bipolar |
| 8 | **Skew** | phase warp, bipolar |
| 9 | **Sync** | hard sync, the slave ratio dives with the drop |
| 10 | **Resonator** | a resonant peak following the pitch, saturated |
| 11 | **Fold** | the wave folds over on itself |
| 12 | **Bend** | the tail slides in pitch over the decay, bipolar, up to 2 octaves |
| 13 | **FM Depth** | phase modulation, the index falls over ~45 ms |
| 14 | **FM Ratio** | 0.5x .. 8x (needs FM Depth) |
| 15 | **Wave Env** (new) | bipolar. + the designed wave on the hit melts into a pure sine tail; - a sine hit grows into the designed wave. Makes every wave pot (Wave, Harm, Skew, Sync, Fold) move along the decay |
| 16 | **Drive** | gain into the saturation, 1x .. 128x, 2x oversampled |
| 17 | **Character** | soft (tube) -> hard clip -> wavefold -> rectified fuzz |
| 18 | **Sputter** (new, its own pot) | a bias the decaying body cannot follow: the square thins into a pulse along the tail (50 % -> ~15 % duty), even harmonics that grow and move. Flips side with Skew so it never loses level |
| 19 | **Punch** | drive envelope, bipolar: + dirty hit, - dirty tail |
| 20 | **Feedback** | a comb tuned to the pitch feeding the drive back into itself, bipolar |
| 21 | **Crush** | sample and hold after the drive, 44 kHz down to ~500 Hz |
| 22 | **Bite** (new) | clip > resonant peak > drive. A first clip makes harmonics, a band pass rings on them, the ring goes into the drive: the clipper bites at that frequency, the "tok". It follows the pitch, so it dives with the drop |
| 23 | **Bite Freq** (new) | where the bite rings, 1.5x .. 12x the pitch (needs Bite) |
| 24 | **Shift** | frequency shifter after the drive, sweeping down with the drop, bipolar |
| 25 | **Click** | click on the hit, into the drive and again after it |
| 26 | **Tone** | click: 300 Hz knock -> 6 kHz noise tick. Noise: dark -> bright |
| 27 | **Noise** | noise with the body, its length follows Decay |
| 28 | **Filter** | one filter following the pitch. Left = low pass closing to 30 Hz, centre = open, right = high pass up to 1.9 kHz |
| 29 | **Reso** | resonance (soft clipped: it growls) |
| 30 | **Env** | filter envelope, bipolar, 6 octaves |
| 31 | **Env Decay** | 5 ms .. 1 s (needs Env) |

## What changed from v4, and why

| Change | Why |
|---|---|
| **BODY is a 4 x 4 block** | the body rows keep their positions; Bend / FM join it (they are body pitch and timbre); **Wave Env** is new: waveform design that moves |
| **DRIVE grows from 8 to 12 pots** | "lots of distortion": Sputter, Bite, Bite Freq, Crush and Shift each do something the others cannot |
| **Sputter is a pot** | in v4 it was hidden in the top of Drive. Now full Drive without sputter is a clean hard square, and sputter works at any drive |
| **Bite + Bite Freq** | the classic hardcore chain "distortion > resonant EQ > distortion" in two pots |
| **Crush is rate only, much deeper** | measured: after a clipper, bit reduction does nothing (a square has two levels). The whole pot is now sample rate, down to ~500 Hz |
| **Filter: 8 pots -> 4** | one LP <> HP pot, Reso, Env, Env Decay. Gone: Cutoff 2, filter FM, Filter Drive (fixed), Vowel |
| **Noise Color merged into Tone** | Color was the weakest pot of v3 / v4 |
| **Defaults are a hard kick** | Drive 60, Sputter 25, filter open |

## Measured (`make test`, numbers from the offline harness)

| Check | v5 | v4 |
|---|---|---|
| NaN / worst peak, 300 random settings x 4 hits | none / 0.950 | none / 0.950 |
| Silence 3.5 s after a hit, all max (Decay 99 %) / min / default | yes | yes |
| Decay at max: level at 3 s vs 11 s | same (holds) | same |
| Mean per-pot impact, 3 contexts | 8.6 dB (3 fewer filter pots, which are always the loudest) | 10.2 dB |
| Pots under 1.5 dB in every context | 0 | 1 |
| Loudness humps | 0 besides Filter (open in the centre by design); Skew is flat now (+-1 dB, was a 5 dB hump) | 1 (Skew) |
| Time per sample (x86) | ~0.39 .. 0.45 us | ~0.42 us |

Dependent pots (zero impact alone, by design): FM Ratio (FM Depth), Bite Freq (Bite), Wave Env (a non-sine wave),
Env Decay (Env + a closed filter). Softest pots: Punch (1.9 dB), Tone (1.3 dB), both kept on purpose.

Against the hard kicks in `~/Music/samples/kick` (energy per band, dB relative to the total, first 370 ms):

| | sub < 100 | 100-300 | 300-1.5k | 1.5-6k |
|---|---|---|---|---|
| KICK00CORE.wav | -1.2 | -7.9 | -11.3 | -21.5 |
| tektribe-bd | -2.5 | -4.1 | -13.1 | -26.0 |
| v5 INIT | -1.3 | -6.3 | -16.0 | -36.3 |
| v5 HARDTEK preset | -1.2 | -7.5 | -13.0 | -21.4 |
| v5 TRIBE preset | -1.9 | -5.8 | -10.6 | -21.8 |
| v3 at full drive (for reference) | -0.3 | -13.1 | -16.5 | -23.9 |

## Emulator

```
make            # build + run            make build_zic   # build only
make test       # offline harness (v5)   make test_v4     # same tests on v4
ZIC_SCREENSHOT=shot.png ZIC_DEMO=1 emu/build/x86/zic
```

Drag / wheel a knob, double click = default. Space = hit, Enter = sequencer, 1..6 = presets
(INIT, HARDTEK, TRIBE, HARDCORE, METAL, TEKNO), D = dice. State autosaves to `~/.config/zicModKick32_v5/state.txt`.

## Code

- `audio/engines/Kick32v5.h` engine
- `core/Kick32v5Panel.h` panel model (blocks, groups, help), shared with the firmware
- `emu/` SFML + ALSA emulator (not part of the module)
- `harness/` offline tests
