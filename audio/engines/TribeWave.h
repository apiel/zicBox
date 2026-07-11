#pragma once

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/MultiFx.h"
#include "audio/Wavetable.h"
#include "audio/engines/EngineBase.h"
#ifdef USE_SVF
#include "audio/filterSVF.h"
#else
#include "audio/filter.h"
#endif
#include "audio/utils/applySampleGain.h"
#include "audio/utils/linearInterpolation.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>
#include <cstring>

class TribeWave : public EngineBase<TribeWave> {
public:
    static constexpr int DELAY_BUF_SIZE = 48000;
    static constexpr int REVERB_BUF_SIZE = 16384;

    MultiFx multiFx;

    enum ModSource {
        SRC_EG_POS,
        SRC_LFO_TRI,
        SRC_LFO_SAW_UP,
        SRC_LFO_SAW_DOWN,
        SRC_LFO_SQU_UP,
        SRC_LFO_SQU_DOWN,
        SRC_LFO_SH,
        SRC_LFO_RAND,
        SRC_EG_EXP_1SHOT
    };

    enum ModDest {
        DST_FILTER,
        DST_PITCH,
        DST_MORPH,
        DST_LEVEL,
        DST_IFX_EDIT,
        DST_NONE
    };

    struct ModRouting {
        const char* name;
        ModSource source;
        ModDest dest;
        bool keySync;
    };

    static constexpr int TOTAL_MOD_TYPES = 33;
    inline static const ModRouting modMatrix[TOTAL_MOD_TYPES] = {
        { "EG Filter", SRC_EG_POS, DST_FILTER, true },
        { "EG Pitch", SRC_EG_POS, DST_PITCH, true },
        { "EG Level", SRC_EG_POS, DST_LEVEL, true },
        { "EG IFX", SRC_EG_POS, DST_IFX_EDIT, true },

        { "Exp1 Filter", SRC_EG_EXP_1SHOT, DST_FILTER, true },
        { "Exp1 Pitch", SRC_EG_EXP_1SHOT, DST_PITCH, true },
        { "Exp1 Level", SRC_EG_EXP_1SHOT, DST_LEVEL, true },
        { "Exp1 IFX", SRC_EG_EXP_1SHOT, DST_IFX_EDIT, true },

        { "LFO Tri Flt", SRC_LFO_TRI, DST_FILTER, false },
        { "LFO Tri Pit", SRC_LFO_TRI, DST_PITCH, false },
        { "LFO Tri Morph", SRC_LFO_TRI, DST_MORPH, false },
        { "LFO Tri Lvl", SRC_LFO_TRI, DST_LEVEL, false },
        { "LFO Tri IFX", SRC_LFO_TRI, DST_IFX_EDIT, false },

        { "Tri Sync Flt", SRC_LFO_TRI, DST_FILTER, true },
        { "Tri Sync Pit", SRC_LFO_TRI, DST_PITCH, true },
        { "Tri Sync Morph", SRC_LFO_TRI, DST_MORPH, true },
        { "Tri Sync Lvl", SRC_LFO_TRI, DST_LEVEL, true },
        { "Tri Sync IFX", SRC_LFO_TRI, DST_IFX_EDIT, true },

        { "SawUp Flt", SRC_LFO_SAW_UP, DST_FILTER, true },
        { "SawUp Pit", SRC_LFO_SAW_UP, DST_PITCH, true },
        { "SawUp Morph", SRC_LFO_SAW_UP, DST_MORPH, true },
        { "SawUp Lvl", SRC_LFO_SAW_UP, DST_LEVEL, true },
        { "SawUp IFX", SRC_LFO_SAW_UP, DST_IFX_EDIT, true },

        { "SawDwn Flt", SRC_LFO_SAW_DOWN, DST_FILTER, true },
        { "SawDwn Pit", SRC_LFO_SAW_DOWN, DST_PITCH, true },
        { "SawDwn Morph", SRC_LFO_SAW_DOWN, DST_MORPH, true },
        { "SawDwn Lvl", SRC_LFO_SAW_DOWN, DST_LEVEL, true },
        { "SawDwn IFX", SRC_LFO_SAW_DOWN, DST_IFX_EDIT, true },

        { "S&H Filter", SRC_LFO_SH, DST_FILTER, true },
        { "S&H Pitch", SRC_LFO_SH, DST_PITCH, true },
        { "S&H Morph", SRC_LFO_SH, DST_MORPH, true },
        { "S&H Level", SRC_LFO_SH, DST_LEVEL, true },
        { "S&H IFX", SRC_LFO_SH, DST_IFX_EDIT, true },
    };

protected:
    const float sampleRate;
    const float sampleRateDiv;

#ifdef USE_SVF
    FilterSVF svfFilter;
#else
    EffectFilterData filterData;
#endif

