# zicModKick32_v2

A second, different take on the 32 pot kick (v1 `zicModKick32` stays as is): a **punch + tail kick forge**
for tekno / hardtek / metal tek / hardcore / tribe. 32 pots, no menu, no switch: every choice is a morph.

Hard kicks are built from two layers: a punchy body for the hit and a tonal, distorted tail.
Here the BODY is the punch, and a second voice, the TAIL, takes over at **Start** with its own
pitch (tuned tails), glide, wave, unison stack, dirt and formant.

```
BODY osc > DRIVE > TONE > CRUSH --+
                                  +-> crossfade at Start > VCA > FILTER -+-> punch/clip > OUT
TAIL osc (stack) > DIRT > TONE ---+                                      +-> RUMBLE -+
```

## Panel: three modules, not four rows

```
+------------ BODY (4 x 4) -------------+ +------------- TAIL -----------------+
| AMP / FM  Decay  Hold   FM     FM Ratio| | VOICE   Tail   Start  Pitch  Glide |
| PITCH     Drop   D.Time D.Curve Bend   | | TIMBRE  Shape  Stack  Dirt   Tone  |
| WAVE      Wave   Harm 2 Harm 3 Feedbk  | +------------ MASTER ---------------+
| DIRT      Drive  Charac. Tone  Crush   | | FILTER  Filter Reso   Env  Env Dec.|
+----------------------------------------+ | SPREAD  Spread Rumble Length Pump  |
                                           +-----------------------------------+
```

Pot index == `Kick32v2::params[]` index (BODY 0..15, TAIL 16..23, MASTER 24..31).
The grid position of each pot is in `core/Kick32v2Panel.h` (`potCol` / `potRow`).

### BODY
- **Decay**, **Hold** (punchy -> linear -> gated), **FM** (index follows the decay), **FM Ratio** (0.5..8x, in between = metal).
- **Drop** (0..6 oct), **Drop Time** (4..512 ms), **Drop Curve** (laser -> linear -> zap), **Bend** (slow fall over the whole decay, up to -2 oct).
- **Wave**: sine -> triangle -> square -> saw. **Harm 2 / Harm 3**: bipolar, strong (as in v1). **Feedback**: the oscillator modulates its own phase (sine -> saw -> chaos).
- **Drive** (gain 1..128), **Character**: soft tube -> hard clip -> wavefold -> rectified fuzz (octave up). **Tone**: low pass after the drive (100 = open). **Crush**: sample rate, then bits.

### TAIL
- **Tail**: how much the tail takes over (0 = body only). **Start**: when (5..300 ms, 20 ms equal power crossfade).
- **Pitch**: -12..+24 st from the note (a tuned tail). **Glide**: the tail starts at the body pitch and phase, then slides to its pitch (0..400 ms).
- **Shape**: sine -> triangle -> saw -> square -> narrow pulse. **Stack**: 3 voice unison detune (up to ~50 cents).
- **Dirt**: its own drive (1..64x). **Tone**: LP <- open -> resonant formant on a harmonic of the tail (screech / talk).
- The tail shares the body Decay / Hold / Bend: the gated Hold makes a sustained tail.

### MASTER
- **Filter**: DJ style, LP <- off -> HP. The cutoff **follows the pitch** (drop + note). **Reso** up to self oscillation, the filter output is saturated (resonance growls).
- **Env** (bipolar): + opens the filter on the hit, - closes it. **Env Decay** 5..1000 ms.
- **Spread**: + a narrow formant sliding from 2.5 kHz to 160 Hz (talking), - a serial high pass closing in (thin, tight).
- **Rumble** (level), **Length** (0.3..4.8 s), **Pump** (each hit ducks the rumble). Warm: low passed at 170 Hz, light saturation.
- Punch (transient) and the output clipper are fixed at their "max" setting: no Volume pot.

No Tune (pitch = note from the external sequencer, note 60 = 52 Hz), no velocity.

## Emulator

```bash
cd zicModKick32_v2 && make     # build + run
ZIC_SCREENSHOT=shot.png ZIC_DEMO=1 emu/build/x86/zic   # frame with a late tuned tail in the scope
```

New in the emulator: the **HIT scope** (the last hit: output in orange, the tail part in cyan, the pitch
curve, markers for Decay and where the tail takes over), previews of the tail wave and of the handover.
Controls are the same as v1. State: `~/.config/zicModKick32_v2/state.txt` (pots stored by key).

## Code

```
audio/engines/Kick32v2.h       ENGINE   no UI / OS dependency (Daisy Seed / STM32H7)
zicModKick32_v2/core/          MODULE   panel model: pot -> param, block layout, ADC smoothing
zicModKick32_v2/emu/           EMULATOR SFML + ALSA + 64 step test sequencer
```

Per sample no std::exp / pow / tan (fast exp2 / sin / tan). Memory ~43 KB (rumble FDN).
Offline, every pot at 80 %: ~0.43 us / sample on x86 (to measure on the H7).
