#pragma once

#include "audio/MultiFx.h"
#include "audio/Wavetable.h"
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
// ─────────────────────────────────────────────────────────────────────────────

class Synth23 : public EngineBase<Synth23> {

public:
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;

    FilterSVF svfFilter;
    FilterTB tbFilter;
    FilterMoog moogFilter;

    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float subPhase = 0.0f;
    float fbSample = 0.0f;

    float currentFreq = 440.0f;
    float targetFreq = 440.0f;
    float velocity = 1.0f;
    bool gateOpen = false;

    // ── WT1 ADSR envelope ─────────────────────────────────────────────────────
    // Stage: 0=off, 1=attack, 2=decay, 3=sustain, 4=release
    float env1 = 0.0f;
    int env1Stage = 0;
    float env1AttackRate = 0.0f;
    float env1DecayRate = 0.0f;
    float env1ReleaseRate = 0.0f;

    // ── WT2 ADSR envelope ─────────────────────────────────────────────────────
    float env2 = 0.0f;
    int env2Stage = 0;
    float env2AttackRate = 0.0f;
    float env2DecayRate = 0.0f;
    float env2ReleaseRate = 0.0f;

    float vcfEnv = 0.0f;
    float hpState = 0.0f;
    float driveFb = 0.0f;
    float lfoPhase = 0.0f;

    float* delayBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    float* reverbBuf = nullptr;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };

    int combOff[4] = {};
    int apOff[3] = {};
    int combIdx[8] = {};
    int apIdx[4] = {};
    float combFb[8] = {};

    char fxName[24] = "Off";

    typedef float (Synth23::*FilterPtr)(float, float, float);
    FilterPtr applyFilterFn = nullptr;

    float applySvf12(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.process12(in).lp;
    }
    float applySvf24(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.processLp24(in);
    }
    float applyArray12(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.processArray12(in).lp;
    }
    float applyArray24(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.processArrayLp24(in);
    }
    float applyTbFilter(float in, float c, float r)
    {
        tbFilter.set(c, r);
        return tbFilter.getSample(in);
    }
    float applyMoogFilter(float in, float c, float r)
    {
        moogFilter.setCutoff(c);
        moogFilter.setResonance(r);
        return moogFilter.process(in);
    }
    float applySvfHp(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.process12(in).hp;
    }
    float applySvfBp(float in, float c, float r)
    {
        svfFilter.setCutoff(c);
        svfFilter.setResonance(r);
        return svfFilter.process12(in).bp;
    }

    static float lerp(float a, float b, float t) { return a + t * (b - a); }
    float tau(float ms) const { return (ms < 0.01f) ? 0.0f : Math::exp(-1.0f / (sampleRate * ms * 0.001f)); }
    float linearRate(float ms) const { return (ms < 0.01f) ? 1.0f : 1.0f / (sampleRate * ms * 0.001f); }

    float wtRead(Wavetable& wt, float pos)
    {
        float sc = (float)wt.sampleCount;
        pos = pos - std::floor(pos / sc) * sc;
        return linearInterpolationAbsolute(pos, wt.sampleCount, wt.samples());
    }

    // ── ADSR envelope tick ────────────────────────────────────────────────────
    float adsrTick(float& env, int& stage, float attackRate, float decayRate, float sustainLevel, float releaseRate, bool gate)
    {
        if (!gate && stage != 0 && stage != 4) stage = 4; // Force release if gate closed

        switch (stage) {
        case 0: // Off
            env = 0.0f;
            break;
        case 1: // Attack
            env += attackRate;
            if (env >= 1.0f) {
                env = 1.0f;
                stage = 2;
            }
            break;
        case 2: // Decay
            env -= decayRate;
            if (env <= sustainLevel) {
                env = sustainLevel;
                stage = 3;
            }
            break;
        case 3: // Sustain
            env = sustainLevel;
            break;
        case 4: // Release
            env -= releaseRate;
            if (env < 0.0001f) {
                env = 0.0f;
                stage = 0;
            }
            break;
        }
        return env;
    }

    float reverbProcess(float in, float mix, float damp)
    {
        if (mix < 0.001f) return in;
        const float decay = 0.88f;
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
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);
        sig = delayProcess(sig);
        sig = reverbProcess(sig, reverbMix.value * 0.01f, reverbDamp.value * 0.01f);
        return sig;
    }