    float* delayBuf = nullptr;
    float* reverbBuf = nullptr;
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    static constexpr int COMB_LEN[4] = { 1559, 1617, 1685, 1751 };
    static constexpr int AP_LEN[3] = { 347, 113, 37 };
    int combOff[4], apOff[3], combIdx[4], apIdx[3];
    float combFb[4] = { 0 };

    float phase = 0.0f;
    float currentFreq = 440.0f;
    float targetFreq = 440.0f;
    float velocity = 1.0f;
    bool gateOpen = false;

    // --- AR Envelope variables ---
    float env = 0.0f;
    int envStage = 0; // 0=off, 1=attack, 2=sustain/on, 3=release
    float envAttackRate = 0.0f;
    float envReleaseRate = 0.0f;

    // --- Modulation Generator Structural Variables ---
    double modLfoPhase = 0.0;
    float modLfoLastValue = 0.0f;
    float modLfoTargetValue = 0.0f;
    uint32_t modLfoRandHoldCounter = 0;

    double modEgPhase = 0.0;
    double modEgExpValue = 1.0;

    uint8_t midiNote = 0;

    static float lerp(float a, float b, float t) { return a + t * (b - a); }
    float linearRate(float ms) const { return (ms < 0.01f) ? 1.0f : 1.0f / (sampleRate * ms * 0.001f); }

    float wtRead(Wavetable& wt, float pos)
    {
        float sc = (float)wt.sampleCount;
        pos = pos - std::floor(pos / sc) * sc;
        return linearInterpolationAbsolute(pos, wt.sampleCount, wt.samples());
    }

    float envTick(bool gate)
    {
        if (!gate && envStage != 0 && envStage != 3) {
            envStage = 3;
            envReleaseRate = linearRate(envRelease.value);
        }

        switch (envStage) {
        case 0:
            env = 0.0f;
            break;
        case 1: // Attack
            env += envAttackRate;
            if (env >= 1.0f) {
                env = 1.0f;
                envStage = 2; // Sustain
            }
            break;
        case 2: // Sustain
            env = 1.0f;
            break;
        case 3: // Release
            env -= envReleaseRate;
            if (env <= 0.0f) {
                env = 0.0f;
                envStage = 0;
            }
            break;
        }
        return env;
    }

public:
    Wavetable wt;
    char wtName[64] = "---";
    char modTypeNameDisplay[16] = "EG Filter";
    char fxName[24] = "Off";

    Param params[18];

