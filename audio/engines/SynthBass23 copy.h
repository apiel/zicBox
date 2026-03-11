#pragma once

#include "audio/MultiFx.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <algorithm>
#include <cmath>
#include <cstring>

class SynthBass23 : public EngineBase<SynthBass23> {
public:
    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    // Voice state
    float velocity = 1.0f;
    float phase = 0.0f;
    float subPhase = 0.0f;
    float currentFreq = 110.0f; // current (glided) frequency
    float targetFreq = 110.0f; // target frequency after noteOn
    bool gateOpen = false;

    // Envelopes
    float ampEnv = 0.0f; // 0..1, decays on noteOff
    float vcfEnv = 0.0f; // 0..1, fast attack, tunable decay
    float accentEnv = 0.0f; // extra brightness burst on accented notes
    bool accented = false;

    // Filter state — 4-pole Moog-style ladder (naive 1-pole cascade)
    float flt[4] = { 0.0f }; // 4 LP stages
    float fltFb = 0.0f; // resonance feedback tap

    // LFO
    float lfoPhase = 0.0f;

    // Delay line  (simple single-tap)
    static constexpr int DELAY_MAX = 48000; // up to 1 s @ 48 kHz
    float* delayBuf = nullptr;
    int delayWrite = 0;
    float delayFeedback = 0.0f; // smoothed

    // Reverb / drive state
    float* reverbBuffer = nullptr;
    int reverbIndex = 0;
    float bassBoostPrevIn = 0.0f;
    float bassBoostPrevOut = 0.0f;
    float compressionState = 0.0f;
    float driveFeedback = 0.0f;

    char fxName[24] = "Off";

    // ── helpers ──────────────────────────────────────────────────────────────
    float lerp(float a, float b, float t) { return a + t * (b - a); }

    // One-pole coefficient: time constant in samples
    float tau(float ms) { return Math::exp(-1.0f / (sampleRate * ms * 0.001f)); }

    // Moog ladder — naive 4-pole cascade with tanh saturation and feedback
    float ladderFilter(float in, float cutoff, float resonance)
    {
        // Clamp cutoff to Nyquist
        float f = std::min(cutoff * sampleRateDiv, 0.45f);
        float k = f * (1.9f - 0.9f * f); // bilinear warp
        float r = resonance * 3.99f; // self-osc at r ≈ 4

        float x = in - r * fltFb;
        // tanh soft-clip at each stage
        x = std::tanh(x * 0.5f) * 2.0f;

        for (int i = 0; i < 4; ++i) {
            flt[i] = flt[i] + k * (std::tanh(x * 0.5f) * 2.0f - std::tanh(flt[i] * 0.5f) * 2.0f);
            x = flt[i];
        }
        fltFb = flt[3];
        return flt[3];
    }

    float noteToFreq(uint8_t note)
    {
        return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
    }

public:
    // ── Params ───────────────────────────────────────────────────────────────
    // PAGE 1 — OSCILLATOR & FILTER (classic 303 controls)
    // PAGE 2 — LFO, FX, GLIDE

    Param params[24] = {
        // 0  Waveform: 0 = saw, 100 = square (morph)
        { .label = "Waveform", .unit = "Saw-Sq", .value = 0.0f },
        // 1  Sub oscillator blend (one octave below)
        { .label = "Sub Mix", .unit = "%", .value = 0.0f },
        // 2  Cutoff frequency
        { .label = "Cutoff", .unit = "Hz", .value = 800.0f, .min = 50.0f, .max = 12000.0f, .step = 50.0f },
        // 3  Resonance (0–100, self-oscillates near 100)
        { .label = "Resonance", .unit = "%", .value = 50.0f },
        // 4  Filter envelope amount
        { .label = "Env Mod", .unit = "%", .value = 50.0f, .min = -100.0f },
        // 5  Filter envelope decay
        { .label = "Env Decay", .unit = "ms", .value = 200.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f },
        // 6  Filter envelope attack (usually very fast on 303 but tunable)
        { .label = "Env Att", .unit = "ms", .value = 5.0f, .min = 1.0f, .max = 200.0f },
        // 7  Amp decay (note length feel)
        { .label = "Amp Decay", .unit = "ms", .value = 400.0f, .min = 20.0f, .max = 4000.0f, .step = 10.0f },
        // 8  Accent sensitivity
        { .label = "Accent", .unit = "%", .value = 60.0f },
        // 9  Glide (portamento) time
        { .label = "Glide", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 1000.0f, .step = 5.0f },
        // 10 Drive / distortion
        { .label = "Drive", .unit = "%", .value = 30.0f, .min = -100.0f },
        // 11 Bass boost
        { .label = "Bass Boost", .unit = "%", .value = 20.0f },

        // 12 LFO Rate
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 20.0f, .step = 0.05f },
        // 13 LFO Depth to filter cutoff
        { .label = "LFO Flt", .unit = "%", .value = 0.0f },
        // 14 LFO Depth to pitch
        { .label = "LFO Pitch", .unit = "%", .value = 0.0f },
        // 15 LFO Waveform: 0=sine 50=tri 100=square
        { .label = "LFO Shape", .unit = "Sin-Sq", .value = 0.0f },

