#pragma once

#include "audio/utils/math.h"
#include <algorithm>
#include <cmath>

class KickBody {
private:
    float sampleRate = 44100.0f;
    float sampleRateDiv = 1.0f / 44100.0f;

    float phase = 0.0f;
    float ampEnv = 0.0f;
    float ampStep = 0.0f;
    float pitchEnv = 1.0f;
    float speedRatio = 0.999f;
    float velocity = 1.0f;

    // Sub-rumble state
    float rumblePhase = 0.0f;
    float rumbleEnv = 0.0f;
    float rumbleStep = 0.0f;
    bool inRumbleGap = false;
    float gapTimer = 0.0f;

    float lerp(float a, float b, float t) { return a + t * (b - a); }

    // Tekno Asymmetrical Wavefolder for raw aggressive warmth
    float applyWavefold(float in, float foldAmt)
    {
        if (foldAmt <= 0.0f) return in;
        float driveSig = in * (1.0f + foldAmt * 4.0f);
        float folded = std::sin(driveSig * (1.0f + foldAmt * 0.5f));
        if (folded > 0.8f) folded = 0.8f + (folded - 0.8f) * 0.2f; // Soft saturation ceiling
        return lerp(in, folded, std::min(foldAmt, 1.0f));
    }

public:
    // Parameters
    float baseFreq = 48.0f;       // 30 - 100 Hz
    float sweepDepth = 75.0f;     // 0 - 200 Hz
    float sweepDecayMs = 60.0f;   // 5 - 200 ms
    float durationMs = 350.0f;    // 50 - 1500 ms
    float vcoMorph = 0.0f;        // 0.0 (Sine) - 1.0 (Triangle/Sub-Dive)
    float wavefoldDrive = 0.3f;   // 0.0 - 1.0
    float rumbleLevel = 0.4f;     // 0.0 - 1.0
    float rumbleGapMs = 80.0f;    // 10 - 300 ms

    KickBody(float sr = 44100.0f)
        : sampleRate(sr)
        , sampleRateDiv(1.0f / sr)
    {
        updateSweepRatio();
    }

    void setSampleRate(float sr)
    {
        sampleRate = sr;
        sampleRateDiv = 1.0f / sr;
        updateSweepRatio();
    }

    void updateSweepRatio()
    {
        float spd = std::clamp(sweepDecayMs * 0.001f, 0.002f, 0.3f);
        speedRatio = Math::exp(-1.0f / (sampleRate * spd));
    }

    void trigger(float vel = 1.0f)
    {
        velocity = vel;
        phase = 0.0f;
        pitchEnv = 1.0f;
        ampEnv = 1.0f;

        float durSamples = std::max(1.0f, sampleRate * (durationMs * 0.001f));
        ampStep = 1.0f / durSamples;

        // Reset rumble tail timing
        inRumbleGap = true;
        gapTimer = sampleRate * (rumbleGapMs * 0.001f);
        rumbleEnv = 0.0f;
        rumblePhase = 0.0f;
    }

    float sample()
    {
        if (ampEnv <= 0.0f && rumbleEnv <= 0.0f && !inRumbleGap) {
            return 0.0f;
        }

        // Kick Main Body
        float kickSig = 0.0f;
        if (ampEnv > 0.0f) {
            float currentAmp = ampEnv;
            ampEnv -= ampStep;
            if (ampEnv < 0.0f) ampEnv = 0.0f;

            pitchEnv *= speedRatio;

            // Frequency calculation with exponential sweep
            float curFreq = baseFreq + (sweepDepth * pitchEnv);
            phase += curFreq * sampleRateDiv;
            if (phase > 1.0f) phase -= 1.0f;

            // VCO Morph (Sine -> Triangle -> Wavefold Sub)
            float sine = Math::fastSin2(PI_X2 * phase);
            float tri = 2.0f * std::abs(2.0f * (phase - std::floor(phase + 0.5f))) - 1.0f;
            float rawVco = lerp(sine, tri, vcoMorph);

            // Apply Tekno Wavefolder Saturation
            kickSig = applyWavefold(rawVco, wavefoldDrive) * currentAmp;
        }

        // Tekno Sub-Rumble Tail Processing
        float rumbleSig = 0.0f;
        if (inRumbleGap) {
            gapTimer -= 1.0f;
            if (gapTimer <= 0.0f) {
                inRumbleGap = false;
                rumbleEnv = 0.8f * velocity;
                float rDurSamples = sampleRate * 0.35f;
                rumbleStep = 1.0f / rDurSamples;
            }
        } else if (rumbleEnv > 0.0f) {
            rumbleEnv -= rumbleStep;
            if (rumbleEnv < 0.0f) rumbleEnv = 0.0f;

            // Sub-rumble frequency (slightly lower than kick base)
            float rFreq = baseFreq * 0.85f;
            rumblePhase += rFreq * sampleRateDiv;
            if (rumblePhase > 1.0f) rumblePhase -= 1.0f;

            float rSine = Math::fastSin2(PI_X2 * rumblePhase);
            rumbleSig = rSine * rumbleEnv * rumbleLevel;
        }

        return (kickSig + rumbleSig) * velocity;
    }

    // Helper for rendering waveform visualizer
    float getWaveformSample(float phaseNorm)
    {
        float pEnv = Math::exp(-phaseNorm * 4.0f);
        float curFreq = baseFreq + (sweepDepth * pEnv);
        float ph = phaseNorm * (curFreq / 50.0f);
        float s = Math::fastSin2(PI_X2 * ph);
        float tri = 2.0f * std::abs(2.0f * (ph - std::floor(ph + 0.5f))) - 1.0f;
        float vco = lerp(s, tri, vcoMorph);
        return applyWavefold(vco, wavefoldDrive) * (1.0f - phaseNorm);
    }
};
