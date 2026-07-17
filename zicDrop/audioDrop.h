#pragma once

#include <cmath>
#include <algorithm>
#include <cstdint>

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

    // --- Noise Engine States ---
    float noiseAmpEnv = 0.0f;
    float noiseFilterState = 0.0f;

    // --- Acid/Drone Engine States ---
    float acidPhase = 0.0f;
    float acidTargetFreq = 110.0f;
    float acidCurrentFreq = 110.0f;
    float acidAmpEnv = 0.0f;
    float acidFilterStage[4] = {0.f, 0.f, 0.f, 0.f};

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

            float sig = s1 + (s2 * kickVco2Level * (0.5f + 0.5f * kickClickEnv));

            // Click generator (highpass noise burst)
            float clickNoise = nextNoise() * kickClickEnv * kickClickAmt;
            sig += clickNoise;

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
            if (performanceMode) {
                kickOut = 0.0f;
            }
        }

        // --- 2. Noise Engine Generation ---
        float noiseOut = 0.0f;
        if (noiseAmpEnv > 0.001f) {
            float noiseDecayCoeff = std::exp(-1.0f / (sampleRate * (noiseDecay * 0.001f)));
            noiseAmpEnv *= noiseDecayCoeff;

            float rawNoise = nextNoise();
            
            // Simple bandpass filter for metallic noise color
            float fCoeff = 0.05f + noiseColor * 0.4f;
            noiseFilterState += fCoeff * (rawNoise - noiseFilterState);
            float bpNoise = rawNoise - noiseFilterState;

            noiseOut = bpNoise * noiseAmpEnv;
        }

        // --- 3. Acid/Drone Engine Generation ---
        // Glide logic
        float glideCoeff = (acidGlide <= 1.0f) ? 1.0f : (1.0f - std::exp(-1.0f / (sampleRate * (acidGlide * 0.001f))));
        acidCurrentFreq += (acidTargetFreq - acidCurrentFreq) * glideCoeff;

        acidPhase += acidCurrentFreq * sampleRateDiv;
        if (acidPhase > 1.0f) acidPhase -= 1.0f;

        // Morph Saw to Square
        float saw = 2.0f * acidPhase - 1.0f;
        float sq = (acidPhase < 0.5f) ? 1.0f : -1.0f;
        float acidOsc = saw + (sq - saw) * acidWaveform;

        // Filter envelope (Decay & Env Mod)
        float acidDecayCoeff = std::exp(-1.0f / (sampleRate * (acidDecay * 0.001f)));
        acidAmpEnv *= acidDecayCoeff;

        // 4-Pole Low Pass Filter (Self-Oscillating Moog-style simulation)
        // Cutoff modulation
        float cutoffMod = acidCutoff + acidAmpEnv * acidEnvAmt;
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

        float acidOut = acidFilterStage[3];

        // --- 4. Master Slices / Germanium Saturation Module ---
        // Summing the 3 voices using mixer levels
        float summed = kickOut * kickLevel + noiseOut * noiseLevel + acidOut * synthLevel;

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