    // --- Params ---
    Param& wtSelect = addParam({ .key = "wtSelect", .label = "Osc", .string = wtName, .value = 0.0f, .min = 0.0f, .max = 0.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                     auto* s = (TribeWave*)ctx;
                                     int i = (int)val;
                                     s->wt.open(i, false);
                                     strncpy(s->wtName, s->wt.fileBrowser.getFileWithoutExtension(i).c_str(), sizeof(s->wtName) - 1); }, .graph = [](void* ctx, float val) {
                                     auto* s = (TribeWave*)ctx;
                                     return *s->wt.sample(&val); }, .stringToFloatFn = [](void* ctx, const char* valStr) { auto s = (TribeWave*)ctx; return (float)s->wt.find(std::string(valStr) + ".wav"); } });
    Param& wtMorph = addParam({ .key = "wtMorph", .label = "Morph", .value = 1.0f, .min = 1.0f, .max = 64.0f, .step = 1.0f, .onUpdate = [](void* ctx, float val) {
                                   auto* s = (TribeWave*)ctx;
                                   s->wt.morph((int)val);
                               } });

    Param& envAttack = addParam({ .key = "attack", .label = "Attack", .unit = "ms", .value = 5.0f, .min = 0.0f, .max = 2000.0f, .step = 5.0f });
    Param& envRelease = addParam({ .key = "release", .label = "Release", .unit = "ms", .value = 500.0f, .min = 0.0f, .max = 4000.0f, .step = 5.0f });

    Param& glide = addParam({ .key = "glide", .label = "Glide", .unit = "ms", .value = 0.0f, .min = 0.0f, .max = 1000.0f, .step = 5.0f });

    Param& modType = addParam({ .key = "modType", .label = "Mod Type", .string = modTypeNameDisplay, .value = 0.0f, .min = 0.0f, .max = (float)(TOTAL_MOD_TYPES - 1), .step = 1.0f, .onUpdate = [](void* ctx, float val) {
            auto* s = (TribeWave*)ctx;
            int idx = CLAMP((int)val, 0, TOTAL_MOD_TYPES - 1);
            strncpy(s->modTypeNameDisplay, modMatrix[idx].name, 15); }, .setStringFn = [](void* ctx, float value, char* str) { strncpy(str, modMatrix[(int)value].name, 15); }, .stringToFloatFn = [](void* ctx, const char* val) {
            for (int i = 0; i < TOTAL_MOD_TYPES; i++) {
                if (strcmp(val, modMatrix[i].name) == 0) {
                    return (float)i;
                }
            }
            return 0.0f; } });
    Param& modDepth = addParam({ .key = "modDepth", .label = "Mod Depth", .value = 0.0f, .min = -100.0f, .max = 100.0f, .step = 1.0f });
    Param& modSpeed = addParam({ .key = "modSpeed", .label = "Mod Speed", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    // Filter & FX
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f });
    Param& resonance = addParam({ .key = "res", .label = "Res", .unit = "%", .value = 0.0f });
    Param& fxType = addParam({ .key = "fxType", .label = "FX Type", .string = fxName, .value = 0.0f, .max = (float)MultiFx::FX_COUNT - 1, .step = 1.0f, .onUpdate = [](void* ctx, float v) { auto e = (TribeWave*)ctx; e->multiFx.setEffect(v); }, .setStringFn = [](void* ctx, float value, char* str) { auto e = (TribeWave*)ctx; strcpy(str, e->multiFx.getEffectName(value)); }, .stringToFloatFn = [](void* ctx, const char* valStr) { auto e = (TribeWave*)ctx; return (float)e->multiFx.getEffect(valStr); } });
    Param& fxAmt = addParam({ .key = "fxAmt", .label = "FX Amount", .unit = "%", .value = 0.0f });

    Param& reverbMix = addParam({ .key = "rvbMix", .label = "Reverb Mix", .unit = "%", .value = 0.0f });
    Param& reverbDamp = addParam({ .key = "rvbDamp", .label = "Rvb Damp", .unit = "%", .value = 50.0f });
    Param& dlyMix = addParam({ .key = "dlyMix", .label = "Dly Mix", .unit = "%", .value = 0.0f });
    Param& dlyTime = addParam({ .key = "dlyTime", .label = "Dly Time", .unit = "ms", .value = 125.0f, .min = 10.0f, .max = 1000.0f });
    Param& dlyFdbk = addParam({ .key = "dlyFdbk", .label = "Dly Fdbk", .unit = "%", .value = 0.0f });

    Param& gain = addParam({ .key = "gain", .label = "Gain", .unit = "%", .value = 50.0f });

    TribeWave(float sr, float* dlBuf, float* rvBuf, float* fxBuf)
        : EngineBase(Synth, "TribeWave", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
        , multiFx(sr, fxBuf)
        , delayBuf(dlBuf)
        , reverbBuf(rvBuf)
    {
        int pos = 0;
        for (int i = 0; i < 4; ++i) {
            combOff[i] = pos;
            pos += COMB_LEN[i];
            combIdx[i] = 0;
        }
        for (int i = 0; i < 3; ++i) {
            apOff[i] = pos;
            pos += AP_LEN[i];
            apIdx[i] = 0;
        }

        wtSelect.max = std::max(0.0f, (float)(wt.fileBrowser.count - 1));
        wt.open(0, true);
        strncpy(wtName, wt.fileBrowser.getFileWithoutExtension(0).c_str(), sizeof(wtName) - 1);
    }

    void noteOnImpl(uint8_t note, float vel)
    {
        velocity = vel;
        midiNote = note;
        float target = 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f);
        targetFreq = target;
        if (!gateOpen || glide.value < 0.5f) {
            currentFreq = targetFreq;
        }
        gateOpen = true;
        phase = 0.0f;

        envAttackRate = linearRate(envAttack.value);
        envReleaseRate = linearRate(envRelease.value);
        envStage = 1; // Attack

        int routeIdx = CLAMP((int)modType.value, 0, TOTAL_MOD_TYPES - 1);
        ModRouting currentRoute = modMatrix[routeIdx];

        if (currentRoute.keySync) {
            modLfoPhase = 0.0;
            modLfoLastValue = 0.0f;
            modLfoTargetValue = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            modLfoRandHoldCounter = 0;
        }

        modEgPhase = 0.0;
        modEgExpValue = 1.0;
    }

    void noteOffImpl(uint8_t note)
    {
        if (gateOpen && midiNote == note) {
            gateOpen = false;
        }
    }

    float sampleImpl()
    {
        if (envStage == 0 && !gateOpen) {
            return fxChain(0.0f);
        }

        if (glide.value > 0.5f) {
            float factor = 1.0f - std::exp(-1.0f / (sampleRate * glide.value * 0.001f));
            currentFreq += (targetFreq - currentFreq) * factor;
        } else {
            currentFreq = targetFreq;
        }

        float eVal = envTick(gateOpen);

        int routeIdx = CLAMP((int)modType.value, 0, TOTAL_MOD_TYPES - 1);
        ModRouting currentRoute = modMatrix[routeIdx];

        float modSourceValue = 0.0f;

        float lfoHz = 0.05f + (modSpeed.value * 0.01f) * (modSpeed.value * 0.01f) * 39.95f;
        double phaseIncr = (double)lfoHz / (double)sampleRate;
        modLfoPhase += phaseIncr;
        if (modLfoPhase >= 1.0) modLfoPhase -= 1.0;

        float egDurationMs = 5.0f + (100.0f - modSpeed.value) * 0.01f * 2995.0f;
        double egSamplesTotal = (egDurationMs * 0.001f) * sampleRate;
        if (modEgPhase < 1.0) {
            modEgPhase += (1.0 / egSamplesTotal);
            if (modEgPhase > 1.0) modEgPhase = 1.0;
        }

        switch (currentRoute.source) {
        case SRC_EG_POS:
            modSourceValue = eVal;
            break;
        case SRC_LFO_TRI:
            modSourceValue = modLfoPhase < 0.5 ? (float)(4.0 * modLfoPhase - 1.0) : (float)(3.0 - 4.0 * modLfoPhase);
            break;
        case SRC_LFO_SAW_UP:
            modSourceValue = (float)(2.0 * modLfoPhase - 1.0);
            break;
        case SRC_LFO_SAW_DOWN:
            modSourceValue = (float)(1.0 - 2.0 * modLfoPhase);
            break;
        case SRC_LFO_SQU_UP:
            modSourceValue = modLfoPhase < 0.5 ? 1.0f : -1.0f;
            break;
        case SRC_LFO_SQU_DOWN:
            modSourceValue = modLfoPhase < 0.5 ? -1.0f : 1.0f;
            break;
        case SRC_LFO_SH: {
            uint32_t samplesPerHold = std::max((uint32_t)1, (uint32_t)(sampleRate / std::max(0.1f, lfoHz)));
            if (++modLfoRandHoldCounter >= samplesPerHold) {
                modLfoRandHoldCounter = 0;
                modLfoLastValue = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            }
            modSourceValue = modLfoLastValue;
            break;
        }
        case SRC_LFO_RAND: {
            uint32_t samplesPerWalk = std::max((uint32_t)1, (uint32_t)(sampleRate / std::max(0.1f, lfoHz)));
            modLfoRandHoldCounter++;
            if (modLfoRandHoldCounter >= samplesPerWalk) {
                modLfoRandHoldCounter = 0;
                modLfoLastValue = modLfoTargetValue;
                modLfoTargetValue = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            }
            float interpFraction = (float)modLfoRandHoldCounter / (float)samplesPerWalk;
            modSourceValue = modLfoLastValue + interpFraction * (modLfoTargetValue - modLfoLastValue);
            break;
        }
        case SRC_EG_EXP_1SHOT: {
            double expFactor = 0.001 + (modSpeed.value * 0.01f) * 0.05;
            modEgExpValue *= (1.0 - expFactor);
            modSourceValue = static_cast<float>(modEgExpValue);
            break;
        }
        }

        float modulationAmount = modSourceValue * (modDepth.value * 0.01f);

        float finalCutoff = cutoff.value;
        float finalPitchInterval = 0.0f;
        float finalMorph = wtMorph.value;
        float finalLevelModifier = 1.0f;
        float finalIfxEditModifier = 0.0f;

        if (currentRoute.dest == DST_FILTER) {
            finalCutoff = CLAMP(cutoff.value + modulationAmount * 100.0f, -100.0f, 100.0f);
        } else if (currentRoute.dest == DST_PITCH) {
            finalPitchInterval = modulationAmount * 24.0f;
        } else if (currentRoute.dest == DST_MORPH) {
            finalMorph = CLAMP(wtMorph.value + modulationAmount * 63.0f, 1.0f, 64.0f);
        } else if (currentRoute.dest == DST_LEVEL) {
            finalLevelModifier = CLAMP(1.0f + modulationAmount, 0.0f, 2.0f);
        } else if (currentRoute.dest == DST_IFX_EDIT) {
            finalIfxEditModifier = modulationAmount;
        }

        wt.morph((int)finalMorph);

        float baseInterval = finalPitchInterval;
        float pitchRatio = std::pow(2.0f, baseInterval / 12.0f);
        float carrierFreq = std::max(1.0f, currentFreq * pitchRatio);
        float inc = carrierFreq * sampleRateDiv * (float)wt.sampleCount;
        phase += inc;
        if (phase >= (float)wt.sampleCount) phase -= (float)wt.sampleCount;

        float out = wtRead(wt, phase);
        out *= (eVal * velocity * finalLevelModifier * (1.0f + gain.value * 0.01f));

        out = applyMorphFilter(out, finalCutoff, resonance.value * 0.01f);

        return fxChain(out);
    }

    const char* getNameXYImpl() { return "Filter"; }

    void setXYImpl(XY xy)
    {
        cutoff.set(xy.x * 200.0f - 100.0f);
        resonance.set(xy.y * 100.0f);
    }

    XY getXYImpl() { return { (cutoff.value + 100.0f) / 200.0f, resonance.value * 0.01f }; }

    int getVoiceCountImpl()
    {
        return (gateOpen || envStage != 0) ? 1 : 0;
    }

