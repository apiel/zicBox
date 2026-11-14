# zicModKick32_v5

**A big BODY > a heavy DRIVE > a one-pot FILTER**, for hardtek / mental tribe / hardcore / tekno.
One voice, 32 pots (4 x 8), no menu, no switch. Engine: `audio/engines/Kick32v5.h` (header only, no UI / OS dependency).

```
BODY + CLICK --> [+ FEEDBACK comb] --> gain (PUNCH) --> BITE --> FUZZ --> SPUTTER bias --> DRIVE 2x (CHARACTER)
   --> FAT --> CRUSH --> FILTER (LP <> HP) --> FILTER DRIVE --> soft clip --> OUT
```

## Panel

```
+-- BODY (amber, 4 x 4) --------+  +-- DRIVE (red, 3 x 4) ----+  +- FILTER -+
| Decay  Shape | Drop  D.Time   |  | Drive   Char    Sputter  |  | Filter   |
| Curve | Wave   Harm 2  Harm 3 |  | Punch   Fdbk    Crush    |  | Reso     |
| Skew   Sync  | Reson   Fold   |  | Bite    B.Freq | Fuzz    |  | Env      |
| Harm4 | Growl  Grit | Steps   |  | Click   Tone  | Fat      |  | Drive    |
+-------------------------------+  +--------------------------+  +----------+
```

Pot index == `Kick32v5::params[]` index: BODY 0..15, DRIVE 16..27, FILTER 28..31 (each block row by row).
The first three BODY rows are the v4 / v3 body, same positions. Row 4 is new.

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
| 12 | **Harm 4** | 4th harmonic, bipolar |
| 13 | **Growl** (new) | period doubling: one cycle runs short, the next one long. The pitch stays, a rough octave below growls under it. It moves the zero crossings, so it survives any clipper |
| 14 | **Grit** (new) | low noise (~100 Hz) shakes the phase of the wave: torn, hoarse, every harmonic gets a noise skirt |
| 15 | **Steps** (new) | the pitch drop falls in steps instead of sliding: 9 small steps -> 2 big ones. A zipper on a short drop, an arpeggio on a long one (needs Drop) |
| 16 | **Drive** | gain into the saturation, 1x .. 128x, 2x oversampled |
| 17 | **Character** | soft (tube) -> hard clip -> wavefold -> rectified fuzz |
| 18 | **Sputter** (new, its own pot) | a bias the decaying body cannot follow: the square thins into a pulse along the tail (50 % -> ~15 % duty), even harmonics that grow and move. Flips side with Skew so it never loses level |
| 19 | **Punch** | drive envelope, bipolar: + dirty hit, - dirty tail |
| 20 | **Feedback** | a comb tuned to the pitch feeding the drive back into itself, bipolar |
| 21 | **Crush** | sample and hold after the drive, 44 kHz down to ~500 Hz |
| 22 | **Bite** (new) | clip > resonant peak > drive. A first clip makes harmonics, a band pass rings on them, the ring goes into the drive: the clipper bites at that frequency, the "tok". It follows the pitch, so it dives with the drop |
| 23 | **Bite Freq** (new) | where the bite rings, 1.5x .. 12x the pitch (needs Bite) |
| 24 | **Fuzz** (new) | octave fuzz: the wave is rectified in front of the saturation, an octave up screams over the fundamental (which stays) |
| 25 | **Click** | click on the hit, into the drive and again after it |
| 26 | **Tone** | 300 Hz knock -> 6 kHz noise tick over the first two thirds, then the tick stretches into a noise burst (a fifth of Decay) |
| 27 | **Fat** (new) | the lows around the pitch are pushed back into a last soft clip: the low end swells, the buzz above is pressed down, the kick gets denser |
| 28 | **Filter** | 24 dB filter (two stages) following the pitch. Left = low pass closing to 30 Hz, centre = open, right = high pass up to 1.9 kHz. Both sides fade into the dry signal around the centre: crossing LP <-> HP makes no jump, even at full Drive (measured: same waveform from -4 % to +4 %) |
| 29 | **Reso** | resonance (soft clipped: it growls) |
| 30 | **Env** | filter envelope, bipolar, 6 octaves. It lasts a quarter of Decay |
| 31 | **Drive** | distortion between the two filter stages, 1 .. 25x: the first stage rings, the drive makes it growl, the second stage smooths what the drive added. It also works with the filter open |

## What changed from v4, and why

| Change | Why |
|---|---|
| **BODY row 4 = Harm 4, Growl, Grit, Steps** | round 2 feedback: Bend, FM and Wave Env out, "something different from the other four versions". Growl, Grit and Steps exist in none of them. A first try, Stairs (the wave quantized to a few levels), measured under 1 dB once driven (a clipper erases amplitude steps) and was replaced by Growl |
| **DRIVE has 12 pots** | Sputter (own pot, was hidden in the top of Drive in v4), Bite + Bite Freq (distortion > resonant EQ > distortion), Crush, Fuzz, Fat |
| **Shift out, Noise out** | round 2 feedback. The click is two pots; the end of Tone still gives a noise burst |
| **Crush is rate only, much deeper** | measured: after a clipper, bit reduction does nothing (a square has two levels) |
| **Filter: 4 pots, with its own Drive** | one LP <> HP pot, Reso, Env, Drive. The envelope time follows Decay. Gone: Cutoff 2, filter FM, Vowel, Env Decay |
| **Flatter squares** | the DC blockers moved from 21 / 10 Hz to 10 / 5 Hz: the top of a driven square sags less, more sub |
| **Defaults are a hard kick** | Drive 60, Sputter 25, filter open |

## Measured (`make test`, numbers from the offline harness)

| Check | v5 | v4 |
|---|---|---|
| NaN / worst peak, 300 random settings x 4 hits | none / 0.950 | none / 0.950 |
| Silence 3.5 s after a hit, all max (Decay 99 %) / min / default | yes | yes |
| Decay at max: level at 3 s vs 11 s | same (holds) | same |
| Mean per-pot impact, 3 contexts | 9.3 dB | 10.2 dB |
| Pots under 1.5 dB in every context | 0 | 1 |
| Loudness humps | 0 besides Filter (open in the centre by design); Skew is flat now (+-1 dB, was a 5 dB hump) | 1 (Skew) |
| Time per sample (x86) | ~0.41 .. 0.45 us | ~0.42 us |

Dependent pots (zero impact alone, by design): Bite Freq (needs Bite). Softest pots: Punch (1.8 dB, kept on purpose),
Steps (2.5 dB with the default 73 ms drop; it grows with Drop Time). Fuzz at 100 % puts the octave above the
fundamental (sub -6 dB). Level over the range: Sputter -2.2 dB at max, Fuzz -1.5 dB, everything else within 1 dB.

Against the hard kicks in `~/Music/samples/kick` (energy per band, dB relative to the total, first 370 ms):

| | sub < 100 | 100-300 | 300-1.5k | 1.5-6k |
|---|---|---|---|---|
| KICK00CORE.wav | -1.2 | -7.9 | -11.3 | -21.5 |
| tektribe-bd | -2.5 | -4.1 | -13.1 | -26.0 |
| v5 INIT | -1.4 | -6.2 | -15.2 | -35.6 |
| v5 HARDTEK preset | -0.9 | -8.4 | -14.0 | -22.8 |
| v5 TRIBE preset | -2.3 | -4.6 | -12.3 | -22.2 |
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
