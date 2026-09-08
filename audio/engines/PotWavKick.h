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

class PotWavKick : public EngineBase<PotWavKick> {
public:
    EnvelopDrumAmp envelopAmp;
    EQ eq;
    std::atomic<bool> isBodyMuted { false };
    float transposeSemitones = 0.0f;

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float carrierPhase = 0.0f;
    float subPhase = 0.0f;
    float modulationEnvelope = 0.0f;
    float clickEnvelope = 0.0f;
    float toneFilterBuf = 0.0f;
    float compressionEnv = 0.0f;
    float boostPrevInput = 0.0f;
    float boostPrevOutput = 0.0f;

    // Fast noise generator (LCG)
    uint32_t noiseState = 9876543;
    float nextNoise()
    {
        noiseState = noiseState * 196314165 + 907633389;
        return (float)int32_t(noiseState) / 2147483648.f;
    }

    static float lerp(float a, float b, float t) { return a + t * (b - a); }

    // Piecewise Linear Phase Symmetry / Bending (Guarantees no dead phase or silent dropouts)
    float applyPhaseSymmetry(float ph, float symNorm)
    {
        float center = 0.05f + std::clamp(symNorm, 0.0f, 1.0f) * 0.90f; // 0.05 to 0.95
        if (ph < center) {
            return (ph / center) * 0.5f;
        } else {
            return 0.5f + ((ph - center) / (1.0f - center)) * 0.5f;
        }
    }

