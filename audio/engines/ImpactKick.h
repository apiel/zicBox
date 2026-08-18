#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyBoost.h"
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

class ImpactKick : public EngineBase<ImpactKick> {
public:
    EnvelopDrumAmp envelopAmp;
    std::atomic<bool> isBodyMuted { false };

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float modulationEnvelope = 0.0f;
    float fmEnv = 0.0f;
    float clickEnvelope = 0.0f;

    // Fast noise generator (LCG)
    uint32_t noiseState = 34567;
    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    // Bass Boost Filter Memory
    float bassBoostPrevInput = 0.0f;
    float bassBoostPrevOutput = 0.0f;

    // Internal Glue Compressor State
    float compressionEnv = 0.0f;

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

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

    // Shaped Pitch Sweep Curve from DrumKickGrid.h
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
            float t = (shape - 0.60f) * 5.0f;
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

    // High-Impact Gabber / Hardtek / Tribe Wavefolder Distortion Engine
    float gabberWavefold(float sig, float amount)
    {
        if (amount < 0.001f) return sig;

        float driveAmt = 1.0f + amount * 5.5f;
        float driven = sig * driveAmt;

        float bias = amount * 0.15f;
        float biased = driven + bias;

        float foldCycles = 1.0f + amount * 1.2f;
        float folded = std::sin(biased * foldCycles);

        float saturated = std::tanh(folded * 1.3f);
        float result = lerp(sig, saturated, amount * 0.85f);

        return std::tanh(result * 1.1f);
    }

public:
    // Declare exact parameter array size (12 params matching addParam calls)
    Param params[12];

    // Core Pitch, Duration, Click
    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .unit = "Hz", .value = 52.0f, .min = 30.0f, .max = 100.0f, .step = 1.0f });
    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 350.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& kickClickAmt = addParam({ .key = "kickClickAmt", .label = "Click Amt", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& kickClickDecay = addParam({ .key = "kickClickDecay", .label = "Click Dec", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 100.0f, .step = 1.0f });

    // Wave Morph, Pitch Sweep & FM Synthesis
    Param& vcoMorph = addParam({ .key = "vcoMorph", .label = "VCO Morph", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& sweepShp = addParam({ .key = "sweepShp", .label = "Sweep Shp", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 25.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "x", .value = 1.5f, .min = 0.5f, .max = 8.0f, .step = 0.25f });

    // FM Decay, Saturation, Bass Boost & Distortion
    Param& fmSnap = addParam({ .key = "fmSnap", .label = "FM Decay", .unit = "ms", .value = 25.0f, .min = 2.0f, .max = 150.0f, .step = 1.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& bassBoost = addParam({ .key = "bassBoost", .label = "Bass Boost", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& fold = addParam({ .key = "fold", .label = "Wavefold", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    std::atomic<int> semitoneOffset { 0 };

    ImpactKick(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "ImpactKick", params)
        , sampleRate(sampleRate)
    {
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
            fmEnv = 1.0f;
            bassBoostPrevInput = 0.0f;
            bassBoostPrevOutput = 0.0f;
            compressionEnv = 0.0f;

            int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
            envelopAmp.reset(totalSamples);
        }
    }

    void noteOffImpl(uint8_t note)
    {
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        float kickOut = 0.0f;

        // 1. Generate Main Kick Body Sample
        if (envAmp > 0.0001f) {
            modulationEnvelope *= std::exp(-1.0f / (sampleRate * 0.035f));
            float pMorph = getShapedPitch(modulationEnvelope, sweepShp.value * 0.01f);

            float rootFreq = baseFreq.value + (pMorph * baseFreq.value * 2.5f);
            int semi = semitoneOffset.load();
            if (semi != 0) {
                rootFreq *= std::pow(2.0f, semi / 12.0f);
            }

            // FM Modulation decay from DrumKickGrid.h
            fmEnv *= std::exp(-1.0f / (sampleRate * (fmSnap.value * 0.001f)));
            float modulatorFreq = rootFreq * fmRatio.value;
            float modulatorSignal = Math::fastSin2(PI_X2 * modulatorPhase);
            modulatorPhase += modulatorFreq / sampleRate;
            if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;

            float fmIntensity = (fmDepth.value * 0.01f) * 0.75f * fmEnv;
            carrierPhase += (rootFreq / sampleRate) + (modulatorSignal * fmIntensity * 0.04f);
            if (carrierPhase > 1.0f) carrierPhase -= 1.0f;

            float sig = getVCO(carrierPhase, vcoMorph.value * 0.01f);

            kickOut = sig * envAmp;
        }

        // 2. Dynamics, Bass Boost, Wavefolder Distortion & Drive
        float out = kickOut;

        if (bassBoost.value > 0.0f) {
            out = applyBoost(out, bassBoost.value * 0.01f, bassBoostPrevInput, bassBoostPrevOutput);
        }

        if (fold.value > 0.0f) {
            out = gabberWavefold(out, fold.value * 0.01f);
        }

        if (drive.value > 0.0f) {
            out = applyDrive(out, (drive.value * 0.01f) * 3.0f);
        }

        out = applyCompression2(out, 0.65f, compressionEnv);

        // 3. Kick Transient Click with kickClickDecay from drop2.h
        if (clickEnvelope > 0.0001f) {
            float clickDecaySec = std::clamp(kickClickDecay.value * 0.001f, 0.001f, 0.200f);
            clickEnvelope *= std::exp(-1.0f / (sampleRate * clickDecaySec));
            float clickSig = nextNoise() * clickEnvelope * (kickClickAmt.value * 0.01f);
            out += clickSig;
        }

        return out * velocity;
    }
};