public:
    Wavetable wt1, wt2;
    char wt1Name[64] = "---";
    char wt2Name[64] = "---";
    char filterType[32] = "Array 12";
    char wt2ModeName[8] = "Add";

    enum ParamTarget { NONE,
        PG_WT1,
        PG_WT2,
        PG_FILTER,
        PG_MOD,
        PG_FX };

    Param params[35];

    Param& gain = addParam({ .label = "Gain", .unit = "%", .value = 50.0f });

    Param& wt1Select = addParam({ .label = "Osc1", .string = wt1Name, .value = 0.0f, .min = 0.0f, .max = 0.0f, .step = 1.0f, .target = PG_WT1, .module = MODULE_OSC_WAVETABLE, .onUpdate = [](void* ctx, float val) {
                                     auto* s = (Synth23*)ctx;
                                     int i = (int)val;
                                     s->wt1.open(i, false);
                                     strncpy(s->wt1Name, s->wt1.fileBrowser.getFileWithoutExtension(i).c_str(), sizeof(s->wt1Name) - 1); }, .graph = [](void* ctx, float val) {
                                     auto* s = (Synth23*)ctx;
                                     return *s->wt1.sample(&val); } });
    Param& wt1Morph = addParam({ .label = "Osc1 Morph", .value = 1.0f, .min = 1.0f, .max = 64.0f, .step = 1.0f, .target = PG_WT1, .module = MODULE_OSC_WAVETABLE, .onUpdate = [](void* ctx, float val) {
                                    auto* s = (Synth23*)ctx;
                                    s->wt1.morph((int)val);
                                } });
    Param& wt1Attack = addParam({ .label = "Osc1 Attack", .unit = "ms", .value = 250.0f, .min = 5.0f, .max = 2000.0f, .step = 5.0f, .target = PG_WT1, .module = MODULE_ENV_ADSR });
    Param& wt1Decay = addParam({ .label = "Osc1 Decay", .unit = "ms", .value = 100.0f, .min = 10.0f, .max = 4000.0f, .step = 5.0f, .target = PG_WT1, .module = MODULE_ENV_ADSR });
    Param& wt1Sustain = addParam({ .label = "Osc1 Sustain", .unit = "%", .value = 70.0f, .target = PG_WT1, .module = MODULE_ENV_ADSR });
    Param& wt1Release = addParam({ .label = "Osc1 Release", .unit = "ms", .value = 1500.0f, .min = 10.0f, .max = 4000.0f, .step = 5.0f, .target = PG_WT1, .module = MODULE_ENV_ADSR });
    Param& freq = addParam({ .label = "Frequency", .unit = "Hz", .value = 440.0f, .min = 20.0f, .max = 2000.0f, .target = PG_WT1 });

    Param& wt2Mode = addParam({ .label = "Osc2 Mode", .string = wt2ModeName, .value = 1.0f, .min = 1.0f, .max = 2.0f, .step = 1.0f, .target = PG_WT2, .onUpdate = [](void* ctx, float val) {
                                   auto* s = (Synth23*)ctx;
                                   strcpy(s->wt2ModeName, ((int)val == 2) ? "FM" : "Add");
                               } });
    Param& wt2Level = addParam({ .label = "Osc2 Level", .unit = "%", .value = 100.0f, .target = PG_WT2 });
    Param& wt2Ratio = addParam({ .label = "Osc2 Detune", .unit = "st", .value = 0.0f, .min = -24.0f, .max = 24.0f, .step = 0.01f, .target = PG_WT2 });
    Param& wt2Select = addParam({ .label = "Osc2", .string = wt2Name, .value = 0.0f, .min = 0.0f, .max = 0.0f, .step = 1.0f, .target = PG_WT2, .module = MODULE_OSC_WAVETABLE, .onUpdate = [](void* ctx, float val) {
                                     auto* s = (Synth23*)ctx;
                                     int i = (int)val;
                                     s->wt2.open(i, false);
                                     strncpy(s->wt2Name, s->wt2.fileBrowser.getFileWithoutExtension(i).c_str(), sizeof(s->wt2Name) - 1); }, .graph = [](void* ctx, float val) {
                                     auto* s = (Synth23*)ctx;
                                     return *s->wt2.sample(&val); } });
    Param& wt2Morph = addParam({ .label = "Osc2 Morph", .value = 1.0f, .min = 1.0f, .max = 64.0f, .step = 1.0f, .target = PG_WT2, .module = MODULE_OSC_WAVETABLE, .onUpdate = [](void* ctx, float val) {
                                    auto* s = (Synth23*)ctx;
                                    s->wt2.morph((int)val);
                                } });
    Param& wt2Attack = addParam({ .label = "Osc2 Attack", .unit = "ms", .value = 100.0f, .min = 5.0f, .max = 2000.0f, .step = 5.0f, .target = PG_WT2, .module = MODULE_ENV_ADSR });
    Param& wt2Decay = addParam({ .label = "Osc2 Decay", .unit = "ms", .value = 50.0f, .min = 10.0f, .max = 4000.0f, .step = 5.0f, .target = PG_WT2, .module = MODULE_ENV_ADSR });
    Param& wt2Sustain = addParam({ .label = "Osc2 Sustain", .unit = "%", .value = 70.0f, .target = PG_WT2, .module = MODULE_ENV_ADSR });
    Param& wt2Release = addParam({ .label = "Osc2 Release", .unit = "ms", .value = 100.0f, .min = 10.0f, .max = 4000.0f, .step = 5.0f, .target = PG_WT2, .module = MODULE_ENV_ADSR });
    Param& feedback = addParam({ .label = "Feedback", .unit = "%", .value = 0.0f, .target = PG_WT2 });

    Param& filterTypePrm = addParam({ .label = "Filter Type", .string = filterType, .value = 1.0f, .min = 1.0f, .max = 11.0f, .step = 1.0f, .target = PG_FILTER, .onUpdate = [](void* ctx, float val) {
                                         auto* s = (Synth23*)ctx;
                                         switch ((int)val) {
                                         case 2:
                                             s->applyFilterFn = &Synth23::applyArray24;
                                             strcpy(s->filterType, "Array 24");
                                             break;
                                         case 3:
                                             s->applyFilterFn = &Synth23::applySvf12;
                                             strcpy(s->filterType, "SVF 12");
                                             break;
                                         case 4:
                                             s->applyFilterFn = &Synth23::applySvf24;
                                             strcpy(s->filterType, "SVF 24");
                                             break;
                                         case 5:
                                             s->applyFilterFn = &Synth23::applyTbFilter;
                                             s->tbFilter.setMode(FilterTB::LP_6);
                                             strcpy(s->filterType, "TB 6");
                                             break;
                                         case 6:
                                             s->applyFilterFn = &Synth23::applyTbFilter;
                                             s->tbFilter.setMode(FilterTB::LP_12);
                                             strcpy(s->filterType, "TB 12");
                                             break;
                                         case 7:
                                             s->applyFilterFn = &Synth23::applyTbFilter;
                                             s->tbFilter.setMode(FilterTB::LP_18);
                                             strcpy(s->filterType, "TB 18");
                                             break;
                                         case 8:
                                             s->applyFilterFn = &Synth23::applyTbFilter;
                                             s->tbFilter.setMode(FilterTB::LP_24);
                                             strcpy(s->filterType, "TB 24");
                                             break;
                                         case 9:
                                             s->applyFilterFn = &Synth23::applySvfHp;
                                             strcpy(s->filterType, "HP SVF");
                                             break;
                                         case 10:
                                             s->applyFilterFn = &Synth23::applySvfBp;
                                             strcpy(s->filterType, "BP SVF");
                                             break;
                                         case 11:
                                             s->applyFilterFn = &Synth23::applyMoogFilter;
                                             strcpy(s->filterType, "Moog");
                                             break;
                                         default:
                                             s->applyFilterFn = &Synth23::applyArray12;
                                             strcpy(s->filterType, "Array 12");
                                             break;
                                         }
                                     } });
    Param& cutoff = addParam({ .label = "Cutoff", .unit = "%", .value = 80.0f, .target = PG_FILTER });
    Param& resonance = addParam({ .label = "Resonance", .unit = "%", .value = 25.0f, .target = PG_FILTER });
    Param& envMod = addParam({ .label = "Env1 Mod", .unit = "%", .value = 50.0f, .min = -100.0f, .max = 100.0f, .target = PG_FILTER });
    Param& hpTrim = addParam({ .label = "HP Trim", .unit = "%", .value = 0.0f, .target = PG_FILTER });

    Param& lfoRate = addParam({ .label = "LFO Rate", .unit = "Hz", .value = 2.0f, .min = 0.05f, .max = 30.0f, .step = 0.05f, .target = PG_MOD });
    Param& lfoToPitch = addParam({ .label = "LFO Pitch", .unit = "st", .value = 0.0f, .min = 0.0f, .max = 12.0f, .step = 0.1f, .target = PG_MOD });
    Param& lfoToCutoff = addParam({ .label = "LFO Cutoff", .unit = "%", .value = 0.0f, .target = PG_MOD });
    Param& lfoToWt2 = addParam({ .label = "LFO Osc2 Morph", .value = 0.0f, .min = 0.0f, .max = 32.0f, .step = 1.0f, .target = PG_WT2 });
    // TODO add LFO osc2 detune
    // TODO add LFO osc2 level
    // TODO add LFO type
    // TODO LFO can go super fast to do fm like things
    // NOTE should there be a second LFO ?? this would allow us to have a fast pitch modulation and slow filter modulation...
    Param& glide = addParam({ .label = "Glide", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 1000.0f, .step = 5.0f, .target = PG_MOD });

    Param& fxType = addParam({ .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, .onUpdate = [](void* ctx, float v) { 
             auto e = (Synth23*)ctx; e->multiFx.setEffect(v); strcpy(e->fxName, e->multiFx.getEffectName()); } });
    Param& fxAmt = addParam({ .label = "FX Amount", .unit = "%", .value = 0.0f });
    Param& reverbMix = addParam({ .label = "Reverb Mix", .unit = "%", .value = 0.0f, .target = PG_FX });
    Param& reverbDamp = addParam({ .label = "Rvb Damp", .unit = "%", .value = 50.0f, .target = PG_FX });
    Param& dlyMix = addParam({ .label = "Dly Mix", .unit = "%", .value = 0.0f, .target = PG_FX });
    Param& dlyTime = addParam({ .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f, .step = 5.0f, .target = PG_FX });
    Param& dlyFdbk = addParam({ .label = "Dly Fdbk", .unit = "%", .value = 0.0f, .target = PG_FX });

    Synth23(float sr, float* dlBuf, float* rvBuf, float* fxBuf)
        : EngineBase(Synth, "Synth23", params)
        , multiFx(sr, fxBuf)
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
        applyFilterFn = &Synth23::applyArray12;
        wt1Select.max = std::max(0.0f, (float)(wt1.fileBrowser.count - 1));
        wt2Select.max = std::max(0.0f, (float)(wt2.fileBrowser.count - 1));
        wt1.open(0, true);
        strncpy(wt1Name, wt1.fileBrowser.getFileWithoutExtension(0).c_str(), sizeof(wt1Name) - 1);
        int wt2Default = std::min(1, wt2.fileBrowser.count - 1);
        wt2.open(wt2Default, true);
        strncpy(wt2Name, wt2.fileBrowser.getFileWithoutExtension(wt2Default).c_str(), sizeof(wt2Name) - 1);
        wt2Select.value = (float)wt2Default;
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        float noteOffset = static_cast<float>(note) - 60.0f;
        targetFreq = freq.value * std::pow(2.0f, noteOffset / 12.0f);
        if (!gateOpen || glide.value < 0.5f) currentFreq = targetFreq;
        gateOpen = true;
        phase1 = 0.0f;
        phase2 = 0.0f;
        fbSample = 0.0f;

        env1AttackRate = linearRate(wt1Attack.value);
        env1DecayRate = linearRate(wt1Decay.value);
        env1ReleaseRate = linearRate(wt1Release.value);
        env1 = 0.0f;
        env1Stage = 1;

        env2AttackRate = linearRate(wt2Attack.value);
        env2DecayRate = linearRate(wt2Decay.value);
        env2ReleaseRate = linearRate(wt2Release.value);
        env2 = 0.0f;
        env2Stage = 1;

        vcfEnv = 1.0f;
    }

    void noteOffImpl(uint8_t) { gateOpen = false; }

    float sampleImpl()
    {
        if (env1Stage == 0 && env2Stage == 0 && !gateOpen)
            return bufferedFxProcess(0.0f);

        if (glide.value > 0.5f) {
            float c = tau(glide.value);
            currentFreq += (1.0f - c) * (targetFreq - currentFreq);
        } else {
            currentFreq = targetFreq;
        }

        lfoPhase += lfoRate.value * sampleRateDiv;
        if (lfoPhase > 1.0f) lfoPhase -= 1.0f;
        float lfoOut = Math::fastSin(PI_X2 * lfoPhase);

        float pitchRatio = std::pow(2.0f, lfoOut * lfoToPitch.value / 12.0f);
        float carrierFreq = std::max(1.0f, currentFreq * pitchRatio);
        float wt2Detune = wt2Ratio.value / 12.0f;
        float wt2Freq = carrierFreq * std::pow(2.0f, wt2Detune);

        // ── ENVELOPES ─────────────────────────────────────────────────────────
        float e1 = adsrTick(env1, env1Stage, env1AttackRate, env1DecayRate, wt1Sustain.value * 0.01f, env1ReleaseRate, gateOpen);
        float e2 = adsrTick(env2, env2Stage, env2AttackRate, env2DecayRate, wt2Sustain.value * 0.01f, env2ReleaseRate, gateOpen);

        // Use the active stage of WT1 to determine if VCF env should decay or release
        if (gateOpen && env1Stage <= 3) vcfEnv = e1;
        else vcfEnv *= env1ReleaseRate;

        int morph2Idx = (int)CLAMP(wt2Morph.value - 1.0f + lfoOut * lfoToWt2.value, 0.0f, 63.0f);
        wt2.morph(morph2Idx);

        float inc1 = carrierFreq * sampleRateDiv * (float)wt1.sampleCount;
        float inc2 = wt2Freq * sampleRateDiv * (float)wt2.sampleCount;
        phase1 += inc1;
        if (phase1 >= (float)wt1.sampleCount) phase1 -= (float)wt1.sampleCount;
        phase2 += inc2;
        if (phase2 >= (float)wt2.sampleCount) phase2 -= (float)wt2.sampleCount;

        float fbOffset = 0.0f;
        if (feedback.value > 0.001f) {
            float fbCurved = (feedback.value * 0.01f) * (feedback.value * 0.01f);
            fbOffset = fbSample * fbCurved * (float)wt1.sampleCount * 0.5f;
        }

        float sig = 0.0f;
        bool isFM = ((int)(wt2Mode.value + 0.5f) == 2);
        if (isFM) {
            float wt2sig = wtRead(wt2, phase2);
            float depthCurved = (wt2Level.value * 0.01f) * (wt2Level.value * 0.01f);
            float fmOffset = wt2sig * e2 * depthCurved * (float)wt1.sampleCount * 4.0f;
            float s1 = wtRead(wt1, phase1 + fmOffset + fbOffset);
            fbSample = s1;
            sig = s1 * e1;
        } else {
            float s1 = wtRead(wt1, phase1 + fbOffset);
            float s2 = wtRead(wt2, phase2);
            fbSample = s1;
            sig = s1 * e1 + s2 * e2 * (wt2Level.value * 0.01f);
            sig *= 0.5f;
        }

        float envSign = (envMod.value >= 0.0f) ? 1.0f : -1.0f;
        float envAbs = std::abs(envMod.value);
        float dynCutoff = cutoff.value * 0.01f + envSign * vcfEnv * envAbs * 0.01f + lfoOut * lfoToCutoff.value * 0.01f * 0.25f;
        dynCutoff = CLAMP(dynCutoff, 0.01f, 0.99f);
        float res = CLAMP(0.90f * (1.0f - Math::pow(1.0f - resonance.value * 0.01f, 2.0f)), 0.0f, 0.98f);
        sig = CLAMP(sig, -1.0f, 1.0f);
        sig = (this->*applyFilterFn)(sig, dynCutoff, res);

        if (hpTrim.value > 0.001f) {
            sig = CLAMP(sig, -1.0f, 1.0f);
            float hpRate = 0.0005f + hpTrim.value * 0.0005f;
            hpState += hpRate * (sig - hpState);
            sig = (sig - hpState) * (1.0f + hpTrim.value * 0.015f);
        }

        sig *= velocity;
        sig *= 1.0f + gain.value * 0.01f;
        return bufferedFxProcess(sig);
    }
};