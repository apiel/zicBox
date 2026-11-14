# zicModKick32_v5

**BODY > a very heavy DRIVE > a driven FILTER**, for hardtek / mental tribe / hardcore / tekno.
One voice, 32 pots (4 x 8), no menu, no switch. Engine: `audio/engines/Kick32v5.h` (header only, no UI / OS dependency).

```
BODY + CLICK --> [+ FEEDBACK comb] --> gain (PUNCH) --> WRAP --> BITE --> FUZZ --> SPUTTER bias --> DRIVE 2x (CHARACTER)
   --> CRUSH --> VOWEL --> PHASER --> COMP --> FILTER (LP <> HP, with its DRIVE) --> soft clip --> OUT
```

## Panel

```
+-- BODY (amber, 4 x 3) --------+  +-- DRIVE (red, 4 x 4) ----------------+
| Decay  Shape | Drop  D.Time   |  | Drive   Char    Sputter | Wrap       |
| Curve | Wave   Harm 2  Harm 3 |  | Punch   Fdbk    Crush   | Comb Tune  |
| Skew   Sync  | Reson   Fold   |  | Bite    B.Freq | Fuzz   | Vowel      |
+-- FILTER (cyan, 4 x 1) -------+  | Click   Tone  | Phaser  | Comp       |
| Filter Reso  | Env   | Drive  |  +--------------------------------------+
+-------------------------------+
```

Pot index == `Kick32v5::params[]` index: BODY 0..11, FILTER 12..15, DRIVE 16..31 (each block row by row).
The BODY is the v3 / v4 body, same positions. The three left DRIVE columns did not move either.

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
| 12 | **Filter** | 24 dB filter (two stages) following the pitch. Left = low pass closing to 30 Hz, centre = open, right = high pass up to 1.9 kHz. Both sides fade into the dry signal around the centre: crossing LP <-> HP makes no jump, even at full Drive |
| 13 | **Reso** | resonance (soft clipped: it growls) |
| 14 | **Env** | filter envelope, bipolar, 6 octaves. It lasts a quarter of Decay |
| 15 | **Drive** | distortion between the two filter stages, 1 .. 25x. It also works with the filter open |
| 16 | **Drive** | gain into the saturation, 1x .. 128x, 2x oversampled |
| 17 | **Character** | soft (tube) -> hard clip -> wavefold -> rectified fuzz |
| 18 | **Sputter** | a bias the decaying body cannot follow: the square thins into a pulse along the tail, even harmonics that grow and move |
| 19 | **Wrap** (new) | a wavefolder on the enveloped signal, before the clipper: it folds on the hit, less and less as the body decays. The harmonics sweep down along the tail on their own. Range limited to the usable part (up to one cycle of folding; three times more was trash) |
| 20 | **Punch** | drive envelope, bipolar: + dirty hit, - dirty tail |
| 21 | **Feedback** | a comb tuned to the pitch feeding the drive back into itself, bipolar |
| 22 | **Crush** | sample and hold after the drive, 44 kHz down to ~500 Hz |
| 23 | **Comb Tune** (new) | tuning of the feedback comb: on the pitch -> two octaves above; in between it rings out of tune (needs Feedback) |
| 24 | **Bite** | clip > resonant peak > drive: the clipper bites at a multiple of the pitch, the "tok" |
| 25 | **Bite Freq** | where the bite rings, 1.5x .. 12x the pitch (needs Bite) |
| 26 | **Fuzz** | octave fuzz: the wave rectified in front of the saturation |
| 27 | **Vowel** (new here) | two formants on the driven sound: 0 = off, then a -> e -> i -> o -> u. The filter drive after it makes it bark |
| 28 | **Click** | click on the hit, into the drive and again after it |
| 29 | **Tone** | 300 Hz knock -> 6 kHz noise tick, then the tick stretches into a noise burst |
| 30 | **Phaser** (new) | four notches sliding from 6 kHz down to 150 Hz along the decay, with feedback: a jet over the driven tail |
| 31 | **Comp** (new) | a compressor used as a sustain: the hit passes, then the gain rises as the level falls and pulls the tail up (up to +30 dB, 15 ms release) |

**About the compressor.** A classic compressor is close to useless in this engine, measured: after the clipper and
the output stage, the level of a hit already stays within 3 dB for its first 200 ms. A first version (90 ms
release) changed the sound by 0.5 dB. The Comp pot is therefore tuned as a sustain: with a 15 ms release it lifts
the end of the tail by 11 dB at 300 ms (default patch). It overlaps with Shape (gated), from the other side.

## What changed from v4, and why

| Change | Why |
|---|---|
| **BODY is the 12 pot body again** | two rounds of extra body pots (Bend, FM, Wave Env, then Harm 4, Growl, Grit, Steps) were all judged useless: removed |
| **FILTER sits under the BODY, 4 pots** | Filter (LP <> HP), Reso, Env, Drive. Two 12 dB stages with the drive between them; both fade to dry around the centre of the pot, so crossing LP <-> HP makes no jump (measured at full Drive: same waveform from -4 % to +4 %) |
| **DRIVE is a 4 x 4 block** | new right column: Wrap, Comb Tune, Vowel, Comp; Phaser replaces Fat ("boring"); Sputter, Bite, Bite Freq, Crush, Fuzz from the earlier rounds stay. Shift and Noise are gone |
| **Crush is rate only, much deeper** | measured: after a clipper, bit reduction does nothing (a square has two levels) |
| **Flatter squares** | the DC blockers moved from 21 / 10 Hz to 10 / 5 Hz: the top of a driven square sags less, more sub |
| **Defaults are a hard kick** | Drive 60, Sputter 25, filter open |

## Measured (`make test`, numbers from the offline harness)

| Check | v5 | v4 |
|---|---|---|
| NaN / worst peak, 300 random settings x 4 hits | none / 0.950 | none / 0.950 |
| Silence 3.5 s after a hit, all max (Decay 99 %) / min / default | yes | yes |
| Decay at max: level at 3 s vs 11 s | same (holds) | same |
| Mean per-pot impact, 3 contexts | 9.1 dB | 10.2 dB |
| Pots under 1.5 dB in every context | 0 | 1 |
| Loudness humps | 0 besides Filter (open in the centre by design); Skew is flat now (+-1 dB, was a 5 dB hump) | 1 (Skew) |
| Time per sample (x86) | ~0.43 us | ~0.42 us |

Dependent pots (zero impact alone, by design): Bite Freq (needs Bite), Comb Tune (needs Feedback).
Softest pots in the numbers: Punch (1.7 dB, kept on purpose), Vowel (2.1 dB), Phaser (2.5 dB: moving notches
average out in a long term spectrum). Level over the range: Sputter -2.2 dB at max, Fuzz -1.5 dB, Vowel -1.2 dB,
everything else within 1.2 dB.

Against the hard kicks in `~/Music/samples/kick` (energy per band, dB relative to the total, first 370 ms):

| | sub < 100 | 100-300 | 300-1.5k | 1.5-6k |
|---|---|---|---|---|
| KICK00CORE.wav | -1.2 | -7.9 | -11.3 | -21.5 |
| tektribe-bd | -2.5 | -4.1 | -13.1 | -26.0 |
| v5 INIT | -1.4 | -6.2 | -15.1 | -35.2 |
| v5 HARDTEK preset | -1.3 | -7.3 | -12.3 | -20.8 |
| v5 TRIBE preset | -2.0 | -6.7 | -8.8 | -16.4 |
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
