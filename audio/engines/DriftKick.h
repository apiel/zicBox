#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class DriftKick : public EngineBase<DriftKick> {
public:
    EnvelopDrumAmp envelopAmp;
    std::atomic<bool> isBodyMuted { false };

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float modulationEnvelope = 0.0f;
    float clickEnvelope = 0.0f;

    // Fast noise generator (LCG)
    uint32_t noiseState = 34567;
    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    // --- Buffer Replay Sub-Oscillator Rumble State ---
    static const int RUMBLE_BUF_SIZE = 8820;
    float kickBuffer[RUMBLE_BUF_SIZE];
    int kickWritePos = 0;
    float kickReadPos = 0.0f;
    double kickElapsedSamples = 0.0;
    float rumbleLP1 = 0.0f;
    float rumbleLP2 = 0.0f;

    // --- Internal Glue Compressor State ---
    float compressionEnv = 0.0f;

    float lerp(float a, float b, float t) { return a + t * (b - a); }

    // Morphing VCO Oscillator: Sine (0%) -> Triangle (33%) -> Saw (66%) -> Square (100%)
    float getVCO(float ph, float morphNorm)
    {
        float s = Math::fastSin2(PI_X2 * ph);
        if (morphNorm <= 0.0f) return s;

        float tri = 2.0f * std::abs(2.0f * (ph - std::floor(ph + 0.5f))) - 1.0f;
        float saw = 2.0f * (ph - std::floor(ph + 0.5f));
        float sq = (s > 0.0f) ? 0.75f : -0.75f;

        if (morphNorm < 0.333f) return lerp(s, tri, morphNorm * 3.0f);
        if (morphNorm < 0.666f) return lerp(tri, saw, (morphNorm - 0.333f) * 3.0f);
        return lerp(saw, sq, (morphNorm - 0.666f) * 3.0f);
    }

public:
    Param params[8];

    // View 1: Core Pitch, Click & Shape
    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .unit = "Hz", .value = 52.0f, .min = 30.0f, .max = 100.0f, .step = 1.0f });
    Param& clickAmt = addParam({ .key = "clickAmt", .label = "Click Amt", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 350.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& vcoMorph = addParam({ .key = "vcoMorph", .label = "VCO Morph", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 0.1f });

    // View 2: FM, Drive & Sub-Rumble Tail
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& rumbleAmt = addParam({ .key = "rumbleAmt", .label = "Rumble", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& rumbleGap = addParam({ .key = "rumbleGap", .label = "Rum Gap", .unit = "ms", .value = 120.0f, .min = 10.0f, .max = 400.0f, .step = 5.0f });

    DriftKick(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "DriftKick", params)
        , sampleRate(sampleRate)
    {
        std::fill_n(kickBuffer, RUMBLE_BUF_SIZE, 0.0f);
    }

    void trigger(float vel = 1.0f)
    {
        noteOnImpl(60, vel);
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        clickEnvelope = 1.0f;

        if (!isBodyMuted) {
            carrierPhase = 0.0f;
            modulatorPhase = 0.0f;
            modulationEnvelope = 1.0f;

            // Reset Buffer & Replay State
            kickWritePos = 0;
            kickReadPos = 0.0f;
            kickElapsedSamples = 0.0;
            rumbleLP1 = 0.0f;
            rumbleLP2 = 0.0f;
            compressionEnv = 0.0f;

            int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
            envelopAmp.reset(totalSamples);
        }
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        float kickOut = 0.0f;

        // 1. Generate Main Kick Body Sample
        if (envAmp > 0.0001f) {
            modulationEnvelope *= Math::exp(-1.0f / (sampleRate * 0.025f));

            float rootFreq = baseFreq.value;
            float modulatorFreq = rootFreq * 1.5f;
            float modulatorSignal = Math::fastSin2(PI_X2 * modulatorPhase);
            modulatorPhase += modulatorFreq / sampleRate;
            if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;

            float fmIntensity = pct(fmDepth) * 0.75f * modulationEnvelope;
            carrierPhase += (rootFreq / sampleRate) + (modulatorSignal * fmIntensity * 0.04f);
            if (carrierPhase > 1.0f) carrierPhase -= 1.0f;

            float sig = getVCO(carrierPhase, pct(vcoMorph));

            kickOut = sig * envAmp;
        }

        // Store original kick body sample into buffer for sub-pitch replay
        if (kickWritePos < RUMBLE_BUF_SIZE) {
            kickBuffer[kickWritePos++] = kickOut;
        }

        // 2. Deep Sub-Bass Pitch-Down Buffer Replay (0.4x Speed Sub Oscillator)
        float rumbleOut = 0.0f;
        float rAmt = pct(rumbleAmt);

        if (rAmt > 0.001f) {
            double targetGapSamples = (rumbleGap.value * 0.001f) * sampleRate;
            if (kickElapsedSamples >= targetGapSamples) {
                float rawReplaySample = 0.0f;
                int idxA = (int)kickReadPos;
                int idxB = idxA + 1;

                if (idxA < kickWritePos) {
                    float frac = kickReadPos - (float)idxA;
                    float sA = kickBuffer[idxA];
                    float sB = (idxB < kickWritePos) ? kickBuffer[idxB] : sA;
                    rawReplaySample = sA + frac * (sB - sA);

                    kickReadPos += 0.70f;
                }

                // Sub LPF cutoff (30 Hz at 0% -> 100 Hz at 100%)
                float cutoffHz = 30.0f + (rAmt * 70.0f);
                float lpfCoeff = std::clamp((float)(1.0f - std::exp(-2.0f * M_PI * cutoffHz / sampleRate)), 0.001f, 0.35f);

                // 2-pole cascaded Low-Pass Filter
                rumbleLP1 += lpfCoeff * (rawReplaySample - rumbleLP1);
                rumbleLP2 += lpfCoeff * (rumbleLP1 - rumbleLP2);

                // Deep Saturated Sub-Oscillator saturation
                float dirtySub = std::tanh(rumbleLP2 * 4.5f);

                // Dynamic Sidechained Sub Envelope
                float timeSinceGap = static_cast<float>(kickElapsedSamples - targetGapSamples) / sampleRate;
                float riseEnv = 1.0f - std::exp(-timeSinceGap / 0.020f); // 20ms rise
                float decayEnv = std::exp(-timeSinceGap / 0.350f); // 350ms sub decay

                rumbleOut = dirtySub * riseEnv * decayEnv * (rAmt * 1.1f);
            }
            kickElapsedSamples += 1.0;
        }

        // 3. Sum Kick + Sub-Rumble & Apply Internal Pre-Tuned Glue Compressor
        float out = kickOut + rumbleOut;
        if (drive.value > 0.0f) {
            out = applyDrive(out, pct(drive) * 3.0f);
        }
        out = applyCompression2(out, 0.65f, compressionEnv);

        if (clickEnvelope > 0.0001f) {
            clickEnvelope *= Math::exp(-1.0f / (sampleRate * 0.010f)); // 10ms sharp transient click
            float clickSig = nextNoise() * clickEnvelope * (pct(clickAmt) * 0.75f);
            out += clickSig;
        }

        return out * velocity;
    }
};
