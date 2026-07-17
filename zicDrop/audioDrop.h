#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>
#include "audio/effects/applyCompression.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class AudioDrop {
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

    // --- Noise Engine States ---
    float noiseAmpEnv = 0.0f;
    float noiseFilterState = 0.0f;

    // --- Acid/Drone Engine States ---
    float acidPhase = 0.0f;
    float acidTargetFreq = 110.0f;
    float acidCurrentFreq = 110.0f;
    float acidAmpEnv = 0.0f;
    float acidFilterStage[4] = {0.f, 0.f, 0.f, 0.f};

    // --- Acid Modulation & Delay States ---
    double acidLfoPhase = 0.0;
    static const int DELAY_BUF_SIZE = 48000;
    float delayBuf[DELAY_BUF_SIZE] = {0.0f};
    int delayWrite = 0;
    float dlyFbSmooth = 0.0f;

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
    // --- Kick Engine Parameters ---
    float kickTune = 50.0f;        // Base freq (Hz)
    float kickDecay = 200.0f;      // Decay (ms)
    float kickPitchEnvAmt = 80.0f; // Sweep depth (0-100)
    float kickSweepLen = 70.0f;    // Sweep length (0-100)
    float kickSweepShp = 50.0f;    // Sweep shape (0-100)
    float kickVcoMorph = 0.0f;     // VCO1 Morph (0-1)
    float kickVco2Level = 0.0f;    // VCO2 Level (0-1)
    float kickVco2Harm = 2.0f;     // VCO2 Harmonic (1-12)
    float kickVco2Morph = 0.0f;    // VCO2 Morph (0-1)
    float kickClickAmt = 0.4f;     // Click level (0-1)
    float kickClickDecay = 4.0f;   // Click decay (2-200 ms)
    float kickDrive = 0.0f;        // Kick-specific drive (0-1)
    float kickWaveshape = 0.0f;    // Kick waveshaper folding/saturation (0-1)
    float kickCompress = 0.3f;     // Kick compression amount (0-1)

    // --- Noise Engine Parameters ---
    float noiseDecay = 100.0f;   // Decay (ms)
    float noiseColor = 0.5f;     // Bandpass/Highpass morph parameter

    // --- Acid/Drone Parameters ---
    float acidCutoff = 0.4f;     // Filter cutoff (0-1)
    float acidResonance = 0.7f;  // Filter resonance (0-1)
    float acidDecay = 300.0f;    // Filter envelope decay (ms)
    float acidEnvAmt = 0.5f;     // Filter env amount
    float acidWaveform = 0.5f;   // Morph Saw to Square
    float acidGlide = 50.0f;     // Glide time (ms)

    // --- Acid Modulation Matrix & Delay Parameters ---
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

    float acidModType = 0.0f;          // Modulation routing type (0 to 11)
    float acidModDepth = 0.0f;         // Modulation depth (-100 to 100)
    float acidModSpeed = 50.0f;        // Modulation speed/LFO rate (0 to 100)
    float acidDelayMix = 0.0f;         // Delay Mix (0-1)
    float acidDelayTime = 250.0f;      // Delay Time (ms)
    float acidDelayFeedback = 0.3f;    // Delay Feedback (0-1)

    // --- Master Saturation Parameters ---
    float masterDrive = 0.3f;    // Germanium saturation drive (0-1)
    float masterVolume = 0.8f;   // Master output volume

    // Mixer volumes (0.0 to 1.0)
    float kickLevel = 0.7f;
    float noiseLevel = 0.3f;
    float synthLevel = 0.5f;

    // Performance spacebar modifiers
    bool performanceMode = false;

    AudioDrop(double sr = 44100.0) : sampleRate(sr), sampleRateDiv(1.0 / sr) {}

    // Trigger Kick
    void triggerKickVoice() {
        kickPhase = 0.0f;
        kickPhaseVCO2 = 0.0f;
        kickAmpEnv = 1.0f;
        kickPitchEnv = 1.0f;
        kickClickEnv = 1.0f;

        // Calculate speed ratio matching DrumKick23
        float spd = lerp(0.005f, 0.15f, (kickSweepLen * 0.9f) * 0.01f);
        kickSpeedRatio = std::exp(-1.0f / (sampleRate * spd));
    }

    // Trigger Noise (Hi-hat / Snare)
    void triggerNoiseVoice() {
        noiseAmpEnv = 1.0f;
    }

    // Set target frequency for Acid Engine (e.g. from MIDI note)
    void setAcidPitch(float midiNote) {
        // Convert MIDI note to Frequency
        acidTargetFreq = 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);
        if (acidGlide <= 1.0f) {
            acidCurrentFreq = acidTargetFreq;
        }
        acidAmpEnv = 1.0f;
    }

    // Process a single audio sample (summed, saturated, mono output)
    float process() {
        // --- 1. Kick Engine Generation ---
        float kickOut = 0.0f;
        if (kickAmpEnv > 0.001f) {
            // Snappy exponential envelopes
            float kickDecayCoeff = std::exp(-1.0f / (sampleRate * (kickDecay * 0.001f)));
            float clickDecayCoeff = std::exp(-1.0f / (sampleRate * (kickClickDecay * 0.001f)));

            kickAmpEnv *= kickDecayCoeff;
            kickClickEnv *= clickDecayCoeff;

            kickPitchEnv *= kickSpeedRatio;
            float pMorph = getShapedPitch(kickPitchEnv, kickSweepShp * 0.01f);

            // Pitch sweep from kickPitchEnvAmt down to kickTune
            float rootFreq = kickTune + (kickPitchEnvAmt * 4.0f * pMorph);
            
            kickPhase += rootFreq * sampleRateDiv;
            if (kickPhase > 1.0f) kickPhase -= 1.0f;

            float s1 = getVCO(kickPhase, kickVcoMorph);

            float s2 = 0.0f;
            if (kickVco2Level > 0.001f) {
                float musicalRatio = std::floor(kickVco2Harm);
                kickPhaseVCO2 += (rootFreq * musicalRatio) * sampleRateDiv;
                if (kickPhaseVCO2 > 1.0f) kickPhaseVCO2 -= 1.0f;
                s2 = getVCO(kickPhaseVCO2, kickVco2Morph);
            }

            float sig = 0.0f;
            if (performanceMode) {
                // Mute body, keep only the click
                sig = nextNoise() * kickClickEnv * kickClickAmt;
            } else {
                sig = s1 + (s2 * kickVco2Level * (0.5f + 0.5f * kickClickEnv));
                sig += nextNoise() * kickClickEnv * kickClickAmt;
            }

            // Apply Kick-specific drive & waveshaping
            if (kickDrive > 0.001f) {
                float gain = 1.0f + kickDrive * 15.0f;
                float driven = sig * gain;

                if (kickWaveshape > 0.001f) {
                    float foldAmt = kickWaveshape * 0.8f;
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

                sig = (sig * (1.0f - kickDrive)) + (saturated * kickDrive);
            }

            kickOut = sig * kickAmpEnv;
            kickOut = applyCompression2(kickOut, kickCompress, kickCompressEnv);
        }

        // --- 3. Acid/Drone Engine Generation ---
        // Glide logic
        float glideCoeff = (acidGlide <= 1.0f) ? 1.0f : (1.0f - std::exp(-1.0f / (sampleRate * (acidGlide * 0.001f))));
        acidCurrentFreq += (acidTargetFreq - acidCurrentFreq) * glideCoeff;

        // Envelope decay
        float acidDecayCoeff = std::exp(-1.0f / (sampleRate * (acidDecay * 0.001f)));
        acidAmpEnv *= acidDecayCoeff;

        // LFO Calculation
        float lfoHz = 0.05f + (acidModSpeed * 0.01f) * (acidModSpeed * 0.01f) * 39.95f;
        acidLfoPhase += lfoHz * sampleRateDiv;
        if (acidLfoPhase >= 1.0) acidLfoPhase -= 1.0;

        float modSourceValue = 0.0f;
        int routeIdx = 0;
        if (!std::isnan(acidModType)) {
            routeIdx = (int)std::round(acidModType);
        }
        if (routeIdx < 0) routeIdx = 0;
        if (routeIdx >= TOTAL_MOD_TYPES) routeIdx = TOTAL_MOD_TYPES - 1;
        ModRouting currentRoute = modMatrix[routeIdx];

        switch (currentRoute.source) {
        case SRC_ENV:
            modSourceValue = acidAmpEnv;
            break;
        case SRC_LFO_TRI:
            modSourceValue = acidLfoPhase < 0.5 ? (float)(4.0 * acidLfoPhase - 1.0) : (float)(3.0 - 4.0 * acidLfoPhase);
            break;
        case SRC_LFO_SAW:
            modSourceValue = (float)(2.0 * acidLfoPhase - 1.0);
            break;
        case SRC_LFO_SH: {
            uint32_t samplesPerHold = std::max((uint32_t)1, (uint32_t)(sampleRate / std::max(0.1f, lfoHz)));
            static uint32_t shCounter = 0;
            static float shValue = 0.0f;
            if (++shCounter >= samplesPerHold) {
                shCounter = 0;
                shValue = nextNoise();
            }
            modSourceValue = shValue;
            break;
        }
        }

        float modulationAmount = modSourceValue * (acidModDepth * 0.01f);

        float finalCutoff = acidCutoff;
        float finalPitchInterval = 0.0f;
        float finalWaveform = acidWaveform;
        float finalLevelModifier = 1.0f;

        if (currentRoute.dest == DST_FILTER) {
            finalCutoff = std::clamp(acidCutoff + modulationAmount, 0.01f, 0.99f);
        } else if (currentRoute.dest == DST_PITCH) {
            finalPitchInterval = modulationAmount * 24.0f;
        } else if (currentRoute.dest == DST_MORPH) {
            finalWaveform = std::clamp(acidWaveform + modulationAmount, 0.0f, 1.0f);
        } else if (currentRoute.dest == DST_LEVEL) {
            finalLevelModifier = std::clamp(1.0f + modulationAmount, 0.0f, 2.0f);
        }

        // Modulate Frequency / Pitch
        float pitchRatio = std::pow(2.0f, finalPitchInterval / 12.0f);
        float modulatedFreq = acidCurrentFreq * pitchRatio;

        acidPhase += modulatedFreq * sampleRateDiv;
        if (acidPhase > 1.0f) acidPhase -= 1.0f;

        // Morph Saw to Square
        float saw = 2.0f * acidPhase - 1.0f;
        float sq = (acidPhase < 0.5f) ? 1.0f : -1.0f;
        float acidOsc = saw + (sq - saw) * finalWaveform;

        // 4-Pole Low Pass Filter (Self-Oscillating Moog-style simulation)
        // Cutoff modulation
        float cutoffMod = finalCutoff + (acidAmpEnv * acidEnvAmt);
        cutoffMod = std::clamp(cutoffMod, 0.01f, 0.99f);

        // Map resonance up to self-oscillation
        float resMod = acidResonance;
        float r = resMod * 3.98f; // 4.0 is self-oscillation threshold

        // process filter (4-stages)
        float input = acidOsc;
        // Feedback loop
        float f = cutoffMod * 1.09f; // Scaling coeff
        float p = f * (1.0f - 0.5f * f);
        float scale = 1.0f + r;

        // 4 poles process
        float stageInput = input - r * acidFilterStage[3];
        acidFilterStage[0] += p * (stageInput - acidFilterStage[0]);
        acidFilterStage[1] += p * (acidFilterStage[0] - acidFilterStage[1]);
        acidFilterStage[2] += p * (acidFilterStage[1] - acidFilterStage[2]);
        acidFilterStage[3] += p * (acidFilterStage[2] - acidFilterStage[3]);

        float acidOut = acidFilterStage[3] * finalLevelModifier;

        // Apply Delay Effect
        if (acidDelayMix > 0.001f) {
            int delaySamples = (int)(acidDelayTime * 0.001f * sampleRate);
            if (delaySamples >= DELAY_BUF_SIZE) delaySamples = DELAY_BUF_SIZE - 1;
            if (delaySamples < 1) delaySamples = 1;

            float delayVal = delayBuf[(delayWrite - delaySamples + DELAY_BUF_SIZE) % DELAY_BUF_SIZE];
            dlyFbSmooth += 0.001f * (acidDelayFeedback - dlyFbSmooth);
            delayBuf[delayWrite] = acidOut + delayVal * dlyFbSmooth;
            delayWrite = (delayWrite + 1) % DELAY_BUF_SIZE;
            acidOut = lerp(acidOut, acidOut + delayVal, acidDelayMix);
        }

        // --- 4. Master Slices / Germanium Saturation Module ---
        // Summing the active voices using mixer levels
        float summed = kickOut * kickLevel + acidOut * synthLevel;

        // Germanium Saturation / Waveshaping
        float driveVal = masterDrive;

        // Apply Waveshaper
        float driveGain = 1.0f + driveVal * 15.0f;
        float driven = summed * driveGain;
        
        // Asymmetric Waveshaping simulating Germanium clipping (warmer, rich intermodulation)
        float saturated = 0.0f;
        if (driven > 0.0f) {
            saturated = 1.0f - std::exp(-driven);
        } else {
            // Negative half cycles clip softer, adding even-order harmonics
            saturated = -0.7f * (1.0f - std::exp(driven * 1.4f));
        }

        // Master Volume
        float finalOut = saturated * masterVolume;
        return std::clamp(finalOut, -1.0f, 1.0f);
    }
};
