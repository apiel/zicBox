# zicModKick32_v2

A second, different take on the 32 pot kick (v1 `zicModKick32` stays as is): a **layered kick forge**
for tekno / hardtek / metal tek / hardcore / tribe. 32 pots, no menu, no switch: every choice is a morph.

The idea comes from the way kicks are built in a DAW (a body + a layer on top) and from the MSF DROP
(tone + click + noise into one saturation): the variety comes from **how two voices are combined**.

```
LAYER ─────────────┬──────(into drive / ring)──┐                     ┌──(on top)──┐
  │ (FM)            │                           ▼                     │            ▼
BODY osc ─► amp ───┴────────────────────────► DRIVE ► TONE ► CRUSH ► VCA ┴─► DJ FILTER ──┬─► punch/clip ─► OUT
                                                                                        └─► RUMBLE ──┘
```

## Panel: three modules, not four rows

```
┌──────────── BODY (4 x 4) ────────────┐ ┌──────────── LAYER ───────────────┐
│ AMP / FM  Decay  Hold   FM    FM Ratio│ │ VOICE  Source Pitch  Follow Decay │
│ PITCH     Drop   D.Time D.Curve Bend  │ │ SHAPE  Delay  Tone   Blend  Level │
│ WAVE      Wave   Harm 2 Harm 3 Feedbk │ ├──────────── MASTER ──────────────┤
│ DIRT      Drive  Charac. Tone  Crush  │ │ FILTER Filter Reso   Spread Sweep │
└───────────────────────────────────────┘ │ RUMBLE Rumble Length Color  Pump  │
                                          └───────────────────────────────────┘
```

Pot index == `Kick32v2::params[]` index (BODY 0..15, LAYER 16..23, MASTER 24..31).
The grid position of each pot is in `core/Kick32v2Panel.h` (`potCol` / `potRow`).

### BODY
- **Decay**, **Hold** (punchy -> linear -> gated), **FM** (index follows the decay), **FM Ratio** (0.5..8x, in between = metal).
- **Drop** (0..6 oct), **Drop Time** (4..512 ms), **Drop Curve** (laser -> linear -> zap), **Bend** (slow fall over the whole decay, up to -2 oct).
- **Wave**: sine -> triangle -> square -> saw. **Harm 2 / Harm 3**: bipolar, strong (as in v1). **Feedback**: the oscillator modulates its own phase (sine -> saw -> chaos).
- **Drive** (gain 1..128), **Character**: soft tube -> hard clip -> wavefold -> rectified fuzz (octave up). **Tone**: low pass after the drive (100 = open). **Crush**: sample rate, then bits.

### LAYER
- **Source** morph: sine -> pulse -> metal -> noise -> screech (hard synced saw sweeping down).
- **Pitch** -24..+48 st from the body note, **Follow**: how much it rides the body drop, **Decay** 2..2000 ms.
- **Delay** 0..300 ms: the layer can be a click on the attack, a flam, or come in on the tail.
- **Tone**: LP <- open -> HP. **Level**.
- **Blend** morph, how the layer joins the body: **on top** (clean, after the drive) -> **into the drive** (crunched together) -> **ring mod** with the body -> **FM** of the body.

### MASTER
- **Filter**: DJ style, LP <- off -> HP. **Reso**. **Spread**: + a narrow formant sliding from 2.5 kHz to 160 Hz (talking), - a serial high pass closing in (thin, tight). **Sweep**: the hit moves the filter over the Decay (+ opens, - closes).
- **Rumble** (level), **Length** (0.3..4.8 s), **Color** (darker / brighter + dirtier), **Pump** (each hit ducks the rumble).
- Punch (transient) and the output clipper are fixed at their "max" setting: no Volume pot.

No Tune (pitch = note from the external sequencer, note 60 = 52 Hz), no velocity.

## Emulator

```bash
cd zicModKick32_v2 && make     # build + run
ZIC_SCREENSHOT=shot.png ZIC_DEMO=1 emu/build/x86/zic   # frame with a late layer in the scope
```

New in the emulator: the **HIT scope** (the last hit: body in orange, layer in cyan, the pitch curve,
markers for Decay and where the layer starts), and previews of the layer source and the blend mode.
Controls are the same as v1. State: `~/.config/zicModKick32_v2/state.txt` (pots stored by key).

## Code

```
audio/engines/Kick32v2.h       ENGINE   no UI / OS dependency (Daisy Seed / STM32H7)
zicModKick32_v2/core/          MODULE   panel model: pot -> param, block layout, ADC smoothing
zicModKick32_v2/emu/           EMULATOR SFML + ALSA + 64 step test sequencer
```

Per sample no std::exp / pow / tan (fast exp2 / sin / tan). Memory ~43 KB (rumble FDN).
Offline, every pot at 80 %: ~0.47 us / sample on x86 (to measure on the H7).