    // High-Impact Multi-stage Waveform Engine
    float getWaveform(float ph, float morphNorm)
    {
        ph = ph - std::floor(ph); // Ensure bounded [0, 1)

        float s = Math::fastSin2(PI_X2 * ph);
        if (morphNorm <= 0.0f) return s + 0.3f * Math::fastSin2(PI_X2 * ph * 2.0f); // Warm Sub Sine + 2nd Harmonic

        // 6 Multi-stage shapes
        float punchSine = s + 0.35f * Math::fastSin2(PI_X2 * ph * 2.0f);                    // Punch Sine + 2nd Harmonic
        float tri = 2.0f * std::abs(2.0f * (ph - std::floor(ph + 0.5f))) - 1.0f;            // Skewed Triangle
        float narrowSpike = (ph < 0.20f) ? 0.95f : -0.25f;                                  // Saturated Pulse Spike
        float hyperSine = (s + 0.5f * Math::fastSin2(PI_X2 * ph * 2.0f)) * 0.85f;           // Formant Fold Wave
        float sq = (s > 0.0f) ? 0.85f : -0.85f;                                             // Square Sub Slam
        float xSaw = (2.0f * (ph - std::floor(ph + 0.5f))) * 3.0f;
        float chaos = xSaw / (1.0f + std::abs(xSaw));                                       // Hyper-Saturated Saw Ramp

        const float pos = std::clamp(morphNorm, 0.0f, 1.0f) * 5.0f;
        const int idx = static_cast<int>(pos);
        const float t = pos - idx;

        switch (idx) {
            case 0: return lerp(punchSine, tri, t);
            case 1: return lerp(tri, narrowSpike, t);
            case 2: return lerp(narrowSpike, hyperSine, t);
            case 3: return lerp(hyperSine, sq, t);
            case 4: default: return lerp(sq, chaos, std::min(t, 1.0f));
        }
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
    Param params[15];

    // Core Pitch, Duration, Click
    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .unit = "Hz", .value = 52.0f, .min = 30.0f, .max = 100.0f, .step = 1.0f });
    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 350.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& kickClickAmt = addParam({ .key = "kickClickAmt", .label = "Click Amt", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& kickClickDecay = addParam({ .key = "kickClickDecay", .label = "Click Dec", .unit = "ms", .value = 10.0f, .min = 1.0f, .max = 100.0f, .step = 1.0f });

    // High-Impact Waveform Synthesis & Pitch Controls
    Param& vcoMorph = addParam({ .key = "vcoMorph", .label = "VCO Morph", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& phaseSym = addParam({ .key = "phaseSym", .label = "Symmetry", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& transientBite = addParam({ .key = "transientBite", .label = "Trans Bite", .unit = "%", .value = 25.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& sweepDepth = addParam({ .key = "sweepDepth", .label = "Sweep Depth", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& sweepShp = addParam({ .key = "sweepShp", .label = "Sweep Shp", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    // Tone, Saturation, Sub Punch & Formant Fold
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& formantFold = addParam({ .key = "formantFold", .label = "Formant", .unit = "%", .value = 0.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& subPunch = addParam({ .key = "subPunch", .label = "Sub Punch", .unit = "%", .value = 30.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& toneFilter = addParam({ .key = "toneFilter", .label = "Tone", .unit = "%", .value = 75.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    // 3-Band Equalizer
    Param& eqLow = addParam({ .key = "eqLow", .label = "EQ Low", .unit = "dB", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 0.5f });
    Param& eqMid = addParam({ .key = "eqMid", .label = "EQ Mid", .unit = "dB", .value = 0.0f, .min = -12.0f, .max = 12.0f, .step = 0.5f });

    PotWavKick(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "PotWavKick", params)
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
        toneFilterBuf = 0.0f;

        if (!isBodyMuted) {
            carrierPhase = 0.0f;
            subPhase = 0.0f;
            modulationEnvelope = 1.0f;
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

        if (envAmp > 0.0001f) {
            float depthNorm = sweepDepth.value * 0.01f;
            float sweepDecaySec = 0.004f + depthNorm * 0.055f;
            modulationEnvelope *= std::exp(-1.0f / (sampleRate * sweepDecaySec));

            float pitchMult = (transposeSemitones != 0.0f) ? std::exp(0.057762265f * transposeSemitones) : 1.0f;
            float effectiveBaseFreq = baseFreq.value * pitchMult;

            float pMorph = getShapedPitch(modulationEnvelope, sweepShp.value * 0.01f);
            float depthMult = depthNorm * 12.0f; // High-impact pitch sweep up to 12x sub freq
            float biteVal = transientBite.value * 0.01f;
            float bitePitchSpike = biteVal * 35.0f * clickEnvelope; // Massive transient pitch snap
            float rootFreq = effectiveBaseFreq + (pMorph * effectiveBaseFreq * depthMult) + (effectiveBaseFreq * bitePitchSpike);

            float phaseInc = rootFreq / sampleRate;
            carrierPhase += phaseInc;
            if (carrierPhase >= 1.0f) carrierPhase -= 1.0f;

            subPhase += phaseInc * 0.5f;
            if (subPhase >= 1.0f) subPhase -= 1.0f;

            // 1. Phase Symmetry / Bending
            float bentPhase = applyPhaseSymmetry(carrierPhase, phaseSym.value * 0.01f);

            // 2. High-Impact Waveform Generation
            float waveSig = getWaveform(bentPhase, vcoMorph.value * 0.01f);

            // 3. Formant Fold Modulation
            if (formantFold.value > 0.0f) {
                float ffMult = 1.0f + (formantFold.value * 0.01f) * 3.5f;
                float ffSig = std::sin(PI_X2 * carrierPhase * ffMult);
                waveSig = lerp(waveSig, waveSig * ffSig * 1.25f, (formantFold.value * 0.01f) * 0.85f);
            }

            // 4. Sub Punch Reinforcer
            if (subPunch.value > 0.0f) {
                float subSine = Math::fastSin2(PI_X2 * subPhase);
                waveSig += subSine * (subPunch.value * 0.01f) * 0.75f;
            }

            // 5. High-Impact Wideband Tone Filter (100 Hz at 0% to 16 kHz at 100%)
            float toneVal = toneFilter.value * 0.01f;
            float cutoffHz = 100.0f * std::exp(5.065687f * toneVal); // 100 Hz to 15,850 Hz
            float toneAlpha = 2.0f * std::sin(M_PI * std::clamp(cutoffHz, 60.0f, sampleRate * 0.45f) / sampleRate);
            toneAlpha = std::clamp(toneAlpha, 0.005f, 1.0f);
            toneFilterBuf += toneAlpha * (waveSig - toneFilterBuf);
            waveSig = toneFilterBuf;

            kickOut = waveSig * envAmp;
        }

        float out = kickOut;

        // Drive Saturation
        if (drive.value > 0.0f) {
            out = applyDrive(out, (drive.value * 0.01f) * 3.5f);
        }
        out = applyCompression2(out, 0.65f, compressionEnv);

        // 3-Band Equalizer
        if (eqLow.value != eq.gainDb[0] || eqMid.value != eq.gainDb[1]) {
            eq.gainDb[0] = eqLow.value;
            eq.gainDb[1] = eqMid.value;
            eq.gainDb[2] = 0.0f;
            eq.recompute(sampleRate);
        }

        if (std::abs(eqLow.value) > 0.01f || std::abs(eqMid.value) > 0.01f) {
            out = eq.process(out);
        }

        // Kick Transient Click & Transient Bite
        if (clickEnvelope > 0.0001f) {
            float clickDecaySec = std::clamp(kickClickDecay.value * 0.001f, 0.001f, 0.200f);
            clickEnvelope *= std::exp(-1.0f / (sampleRate * clickDecaySec));

            float biteVal = (transientBite.value * 0.01f);
            float clickSig = (nextNoise() * 0.7f + (clickEnvelope > 0.6f ? 0.9f : 0.0f)) * clickEnvelope * ((kickClickAmt.value * 0.01f) + biteVal * 1.5f);
            out += clickSig;
        }

        return out * velocity;
    }
};
