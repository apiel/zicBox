#pragma once

#include "audio/Wavetable.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"

#include <algorithm>
#include <cmath>
#include <cstring>

// 1-oscillator wavetable synth with morphing, sub-oscillator, filter, and FX.
//
// Wavetable morphing  : The morph position scans across all waveforms in the
//                       loaded wavetable file (up to 64 frames × 2048 samples).
//                       An LFO can modulate morph position in real time.
//
// Oscillator          : Main wavetable osc + sub (one octave below, sine or square).
//
// Filter              : SVF 12-pole LP with cutoff envelope (AD) and accent path,
//                       matching the Bass23 approach.
//
// Envelopes           : AMP: Attack / hold-while-gate / Release.
//                       VCF: Decay-only (gate resets to 1 on note-on).
//
// Glide               : Portamento in frequency space (same as Bass23).
//
// FX chain            : Drive → Waveshape → HP filter → Delay → Reverb.

class SynthWavetable23 : public EngineBase<SynthWavetable23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    FilterSVF svfFilter;

    // ── Oscillator state ──────────────────────────────────────────────────────
    float phase = 0.0f;
    float subPhase = 0.0f;
    float lfoPhase = 0.0f;
    float sampleIdx = 0.0f; // wavetable read head

    // ── Voice state ───────────────────────────────────────────────────────────
    float currentFreq = 440.0f;
    float targetFreq = 440.0f;
    float velocity = 1.0f;
    bool gateOpen = false;
    bool accented = false;

    // ── Envelopes ─────────────────────────────────────────────────────────────
    float vcfEnv = 0.0f;

    // AMP: linear attack, exponential release
    float ampEnv = 0.0f;
    int ampStage = 0; // 0=off 1=attack 2=hold 3=release
    float ampAttackRate = 0.0f;
    float ampRelRate = 0.0f;

    // ── Accent ────────────────────────────────────────────────────────────────
    float accentVcf = 0.0f;
    float accentVca = 0.0f;
    float accentC = 0.0f;

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

    /** One-pole coefficient for exponential decay of 'ms' milliseconds. */
    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    /** Linear rate: fraction of full-scale per sample to cover 'ms'. */
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
        case 1: // attack
            ampEnv += ampAttackRate;
            if (ampEnv >= 1.0f) {
                ampEnv = 1.0f;
                ampStage = gateOpen ? 2 : 3;
            }
            break;
        case 2: // hold (gate open)
            ampEnv = 1.0f;
            if (!gateOpen) ampStage = 3;
            break;
        case 3: // release
            ampEnv *= ampRelRate;
            if (ampEnv < 0.0001f) {
                ampEnv = 0.0f;
                ampStage = 0;
            }
            break;
        }
        return ampEnv;
    }

    // ── FX ───────────────────────────────────────────────────────────────────
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
    // ── Wavetable ─────────────────────────────────────────────────────────────
    Wavetable wavetable;
    char wtName[64] = "---";

    // ── Parameter block (max 24) ──────────────────────────────────────────────
    Param params[24] = {
        // 0  Base frequency
        { .label = "Frequency", .unit = "Hz", .value = 440.0f, .min = 20.0f, .max = 2000.0f, .step = 0.5f },
        // 1  Wavetable morph position (0–100 → first…last frame)
        { .label = "Morph", .unit = "%", .value = 0.0f },
        // 2  LFO → morph amount
        { .label = "LFO Morph", .unit = "%", .value = 0.0f },
        // 3  LFO → pitch (semitones)
        { .label = "LFO Pitch", .unit = "st", .value = 0.0f, .max = 12.0f, .step = 0.1f },
        // 4  LFO rate
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 30.0f, .step = 0.05f },
        // 5  Sub-oscillator level
        { .label = "Sub Mix", .unit = "%", .value = 0.0f },
        // 6  Sub waveform: 0 = sine, 100 = square
        { .label = "Sub Wave", .unit = "Sin-Sq", .value = 0.0f },
        // 7  Filter cutoff (base)
        { .label = "Cutoff", .unit = "%", .value = 60.0f },
        // 8  Filter resonance
        { .label = "Resonance", .unit = "%", .value = 25.0f },
        // 9  VCF env amount
        { .label = "Env Mod", .unit = "%", .value = 50.0f },
        // 10 VCF decay
        { .label = "VCF Decay", .unit = "ms", .value = 300.0f, .min = 10.0f, .max = 4000.0f, .step = 5.0f },
        // 11 Amp attack
        { .label = "Attack", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 2000.0f, .step = 1.0f },
        // 12 Amp release
        { .label = "Release", .unit = "ms", .value = 400.0f, .min = 5.0f, .max = 4000.0f, .step = 5.0f },
        // 13 Accent amount
        { .label = "Accent", .unit = "%", .value = 60.0f },
        // 14 HP filter
        { .label = "HP", .unit = "%", .value = 5.0f },
        // 15 Portamento
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f, .step = 5.0f },
        // 16 Drive
        { .label = "Drive", .unit = "%", .value = 0.0f, .min = -100.0f },
        // 17 Waveshape
        { .label = "Waveshape", .unit = "%", .value = 0.0f },
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
    Param& morph = params[1];
    Param& lfoToMorph = params[2];
    Param& lfoToPitch = params[3];
    Param& lfoRate = params[4];
    Param& subMix = params[5];
    Param& subWave = params[6];
    Param& cutoff = params[7];
    Param& resonance = params[8];
    Param& envMod = params[9];
    Param& vcfDecay = params[10];
    Param& ampAttack = params[11];
    Param& ampRelease = params[12];
    Param& accentAmt = params[13];
    Param& hpCutoff = params[14];
    Param& glide = params[15];
    Param& drive = params[16];
    Param& waveshape = params[17];
    Param& reverbMix = params[18];
    Param& reverbSize = params[19];
    Param& reverbDamp = params[20];
    Param& dlyMix = params[21];
    Param& dlyTime = params[22];
    Param& dlyFdbk = params[23];

    // ── Constructor ───────────────────────────────────────────────────────────
    SynthWavetable23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "Wavetable23", params)
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

        accentC = tau(60.0f);

        // Copy initial wavetable filename into display string
        strncpy(wtName, wavetable.fileBrowser.getFileWithoutExtension(0).c_str(), sizeof(wtName) - 1);

        init();
    }

    // ── Wavetable file selection ───────────────────────────────────────────────
    /** Load wavetable by browser index (0-based). Safe to call at any time. */
    void loadWavetable(int position)
    {
        wavetable.open(position, false);
        strncpy(wtName, wavetable.fileBrowser.getFileWithoutExtension(position).c_str(), sizeof(wtName) - 1);
    }

    /** Step forward/back through available files. delta = ±1. */
    void browseWavetable(int delta)
    {
        int next = wavetable.fileBrowser.position + delta;
        next = std::max(0, std::min(next, (int)wavetable.fileBrowser.count - 1));
        loadWavetable(next);
    }

    // ── EngineBase overrides ──────────────────────────────────────────────────
    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        accented = (vel > 0.75f);

        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq = freq.value * std::pow(2.0f, noteOffset / 12.0f);

        if (!gateOpen || glide.value < 0.5f)
            currentFreq = targetFreq;

        gateOpen = true;

        // Reset oscillator phases for a clean attack
        sampleIdx = 0.0f;
        subPhase = 0.0f;

        // VCF envelope: instant attack, then decays
        vcfEnv = 1.0f;

        // AMP envelope
        ampAttackRate = linearRate(ampAttack.value);
        ampRelRate = tau(ampRelease.value);
        ampStage = 1;

        if (accented) {
            float a = accentAmt.value * 0.01f;
            accentVcf = a * 1.0f;
            accentVca = a * 0.5f;
        } else {
            accentVcf = 0.0f;
            accentVca = 0.0f;
        }
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

        // ── 2. LFO ────────────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase); // -1 … +1

        // ── 3. WAVETABLE OSCILLATOR ───────────────────────────────────────────
        // Determine morph position [0, 1]: base knob + LFO modulation
        float morphPos = CLAMP(morph.value * 0.01f + lfoOut * lfoToMorph.value * 0.01f * 0.5f, 0.0f, 1.0f);
        wavetable.morph(morphPos);

        // Pitch modulation from LFO (semitones → ratio)
        float pitchRatio = std::pow(2.0f, lfoOut * lfoToPitch.value / 12.0f);
        float oscFreq = std::max(1.0f, currentFreq * pitchRatio);

        // Advance phase; sampleCount = wavetable frame length (typically 2048)
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
        accentVcf *= accentC;
        accentVca *= accentC;

        // ── 6. FILTER (SVF 12 LP) ─────────────────────────────────────────────
        float dynCutoff = CLAMP(
            cutoff.value * 0.01f * 0.85f
                + vcfEnv * envMod.value * 0.01f * 0.85f
                + accentVcf * 0.5f,
            0.01f, 0.99f);

        float res = CLAMP(
            0.90f * (1.0f - Math::pow(1.0f - resonance.value * 0.01f, 2.0f)) + accentVcf * 0.15f,
            0.0f, 0.98f);

        svfFilter.setCutoff(dynCutoff);
        svfFilter.setResonance(res);
        float sig = svfFilter.process12(osc).lp;

        // ── 7. DRIVE / WAVESHAPE ─────────────────────────────────────────────
        if (drive.value < 0.0f)
            sig = applyDriveFeedback(sig, -drive.value * 0.01f, driveFeedback);
        else
            sig = applyDrive(sig, drive.value * 0.01f);
        sig = applyWaveshape2(sig, waveshape.value * 0.01f);

        // ── 8. HP FILTER ─────────────────────────────────────────────────────
        sig = CLAMP(sig, -1.0f, 1.0f);
        float hpCoeff = 0.0005f + hpCutoff.value * 0.0005f;
        hpState += hpCoeff * (sig - hpState);
        sig = (sig - hpState) * (1.0f + hpCutoff.value * 0.015f);

        // ── 9. AMP ENVELOPE ──────────────────────────────────────────────────
        float amp = ampEnvTick();
        sig *= amp * (velocity + accentVca);

        // ── 10. FX ───────────────────────────────────────────────────────────
        return bufferedFxProcess(sig);
    }

protected:
    // Drive feedback state (used by applyDriveFeedback)
    float driveFeedback = 0.0f;
};