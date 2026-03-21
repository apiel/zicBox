#pragma once

#include "audio/Wavetable.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterMoog.h"
#include "audio/filterSVF.h"
#include "audio/filterTB.h"
#include "audio/utils/linearInterpolation.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// ─────────────────────────────────────────────────────────────────────────────
// Synth23  —  dual-wavetable synthesiser
//
// OSCILLATORS
//   WT1  carrier wavetable.  Always active.
//   WT2  secondary wavetable.  Mode selects its role:
//          Additive : mixed into the signal at its own level + envelope.
//          FM       : phase-modulates WT1 (classic 2-op FM).
//   Both oscillators share a base pitch derived from the MIDI note.
//   WT2 can be retuned via Coarse (semitones) + Fine (cents).
//
//   Sub oscillator: sine↔square blend, one octave below, mixed pre-filter.
//
// ENVELOPES
//   Each wavetable has an independent Attack + Decay (AD, looping sustain
//   while gate is open, release on note-off mirrors the Decay time).
//   A master AMP envelope (Attack + Release) shapes the final output.
//   VCF envelope = same AD shape as WT1, depth set by Env Mod.
//
// FILTER
//   Shared post-mix filter.  Selectable type (all Bass23 types + HP + BP).
//   Always-on HP trim stage after the main filter.
//
// SIGNAL FLOW
//   Additive:  mix = WT1×env1×lv1  +  WT2×env2×lv2
//   FM:        WT2×env2 modulates WT1 phase → WT1×env1×lv1
//   → sub mix → filter → drive → waveshape → HP trim → amp env → FX
//
// FX CHAIN
//   Drive → Waveshape → Delay → Reverb (Schroeder 4-comb / 3-AP)
//
// ─────────────────────────────────────────────────────────────────────────────

class Synth23 : public EngineBase<Synth23> {

public:
    static constexpr int DELAY_BUF_SIZE  = 48000;   // 1 s @ 48 kHz
    static constexpr int REVERB_BUF_SIZE = 16384;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    // ── Filter objects ────────────────────────────────────────────────────────
    FilterSVF  svfFilter;
    FilterTB   tbFilter;
    FilterMoog moogFilter;

    // ── Oscillator phases ─────────────────────────────────────────────────────
    float phase1   = 0.0f;   // WT1, in wavetable sample units
    float phase2   = 0.0f;   // WT2, in wavetable sample units
    float subPhase = 0.0f;   // sub osc [0, 1)
    float fbSample = 0.0f;   // previous WT1 output for feedback

    // ── Voice state ───────────────────────────────────────────────────────────
    float currentFreq = 440.0f;
    float targetFreq  = 440.0f;
    float velocity    = 1.0f;
    bool  gateOpen    = false;

    // ── WT1 AD envelope ───────────────────────────────────────────────────────
    // Stage: 0=off  1=attack  2=hold (gate open)  3=decay/release
    float env1     = 0.0f;
    int   env1Stage = 0;
    float env1AttackRate  = 0.0f;
    float env1DecayCoeff  = 0.0f;   // one-pole coeff for exponential decay

    // ── WT2 AD envelope ───────────────────────────────────────────────────────
    float env2     = 0.0f;
    int   env2Stage = 0;
    float env2AttackRate  = 0.0f;
    float env2DecayCoeff  = 0.0f;

    // ── Master AMP envelope (Attack / hold / Release) ─────────────────────────
    float ampEnv      = 0.0f;
    int   ampStage    = 0;
    float ampAttackRate = 0.0f;
    float ampRelCoeff   = 0.0f;

    // ── VCF envelope (mirrors WT1 AD) ─────────────────────────────────────────
    float vcfEnv = 0.0f;

    // ── HP trim one-pole state ────────────────────────────────────────────────
    float hpState = 0.0f;

    // ── Drive feedback state ──────────────────────────────────────────────────
    float driveFb = 0.0f;

    // ── LFO ───────────────────────────────────────────────────────────────────
    float lfoPhase = 0.0f;