        // 16 Delay time
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f },
        // 17 Delay feedback
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f },
        // 18 Delay mix
        { .label = "Dly Mix", .unit = "%", .value = 0.0f },
        // 19 Reverb mix
        { .label = "Reverb", .unit = "%", .value = 0.0f },

        // 20 Compression
        { .label = "Compress", .unit = "%", .value = 0.0f },
        // 21 Tone / low-pass post filter
        { .label = "Tone", .unit = "%", .value = 100.0f },
        // 22 MultiFx type
        { .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, .onUpdate = [](void* ctx, float v) {
             auto e = (SynthBass23*)ctx;
             e->multiFx.setEffect(v);
             strcpy(e->fxName, e->multiFx.getEffectName());
         } },
        // 23 MultiFx amount
        { .label = "FX Amt", .unit = "%", .value = 0.0f },
    };

    // ── Named references ─────────────────────────────────────────────────────
    Param& waveform = params[0];
    Param& subMix = params[1];
    Param& cutoff = params[2];
    Param& resonance = params[3];
    Param& envMod = params[4];
    Param& envDecay = params[5];
    Param& envAtt = params[6];
    Param& ampDecay = params[7];
    Param& accent = params[8];
    Param& glide = params[9];
    Param& drive = params[10];
    Param& bassBoost = params[11];
    Param& lfoRate = params[12];
    Param& lfoToFlt = params[13];
    Param& lfToPitch = params[14];
    Param& lfoShape = params[15];
    Param& dlyTime = params[16];
    Param& dlyFdbk = params[17];
    Param& dlyMix = params[18];
    Param& reverbAmt = params[19];
    Param& compress = params[20];
    Param& tone = params[21];
    Param& fxType = params[22];
    Param& fxAmt = params[23];

    // ── Constructor ──────────────────────────────────────────────────────────
    SynthBass23(const float sampleRate, float* rvBuffer, float* dlBuffer)
        : EngineBase(Synth, "Bass23", params)
        , multiFx(sampleRate, nullptr) // MultiFx doesn't need separate buffer here
        , sampleRate(sampleRate)
        , sampleRateDiv(1.0f / sampleRate)
        , reverbBuffer(rvBuffer)
        , delayBuf(dlBuffer)
    {
        if (delayBuf) {
            for (int i = 0; i < DELAY_MAX; ++i)
                delayBuf[i] = 0.0f;
        }
        init();
    }

    // ── noteOn ───────────────────────────────────────────────────────────────
    void noteOnImpl(uint8_t note, float _velocity)
    {
        targetFreq = noteToFreq(note);
        accented = (_velocity > 0.75f); // accent = hard velocity

        // If glide is active and we were already playing, keep current freq
        if (glide.value < 1.0f || !gateOpen) {
            currentFreq = targetFreq;
        }

        velocity = _velocity;
        gateOpen = true;

        // Trigger amp env
        ampEnv = 1.0f;

        // Trigger filter env (always re-trigger on note on)
        vcfEnv = 1.0f;

        // Accent burst
        if (accented) {
            accentEnv = accent.value * 0.01f;
        }
    }

    // ── noteOff ──────────────────────────────────────────────────────────────
    void noteOffImpl(uint8_t /*note*/)
    {
        gateOpen = false;
        // Amp will decay naturally via ampDecay param
        // VCF env also starts decaying (it always decays, note off just closes gate)
    }

    // ── sample ───────────────────────────────────────────────────────────────
    float sampleImpl()
    {
        if (ampEnv < 0.0001f && !gateOpen) return 0.0f;

        // ── 1. GLIDE ────────────────────────────────────────────────────────
        float glideCoeff = (glide.value > 0.5f)
            ? tau(glide.value)
            : 0.0f; // instant at 0
        if (glideCoeff > 0.0f)
            currentFreq = currentFreq + (1.0f - glideCoeff) * (targetFreq - currentFreq);
        else
            currentFreq = targetFreq;

        // ── 2. LFO ──────────────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;

        float lfoBlend = lfoShape.value * 0.01f;
        float lfoSin = Math::fastSin(PI_X2 * lfoPhase);
        float lfoTri = 2.0f * std::abs(2.0f * (lfoPhase - std::floor(lfoPhase + 0.5f))) - 1.0f;
        float lfoSq = (lfoPhase > 0.5f) ? 1.0f : -1.0f;
        float lfoOut;
        if (lfoBlend < 0.5f) lfoOut = lerp(lfoSin, lfoTri, lfoBlend * 2.0f);
        else lfoOut = lerp(lfoTri, lfoSq, (lfoBlend - 0.5f) * 2.0f);

        // ── 3. PITCH (with LFO) ─────────────────────────────────────────────
        float pitchMod = 1.0f + lfoOut * (lfToPitch.value * 0.01f) * 0.05f; // ±5 semitones max
        float freq = currentFreq * pitchMod;

        // ── 4. OSCILLATOR ───────────────────────────────────────────────────
        phase += freq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        // Saw wave (naive, fine for 303 character — ladder will tame it)
        float sawWave = 2.0f * phase - 1.0f;

        // Square wave
        float sqWave = (phase > 0.5f) ? 1.0f : -1.0f;

        // Morph saw → square
        float wMorph = waveform.value * 0.01f;
        float osc = lerp(sawWave, sqWave, wMorph);

        // Sub oscillator (octave down, always sine)
        subPhase += (freq * 0.5f) * sampleRateDiv;
        if (subPhase > 1.0f) subPhase -= 1.0f;
        float sub = Math::fastSin(PI_X2 * subPhase);
        osc = osc * (1.0f - subMix.value * 0.01f) + sub * (subMix.value * 0.01f);

        // ── 5. FILTER ENVELOPE ──────────────────────────────────────────────
        float attCoeff = tau(envAtt.value);
        float decCoeff = tau(envDecay.value);
        // While gate open: attack; when closed: decay only
        float envTarget = gateOpen ? 1.0f : 0.0f;
        if (vcfEnv < envTarget)
            vcfEnv = vcfEnv + (1.0f - attCoeff) * (envTarget - vcfEnv);
        else
            vcfEnv = vcfEnv * decCoeff; // always decaying toward 0

        // Accent adds extra envelope push
        float accentCoeff = tau(30.0f);
        accentEnv = accentEnv * accentCoeff;
        float envLevel = vcfEnv + accentEnv;

        // ── 6. CUTOFF MODULATION ────────────────────────────────────────────
        float envModHz = envMod.value * 0.01f * 6000.0f; // up to ±6 kHz sweep
        float lfoModHz = lfoOut * (lfoToFlt.value * 0.01f) * 3000.0f;
        float dynCutoff = cutoff.value + envModHz * envLevel + lfoModHz;
        dynCutoff = std::max(30.0f, std::min(dynCutoff, sampleRate * 0.45f));

        // ── 7. LADDER FILTER ────────────────────────────────────────────────
        float res = resonance.value * 0.01f;
        float sig = ladderFilter(osc, dynCutoff, res);

        // ── 8. AMP ENVELOPE ─────────────────────────────────────────────────
        float ampDecCoeff = tau(ampDecay.value);
        if (gateOpen)
            ampEnv = 1.0f; // hold while gate open
        else
            ampEnv = ampEnv * ampDecCoeff; // decay on noteOff

        sig *= ampEnv * velocity;

        // ── 9. DRIVE ────────────────────────────────────────────────────────
        if (drive.value > 0.0f)
            sig = applyDrive(sig, -drive.value * 0.01f);
        else if (drive.value < 0.0f)
            sig = applyDriveFeedback(sig, drive.value * 0.01f, driveFeedback);

        // Bass boost
        if (bassBoost.value > 0.0f)
            sig = applyBoost(sig, bassBoost.value * 0.01f, bassBoostPrevIn, bassBoostPrevOut);

        // ── 10. TONE (post LP) ───────────────────────────────────────────────
        // Simple one-pole LP as a brightness knob
        static float toneState = 0.0f;
        float tCoeff = tone.value * 0.005f; // 0→very dark, 100→almost full-open
        toneState += tCoeff * (sig - toneState);
        sig = lerp(toneState, sig, tone.value * 0.01f);

        // ── 11. COMPRESSION ─────────────────────────────────────────────────
        if (compress.value > 0.0f)
            sig = applyCompression2(sig, compress.value * 0.01f, compressionState);

        // ── 12. DELAY ───────────────────────────────────────────────────────
        if (delayBuf && (dlyMix.value > 0.0f || dlyFdbk.value > 0.0f)) {
            int delaySamples = std::max(1, std::min((int)(dlyTime.value * 0.001f * sampleRate), DELAY_MAX - 1));
            int readIdx = (delayWrite - delaySamples + DELAY_MAX) % DELAY_MAX;
            float delayed = delayBuf[readIdx];

            // smooth feedback to avoid zipper on param change
            float fbTarget = dlyFdbk.value * 0.01f * 0.85f; // cap at 85% to avoid runaway
            delayFeedback += 0.001f * (fbTarget - delayFeedback);

            delayBuf[delayWrite] = sig + delayed * delayFeedback;
            delayWrite = (delayWrite + 1) % DELAY_MAX;

            sig = lerp(sig, sig + delayed, dlyMix.value * 0.01f);
        }

        // ── 13. REVERB ──────────────────────────────────────────────────────
        sig = applyReverb(sig, reverbAmt.value * 0.01f, reverbBuffer, reverbIndex);

        // ── 14. MULTIFX ─────────────────────────────────────────────────────
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);

        return sig;
    }
};