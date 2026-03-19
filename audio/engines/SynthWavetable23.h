#pragma once

#include "audio/MultiFx.h"
#include "audio/Wavetable.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// 1-oscillator wavetable synth with morphing, sub-oscillator, filter, and FX.
//
// Wavetable file      : Selected via a string param (same pattern as DrumKick23
//                       FX Type). onUpdate reloads the file and updates wtName[].
//
// Wavetable morphing  : morph knob (0–100%) scans across all frames in the
//                       loaded file (up to 64 × 2048 samples). An LFO can
//                       modulate morph position in real time.
//
// Oscillator          : Main wavetable osc + sub (one octave below, sine↔square).
//
// Filter              : SVF 12-pole LP with decay-only VCF envelope.
//
// Envelopes           : AMP: linear Attack / hold-while-gate / exponential Release.
//                       VCF: Decay-only (gate resets to 1 on note-on).
//
// Glide               : Portamento in frequency space.
//
// FX chain            : HP filter → MultiFx → Delay → Reverb.
//
// Constructor buffers : dlBuf  (DELAY_BUF_SIZE floats)
//                       rvBuf  (REVERB_BUF_SIZE floats)
//                       fxBuf  (MultiFx::bufferSize() floats)

class SynthWavetable23 : public EngineBase<SynthWavetable23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    FilterSVF svfFilter;

    // ── Oscillator state ──────────────────────────────────────────────────────
    float phase = 0.0f;
    float subPhase = 0.0f;
    float lfoPhase = 0.0f;
    float sampleIdx = 0.0f;

    // ── Voice state ───────────────────────────────────────────────────────────
    float currentFreq = 440.0f;
    float targetFreq = 440.0f;
    float velocity = 1.0f;
    bool gateOpen = false;

    // ── Envelopes ─────────────────────────────────────────────────────────────
    float vcfEnv = 0.0f;
    float ampEnv = 0.0f;
    int ampStage = 0; // 0=off 1=attack 2=hold 3=release
    float ampAttackRate = 0.0f;
    float ampRelRate = 0.0f;

    // ── HP one-pole ───────────────────────────────────────────────────────────
    float hpState = 0.0f;

    // ── Delay ─────────────────────────────────────────────────────────────────
    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    // ── Reverb (Schroeder: 4 comb + 3 AP) ────────────────────────────────────
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };

    int combOff[4] = {};
    int apOff[3] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {};

    // ── Helpers ───────────────────────────────────────────────────────────────
    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    float linearRate(float ms) const
    {
        if (ms < 0.01f) return 1.0f;
        return 1.0f / (sampleRate * ms * 0.001f);
    }

    // ── AMP envelope tick ─────────────────────────────────────────────────────
    float ampEnvTick()
    {
        switch (ampStage) {
        case 0:
            ampEnv = 0.0f;
            break;
        case 1:
            ampEnv += ampAttackRate;
            if (ampEnv >= 1.0f) {
                ampEnv = 1.0f;
                ampStage = gateOpen ? 2 : 3;
            }
            break;
        case 2:
            ampEnv = 1.0f;
            if (!gateOpen) ampStage = 3;
            break;
        case 3:
            ampEnv *= ampRelRate;
            if (ampEnv < 0.0001f) {
                ampEnv = 0.0f;
                ampStage = 0;
            }
            break;
        }
        return ampEnv;
    }

    // ── Reverb ────────────────────────────────────────────────────────────────
    float reverbProcess(float in, float mix, float size, float damp)
    {
        if (mix < 0.001f) return in;

        float decay = 0.7f + size * 0.28f;
        float d = 0.2f + damp * 0.7f;
        float invD = 1.0f - d;
        float wet = 0.0f;

        for (int c = 0; c < 4; ++c) {
            float* buf = &reverbBuf[combOff[c]];
            int idx = combIdx[c];
            float del = buf[idx];
            combFb[c] = del * invD + combFb[c] * d;
            buf[idx] = in + combFb[c] * decay;
            if (++idx >= COMB_LEN[c]) idx = 0;
            combIdx[c] = idx;
            wet += del;
        }
        wet *= 0.25f;

        for (int a = 0; a < 3; ++a) {
            float* buf = &reverbBuf[apOff[a]];
            int idx = apIdx[a];
            float del = buf[idx];
            float v = wet + del * 0.5f;
            buf[idx] = v;
            wet = del - v * 0.5f;
            if (++idx >= AP_LEN[a]) idx = 0;
            apIdx[a] = idx;
        }

        return in + wet * mix;
    }

    // ── Delay ─────────────────────────────────────────────────────────────────
    float delayProcess(float sig)
    {
        if (dlyMix.value < 0.001f) return sig;

        int delaySmp = std::max(1, std::min((int)(dlyTime.value * 0.001f * sampleRate), DELAY_BUF_SIZE - 1));
        int readIdx = (delayWrite - delaySmp + DELAY_BUF_SIZE) % DELAY_BUF_SIZE;
        float delayed = delayBuf[readIdx];

        float fbTarget = dlyFdbk.value * 0.01f * 0.85f;
        dlyFbSmooth += 0.001f * (fbTarget - dlyFbSmooth);

        delayBuf[delayWrite] = sig + delayed * dlyFbSmooth;
        delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;

        return lerp(sig, sig + delayed * 0.7f, dlyMix.value * 0.01f);
    }

    float bufferedFxProcess(float sig)
    {
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbSize.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    // ── Wavetable + FX display strings ────────────────────────────────────────
    Wavetable wavetable;
    char wtName[64] = "---";
    char fxName[24] = "Off";

    // ── Parameters (24) ───────────────────────────────────────────────────────
    Param params[24] = {
        // 0  Base frequency
        { .label = "Frequency", .unit = "Hz", .value = 440.0f, .min = 20.0f, .max = 2000.0f, .step = 0.5f },
        // 1  Wavetable file selector (string param, same pattern as Kick23 FX Type)
        { .label = "Wavetable", .string = wtName, .value = 0.0f, .min = 0.0f, .max = 0.0f /* set in ctor */, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* self = (SynthWavetable23*)ctx;
             int pos = (int)val;
             self->wavetable.open(pos, false);
             strncpy(self->wtName,
                 self->wavetable.fileBrowser.getFileWithoutExtension(pos).c_str(),
                 sizeof(self->wtName) - 1);
         } },
        // 2  Morph position (0–100% → first…last wavetable frame)
        { .label = "Morph", .unit = "%", .value = 0.0f },
        // 3  LFO → morph depth
        { .label = "LFO Morph", .unit = "%", .value = 0.0f },
        // 4  LFO → pitch (semitones peak deviation)
        { .label = "LFO Pitch", .unit = "st", .value = 0.0f, .max = 12.0f, .step = 0.1f },
        // 5  LFO rate
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 30.0f, .step = 0.05f },
        // 6  Sub-oscillator level
        { .label = "Sub Mix", .unit = "%", .value = 0.0f },
        // 7  Sub waveform blend: 0 = sine, 100 = square
        { .label = "Sub Wave", .unit = "Sin-Sq", .value = 0.0f },
        // 8  Filter cutoff base
        { .label = "Cutoff", .unit = "%", .value = 60.0f },
        // 9  Filter resonance
        { .label = "Resonance", .unit = "%", .value = 25.0f },
        // 10 VCF envelope amount
        { .label = "Env Mod", .unit = "%", .value = 50.0f },
        // 11 VCF decay time
        { .label = "VCF Decay", .unit = "ms", .value = 300.0f, .min = 10.0f, .max = 4000.0f, .step = 5.0f },
        // 12 Amp attack
        { .label = "Attack", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 2000.0f, .step = 1.0f },
        // 13 Amp release
        { .label = "Release", .unit = "ms", .value = 400.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f },
        // 14 HP filter amount
        { .label = "HP", .unit = "%", .value = 5.0f },
        // 15 Portamento time
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f, .step = 5.0f },
        // 16 MultiFx type (string param, same pattern as Kick23 FX Type)
        { .label = "FX Type", .string = fxName, .value = 0.0f, .min = 0.0f, .max = 0.0f /* set in ctor */, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
             auto* self = (SynthWavetable23*)ctx;
             self->multiFx.setEffect(val);
             strcpy(self->fxName, self->multiFx.getEffectName());
         } },
        // 17 MultiFx wet amount
        { .label = "FX Amount", .unit = "%", .value = 0.0f },
        // 18 Reverb mix
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f },
        // 19 Reverb size
        { .label = "Rvb Size", .unit = "%", .value = 50.0f },
        // 20 Reverb damp
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f },
        // 21 Delay mix
        { .label = "Dly Mix", .unit = "%", .value = 0.0f },
        // 22 Delay time
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f },
        // 23 Delay feedback
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f },
    };

    // Convenience references
    Param& freq = params[0];
    Param& wtSelect = params[1];
    Param& morph = params[2];
    Param& lfoToMorph = params[3];
    Param& lfoToPitch = params[4];
    Param& lfoRate = params[5];
    Param& subMix = params[6];
    Param& subWave = params[7];
    Param& cutoff = params[8];
    Param& resonance = params[9];
    Param& envMod = params[10];
    Param& vcfDecay = params[11];
    Param& ampAttack = params[12];
    Param& ampRelease = params[13];
    Param& hpCutoff = params[14];
    Param& glide = params[15];
    Param& fxType = params[16];
    Param& fxAmount = params[17];
    Param& reverbMix = params[18];
    Param& reverbSize = params[19];
    Param& reverbDamp = params[20];
    Param& dlyMix = params[21];
    Param& dlyTime = params[22];
    Param& dlyFdbk = params[23];

    // ── Constructor ───────────────────────────────────────────────────────────
    SynthWavetable23(float sr, float* dlBuf, float* rvBuf, float* fxBuf)
        : EngineBase(Synth, "Wavetable23", params)
        , multiFx(sr, fxBuf)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        if (delayBuf)
            for (int i = 0; i < DELAY_BUF_SIZE; ++i)
                delayBuf[i] = 0.0f;

        if (reverbBuf) {
            int pos = 0;
            for (int c = 0; c < 4; ++c) {
                combOff[c] = pos;
                pos += COMB_LEN[c];
            }
            for (int a = 0; a < 3; ++a) {
                apOff[a] = pos;
                pos += AP_LEN[a];
            }
            for (int i = 0; i < REVERB_BUF_SIZE; ++i)
                reverbBuf[i] = 0.0f;
        }

        // Set runtime param ranges
        wtSelect.max = std::max(0.0f, (float)(wavetable.fileBrowser.count - 1));
        fxType.max = (float)(MultiFx::FX_COUNT - 1);

        // Load first wavetable and sync display string
        wavetable.open(0, true);
        strncpy(wtName,
            wavetable.fileBrowser.getFileWithoutExtension(0).c_str(),
            sizeof(wtName) - 1);

        // Sync fxName with default effect
        multiFx.setEffect(0.0f);
        strcpy(fxName, multiFx.getEffectName());

        init();
    }

    // ── EngineBase overrides ──────────────────────────────────────────────────
    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;

        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq = freq.value * std::pow(2.0f, noteOffset / 12.0f);

        if (!gateOpen || glide.value < 0.5f)
            currentFreq = targetFreq;

        gateOpen = true;
        sampleIdx = 0.0f;
        subPhase = 0.0f;
        vcfEnv = 1.0f;

        ampAttackRate = linearRate(ampAttack.value);
        ampRelRate = tau(ampRelease.value);
        ampStage = 1;
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
    }

    float sampleImpl()
    {
        if (ampStage == 0 && !gateOpen)
            return bufferedFxProcess(0.0f);

        // ── 1. GLIDE ──────────────────────────────────────────────────────────
        if (glide.value > 0.5f) {
            float c = tau(glide.value);
            currentFreq += (1.0f - c) * (targetFreq - currentFreq);
        } else {
            currentFreq = targetFreq;
        }

        // ── 2. LFO (sine) ─────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase); // -1 … +1

        // ── 3. WAVETABLE OSCILLATOR ───────────────────────────────────────────
        float morphPos = CLAMP(morph.value * 0.01f + lfoOut * lfoToMorph.value * 0.005f, 0.0f, 1.0f);
        wavetable.morph(morphPos);

        float pitchRatio = std::pow(2.0f, lfoOut * lfoToPitch.value / 12.0f);
        float oscFreq = std::max(1.0f, currentFreq * pitchRatio);

        float sampleInc = oscFreq * (float)wavetable.sampleCount * sampleRateDiv;
        float osc = wavetable.sample(&sampleIdx, sampleInc);

        // ── 4. SUB-OSCILLATOR (one octave below) ──────────────────────────────
        subPhase += (oscFreq * 0.5f) * sampleRateDiv;
        if (subPhase > 1.0f) subPhase -= 1.0f;
        float subSine = Math::fastSin(PI_X2 * subPhase);
        float subSq = (subPhase < 0.5f) ? 1.0f : -1.0f;
        float sub = lerp(subSine, subSq, subWave.value * 0.01f);
        osc = lerp(osc, sub, subMix.value * 0.01f);

        // ── 5. VCF ENVELOPE (decay only) ─────────────────────────────────────
        vcfEnv *= tau(vcfDecay.value);

        // ── 6. FILTER (SVF 12-pole LP) ────────────────────────────────────────
        float dynCutoff = CLAMP(
            cutoff.value * 0.01f * 0.85f + vcfEnv * envMod.value * 0.01f * 0.85f,
            0.01f, 0.99f);
        float res = CLAMP(
            0.90f * (1.0f - Math::pow(1.0f - resonance.value * 0.01f, 2.0f)),
            0.0f, 0.98f);

        svfFilter.setCutoff(dynCutoff);
        svfFilter.setResonance(res);
        float sig = svfFilter.process12(osc).lp;

        // ── 7. HP FILTER ─────────────────────────────────────────────────────
        sig = CLAMP(sig, -1.0f, 1.0f);
        float hpCoeff = 0.0005f + hpCutoff.value * 0.0005f;
        hpState += hpCoeff * (sig - hpState);
        sig = (sig - hpState) * (1.0f + hpCutoff.value * 0.015f);

        // ── 8. MULTI FX ──────────────────────────────────────────────────────
        sig = multiFx.apply(sig, fxAmount.value * 0.01f);

        // ── 9. AMP ENVELOPE + VELOCITY ───────────────────────────────────────
        sig *= ampEnvTick() * velocity;

        // ── 10. BUFFERED FX (delay + reverb) ─────────────────────────────────
        return bufferedFxProcess(sig);
    }
};