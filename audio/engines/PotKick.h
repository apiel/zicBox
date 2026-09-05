#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/Eq.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyBoost.h"
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

class PotKick : public EngineBase<PotKick> {
public:
    EnvelopDrumAmp envelopAmp;
    EQ eq;
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

    // Shaped Pitch Sweep Curve
    float getShapedPitch(float p, float shape)
    {
        p = std::clamp(p, 0.0f, 1.0f);
        if (shape < 0.20f) {
            float t = shape * 5.0f;
            return lerp(std::sqrt(p), p, t);
        } else if (shape < 0.40f) {
            float t = (shape - 0.20f) * 5.0f;
            return lerp(p, p * p, t);
        } else if (shape < 0.65f) {
            float t = (shape - 0.40f) / 0.25f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            return lerp(p * p, sCurve, t);
        } else if (shape < 0.85f) {
            float t = (shape - 0.65f) / 0.20f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float pitchDip = sCurve - 0.35f * std::sin(M_PI * p) * (1.0f - p * p);
            return lerp(sCurve, pitchDip, t);
        } else {
            float t = (shape - 0.85f) / 0.15f;
            float sCurve = p * p * (3.0f - 2.0f * p);
            float pitchDip = sCurve - 0.35f * std::sin(M_PI * p) * (1.0f - p * p);
            float subSlide = p * p * p + 0.4f * std::sqrt(p) * (1.0f - p) * (1.0f - p);
            return lerp(pitchDip, subSlide, t);
        }
    }

public:
    // Declare exact parameter array size (15 params matching addParam calls)
    Param params[15];

    // Core Pitch, Duration, Click
    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .unit = "Hz", .value = 52.0f, .min = 30.0f, .max = 100.0f, .step = 1.0f });
    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 350.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& kickClickAmt = addParam({ .key = "kickClickAmt", .label = "Click Amt", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& kickClickDecay = addParam({ .key = "kickClickDecay", .label = "Click Dec", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 100.0f, .step = 1.0f });

    // Wave Morph, Pitch Sweep & FM Synthesis
    Param& vcoMorph = addParam({ .key = "vcoMorph", .label = "VCO Morph", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& sweepShp = addParam({ .key = "sweepShp", .label = "Sweep Shp", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& sweepDepth = addParam({ .key = "sweepDepth", .label = "Sweep Depth", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 25.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "x", .value = 1.5f, .min = 0.5f, .max = 8.0f, .step = 0.25f });

    // FM Decay, Saturation, Drive & Waveshaping
    Param& fmSnap = addParam({ .key = "fmSnap", .label = "FM Snap", .unit = "ms", .value = 25.0f, .min = 2.0f, .max = 150.0f, .step = 1.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& bassBoost = addParam({ .key = "bassBoost", .label = "Bass boost", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    // 3-Band Equalizer (Low Shelf | Mid Peak | High Shelf)
    Param& eqLow = addParam({ .key = "eqLow", .label = "EQ Low", .unit = "dB", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 0.5f });
    Param& eqMid = addParam({ .key = "eqMid", .label = "EQ Mid", .unit = "dB", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 0.5f });
    Param& eqHigh = addParam({ .key = "eqHigh", .label = "EQ High", .unit = "dB", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 0.5f });

    PotKick(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "PotKick", params)
        , sampleRate(sampleRate)
    {
        eq.crossoverLow = 150.0f;
        eq.crossoverHigh = 3000.0f;
        eq.gainDb[0] = 0.0f;
        eq.gainDb[1] = 0.0f;
        eq.gainDb[2] = 0.0f;
        eq.recompute(sampleRate);
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
            compressionEnv = 0.0f;

            int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
            envelopAmp.reset(totalSamples);
        }
    }

    void noteOffImpl(uint8_t note)
    {
    }

    float boostPrevInput, boostPrevOutput;

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        float kickOut = 0.0f;

        // 1. Generate Main Kick Body Sample
        if (envAmp > 0.0001f) {
            float depthNorm = sweepDepth.value * 0.01f;
            float sweepDecaySec = 0.005f + depthNorm * 0.060f;
            modulationEnvelope *= std::exp(-1.0f / (sampleRate * sweepDecaySec));

            float pMorph = getShapedPitch(modulationEnvelope, sweepShp.value * 0.01f);

            float depthMult = depthNorm * 5.0f;
            float rootFreq = baseFreq.value + (pMorph * baseFreq.value * depthMult);

            // FM Modulation decay
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

        // 2. Saturation, Drive & boost
        float out = kickOut;
        if (drive.value > 0.0f) {
            out = applyDrive(out, (drive.value * 0.01f) * 3.0f);
        }
        if (bassBoost.value > 0.0f) {
            out = applyBoost(out, bassBoost.value * 0.01f, boostPrevInput, boostPrevOutput);
        }
        out = applyCompression2(out, 0.65f, compressionEnv);

        // 3. 3-Band Equalizer (Low Shelf | Peak Mid | High Shelf)
        if (eqLow.value != eq.gainDb[0] || eqMid.value != eq.gainDb[1] || eqHigh.value != eq.gainDb[2]) {
            eq.gainDb[0] = eqLow.value;
            eq.gainDb[1] = eqMid.value;
            eq.gainDb[2] = eqHigh.value;
            eq.recompute(sampleRate);
        }

        if (std::abs(eqLow.value) > 0.01f || std::abs(eqMid.value) > 0.01f || std::abs(eqHigh.value) > 0.01f) {
            out = eq.process(out);
        }

        // 4. Kick Transient Click with kickClickDecay
        if (clickEnvelope > 0.0001f) {
            float clickDecaySec = std::clamp(kickClickDecay.value * 0.001f, 0.001f, 0.200f);
            clickEnvelope *= std::exp(-1.0f / (sampleRate * clickDecaySec));
            float clickSig = nextNoise() * clickEnvelope * (kickClickAmt.value * 0.01f);
            out += clickSig;
        }

        return out * velocity;
    }
};
