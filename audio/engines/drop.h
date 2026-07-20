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

    // --- Kick Engine States ---
    float kickPhase = 0.0f;
    float kickPhaseVCO2 = 0.0f;
    float kickAmpEnv = 0.0f;
    float kickPitchEnv = 0.0f;
    float kickClickEnv = 0.0f;
    float kickSpeedRatio = 1.0f;
    float kickCompressEnv = 0.0f;
    float kickBodyGain = 1.0f;
    float bodyGainSlew = 0.05f;

    // --- Noise Engine States ---
    float noiseAmpEnv = 0.0f;
    float noiseFilterState = 0.0f;

    // --- Synth Engine States ---
    float synthPhase = 0.0f;
    float synthTargetFreq = 110.0f;
    float synthCurrentFreq = 110.0f;
    float synthAmpEnv = 0.0f;
    float synthFilterStage[4] = {0.f, 0.f, 0.f, 0.f};

    // --- Synth Modulation & Delay States ---
    double synthLfoPhase = 0.0;
    static const int DELAY_BUF_SIZE = 48000;
    float delayBuf[DELAY_BUF_SIZE] = {0.0f};
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

    // --- Particles State ---
    float shValue = 0.0f;
    uint32_t shCounter = 0;
    FilterSVF detailFilter;

    float masterCompressEnv = 0.0f;

    // --- Scream & Interfere DSP Helper ---
    inline float wavefold(float sig, float threshold) {
        if (threshold < 0.001f) threshold = 0.001f;
        float x = sig / threshold;
        return 1.5f * std::sin(x) * threshold;
    }

    float lerp(float a, float b, float t) { return a + t * (b - a); }

    float getShapedPitch(float p, float shape)
    {
        if (shape < 0.20f) {
            return lerp(std::sqrt(p), p, shape * 5.0f);
        } else if (shape < 0.40f) {
            return lerp(p, p * p, (shape - 0.20f) * 5.0f);
        } else if (shape < 0.60f) {
            float t = (shape - 0.40f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            return lerp(p * p, sCurve * sCurve, t);
        } else if (shape < 0.80f) {
            float t = (shape - 0.80f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float subDive = std::pow(p, 4.0f);
            return lerp(sCurve * sCurve, subDive, t);
        } else {
            float t = (shape - 0.80f) * 5.0f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float bounce = sCurve * sCurve + (0.15f * std::sin(M_PI * p) * p);
            float subDive = std::pow(p, 4.0f);
            return lerp(subDive, bounce, t);
        }
    }

    float getVCO(float ph, float morph)
    {
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

    Param params[36];

    // --- Kick Engine Parameters ---
    Param& kickTune = addParam({ .key = "kickTune", .label = "Tune", .unit = " Hz", .value = 50.0f, .min = 30.0f, .max = 150.0f });
    Param& kickDecay = addParam({ .key = "kickDecay", .label = "Decay", .unit = " ms", .value = 200.0f, .min = 30.0f, .max = 1000.0f });
    Param& kickPitchEnvAmt = addParam({ .key = "kickPitchEnvAmt", .label = "Sweep Dep", .unit = "", .value = 80.0f, .min = 0.0f, .max = 150.0f });
    Param& kickSweepLen = addParam({ .key = "kickSweepLen", .label = "Sweep Len", .unit = " %", .value = 70.0f, .min = 0.0f, .max = 100.0f });
    Param& kickSweepShp = addParam({ .key = "kickSweepShp", .label = "Sweep Shp", .unit = " %", .value = 50.0f, .min = 0.0f, .max = 100.0f });
    Param& kickVcoMorph = addParam({ .key = "kickVcoMorph", .label = "VCO Morph", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& kickVco2Level = addParam({ .key = "kickVco2Level", .label = "VCO2 Lvl", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& kickVco2Harm = addParam({ .key = "kickVco2Harm", .label = "VCO2 Harm", .unit = "x", .value = 2.0f, .min = 1.0f, .max = 12.0f });
    Param& kickVco2Morph = addParam({ .key = "kickVco2Morph", .label = "VCO2 MRP", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& kickClickAmt = addParam({ .key = "kickClickAmt", .label = "Click Amt", .unit = "", .value = 0.4f, .min = 0.0f, .max = 1.0f });
    Param& kickClickDecay = addParam({ .key = "kickClickDecay", .label = "Click Dec", .unit = " ms", .value = 4.0f, .min = 2.0f, .max = 200.0f });
    Param& kickDrive = addParam({ .key = "kickDrive", .label = "Kick Drv", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& kickWaveshape = addParam({ .key = "kickWaveshape", .label = "Kick Shp", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& kickCompress = addParam({ .key = "kickCompress", .label = "Kick Comp", .unit = "", .value = 0.3f, .min = 0.0f, .max = 1.0f });
    Param& kickClipping = addParam({ .key = "kickClipping", .label = "Kick Clip", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });

    // --- Noise Engine Parameters ---
    Param& noiseDecay = addParam({ .key = "noiseDecay", .label = "Noise Decay", .unit = " ms", .value = 100.0f, .min = 0.0f, .max = 1000.0f });
    Param& noiseColor = addParam({ .key = "noiseColor", .label = "Noise Color", .unit = "", .value = 0.5f, .min = 0.0f, .max = 1.0f });

    Param& synthCutoff = addParam({ .key = "synthCutoff", .label = "Cutoff", .unit = "", .value = 0.4f, .min = 0.02f, .max = 0.98f });
    Param& synthResonance = addParam({ .key = "synthResonance", .label = "Reso", .unit = "", .value = 0.7f, .min = 0.0f, .max = 0.99f });
    Param& synthWaveform = addParam({ .key = "synthWaveform", .label = "Wave", .unit = "", .value = 0.5f, .min = 0.0f, .max = 1.0f });
    Param& synthRelease = addParam({ .key = "synthRelease", .label = "Release", .unit = " ms", .value = 300.0f, .min = 10.0f, .max = 2000.0f });
    Param& synthEnvAmt = addParam({ .key = "synthEnvAmt", .label = "Env Amt", .unit = "", .value = 0.5f, .min = 0.0f, .max = 1.0f });

    // --- Synth Modulation Matrix & Delay Parameters ---
    Param& synthModType = addParam({ .key = "synthModType", .label = "Mod Type", .unit = "", .value = 0.0f, .min = 0.0f, .max = 11.0f });
    Param& synthModDepth = addParam({ .key = "synthModDepth", .label = "Mod Depth", .unit = " %", .value = 0.0f, .min = -100.0f, .max = 100.0f });
    Param& synthModSpeed = addParam({ .key = "synthModSpeed", .label = "Mod Speed", .unit = " %", .value = 50.0f, .min = 0.0f, .max = 100.0f });
    Param& synthDelayMix = addParam({ .key = "synthDelayMix", .label = "Dly Mix", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& synthDelayTime = addParam({ .key = "synthDelayTime", .label = "Dly Time", .unit = " ms", .value = 250.0f, .min = 10.0f, .max = 1000.0f });
    Param& synthDelayFeedback = addParam({ .key = "synthDelayFeedback", .label = "Dly Feed", .unit = "", .value = 0.3f, .min = 0.0f, .max = 0.95f });

    // // --- Particles Parameters ---
    // Param& dripLevel = addParam({ .key = "dLevel", .label = "Drip Level", .unit = "%", .value = 0.0f, .max = 100.0f });
    // Param& dripRate = addParam({ .key = "dRate", .label = "Drip Rate", .unit = "Hz", .value = 15.0f, .max = 200.0f });
    // Param& dripRand = addParam({ .key = "dRand", .label = "Drip Chaos", .unit = "%", .value = 50.0f });

    Param& synthBasePitch = addParam({ .key = "synthBasePitch", .label = "Base Pitch", .unit = "", .value = 36.0f, .min = 24.0f, .max = 72.0f });
    Param& kickLevel = addParam({ .key = "kickLevel", .label = "Kick Lvl", .unit = "", .value = 0.7f, .min = 0.0f, .max = 1.0f });
    Param& noiseLevel = addParam({ .key = "noiseLevel", .label = "Noise Lvl", .unit = "", .value = 0.3f, .min = 0.0f, .max = 1.0f });
    Param& synthLevel = addParam({ .key = "synthLevel", .label = "Synth Lvl", .unit = "", .value = 0.5f, .min = 0.0f, .max = 1.0f });
    Param& masterDrive = addParam({ .key = "masterDrive", .label = "Sat Drive", .unit = "", .value = 0.3f, .min = 0.0f, .max = 1.0f });
    Param& masterVolume = addParam({ .key = "masterVolume", .label = "Volume", .unit = "", .value = 0.8f, .min = 0.0f, .max = 1.0f });

    // --- Scream & Interfere Parameters ---
    Param& mstScream = addParam({ .key = "mstScream", .label = "Scream", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });
    Param& mstFold = addParam({ .key = "mstFold", .label = "Fold", .unit = "", .value = 0.0f, .min = 0.0f, .max = 1.0f });

    // Performance spacebar modifiers
    bool performanceMode = false;

    Drop(double sr = 44100.0)
        : EngineBase(Synth, "Drop", params)
        , sampleRate(sr)
        , sampleRateDiv(1.0 / sr)
    {
        bodyGainSlew = 1.0f - std::exp(-1.0f / (sr * 0.02f)); // 20ms transition
        init();
    }

    // Trigger Kick
    void triggerKickVoice() {
        kickPhase = 0.0f;
        kickPhaseVCO2 = 0.0f;
        kickAmpEnv = 1.0f;
        kickPitchEnv = 1.0f;
        kickClickEnv = 1.0f;

        // Calculate speed ratio matching DrumKick23
        float spd = lerp(0.005f, 0.15f, (kickSweepLen.value * 0.9f) * 0.01f);
        kickSpeedRatio = std::exp(-1.0f / (sampleRate * spd));
    }

    // Trigger Noise (Hi-hat / Snare)
    void triggerNoiseVoice() {
        noiseAmpEnv = 1.0f;
    }

    // Trigger Synth Voice (Unified with kick trigger)
    void triggerSynthVoice(float midiNote) {
        synthTargetFreq = 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);
        synthCurrentFreq = synthTargetFreq;
        synthAmpEnv = 1.0f;
    }

    void noteOnImpl(uint8_t note, float velocity)
    {
        triggerSynthVoice(note);
        if (note == 36) {
            triggerKickVoice();
        }
    }

    // Process a single audio sample (summed, saturated, mono output)
    float sampleImpl() {
        // --- 1. Kick Engine Generation ---
        float kickOut = 0.0f;
        if (kickAmpEnv > 0.001f) {
            // Snappy exponential envelopes
            float kickDecayCoeff = std::exp(-1.0f / (sampleRate * (kickDecay.value * 0.001f)));
            float clickDecayCoeff = std::exp(-1.0f / (sampleRate * (kickClickDecay.value * 0.001f)));

            kickAmpEnv *= kickDecayCoeff;
            kickClickEnv *= clickDecayCoeff;

            kickPitchEnv *= kickSpeedRatio;
            float pMorph = getShapedPitch(kickPitchEnv, kickSweepShp.value * 0.01f);

            // Pitch sweep from kickPitchEnvAmt down to kickTune
            float rootFreq = kickTune.value + (kickPitchEnvAmt.value * 4.0f * pMorph);
            
            kickPhase += rootFreq * sampleRateDiv;
            if (kickPhase > 1.0f) kickPhase -= 1.0f;

            float s1 = getVCO(kickPhase, kickVcoMorph.value);

            float s2 = 0.0f;
            if (kickVco2Level.value > 0.001f) {
                float musicalRatio = std::floor(kickVco2Harm.value);
                kickPhaseVCO2 += (rootFreq * musicalRatio) * sampleRateDiv;
                if (kickPhaseVCO2 > 1.0f) kickPhaseVCO2 -= 1.0f;
                s2 = getVCO(kickPhaseVCO2, kickVco2Morph.value);
            }

            float targetGain = performanceMode ? 0.0f : 1.0f;
            kickBodyGain += (targetGain - kickBodyGain) * bodyGainSlew;

            float body = s1 + (s2 * kickVco2Level.value * (0.5f + 0.5f * kickClickEnv));
            float sig = body * kickBodyGain + nextNoise() * kickClickEnv * kickClickAmt.value;

            // Apply Kick-specific drive & waveshaping
            if (kickDrive.value > 0.001f) {
                float gain = 1.0f + kickDrive.value * 15.0f;
                float driven = sig * gain;

                if (kickWaveshape.value > 0.001f) {
                    float foldAmt = kickWaveshape.value * 0.8f;
                    float thresh = 1.0f - foldAmt;
                    if (std::abs(driven) > thresh) {
                        driven = (driven > 0 ? thresh : -thresh) - (driven - (driven > 0 ? thresh : -thresh));
                    }
                    driven *= (1.0f / thresh);
                }

                float saturated = 0.0f;
                if (driven > 0.0f) {
                    saturated = 1.0f - std::exp(-driven);
                } else {
                    saturated = -0.8f * (1.0f - std::exp(driven * 1.2f));
                }

                sig = (sig * (1.0f - kickDrive.value)) + (saturated * kickDrive.value);
            }

            if (kickClipping.value > 0.001f) {
                float thresh = 1.0f - (kickClipping.value * 0.85f);
                if (sig > thresh) sig = thresh;
                else if (sig < -thresh) sig = -thresh;
                sig /= thresh;
            }

            kickOut = sig * kickAmpEnv;
            kickOut = applyCompression2(kickOut, kickCompress.value, kickCompressEnv);
        }

        // --- 3. Synth Engine Generation ---
        synthCurrentFreq = synthTargetFreq;

        // Envelope decay
        float synthReleaseCoeff = std::exp(-1.0f / (sampleRate * (synthRelease.value * 0.001f)));
        synthAmpEnv *= synthReleaseCoeff;

        // LFO Calculation
        float lfoHz = 0.05f + (synthModSpeed.value * 0.01f) * (synthModSpeed.value * 0.01f) * 39.95f;
        synthLfoPhase += lfoHz * sampleRateDiv;
        if (synthLfoPhase >= 1.0) synthLfoPhase -= 1.0;

        int routeIdx = 0;
        if (!std::isnan(synthModType.value)) {
            routeIdx = (int)std::round(synthModType.value);
        }
        if (routeIdx < 0) routeIdx = 0;
        if (routeIdx >= TOTAL_MOD_TYPES) routeIdx = TOTAL_MOD_TYPES - 1;
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

        // Apply modulation destinations
        float finalCutoff = synthCutoff.value;
        float finalPitchInterval = 0.0f;
        float finalWaveform = synthWaveform.value;
        float finalLevelModifier = 1.0f;

        if (currentRoute.dest == DST_FILTER) {
            finalCutoff = std::clamp(synthCutoff.value + modulationAmount, 0.01f, 0.99f);
        } else if (currentRoute.dest == DST_PITCH) {
            float semitones = modulationAmount * 24.0f;
            finalPitchInterval = std::round(semitones); // Quantize to nearest semitone
        } else if (currentRoute.dest == DST_MORPH) {
            finalWaveform = std::clamp(synthWaveform.value + modulationAmount, 0.0f, 1.0f);
        } else if (currentRoute.dest == DST_LEVEL) {
            finalLevelModifier = std::clamp(1.0f + modulationAmount, 0.0f, 2.0f);
        }

        // Modulate Frequency / Pitch
        float pitchRatio = std::pow(2.0f, finalPitchInterval / 12.0f);
        float modulatedFreq = synthCurrentFreq * pitchRatio;

        synthPhase += modulatedFreq * sampleRateDiv;
        if (synthPhase > 1.0f) synthPhase -= 1.0f;

        // 3-Waveform Morphing (Saw -> Square -> Noise)
        float ph = synthPhase;
        float saw = 2.0f * ph - 1.0f;
        float sq = (ph < 0.5f) ? 1.0f : -1.0f;
        float ns = nextNoise();

        float synthOsc = 0.0f;
        if (finalWaveform < 0.50f) {
            synthOsc = lerp(saw, sq, finalWaveform * 2.0f);
        } else {
            synthOsc = lerp(sq, ns, (finalWaveform - 0.50f) * 2.0f);
        }

        // 4-Pole Low Pass Filter (Self-Oscillating Moog-style simulation)
        // Cutoff modulation
        float cutoffMod = finalCutoff + (synthAmpEnv * synthEnvAmt.value);
        cutoffMod = std::clamp(cutoffMod, 0.01f, 0.99f);

        // Map resonance up to self-oscillation
        float resMod = synthResonance.value;
        float r = resMod * 3.98f; // 4.0 is self-oscillation threshold

        // process filter (4-stages)
        float input = synthOsc;
        // Feedback loop
        float f = cutoffMod * 1.09f; // Scaling coeff
        float p = f * (1.0f - 0.5f * f);

        // 4 poles process
        float stageInput = input - r * synthFilterStage[3];
        synthFilterStage[0] += p * (stageInput - synthFilterStage[0]);
        synthFilterStage[1] += p * (synthFilterStage[0] - synthFilterStage[1]);
        synthFilterStage[2] += p * (synthFilterStage[1] - synthFilterStage[2]);
        synthFilterStage[3] += p * (synthFilterStage[2] - synthFilterStage[3]);

        float synthOut = synthFilterStage[3] * finalLevelModifier * synthAmpEnv;

        float lfoOut = synthLfoPhase < 0.5f ? (4.0f * (float)synthLfoPhase - 1.0f) : (3.0f - 4.0f * (float)synthLfoPhase);

        // // PARTICLES
        // if (dripLevel.value > 0.0f) {
        //     float dripTarget = (sampleRate / std::max(0.1f, dripRate.value));
        //     if (++shCounter > dripTarget * (1.0f + (float)Noise::sample() * dripRand.value * 0.01f)) {
        //         shValue = (float)Noise::sample();
        //         shCounter = 0;
        //     }
        //     float particle = CLAMP(0.0f + shValue * (dripRand.value * 0.01f), 0.01f, 0.99f);
        //     detailFilter.setCutoff(particle);
        //     detailFilter.setResonance(1.0f);
        //     float detail = detailFilter.process12(synthOut).bp;

        //     synthOut = lerp(synthOut, detail, CLAMP(dripLevel.value * 0.01f, 0.0f, 1.0f));
        // }

        // Apply Delay Effect
        if (synthDelayMix.value > 0.001f) {
            int delaySamples = (int)(synthDelayTime.value * 0.001f * sampleRate);
            if (delaySamples >= DELAY_BUF_SIZE) delaySamples = DELAY_BUF_SIZE - 1;
            if (delaySamples < 1) delaySamples = 1;

            float delayVal = delayBuf[(delayWrite - delaySamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE];
            dlyFbSmooth += 0.001f * (synthDelayFeedback.value - dlyFbSmooth);
            delayBuf[delayWrite] = synthOut + delayVal * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
            synthOut = lerp(synthOut, synthOut + delayVal, synthDelayMix.value);
        }

        // --- 4. Master Slices / Germanium Saturation Module ---
        // Summing the active voices using mixer levels with compensated synth volume
        float synthComp = 1.0f - (1.0f - kickBodyGain) * masterDrive.value * kickLevel.value * 0.35f;
        
        float foldedSynth = synthOut * synthLevel.value * synthComp;
        if (mstFold.value > 0.001f) {
            float synthThreshold = 1.0f - mstFold.value * 0.8f;
            foldedSynth = wavefold(foldedSynth, synthThreshold);
        }

        float summed = kickOut * kickLevel.value + foldedSynth;

        // Germanium Saturation / Waveshaping
        float driveVal = masterDrive.value;
        float driven = summed;
        if (driveVal > 0.001f) {
            float driveGain = 1.0f + driveVal * 15.0f;
            float drivenInput = summed * driveGain;
            float saturated = 0.0f;
            if (drivenInput > 0.0f) {
                saturated = 1.0f - std::exp(-drivenInput);
            } else {
                saturated = -0.7f * (1.0f - std::exp(drivenInput * 1.4f));
            }
            driven = saturated;
        }

        // Scream & Fold on the mixed master output
        float finalSummed = driven;
        if (mstScream.value > 0.001f) {
            float masterDriveGain = 1.0f + mstScream.value * 15.0f;
            finalSummed *= masterDriveGain;
            // Perceived volume compensation scale
            // float compensation = 1.0f / (1.0f + mstScream.value * 4.0f);
            // finalSummed *= compensation;

            finalSummed = applyCompression2(finalSummed, 1.0f, masterCompressEnv);
        }
        if (mstFold.value > 0.001f) {
            float masterFoldThreshold = 1.0f - mstFold.value * 0.8f;
            finalSummed = wavefold(finalSummed, masterFoldThreshold);
        }

        // Master Volume
        float finalOut = finalSummed * masterVolume.value;
        return std::clamp(finalOut, -1.0f, 1.0f);
    }

    float drawImpl(float x) {
        float ph = x;
        float saw = 2.0f * ph - 1.0f;
        float sq = (ph < 0.5f) ? 1.0f : -1.0f;
        
        // Consistent pseudo-random noise visual
        float ns = std::sin(ph * 100.0f) * std::cos(ph * 230.0f);

        float waveform = synthWaveform.value;
        float synthOsc = 0.0f;
        if (waveform < 0.50f) {
            synthOsc = lerp(saw, sq, waveform * 2.0f);
        } else {
            synthOsc = lerp(sq, ns, (waveform - 0.50f) * 2.0f);
        }
        return synthOsc;
    }

    float process() {
        return sampleImpl();
    }
};