    // ── Delay ─────────────────────────────────────────────────────────────────
    float* delayBuf    = nullptr;
    int    delayWrite  = 0;
    float  dlyFbSmooth = 0.0f;

    // ── Reverb (Schroeder: 4 comb + 3 allpass) ────────────────────────────────
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3]   = { 347,  113,  37   };

    int   combOff[4] = {};
    int   apOff[3]   = {};
    int   combIdx[8] = {};
    int   apIdx[4]   = {};
    float combFb[8]  = {};

    // ── Filter function pointer ───────────────────────────────────────────────
    typedef float (Synth23::*FilterPtr)(float, float, float);
    FilterPtr applyFilterFn = nullptr;

    // ─────────────────────────────────────────────────────────────────────────
    // Filter implementations
    // ─────────────────────────────────────────────────────────────────────────

    float applySvf12(float in, float c, float r)
    {
        svfFilter.setCutoff(c); svfFilter.setResonance(r);
        return svfFilter.process12(in).lp;
    }
    float applySvf24(float in, float c, float r)
    {
        svfFilter.setCutoff(c); svfFilter.setResonance(r);
        return svfFilter.processLp24(in);
    }
    float applyArray12(float in, float c, float r)
    {
        svfFilter.setCutoff(c); svfFilter.setResonance(r);
        return svfFilter.processArray12(in).lp;
    }
    float applyArray24(float in, float c, float r)
    {
        svfFilter.setCutoff(c); svfFilter.setResonance(r);
        return svfFilter.processArrayLp24(in);
    }
    float applyTbFilter(float in, float c, float r)
    {
        tbFilter.set(c, r);
        return tbFilter.getSample(in);
    }
    float applyMoogFilter(float in, float c, float r)
    {
        moogFilter.setCutoff(c); moogFilter.setResonance(r);
        return moogFilter.process(in);
    }
    float applySvfHp(float in, float c, float r)
    {
        svfFilter.setCutoff(c); svfFilter.setResonance(r);
        return svfFilter.process12(in).hp;
    }
    float applySvfBp(float in, float c, float r)
    {
        svfFilter.setCutoff(c); svfFilter.setResonance(r);
        return svfFilter.process12(in).bp;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Helpers
    // ─────────────────────────────────────────────────────────────────────────

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    float linearRate(float ms) const
    {
        return (ms < 0.01f) ? 1.0f : 1.0f / (sampleRate * ms * 0.001f);
    }

    float wtRead(const Wavetable& wt, float pos)
    {
        float sc = (float)wt.sampleCount;
        pos = pos - std::floor(pos / sc) * sc;
        return linearInterpolationAbsolute(pos, wt.sampleCount, wt.samples());
    }

    // ── AD envelope tick ──────────────────────────────────────────────────────
    // Returns the current envelope level and advances the stage.
    // While gate is open the envelope holds at peak; on note-off it decays.
    float adTick(float& env, int& stage, float attackRate, float decayCoeff, bool gate)
    {
        switch (stage) {
        case 0:
            env = 0.0f;
            break;
        case 1:  // attack
            env += attackRate;
            if (env >= 1.0f) {
                env   = 1.0f;
                stage = gate ? 2 : 3;
            }
            break;
        case 2:  // hold while gate open
            env = 1.0f;
            if (!gate) stage = 3;
            break;
        case 3:  // decay / release
            env *= decayCoeff;
            if (env < 0.0001f) { env = 0.0f; stage = 0; }
            break;
        }
        return env;
    }

    // ── Master AMP envelope tick ──────────────────────────────────────────────
    float ampEnvTick()
    {
        switch (ampStage) {
        case 0: ampEnv = 0.0f; break;
        case 1:
            ampEnv += ampAttackRate;
            if (ampEnv >= 1.0f) { ampEnv = 1.0f; ampStage = gateOpen ? 2 : 3; }
            break;
        case 2:
            ampEnv = 1.0f;
            if (!gateOpen) ampStage = 3;
            break;
        case 3:
            ampEnv *= ampRelCoeff;
            if (ampEnv < 0.0001f) { ampEnv = 0.0f; ampStage = 0; }
            break;
        }
        return ampEnv;
    }

    // ── Reverb ────────────────────────────────────────────────────────────────
    float reverbProcess(float in, float mix, float damp)
    {
        if (mix < 0.001f) return in;
        const float decay = 0.88f;
        float d    = 0.2f + damp * 0.7f;
        float invD = 1.0f - d;
        float wet  = 0.0f;

        for (int c = 0; c < 4; ++c) {
            float* buf = &reverbBuf[combOff[c]];
            int    idx = combIdx[c];
            float  del = buf[idx];
            combFb[c]  = del * invD + combFb[c] * d;
            buf[idx]   = in + combFb[c] * decay;
            if (++idx >= COMB_LEN[c]) idx = 0;
            combIdx[c] = idx;
            wet += del;
        }
        wet *= 0.25f;

        for (int a = 0; a < 3; ++a) {
            float* buf = &reverbBuf[apOff[a]];
            int    idx = apIdx[a];
            float  del = buf[idx];
            float  v   = wet + del * 0.5f;
            buf[idx]   = v;
            wet        = del - v * 0.5f;
            if (++idx >= AP_LEN[a]) idx = 0;
            apIdx[a]   = idx;
        }

        return in + wet * mix;
    }

    // ── Delay ─────────────────────────────────────────────────────────────────
    float delayProcess(float sig)
    {
        if (dlyMix.value < 0.001f) return sig;

        int   delaySmp = std::max(1, std::min(
            (int)(dlyTime.value * 0.001f * sampleRate), DELAY_BUF_SIZE - 1));
        int   readIdx  = (delayWrite - delaySmp + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayed  = delayBuf[readIdx];

        float fbTarget  = dlyFdbk.value * 0.01f * 0.85f;
        dlyFbSmooth    += 0.001f * (fbTarget - dlyFbSmooth);
        delayBuf[delayWrite] = sig + delayed * dlyFbSmooth;
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

        return lerp(sig, sig + delayed * 0.7f, dlyMix.value * 0.01f);
    }

    float bufferedFxProcess(float sig)
    {
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    // ── Wavetables ────────────────────────────────────────────────────────────
    Wavetable wt1, wt2;
    char wt1Name[64]    = "---";
    char wt2Name[64]    = "---";
    char filterType[32] = "Array 12";
    char wt2ModeName[8] = "Add";   // "Add" or "FM"

    // ─────────────────────────────────────────────────────────────────────────
    // Parameters  (44 total, no padding)
    //
    //  PAGE 1 — WAVETABLE 1
    //   0  Frequency      Hz
    //   1  WT1 Select     —       wavetable file index
    //   2  WT1 Morph      wave    frame [1..64]
    //   3  WT1 Level      %       oscillator output level
    //   4  WT1 Attack     ms
    //   5  WT1 Decay      ms      also controls VCF env decay + amp release
    //   6  LFO→WT1 Morph  waves
    //   7  Glide          ms
    //
    //  PAGE 2 — WAVETABLE 2
    //   8  WT2 Mode       —       1=Additive  2=FM
    //   9  WT2 Select     —       wavetable file index
    //  10  WT2 Morph      wave    frame [1..64]
    //  11  WT2 Level      %       level in Additive; FM depth in FM mode
    //  12  WT2 Attack     ms
    //  13  WT2 Decay      ms
    //  14  LFO→WT2 Morph  waves
    //  15  WT2 Coarse     st      semitone detune of WT2 (-24..+24)
    //  16  WT2 Fine       cents   fine detune of WT2 (-100..+100)
    //  17  Feedback       %       WT1 self-feedback as phase offset
    //
    //  PAGE 3 — FILTER
    //  18  Filter Type    —       selector
    //  19  Cutoff         %
    //  20  Resonance      %
    //  21  Env Mod        %       signed VCF env depth (-100..+100)
    //  22  HP Trim        %       always-on post-filter HP stage
    //
    //  PAGE 4 — AMP + SUB + LFO
    //  23  AMP Attack     ms      master amp envelope attack
    //  24  AMP Release    ms      master amp envelope release
    //  25  Sub Mix        %
    //  26  Sub Wave       %       0=sine … 100=square
    //  27  LFO Rate       Hz
    //  28  LFO→Pitch      st
    //  29  LFO→Cutoff     %
    //
    //  PAGE 5 — FX
    //  30  Drive          %       negative=feedback drive, positive=soft clip
    //  31  Waveshape      %
    //  32  Reverb Mix     %
    //  33  Rvb Damp       %
    //  34  Dly Mix        %
    //  35  Dly Time       ms
    //  36  Dly Fdbk       %
    // ─────────────────────────────────────────────────────────────────────────

    Param params[37] = {
        // PAGE 1 — WAVETABLE 1
        { .label = "Frequency",    .unit = "Hz",      .value = 440.0f,  .min = 20.0f,   .max = 2000.0f, .step = 0.5f  }, // 0
        { .label = "WT1 Select",   .string = wt1Name, .value = 0.0f,    .min = 0.0f,    .max = 0.0f,    .step = 1.0f,
          .onUpdate = [](void* ctx, float val) {
              auto* s = (Synth23*)ctx;
              int i = (int)val;
              s->wt1.open(i, false);
              strncpy(s->wt1Name, s->wt1.fileBrowser.getFileWithoutExtension(i).c_str(), sizeof(s->wt1Name)-1);
          }
        },                                                                                                                 // 1
        { .label = "WT1 Morph",    .unit = "wave",    .value = 1.0f,    .min = 1.0f,    .max = 64.0f,   .step = 1.0f  }, // 2
        { .label = "WT1 Level",    .unit = "%",       .value = 100.0f                                                  }, // 3
        { .label = "WT1 Attack",   .unit = "ms",      .value = 10.0f,   .min = 1.0f,    .max = 2000.0f, .step = 1.0f  }, // 4
        { .label = "WT1 Decay",    .unit = "ms",      .value = 500.0f,  .min = 10.0f,   .max = 4000.0f, .step = 5.0f  }, // 5
        { .label = "LFO WT1 Morf", .unit = "waves",   .value = 0.0f,    .min = 0.0f,    .max = 32.0f,   .step = 1.0f  }, // 6
        { .label = "Glide",        .unit = "ms",      .value = 0.0f,    .min = 0.0f,    .max = 1000.0f, .step = 5.0f  }, // 7

        // PAGE 2 — WAVETABLE 2
        { .label = "WT2 Mode",     .string = wt2ModeName, .value = 1.0f, .min = 1.0f,  .max = 2.0f,    .step = 1.0f,
          .onUpdate = [](void* ctx, float val) {
              auto* s = (Synth23*)ctx;
              strcpy(s->wt2ModeName, ((int)val == 2) ? "FM" : "Add");
          }
        },                                                                                                                 // 8
        { .label = "WT2 Select",   .string = wt2Name, .value = 0.0f,    .min = 0.0f,    .max = 0.0f,    .step = 1.0f,
          .onUpdate = [](void* ctx, float val) {
              auto* s = (Synth23*)ctx;
              int i = (int)val;
              s->wt2.open(i, false);
              strncpy(s->wt2Name, s->wt2.fileBrowser.getFileWithoutExtension(i).c_str(), sizeof(s->wt2Name)-1);
          }
        },                                                                                                                 // 9
        { .label = "WT2 Morph",    .unit = "wave",    .value = 1.0f,    .min = 1.0f,    .max = 64.0f,   .step = 1.0f  }, // 10
        { .label = "WT2 Level",    .unit = "%",       .value = 100.0f                                                  }, // 11
        { .label = "WT2 Attack",   .unit = "ms",      .value = 10.0f,   .min = 1.0f,    .max = 2000.0f, .step = 1.0f  }, // 12
        { .label = "WT2 Decay",    .unit = "ms",      .value = 500.0f,  .min = 10.0f,   .max = 4000.0f, .step = 5.0f  }, // 13
        { .label = "LFO WT2 Morf", .unit = "waves",   .value = 0.0f,    .min = 0.0f,    .max = 32.0f,   .step = 1.0f  }, // 14
        { .label = "WT2 Coarse",   .unit = "st",      .value = 0.0f,    .min = -24.0f,  .max = 24.0f,   .step = 1.0f  }, // 15
        { .label = "WT2 Fine",     .unit = "ct",      .value = 0.0f,    .min = -100.0f, .max = 100.0f,  .step = 1.0f  }, // 16
        { .label = "Feedback",     .unit = "%",       .value = 0.0f                                                    }, // 17

        // PAGE 3 — FILTER
        { .label = "Filter Type",  .string = filterType, .value = 1.0f, .min = 1.0f,   .max = 11.0f,   .step = 1.0f,
          .onUpdate = [](void* ctx, float val) {
              auto* s = (Synth23*)ctx;
              switch ((int)val) {
              case  2: s->applyFilterFn = &Synth23::applyArray24;   strcpy(s->filterType, "Array 24"); break;
              case  3: s->applyFilterFn = &Synth23::applySvf12;     strcpy(s->filterType, "SVF 12");   break;
              case  4: s->applyFilterFn = &Synth23::applySvf24;     strcpy(s->filterType, "SVF 24");   break;
              case  5: s->applyFilterFn = &Synth23::applyTbFilter;
                       s->tbFilter.setMode(FilterTB::LP_6);         strcpy(s->filterType, "TB 6");     break;
              case  6: s->applyFilterFn = &Synth23::applyTbFilter;
                       s->tbFilter.setMode(FilterTB::LP_12);        strcpy(s->filterType, "TB 12");    break;
              case  7: s->applyFilterFn = &Synth23::applyTbFilter;
                       s->tbFilter.setMode(FilterTB::LP_18);        strcpy(s->filterType, "TB 18");    break;
              case  8: s->applyFilterFn = &Synth23::applyTbFilter;
                       s->tbFilter.setMode(FilterTB::LP_24);        strcpy(s->filterType, "TB 24");    break;
              case  9: s->applyFilterFn = &Synth23::applySvfHp;     strcpy(s->filterType, "HP SVF");   break;
              case 10: s->applyFilterFn = &Synth23::applySvfBp;     strcpy(s->filterType, "BP SVF");   break;
              case 11: s->applyFilterFn = &Synth23::applyMoogFilter; strcpy(s->filterType, "Moog");    break;
              default: s->applyFilterFn = &Synth23::applyArray12;   strcpy(s->filterType, "Array 12"); break;
              }
          }
        },                                                                                                                 // 18
        { .label = "Cutoff",       .unit = "%",       .value = 80.0f                                                   }, // 19
        { .label = "Resonance",    .unit = "%",       .value = 25.0f                                                   }, // 20
        { .label = "Env Mod",      .unit = "%",       .value = 50.0f,  .min = -100.0f, .max = 100.0f                   }, // 21
        { .label = "HP Trim",      .unit = "%",       .value = 0.0f                                                    }, // 22

        // PAGE 4 — AMP + SUB + LFO
        { .label = "AMP Attack",   .unit = "ms",      .value = 10.0f,  .min = 1.0f,    .max = 2000.0f, .step = 1.0f  }, // 23
        { .label = "AMP Release",  .unit = "ms",      .value = 400.0f, .min = 5.0f,    .max = 4000.0f, .step = 5.0f  }, // 24
        { .label = "Sub Mix",      .unit = "%",       .value = 0.0f                                                   }, // 25
        { .label = "Sub Wave",     .unit = "Sin-Sq",  .value = 0.0f                                                   }, // 26
        { .label = "LFO Rate",     .unit = "Hz",      .value = 2.0f,   .min = 0.05f,   .max = 30.0f,   .step = 0.05f }, // 27
        { .label = "LFO Pitch",    .unit = "st",      .value = 0.0f,   .min = 0.0f,    .max = 12.0f,   .step = 0.1f  }, // 28
        { .label = "LFO Cutoff",   .unit = "%",       .value = 0.0f                                                   }, // 29

        // PAGE 5 — FX
        { .label = "Drive",        .unit = "%",       .value = 0.0f,   .min = -100.0f                                 }, // 30
        { .label = "Waveshape",    .unit = "%",       .value = 50.0f                                                  }, // 31
        { .label = "Reverb Mix",   .unit = "%",       .value = 0.0f                                                   }, // 32
        { .label = "Rvb Damp",     .unit = "%",       .value = 50.0f                                                  }, // 33
        { .label = "Dly Mix",      .unit = "%",       .value = 0.0f                                                   }, // 34
        { .label = "Dly Time",     .unit = "ms",      .value = 125.0f, .min = 10.0f,   .max = 1000.0f, .step = 5.0f  }, // 35
        { .label = "Dly Fdbk",     .unit = "%",       .value = 0.0f                                                   }, // 36
    };

    // ── Named refs ────────────────────────────────────────────────────────────
    Param& freq       = params[0];
    Param& wt1Select  = params[1];
    Param& wt1Morph   = params[2];
    Param& wt1Level   = params[3];
    Param& wt1Attack  = params[4];
    Param& wt1Decay   = params[5];
    Param& lfoToWt1   = params[6];
    Param& glide      = params[7];
    Param& wt2Mode    = params[8];
    Param& wt2Select  = params[9];
    Param& wt2Morph   = params[10];
    Param& wt2Level   = params[11];
    Param& wt2Attack  = params[12];
    Param& wt2Decay   = params[13];
    Param& lfoToWt2   = params[14];
    Param& wt2Coarse  = params[15];
    Param& wt2Fine    = params[16];
    Param& feedback   = params[17];
    Param& filterTypePrm = params[18];
    Param& cutoff     = params[19];
    Param& resonance  = params[20];
    Param& envMod     = params[21];
    Param& hpTrim     = params[22];
    Param& ampAttack  = params[23];
    Param& ampRelease = params[24];
    Param& subMix     = params[25];
    Param& subWave    = params[26];
    Param& lfoRate    = params[27];
    Param& lfoToPitch = params[28];
    Param& lfoToCutoff= params[29];
    Param& drive      = params[30];
    Param& waveshape  = params[31];
    Param& reverbMix  = params[32];
    Param& reverbDamp = params[33];
    Param& dlyMix     = params[34];
    Param& dlyTime    = params[35];
    Param& dlyFdbk    = params[36];

    // ─────────────────────────────────────────────────────────────────────────
    // Constructor
    // ─────────────────────────────────────────────────────────────────────────

    Synth23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "Synth23", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
        , tbFilter(sr)
    {
        if (delayBuf)
            for (int i = 0; i < DELAY_BUF_SIZE; ++i)
                delayBuf[i] = 0.0f;

        if (reverbBuf) {
            int pos = 0;
            for (int c = 0; c < 4; ++c) { combOff[c] = pos; pos += COMB_LEN[c]; }
            for (int a = 0; a < 3; ++a) { apOff[a]   = pos; pos += AP_LEN[a];   }
            for (int i = 0; i < REVERB_BUF_SIZE; ++i) reverbBuf[i] = 0.0f;
        }

        applyFilterFn = &Synth23::applyArray12;

        // Discover available wavetable files
        wt1Select.max = std::max(0.0f, (float)(wt1.fileBrowser.count - 1));
        wt2Select.max = std::max(0.0f, (float)(wt2.fileBrowser.count - 1));

        wt1.open(0, true);
        strncpy(wt1Name, wt1.fileBrowser.getFileWithoutExtension(0).c_str(), sizeof(wt1Name)-1);

        int wt2Default = std::min(1, wt2.fileBrowser.count - 1);
        wt2.open(wt2Default, true);
        strncpy(wt2Name, wt2.fileBrowser.getFileWithoutExtension(wt2Default).c_str(), sizeof(wt2Name)-1);
        wt2Select.value = (float)wt2Default;

        init();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Note on / off
    // ─────────────────────────────────────────────────────────────────────────

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;

        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq = freq.value * std::pow(2.0f, noteOffset / 12.0f);

        if (!gateOpen || glide.value < 0.5f)
            currentFreq = targetFreq;

        gateOpen = true;

        // Reset oscillator phases
        phase1   = 0.0f;
        phase2   = 0.0f;
        fbSample = 0.0f;

        // WT1 envelope
        env1AttackRate = linearRate(wt1Attack.value);
        env1DecayCoeff = tau(wt1Decay.value);
        env1           = 0.0f;
        env1Stage      = 1;

        // WT2 envelope
        env2AttackRate = linearRate(wt2Attack.value);
        env2DecayCoeff = tau(wt2Decay.value);
        env2           = 0.0f;
        env2Stage      = 1;

        // VCF envelope (same decay as WT1)
        vcfEnv = 1.0f;

        // Master amp envelope
        ampAttackRate = linearRate(ampAttack.value);
        ampRelCoeff   = tau(ampRelease.value);
        ampStage      = 1;
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // sampleImpl
    // ─────────────────────────────────────────────────────────────────────────

    float sampleImpl()
    {
        if (ampStage == 0 && env1Stage == 0 && env2Stage == 0 && !gateOpen)
            return bufferedFxProcess(0.0f);

        // ── 1. GLIDE ──────────────────────────────────────────────────────────
        if (glide.value > 0.5f) {
            float c = tau(glide.value);
            currentFreq += (1.0f - c) * (targetFreq - currentFreq);
        } else {
            currentFreq = targetFreq;
        }

        // ── 2. LFO ────────────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        // ── 3. FREQUENCIES ────────────────────────────────────────────────────
        float pitchRatio  = std::pow(2.0f, lfoOut * lfoToPitch.value / 12.0f);
        float carrierFreq = std::max(1.0f, currentFreq * pitchRatio);

        // WT2 frequency: carrier shifted by Coarse semitones + Fine cents
        float wt2Detune = (wt2Coarse.value + wt2Fine.value * 0.01f) / 12.0f;
        float wt2Freq   = carrierFreq * std::pow(2.0f, wt2Detune);

        // ── 4. ENVELOPES ──────────────────────────────────────────────────────
        float e1 = adTick(env1, env1Stage, env1AttackRate, env1DecayCoeff, gateOpen);
        float e2 = adTick(env2, env2Stage, env2AttackRate, env2DecayCoeff, gateOpen);

        // VCF envelope decays with WT1 Decay time
        vcfEnv *= env1DecayCoeff;

        // ── 5. WAVETABLE MORPH INDICES ────────────────────────────────────────
        int morph1Idx = (int)CLAMP(wt1Morph.value - 1.0f + lfoOut * lfoToWt1.value, 0.0f, 63.0f);
        int morph2Idx = (int)CLAMP(wt2Morph.value - 1.0f + lfoOut * lfoToWt2.value, 0.0f, 63.0f);
        wt1.morph(morph1Idx);
        wt2.morph(morph2Idx);

        // ── 6. ADVANCE PHASES ─────────────────────────────────────────────────
        float inc1 = carrierFreq * sampleRateDiv * (float)wt1.sampleCount;
        float inc2 = wt2Freq     * sampleRateDiv * (float)wt2.sampleCount;

        phase1 += inc1;
        if (phase1 >= (float)wt1.sampleCount) phase1 -= (float)wt1.sampleCount;

        phase2 += inc2;
        if (phase2 >= (float)wt2.sampleCount) phase2 -= (float)wt2.sampleCount;

        // ── 7. FEEDBACK OFFSET (applied to WT1 read) ─────────────────────────
        float fbOffset = 0.0f;
        if (feedback.value > 0.001f) {
            float fbCurved = (feedback.value * 0.01f) * (feedback.value * 0.01f);
            fbOffset = fbSample * fbCurved * (float)wt1.sampleCount * 0.5f;
        }

        // ── 8. OSCILLATOR MIX ─────────────────────────────────────────────────
        float sig = 0.0f;
        bool  isFM = ((int)(wt2Mode.value + 0.5f) == 2);

        if (isFM) {
            // FM mode: WT2 (weighted by env2 and Level) modulates WT1 phase.
            // WT2 Level acts as FM depth scale.  Squared curve = perceptually
            // even depth across the range.
            float wt2sig       = wtRead(wt2, phase2);
            float depthCurved  = (wt2Level.value * 0.01f) * (wt2Level.value * 0.01f);
            float fmOffset     = wt2sig * e2 * depthCurved * (float)wt1.sampleCount * 4.0f;
            float s1           = wtRead(wt1, phase1 + fmOffset + fbOffset);
            fbSample           = s1;
            sig                = s1 * e1 * (wt1Level.value * 0.01f);

        } else {
            // Additive mode: sum both oscillators, each scaled by env + level.
            float s1 = wtRead(wt1, phase1 + fbOffset);
            float s2 = wtRead(wt2, phase2);
            fbSample = s1;
            sig      = s1 * e1 * (wt1Level.value * 0.01f)
                     + s2 * e2 * (wt2Level.value * 0.01f);
            // Normalise so two oscillators at full level don't clip
            sig *= 0.5f;
        }

        // ── 9. SUB OSCILLATOR ─────────────────────────────────────────────────
        if (subMix.value > 0.001f) {
            subPhase += (carrierFreq * 0.5f) * sampleRateDiv;
            if (subPhase > 1.0f) subPhase -= 1.0f;
            float subSine = Math::fastSin(PI_X2 * subPhase);
            float subSq   = (subPhase < 0.5f) ? 1.0f : -1.0f;
            float sub     = lerp(subSine, subSq, subWave.value * 0.01f);
            sig = lerp(sig, sub, subMix.value * 0.01f);
        }

        // ── 10. FILTER ────────────────────────────────────────────────────────
        float envSign   = (envMod.value >= 0.0f) ? 1.0f : -1.0f;
        float envAbs    = std::abs(envMod.value);
        float dynCutoff = cutoff.value * 0.01f
                        + envSign * vcfEnv * envAbs * 0.01f
                        + lfoOut  * lfoToCutoff.value * 0.01f * 0.25f;
        dynCutoff = CLAMP(dynCutoff, 0.01f, 0.99f);

        float res = CLAMP(
            0.90f * (1.0f - Math::pow(1.0f - resonance.value * 0.01f, 2.0f)),
            0.0f, 0.98f);

        sig = CLAMP(sig, -1.0f, 1.0f);
        sig = (this->*applyFilterFn)(sig, dynCutoff, res);

        // ── 11. DRIVE + WAVESHAPE ─────────────────────────────────────────────
        if (drive.value < 0.0f)
            sig = applyDriveFeedback(sig, -drive.value * 0.01f, driveFb);
        else
            sig = applyDrive(sig, drive.value * 0.01f);
        sig = applyWaveshape2(sig, waveshape.value * 0.01f);

        // ── 12. HP TRIM ───────────────────────────────────────────────────────
        if (hpTrim.value > 0.001f) {
            sig = CLAMP(sig, -1.0f, 1.0f);
            float hpCoeff = 0.0005f + hpTrim.value * 0.0005f;
            hpState += hpCoeff * (sig - hpState);
            sig = (sig - hpState) * (1.0f + hpTrim.value * 0.015f);
        }

        // ── 13. MASTER AMP ENVELOPE + VELOCITY ───────────────────────────────
        sig *= ampEnvTick() * velocity;

        // ── 14. FX ────────────────────────────────────────────────────────────
        return bufferedFxProcess(sig);
    }
};