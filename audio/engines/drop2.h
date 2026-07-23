#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>
#include "audio/effects/applyCompression.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Streamlined zicDropV2 Engine:
// Texture replaces VCO 2 as the secondary harmonic/tonal body layer.
// Parts:
// 1. Kick Body (VCO 1 sub sweep)
// 2. Kick Click (Noise transient)
// 3. Kick Rumble (Techno sub tail)
// 4. Texture (Saw->Square->Noise, Modulation, FM, Filter Morph, Delay - replaces VCO2!)
class Drop : public EngineBase<Drop> {
private:
    double sampleRate;
    double sampleRateDiv;

    // Fast noise generator (LCG)
    uint32_t noiseState = 22222;
    float nextNoise() {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    // --- Kick Body & Click States ---
    float kickPhase = 0.0f;
    float kickAmpEnv = 0.0f;
    float kickPitchEnv = 0.0f;
    float kickClickEnv = 0.0f;
    float kickSpeedRatio = 1.0f;
    double kickElapsedSamples = 0.0;
    float rumbleLP = 0.0f;
    float rumbleDelaySample = 0.0f;

    // --- Texture States (Replaces VCO 2) ---
    float synthPhase = 0.0f;
    float synthFmPhase = 0.0f;
    float synthTargetFreq = 110.0f;
    float synthCurrentFreq = 110.0f;
    float synthAmpEnv = 0.0f;
    FilterSVF synthFilter;

    // Mod & Delay
    double synthLfoPhase = 0.0;
    static const int DELAY_BUF_SIZE = 48000;
    float delayBuf[DELAY_BUF_SIZE] = {0.0f};
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    float kickCompressEnv = 0.0f;
    float masterCompressEnv = 0.0f;

    // --- DSP Helpers ---
    inline float wavefold(float sig, float threshold) {
        if (threshold < 0.001f) threshold = 0.001f;
        float x = sig / threshold;
        return 1.5f * std::sin(x) * threshold;
    }

    // High-Impact Gabber / Hardtek / Tribe Distortion Engine (Balanced & Musical)
    inline float gabberWavefold(float sig, float amount) {
        if (amount < 0.001f) return sig;

        // 1. Moderate Pre-gain drive scaling (1.0x to 6.5x for warm punch without harshness)
        float drive = 1.0f + amount * 5.5f;
        float driven = sig * drive;

        // 2. Subtle Asymmetric DC Bias for even-harmonic Gabber/Tribe "bark"
        float bias = amount * 0.15f;
        float biased = driven + bias;

        // 3. Smooth 1 to 2.2 Multi-cycle Wavefolding (musical crunch, no harsh fuzz)
        float foldCycles = 1.0f + amount * 1.2f;
        float folded = std::sin(biased * foldCycles);

        // 4. Soft Tanh Saturation (smooth warmth instead of brutal hard clip)
        float saturated = std::tanh(folded * 1.3f);

        // 5. Dynamic Mix & Sub Preservation
        float result = lerp(sig, saturated, amount * 0.85f);

        // Output normalization to preserve kick sub punch
        return std::tanh(result * 1.1f);
    }

    float lerp(float a, float b, float t) { return a + t * (b - a); }

    float getShapedPitch(float p, float shape) {
        if (shape < 0.20f) return lerp(std::sqrt(p), p, shape * 5.0f);
        if (shape < 0.40f) return lerp(p, p * p, (shape - 0.20f) * 5.0f);
        if (shape < 0.60f) {
            float t = (shape - 0.40f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            return lerp(p * p, sCurve * sCurve, t);
        }
        if (shape < 0.80f) {
            float t = (shape - 0.80f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float subDive = std::pow(p, 4.0f);
            return lerp(sCurve * sCurve, subDive, t);
        }
        float t = (shape - 0.80f) * 5.0f;
        float sCurve = p * p * (3.0f - 2.0f * p);
        float bounce = sCurve * sCurve + (0.15f * std::sin(M_PI * p) * p);
        float subDive = std::pow(p, 4.0f);
        return lerp(subDive, bounce, t);
    }

    float getVCO(float ph, float morph) {
        float s = std::sin(2.0f * M_PI * ph);
        if (morph <= 0.0f) return s;
        float tri = 2.0f * std::abs(2.0f * (ph - std::floor(ph + 0.5f))) - 1.0f;
        float saw = 2.0f * (ph - std::floor(ph + 0.5f));
        float sq = (s > 0.0f) ? 0.7f : -0.7f;
        if (morph < 0.33f) return lerp(s, tri, morph * 3.03f);
        if (morph < 0.66f) return lerp(tri, saw, (morph - 0.33f) * 3.03f);
        return lerp(saw, sq, (morph - 0.66f) * 3.03f);
    }

public:
    enum ModSource {
        SRC_ENV,
        SRC_LFO_TRI,
        SRC_LFO_SAW,
        SRC_LFO_SH
    };

    enum ModDest {
        DST_FILTER,
        DST_PITCH,
        DST_MORPH,
        DST_LEVEL,
        DST_NONE
    };

    struct ModRouting {
        const char* name;
        ModSource source;
        ModDest dest;
    };

    static constexpr int TOTAL_MOD_TYPES = 12;
    inline static const ModRouting modMatrix[TOTAL_MOD_TYPES] = {
        { "ENV Cutoff", SRC_ENV, DST_FILTER },
        { "ENV Pitch", SRC_ENV, DST_PITCH },
        { "ENV Wave", SRC_ENV, DST_MORPH },
        { "LFO Tri Cut", SRC_LFO_TRI, DST_FILTER },
        { "LFO Tri Pit", SRC_LFO_TRI, DST_PITCH },
        { "LFO Tri Wave", SRC_LFO_TRI, DST_MORPH },
        { "LFO Tri Lvl", SRC_LFO_TRI, DST_LEVEL },
        { "LFO Saw Cut", SRC_LFO_SAW, DST_FILTER },
        { "LFO Saw Pit", SRC_LFO_SAW, DST_PITCH },
        { "LFO Saw Wave", SRC_LFO_SAW, DST_MORPH },
        { "LFO S&H Cut", SRC_LFO_SH, DST_FILTER },
        { "LFO S&H Pit", SRC_LFO_SH, DST_PITCH }
    };

    Param params[41];

    // --- Kick Body Parameters ---
    Param& kickTune = addParam({ .key = "kickTune", .label = "Tune", .unit = " Hz", .value = 50.0f, .min = 30.0f, .max = 150.0f });
    Param& kickDecay = addParam({ .key = "kickDecay", .label = "Decay", .unit = " ms", .value = 200.0f, .min = 30.0f, .max = 2500.0f });
    Param& kickPitchEnvAmt = addParam({ .key = "kickPitchEnvAmt", .label = "Sweep Dep", .unit = "", .value = 80.0f, .min = 0.0f, .max = 150.0f });
    Param& kickSweepLen = addParam({ .key = "kickSweepLen", .label = "Sweep Len", .unit = " %", .value = 70.0f, .min = 0.0f, .max = 100.0f });
    Param& kickVcoMorph = addParam({ .key = "kickVcoMorph", .label = "VCO Morph", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });

    // --- Kick Click Parameters ---
    Param& kickClickAmt = addParam({ .key = "kickClickAmt", .label = "Click Amt", .unit = "", .value = 0.4f, .min = 0.0f, .max = 1.0f });
    Param& kickClickDecay = addParam({ .key = "kickClickDecay", .label = "Click Dec", .unit = " ms", .value = 4.0f, .min = 2.0f, .max = 200.0f });

    // --- Kick Drive & Rumble Parameters ---
    Param& kickDrive = addParam({ .key = "kickDrive", .label = "Kick Drv", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& rumbleAmt = addParam({ .key = "rumbleAmt", .label = "Rumble", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f });
    Param& rumbleGap = addParam({ .key = "rumbleGap", .label = "Rum. Gap", .unit = "ms", .value = 80.0f, .min = 10.0f, .max = 400.0f });

    // --- Texture Parameters (Replaces VCO2) ---
    Param& synthCutoff = addParam({ .key = "synthCutoff", .label = "Cutoff", .unit = "", .value = 0.1f, .min = 0.02f, .max = 0.98f });
    Param& synthResonance = addParam({ .key = "synthResonance", .label = "Reso", .unit = "", .value = 0.9f, .min = 0.0f, .max = 0.99f });
    Param& synthWaveform = addParam({ .key = "synthWaveform", .label = "Wave", .unit = "", .value = 0.7f, .min = 0.0f, .max = 1.0f });
    Param& synthRelease = addParam({ .key = "synthRelease", .label = "Release", .unit = " ms", .value = 180.0f, .min = 10.0f, .max = 2000.0f });
    Param& synthEnvAmt = addParam({ .key = "synthEnvAmt", .label = "Env Amt", .unit = "", .value = 0.2f, .min = 0.0f, .max = 1.0f });

    Param& synthModType = addParam({ .key = "synthModType", .label = "Mod Type", .unit = "", .value = 0.0f, .min = 0.0f, .max = 11.0f });
    Param& synthModDepth = addParam({ .key = "synthModDepth", .label = "Mod Depth", .unit = " %", .value = 0.0f, .min = -100.0f, .max = 100.0f });
    Param& synthModSpeed = addParam({ .key = "synthModSpeed", .label = "Mod Speed", .unit = " %", .value = 50.0f, .min = 0.0f, .max = 100.0f });
    Param& synthDelayMix = addParam({ .key = "synthDelayMix", .label = "Dly Mix", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& synthDelayTime = addParam({ .key = "synthDelayTime", .label = "Dly Time", .unit = " ms", .value = 250.0f, .min = 10.0f, .max = 1000.0f });
    Param& synthDelayFeedback = addParam({ .key = "synthDelayFeedback", .label = "Dly Feed", .unit = "", .value = 0.3f, .min = 0.0f, .max = 0.95f });
    Param& synthDrive = addParam({ .key = "synthDrive", .label = "Synth Drv", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& synthWaveshape = addParam({ .key = "synthWaveshape", .label = "Synth Shp", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& synthFmAmt = addParam({ .key = "synthFmAmt", .label = "FM Amt", .unit = "", .value = 0.0f, .min = 0.0f, .max = 5.0f });
    Param& synthFmRatio = addParam({ .key = "synthFmRatio", .label = "FM Ratio", .unit = "x", .value = 1.0f, .min = 1.0f, .max = 8.0f, .step = 1.0f });
    Param& synthFilterMorph = addParam({ .key = "synthFiltMorph", .label = "Filt Morph", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });

    Param& synthBasePitch = addParam({ .key = "synthBasePitch", .label = "Base Pitch", .unit = "", .value = 36.0f, .min = 24.0f, .max = 72.0f });
    Param& mix = addParam({ .key = "mix", .label = "Mix", .unit = "", .value = 0.5f, .min = 0.0f, .max = 1.0f });
    Param& masterVolume = addParam({ .key = "masterVolume", .label = "Volume", .unit = "", .value = 0.6f, .min = 0.0f, .max = 1.0f });
    Param& mstFold = addParam({ .key = "mstFold", .label = "Wavefold", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });

    bool performanceMode = false;

    Drop(double sr = 44100.0)
        : EngineBase(Synth, "Drop", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0 / sr)
    {
        init();
    }

    void triggerKickVoice() {
        kickClickEnv = 1.0f;
        if (!performanceMode) {
            kickPhase = 0.0f;
            kickAmpEnv = 1.0f;
            kickPitchEnv = 1.0f;
            kickElapsedSamples = 0.0;
            rumbleLP = 0.0f;
            rumbleDelaySample = 0.0f;

            float spd = lerp(0.005f, 0.15f, (kickSweepLen.value * 0.9f) * 0.01f);
            kickSpeedRatio = std::exp(-1.0f / (sampleRate * spd));
        }
    }

    void triggerSynthVoice(float midiNote) {
        synthTargetFreq = 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);
        synthCurrentFreq = synthTargetFreq;
        synthAmpEnv = 1.0f;
    }

    void noteOnImpl(uint8_t note, float velocity) {
        triggerSynthVoice(note);
        if (note == 36) triggerKickVoice();
    }

    float sampleImpl() {
        // --- 1. Kick Body & Click Generation ---
        float kickOut = 0.0f;
        if (kickAmpEnv > 0.001f || kickClickEnv > 0.001f) {
            float kickDecayCoeff = std::exp(-1.0f / (sampleRate * (kickDecay.value * 0.001f)));
            float clickDecayCoeff = std::exp(-1.0f / (sampleRate * (kickClickDecay.value * 0.001f)));

            kickAmpEnv *= kickDecayCoeff;
            kickClickEnv *= clickDecayCoeff;

            kickPitchEnv *= kickSpeedRatio;
            float autoShape = lerp(0.7f, 0.2f, kickSweepLen.value * 0.01f);
            float pMorph = getShapedPitch(kickPitchEnv, autoShape);

            float rootFreq = kickTune.value + (kickPitchEnvAmt.value * 4.0f * pMorph);
            kickPhase += rootFreq * sampleRateDiv;
            if (kickPhase > 1.0f) kickPhase -= 1.0f;

            float body = getVCO(kickPhase, kickVcoMorph.value);
            float sig = body * kickAmpEnv + nextNoise() * kickClickEnv * kickClickAmt.value;

            if (kickDrive.value > 0.001f) {
                float driven = sig * (1.0f + kickDrive.value * 10.0f);
                sig = std::tanh(driven);
            }

            kickOut = sig;

            // Techno Rumble Module
            if (rumbleAmt.value > 0.01f) {
                double targetGapSamples = (rumbleGap.value * 0.001f) * sampleRate;
                if (kickElapsedSamples >= targetGapSamples) {
                    float timeSinceGap = static_cast<float>(kickElapsedSamples - targetGapSamples) / sampleRate;
                    float riseEnv = 1.0f - std::exp(-timeSinceGap / 0.030f);
                    float decayEnv = std::exp(-timeSinceGap / 0.350f);

                    // float targetHz = 120.0f; // Fixed sub-bass lowpass cutoff
                    float targetHz = 40.0f + (rumbleAmt.value > 0.5f ? ((rumbleAmt.value - 0.5f) * 60.0f) : 0.0f);
                    float lowPassCoeff = std::clamp((float)(1.0f - std::exp(-2.0f * M_PI * targetHz / sampleRate)), 0.001f, 1.0f);

                    rumbleLP += lowPassCoeff * (rumbleDelaySample - rumbleLP);
                    float dirtySub = std::tanh(rumbleLP * 2.5f);
                    // kickOut += dirtySub * riseEnv * decayEnv * (rumbleAmt.value * 0.015f);
                    kickOut += dirtySub * riseEnv * decayEnv * (rumbleAmt.value * 0.01f);
                }
                rumbleDelaySample = kickOut;
            }
            kickElapsedSamples += 1.0;
        }

        // --- 2. Texture Generation (Full Modulation, FM, Filter Morph & Waveform restored) ---
        synthCurrentFreq = synthTargetFreq;

        float synthReleaseCoeff = std::exp(-1.0f / (sampleRate * (synthRelease.value * 0.001f)));
        synthAmpEnv *= synthReleaseCoeff;

        float lfoHz = 0.05f + (synthModSpeed.value * 0.01f) * (synthModSpeed.value * 0.01f) * 39.95f;
        synthLfoPhase += lfoHz * sampleRateDiv;
        if (synthLfoPhase >= 1.0) synthLfoPhase -= 1.0;

        int routeIdx = 0;
        if (!std::isnan(synthModType.value)) {
            routeIdx = (int)std::round(synthModType.value);
        }
        routeIdx = std::clamp(routeIdx, 0, TOTAL_MOD_TYPES - 1);
        ModRouting currentRoute = modMatrix[routeIdx];

        float srcVal = 0.0f;
        switch (currentRoute.source) {
        case SRC_ENV:
            srcVal = synthAmpEnv;
            break;
        case SRC_LFO_TRI:
            srcVal = synthLfoPhase < 0.5 ? (float)(4.0 * synthLfoPhase - 1.0) : (float)(3.0 - 4.0 * synthLfoPhase);
            break;
        case SRC_LFO_SAW:
            srcVal = (float)(2.0 * synthLfoPhase - 1.0);
            break;
        case SRC_LFO_SH: {
            uint32_t samplesPerHold = std::max((uint32_t)1, (uint32_t)(sampleRate / std::max(0.1f, lfoHz)));
            static uint32_t shCounter = 0;
            static float shValue = 0.0f;
            if (++shCounter >= samplesPerHold) {
                shCounter = 0;
                shValue = nextNoise();
            }
            srcVal = shValue;
            break;
        }
        }
        float modulationAmount = srcVal * (synthModDepth.value * 0.01f);

        float finalCutoff = synthCutoff.value;
        float finalPitchInterval = 0.0f;
        float finalWaveform = synthWaveform.value;
        float finalLevelModifier = 1.0f;

        if (currentRoute.dest == DST_FILTER) {
            finalCutoff = std::clamp(synthCutoff.value + modulationAmount, 0.01f, 0.99f);
        } else if (currentRoute.dest == DST_PITCH) {
            float semitones = modulationAmount * 24.0f;
            finalPitchInterval = std::round(semitones);
        } else if (currentRoute.dest == DST_MORPH) {
            finalWaveform = std::clamp(synthWaveform.value + modulationAmount, 0.0f, 1.0f);
        } else if (currentRoute.dest == DST_LEVEL) {
            finalLevelModifier = std::clamp(1.0f + modulationAmount, 0.0f, 2.0f);
        }

        float pitchRatio = std::pow(2.0f, finalPitchInterval / 12.0f);
        float modulatedFreq = synthCurrentFreq * pitchRatio;

        float modFreq = modulatedFreq * synthFmRatio.value;
        synthFmPhase += modFreq * sampleRateDiv;
        if (synthFmPhase > 1.0f) synthFmPhase -= 1.0f;

        float modOsc = std::sin(2.0f * M_PI * synthFmPhase);
        float fmDeviation = modOsc * synthFmAmt.value;

        synthPhase += modulatedFreq * sampleRateDiv;
        if (synthPhase > 1.0f) synthPhase -= 1.0f;

        float modulatedCarrierPhase = synthPhase + fmDeviation;
        modulatedCarrierPhase = modulatedCarrierPhase - std::floor(modulatedCarrierPhase);

        // Restored 3-Waveform Morphing: Saw -> Square -> Noise
        float ph = modulatedCarrierPhase;
        float saw = 2.0f * ph - 1.0f;
        float sq = (ph < 0.5f) ? 1.0f : -1.0f;
        float ns = nextNoise();

        float synthOsc = 0.0f;
        if (finalWaveform < 0.50f) {
            synthOsc = lerp(saw, sq, finalWaveform * 2.0f);
        } else {
            synthOsc = lerp(sq, ns, (finalWaveform - 0.50f) * 2.0f);
        }

        float cutoffMod = std::clamp(finalCutoff + (synthAmpEnv * synthEnvAmt.value), 0.01f, 0.99f);
        synthFilter.setCutoff(cutoffMod);
        synthFilter.setResonance(synthResonance.value);
        FilterSVF::Data& svfData = synthFilter.process12(synthOsc);

        float synthOut = 0.0f;
        float fMorph = synthFilterMorph.value;
        if (fMorph < 0.5f) {
            synthOut = lerp(svfData.lp, svfData.bp, fMorph * 2.0f);
        } else {
            synthOut = lerp(svfData.bp, svfData.hp, (fMorph - 0.5f) * 2.0f);
        }
        synthOut *= finalLevelModifier * synthAmpEnv;

        if (synthDrive.value > 0.001f || synthWaveshape.value > 0.001f) {
            float driveGain = 1.0f + synthDrive.value * 8.0f;
            float driven = synthOut * driveGain;

            if (synthWaveshape.value > 0.001f) {
                float foldAmt = synthWaveshape.value * 0.8f;
                float thresh = 1.0f - foldAmt;
                if (std::abs(driven) > thresh) {
                    driven = (driven > 0 ? thresh : -thresh) - (driven - (driven > 0 ? thresh : -thresh));
                }
                driven *= (1.0f / thresh);
            }

            float saturated = driven / (1.0f + std::abs(driven));
            float mixAmt = std::max(synthDrive.value, synthWaveshape.value);
            synthOut = lerp(synthOut, saturated, mixAmt);
        }

        if (synthDelayMix.value > 0.001f) {
            int delaySamples = std::clamp((int)(synthDelayTime.value * 0.001f * sampleRate), 1, DELAY_BUF_SIZE - 1);
            float delayVal = delayBuf[(delayWrite - delaySamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE];
            dlyFbSmooth += 0.001f * (synthDelayFeedback.value - dlyFbSmooth);
            delayBuf[delayWrite] = synthOut + delayVal * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
            synthOut = lerp(synthOut, synthOut + delayVal, synthDelayMix.value);
        }

        // --- 3. Master Mixer & Output Processing ---
        float kickGain = 1.0f - mix.value;
        float texGain = mix.value;
        float summed = kickOut * kickGain + synthOut * texGain;

        // Master Volume dual function:
        // 0.0 - 0.6: Clean Volume (0.0 -> 1.0 real gain)
        // 0.6 - 1.0: Full Gain (1.0) + Overdrive / Distortion
        float vol = masterVolume.value;
        float finalGain = 1.0f;
        float driveAmt = 0.0f;

        if (vol <= 0.6f) {
            finalGain = vol / 0.6f;
        } else {
            finalGain = 1.0f;
            driveAmt = (vol - 0.6f) / 0.4f;
        }

        summed *= finalGain;

        if (driveAmt > 0.001f) {
            summed = std::tanh(summed * (1.0f + driveAmt * 8.0f));
        }

        // High-Impact Gabber / Hardtek / Tribe Hard Wavefolder
        if (mstFold.value > 0.001f) {
            summed = gabberWavefold(summed, mstFold.value);
        }

        return std::clamp(summed, -1.0f, 1.0f);
    }

    float drawImpl(float x) {
        float ph = x;
        float saw = 2.0f * ph - 1.0f;
        float sq = (ph < 0.5f) ? 1.0f : -1.0f;
        float ns = std::sin(ph * 100.0f) * std::cos(ph * 230.0f);

        float waveform = synthWaveform.value;
        if (waveform < 0.50f) {
            return lerp(saw, sq, waveform * 2.0f);
        } else {
            return lerp(sq, ns, (waveform - 0.50f) * 2.0f);
        }
    }

    float process() {
        return sampleImpl();
    }
};
