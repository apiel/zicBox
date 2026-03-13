#pragma once

#include "audio/effects/applyDrive.h"
#include "audio/effects/applyFilter.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterArray.h"
#include "audio/filterSVF.h"
#include "audio/teeBeeFilter.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <algorithm>
#include <cmath>
#include <cstring>

class SynthBass23 : public EngineBase<SynthBass23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000; // 1 s @ 48 kHz
    static constexpr int REVERB_BUF_SIZE = 16384; // covers all comb+allpass

protected:
    const float sampleRate;
    const float sampleRateDiv;

    EffectFilterArray<2> aFilter;
    FilterSVF svfFilter;
    TeeBeeFilter tbFilter;

    float velocity = 1.0f;
    float phase = 0.0f;
    float subPhase = 0.0f; // sub oscillator (one octave below)
    float currentFreq = 110.0f;
    float targetFreq = 110.0f;
    bool gateOpen = false;
    bool accented = false;

    float vcfEnv = 0.0f;
    float ampEnv = 0.0f;

    float accentVcf = 0.0f; // decaying accent boost for cutoff
    float accentVca = 0.0f; // decaying accent boost for volume
    float vcaSmoothSt = 0.0f; // VCA click-smoother one-pole state
    float driveFeedback = 0.0f;

    float accentC = 0.0f;

    float hpState = 0.0f;

    float lfoPhase = 0.0f;

    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;
    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[8] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[4] = { 347, 113, 37 };

    int combOff[4] = {};
    int apOff[3] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {}; // per-comb LP state (damping)

    double ellipticState[12] = { 0 };

    typedef float (SynthBass23::*FilterPtr)(float, float, float);
    FilterPtr applyFilter = nullptr;

    float applySvf24(float input, float cutoff, float resonance)
    {
        svfFilter.setCutoff(cutoff);
        svfFilter.setResonance(resonance);
        return svfFilter.processArray24(input);
    }

    float applySvf12(float input, float cutoff, float resonance)
    {
        svfFilter.setCutoff(cutoff);
        svfFilter.setResonance(resonance);
        return svfFilter.process12(input);
    }

    float applyTbFilter(float input, float cutoff, float resonance)
    {
        tbFilter.setCutoff(cutoff);
        tbFilter.setResonance(resonance);
        return tbFilter.getSample(input);
    }

    float applyFilterArray(float input, float cutoff, float resonance)
    {
        aFilter.setCutoff(cutoff);
        aFilter.setResonance(resonance);
        aFilter.setSampleData(input, 0);
        return aFilter.lp[0];
    }

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    float tau(float ms) const
    {
        return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f));
    }

    static float noteToFreq(uint8_t note)
    {
        return 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
    }

    float distort(float in, float distAmt, float color, float bias) const
    {
        if (distAmt < 0.001f) return in;

        float gain = 1.0f + distAmt * 14.0f;
        float x = in * gain + bias * 0.3f;

        float folded = Math::fastSin(x * (1.0f + color * 2.5f) * 0.5f * (float)M_PI);
        x = lerp(x, folded, color);

        float clipped = (x >= 0.0f) ? std::tanh(x) : x / (1.0f - x * 0.4f);

        return clipped / (1.0f + distAmt * 3.0f);
    }

    float reverbProcess(float in, float mix, float size, float damp)
    {
        if (mix < 0.001f) return in;

        float decay = 0.7f + size * 0.28f;
        float d = 0.2f + damp * 0.7f;
        float invD = 1.0f - d;
        float wet = 0.0f;

        for (int c = 0; c < 4; ++c) {
            float* bufStart = &reverbBuf[combOff[c]];
            int idx = combIdx[c];

            float delayed = bufStart[idx];
            combFb[c] = delayed * invD + combFb[c] * d;
            bufStart[idx] = in + combFb[c] * decay;
            idx++;
            if (idx >= COMB_LEN[c]) idx = 0;
            combIdx[c] = idx;

            wet += delayed;
        }

        wet *= 0.25f;

        for (int a = 0; a < 3; ++a) {
            float* bufStart = &reverbBuf[apOff[a]];
            int idx = apIdx[a];

            float delayed = bufStart[idx];
            float v = wet + delayed * 0.5f;
            bufStart[idx] = v;
            wet = delayed - v * 0.5f;

            idx++;
            if (idx >= AP_LEN[a]) idx = 0;
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

        sig = lerp(sig, sig + delayed * 0.7f, dlyMix.value * 0.01f);
        return sig;
    }

    float bufferedFxProcess(float sig)
    {
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbSize.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    char filterType[24] = "Off";

    Param params[24] = {
        { .label = "Tuning", .unit = "semi", .value = 0.0f, .min = -24.0f, .max = 24.0f, .step = 1.0f },
        { .label = "Waveform", .unit = "Sq-Saw", .value = 0.0f },
        { .label = "Pulse Width", .unit = "%", .value = 50.0f, .min = 5.0f, .max = 95.0f },
        { .label = "Sub Mix", .unit = "%", .value = 30.0f },
        { .label = "Cutoff", .unit = "%", .value = 50.0f },
        { .label = "Resonance", .unit = "%", .value = 30.0f },
        { .label = "Env Mod", .unit = "%", .value = 50.0f },
        { .label = "Decay", .unit = "ms", .value = 200.0f, .min = 10.0f, .max = 2000.0f, .step = 5.0f },
        { .label = "Accent", .unit = "%", .value = 60.0f },
        { .label = "HP", .unit = "%", .value = 20.0f },
        { .label = "LFO PW", .unit = "%", .value = 0.0f },
        { .label = "LFO Pitch", .unit = "%", .value = 0.0f },
        { .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 30.0f, .step = 0.05f },
        { .label = "Glide", .unit = "ms", .value = 0.0f, .max = 1000.0f, .step = 5.0f },
        { .label = "Drive", .unit = "%", .value = 20.0f, .min = -100.0f },
        { .label = "Waveshape", .unit = "%", .value = 50.0f },
        { .label = "Reverb Mix", .unit = "%", .value = 0.0f },
        { .label = "Rvb Size", .unit = "%", .value = 50.0f },
        { .label = "Rvb Damp", .unit = "%", .value = 50.0f },
        { .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f },
        { .label = "Dly Fdbk", .unit = "%", .value = 0.0f },
        { .label = "Dly Mix", .unit = "%", .value = 0.0f },
        { .label = "Sub Wave", .unit = "Sin-Sq", .value = 0.0f },
        { .label = "Filter type", .string = filterType, .value = 1.0f, .min = 0, .max = 7, .onUpdate = [](void* ctx, float val) {
             auto synthBass = (SynthBass23*)ctx;
             switch ((int)val) {
             case 0:
                 synthBass->applyFilter = &SynthBass23::applyFilterArray;
                 strcpy(synthBass->filterType, "Array 24");
                 break;
             case 2:
                 synthBass->applyFilter = &SynthBass23::applySvf12;
                 strcpy(synthBass->filterType, "SVF 12");
                 break;
             case 3:
                 synthBass->applyFilter = &SynthBass23::applyTbFilter;
                 synthBass->tbFilter.setMode(TeeBeeFilter::LP_6);
                 strcpy(synthBass->filterType, "LP 6");
                 break;
             case 4:
                 synthBass->applyFilter = &SynthBass23::applyTbFilter;
                 synthBass->tbFilter.setMode(TeeBeeFilter::LP_12);
                 strcpy(synthBass->filterType, "LP 12");
                 break;
             case 5:
                 synthBass->applyFilter = &SynthBass23::applyTbFilter;
                 synthBass->tbFilter.setMode(TeeBeeFilter::LP_18);
                 strcpy(synthBass->filterType, "LP 18");
                 break;
             case 6:
                 synthBass->applyFilter = &SynthBass23::applyTbFilter;
                 synthBass->tbFilter.setMode(TeeBeeFilter::LP_24);
                 strcpy(synthBass->filterType, "LP 24");
                 break;
             case 7:
                 synthBass->applyFilter = &SynthBass23::applyTbFilter;
                 synthBass->tbFilter.setMode(TeeBeeFilter::FLAT);
                 strcpy(synthBass->filterType, "Flat");
                 break;
             default: // array filter
                 synthBass->applyFilter = &SynthBass23::applySvf24;
                 strcpy(synthBass->filterType, "SVF 24");
                 break;
             }
         } },
    };

    Param& tuning = params[0];
    Param& waveform = params[1];
    Param& pw = params[2];
    Param& subMix = params[3];
    Param& cutoff = params[4];
    Param& resonance = params[5];
    Param& envMod = params[6];
    Param& decayTime = params[7];
    Param& accentAmt = params[8];
    Param& hpCutoff = params[9];
    Param& lfoToPW = params[10];
    Param& lfoToPitch = params[11];
    Param& lfoRate = params[12];
    Param& glide = params[13];
    Param& drive = params[14];
    Param& waveshape = params[15];
    Param& reverbMix = params[16];
    Param& reverbSize = params[17];
    Param& reverbDamp = params[18];
    Param& dlyTime = params[19];
    Param& dlyFdbk = params[20];
    Param& dlyMix = params[21];
    Param& subWave = params[22];
    Param& type = params[23];

    SynthBass23(float sr, float* dlBuf, float* rvBuf)
        : EngineBase(Synth, "Bass23", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        if (delayBuf) {
            for (int i = 0; i < DELAY_BUF_SIZE; ++i)
                delayBuf[i] = 0.0f;
        }

        if (reverbBuf) {
            int pos = 0;
            for (int c = 0; c < 8; ++c) {
                combOff[c] = pos;
                pos += COMB_LEN[c];
            }
            for (int a = 0; a < 4; ++a) {
                apOff[a] = pos;
                pos += AP_LEN[a];
            }
            for (int i = 0; i < REVERB_BUF_SIZE; ++i) {
                reverbBuf[i] = 0.0f;
            }
        }

        applyFilter = &applySvf24;

        accentC = tau(60.0f);

        init();
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        accented = (vel > 0.75f);
        // Let's remove 1 octave because it's bass :p
        targetFreq = noteToFreq(note - 12) * std::pow(2.0f, tuning.value / 12.0f);

        // Keep current frequency for glide only if gate was already open
        if (!gateOpen || glide.value < 0.5f)
            currentFreq = targetFreq;

        gateOpen = true;
        vcfEnv = 1.0f;
        ampEnv = 1.0f;

        if (accented) {
            float a = accentAmt.value * 0.01f;
            accentVcf = a * 0.8f;
            accentVca = a * 0.35f;
        }
    }

    void noteOffImpl(uint8_t)
    {
        gateOpen = false;
    }

    float sampleImpl()
    {
        if (ampEnv < 0.0001f && !gateOpen) return bufferedFxProcess(0.0f);

        // ── 1. GLIDE ────────────────────────────────────────────────────────
        if (glide.value > 0.5f) {
            float c = tau(glide.value);
            currentFreq += (1.0f - c) * (targetFreq - currentFreq);
        } else {
            currentFreq = targetFreq;
        }

        // ── 2. LFO (sine) ───────────────────────────────────────────────────
        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        // ── 3. OSCILLATOR ───────────────────────────────────────────────────
        float modFreq = currentFreq + (lfoOut * (lfoToPitch.value * 0.01f));
        phase += modFreq * sampleRateDiv;
        if (phase > 1.0f) phase -= 1.0f;

        float sawWave = 2.0f * phase - 1.0f;

        float pwMod = pw.value * 0.01f + lfoOut * (lfoToPW.value * 0.01f) * 0.45f;
        pwMod = std::max(0.02f, std::min(pwMod, 0.98f));
        float sqWave = (phase < pwMod) ? 1.0f : -1.0f;

        // 0=Square → 100=Saw
        float osc = lerp(sqWave, sawWave, waveform.value * 0.01f);

        // Sub oscillator: sine→square blend, one octave below, mixed in before filter
        subPhase += (currentFreq * 0.5f) * sampleRateDiv;
        if (subPhase > 1.0f) subPhase -= 1.0f;
        float subSine = Math::fastSin(PI_X2 * subPhase);
        float subSq = (subPhase < 0.5f) ? 1.0f : -1.0f;
        float sub = lerp(subSine, subSq, subWave.value * 0.01f);
        osc = lerp(osc, sub, subMix.value * 0.01f);

        // ── 5. FILTER ENVELOPE (AD) ─────────────────────────────────────────
        // vcfEnv *= tau(decayTime.value);

        accentVcf *= accentC;
        accentVca *= accentC;

        // ── 6. Filter ────────────────────────────────────────────────
        float dynamicCutoff = 0.85f * cutoff.value * 0.01f * (vcfEnv * envMod.value * 0.01f) + 0.1f;
        float res = 0.90f * ((1.0f - Math::pow(1.0f - resonance.value * 0.01f, 2.0f)) + accentVcf * 0.15f);
        float sig = (this->*applyFilter)(osc, dynamicCutoff, res);

        // ── 7. Distortion ──────────────────────────────────────────────────────
        if (drive.value < 0.0f) sig = applyDriveFeedback(sig, -drive.value * 0.01f, driveFeedback);
        else sig = applyDrive(sig, drive.value * 0.01f);
        sig = applyWaveshape2(sig, waveshape.value * 0.01f);

        // ── 9. HP FILTER ────────────────────────────────────────────────────
        float hpCoeff = 0.0005f + hpCutoff.value * 0.0005f;
        hpState += hpCoeff * (sig - hpState);
        sig = (sig - hpState) * (1.0f + hpCutoff.value * 0.015f);

        // ── 10. AMP ENVELOPE ────────────────────────────────────────────────
        if (gateOpen) {
            ampEnv = 1.0f;
        } else {
            float ampDecMs = decayTime.value * (accented ? 0.6f : 1.2f);
            ampEnv *= tau(ampDecMs);
        }

        sig *= ampEnv * (velocity + accentVca);

        // ── 11. buffered FX ─────────────────────────────────────────────────
        sig = bufferedFxProcess(sig);

        return sig;
    }
};