private:
    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam);
        if (absC < 0.5f) return sig;

        float normCutoff = 0.01f + absC * 0.0098f;
        float normRes = CLAMP(res, 0.0f, 0.98f);

        float t = CLAMP((cutoffParam + 100.0f) * 0.005f, 0.0f, 1.0f);
#ifdef USE_SVF
        svfFilter.setCutoff(normCutoff);
        svfFilter.setResonance(normRes);
        auto out = svfFilter.process12(sig);
        float filtered = lerp(out.lp, out.hp, t);
#else
        filterData.set(normCutoff, normRes);
        filterData.setSampleData(sig);
        float filtered = lerp(filterData.lp, filterData.hp, t);
#endif
        return lerp(sig, filtered, absC * 0.01f);
    }

    float fxChain(float sig)
    {
        sig = multiFx.apply(sig, fxAmt.value * 0.01f);
        if (dlyMix.value > 0.001f) {
            int n = (int)(dlyTime.value * 0.001f * sampleRate);
            float del = delayBuf[(delayWrite - n + DELAY_BUF_SIZE) % DELAY_BUF_SIZE];
            dlyFbSmooth += 0.001f * (dlyFdbk.value * 0.0085f - dlyFbSmooth);
            delayBuf[delayWrite] = sig + del * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
            sig = lerp(sig, sig + del, dlyMix.value * 0.01f);
        }
        if (reverbMix.value > 0.001f) {
            float wet = 0, d = 0.2f + (reverbDamp.value * 0.007f);
            for (int i = 0; i < 4; ++i) {
                float val = reverbBuf[combOff[i] + combIdx[i]];
                combFb[i] = val * (1.0f - d) + combFb[i] * d;
                reverbBuf[combOff[i] + combIdx[i]] = sig + combFb[i] * 0.88f;
                if (++combIdx[i] >= COMB_LEN[i]) combIdx[i] = 0;
                wet += val;
            }
            sig = lerp(sig, wet * 0.25f, reverbMix.value * 0.01f);
        }
        return sig;
    }
};
