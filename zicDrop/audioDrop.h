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
    float kickAmpEnv = 0.0f;
    float kickPitchEnv = 0.0f;
    float kickClickEnv = 0.0f;

    // --- Noise Engine States ---
    float noiseAmpEnv = 0.0f;
    float noiseFilterState = 0.0f;

    // --- Acid/Drone Engine States ---
    float acidPhase = 0.0f;
    float acidTargetFreq = 110.0f;
    float acidCurrentFreq = 110.0f;
    float acidAmpEnv = 0.0f;
    float acidFilterStage[4] = {0.f, 0.f, 0.f, 0.f};

public:
    // --- Kick Engine Parameters ---
    float kickTune = 50.0f;      // Base freq (Hz)
    float kickDecay = 200.0f;    // Decay (ms)
    float kickPitchEnvAmt = 80.0f; // Sweep amount (0-100)
    float kickClickAmt = 0.4f;   // Click level (0-1)

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

    // Performance spacebar modifiers
    bool performanceMode = false;

    AudioDrop(double sr = 44100.0) : sampleRate(sr), sampleRateDiv(1.0 / sr) {}

    // Trigger Kick
    void triggerKickVoice() {
        kickPhase = 0.0f;
        kickAmpEnv = 1.0f;
        kickPitchEnv = 1.0f;
        kickClickEnv = 1.0f;
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
            float pitchDecayCoeff = std::exp(-1.0f / (sampleRate * 0.025f)); // Fast pitch sweep
            float clickDecayCoeff = std::exp(-1.0f / (sampleRate * 0.004f)); // 4ms click transient

            kickAmpEnv *= kickDecayCoeff;
            kickPitchEnv *= pitchDecayCoeff;
            kickClickEnv *= clickDecayCoeff;

            // Pitch sweep from kickPitchEnvAmt * 10 down to kickTune
            float freq = kickTune + kickPitchEnv * kickPitchEnvAmt * 8.0f;
            kickPhase += freq * sampleRateDiv;
            if (kickPhase > 1.0f) kickPhase -= 1.0f;

            // Sine wave bass drum
            float sineOsc = std::sin(2.0 * M_PI * kickPhase);
            // Click generator (highpass noise burst)
            float clickNoise = nextNoise() * kickClickEnv * kickClickAmt;

            kickOut = (sineOsc + clickNoise) * kickAmpEnv;
        }

        // --- 2. Noise Engine Generation ---
        float noiseOut = 0.0f;
        if (noiseAmpEnv > 0.001f || performanceMode) {
            // If performance mode is held, decay is frozen high
            float effectiveDecay = performanceMode ? 1000.0f : noiseDecay;
            float noiseDecayCoeff = std::exp(-1.0f / (sampleRate * (effectiveDecay * 0.001f)));
            
            if (!performanceMode) {
                noiseAmpEnv *= noiseDecayCoeff;
            }

            float rawNoise = nextNoise();
            
            // Simple bandpass filter for metallic noise color
            float fCoeff = 0.05f + noiseColor * 0.4f;
            noiseFilterState += fCoeff * (rawNoise - noiseFilterState);
            float bpNoise = rawNoise - noiseFilterState;

            noiseOut = bpNoise * (performanceMode ? 0.6f : noiseAmpEnv);
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
        if (performanceMode) {
            cutoffMod += 0.3f; // Performance filter spike
        }
        cutoffMod = std::clamp(cutoffMod, 0.01f, 0.99f);

        // Map resonance up to self-oscillation
        float resMod = acidResonance;
        if (performanceMode) {
            resMod = std::min(0.99f, resMod + 0.15f);
        }
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
        // Summing the 3 voices
        float summed = kickOut * 0.7f + noiseOut * 0.3f + acidOut * 0.5f;

        // Germanium Saturation / Waveshaping
        float driveVal = masterDrive;
        if (performanceMode) {
            driveVal = std::min(1.0f, driveVal + 0.50f); // Boost drive significantly under spacebar
        }

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
