#pragma once

#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/engines/EngineBase.h"
#include "audio/filterSVF.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include "helpers/clamp.h"

#include <algorithm>
#include <cmath>

class DrumGeneric : public EngineBase<DrumGeneric> {

protected:
    const float sampleRate;

    // --- Global Reverb Routing ---
    float* reverbBuffer = nullptr;
    int reverbIndex = 0;

    // --- Global Filtering Node ---
    FilterSVF svfFilter;

    // --- Voice Lifecycles ---
    float velocity = 1.0f;
    float ampEnv = 0.0f;
    float ampStep = 0.0f;

    // --- Tonal Body Internal Registers ---
    float bodyEnvelope = 0.0f;
    float tonalPhase = 0.0f;
    float ringPhase = 0.0f;

    // --- Hi-Hat Layer Internal Registers ---
    float osc1Phases[6] = {};
    float osc2Phases[6] = {};
    float bp1Lp = 0.0f, bp1Bp = 0.0f;
    float bp2Lp = 0.0f, bp2Bp = 0.0f;
    float hatDcState = 0.0f;

    // --- Clap Layer Internal Registers ---
    float clapTime = 0.0f;
    float burstTimer = 0.0f;
    int burstIndex = 0;
    float burstEnv = 0.0f;
    float pinkNoiseState = 0.0f;
    float clapLpState = 0.0f;

    // Biquad Bandpass Coeffs for Clap Coloration
    float b0_a0 = 0.0f, b1_a0 = 0.0f, b2_a0 = 0.0f;
    float a1_a0 = 0.0f, a2_a0 = 0.0f;
    float biquadGainComp = 1.0f;
    float bp_x1 = 0.0f, bp_x2 = 0.0f;
    float bp_y1 = 0.0f, bp_y2 = 0.0f;

    // --- Inline Struct Helpers ---
    static float lerp(float a, float b, float t) { return a + t * (b - a); }
    static float pct(const Param& p) { return p.value * 0.01f; }

    void updateClapBiquad()
    {
        // Frequency target moves linearly from 1500Hz up to 3000Hz based on color knob
        float f0 = 1500.0f + pct(clapNoiseClr) * 1500.0f;
        float Q = 0.8f;
        float omega = 2.0f * (float)M_PI * f0 / sampleRate;
        float s = Math::sin(omega);
        float c = Math::cos(omega);
        float alpha = s / (2.0f * Q);
        float a0 = 1.0f + alpha;
        float inv = 1.0f / a0;

        b0_a0 = alpha * inv;
        b1_a0 = 0.0f;
        b2_a0 = -alpha * inv;
        a1_a0 = (-2.0f * c) * inv;
        a2_a0 = (1.0f - alpha) * inv;
        biquadGainComp = 1.0f + Q;
    }

    float applyClapBiquadBP(float in)
    {
        float y = b0_a0 * in + b1_a0 * bp_x1 + b2_a0 * bp_x2
            - a1_a0 * bp_y1 - a2_a0 * bp_y2;
        bp_x2 = bp_x1;
        bp_x1 = in;
        bp_y2 = bp_y1;
        bp_y1 = y;
        return y * biquadGainComp;
    }

    float applyMorphFilter(float sig, float cutoffParam, float res)
    {
        float absC = std::abs(cutoffParam);
        if (absC < 0.5f) return sig;

        float normCutoff = CLAMP(0.01f + absC * 0.0098f, 0.01f, 0.99f);
        float normRes = CLAMP(res, 0.0f, 0.98f);
        float t = CLAMP((cutoffParam + 100.0f) * 0.005f, 0.0f, 1.0f); // Map [-100, 100] -> [0 (LP), 1 (HP)]

        svfFilter.setCutoff(normCutoff);
        svfFilter.setResonance(normRes);
        auto out = svfFilter.process12(sig);
        float filtered = lerp(out.lp, out.hp, t);

        return lerp(sig, filtered, absC * 0.01f);
    }

public:
    enum ParamTarget { GLOBAL,
        TONAL,
        NOISE };

    Param params[21];

    // --- Dynamic Macro Allocation Mapping Structures ---
    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 300.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f, .target = GLOBAL });
    Param& mixBlend = addParam({ .key = "mixBlend", .label = "Body/Noise", .unit = "%", .value = 50.0f, .target = GLOBAL }); // 0% = Pure Body Tone, 100% = Pure Noise Layer

    // --- Tonal Core Group ---
    Param& baseFrequency = addParam({ .key = "baseFreq", .label = "Body Freq", .unit = "Hz", .value = 150.0f, .min = 50.0f, .max = 500.0f, .target = TONAL });
    Param& bodyDecay = addParam({ .key = "bodyDec", .label = "Body Decay", .unit = "%", .value = 40.0f, .target = TONAL });
    Param& ringAmount = addParam({ .key = "ringAmt", .label = "Body Ring", .unit = "%", .value = 10.0f, .target = TONAL });
    Param& bodyShape = addParam({ .key = "bodyShape", .label = "Body Shape", .unit = "%", .value = 5.0f, .target = TONAL });
    Param& bodyBend = addParam({ .key = "bodyBend", .label = "Body Bend", .unit = "%", .value = 30.0f, .target = TONAL });
    Param& bendShape = addParam({ .key = "bendShape", .label = "Bend Shape", .unit = "%", .value = 10.0f, .target = TONAL });

    // --- Noise & Transient Hybrid Matrix Core ---
    Param& character = addParam({ .key = "character", .label = "Hat / Clap", .unit = "%", .value = 50.0f, .target = NOISE }); // 0% = Hat, 100% = Clap
    Param& noiseDecay = addParam({ .key = "noiseDec", .label = "Noise Decay", .unit = "%", .value = 35.0f, .target = NOISE });

    // Clap-Specific Parameters
    Param& clapNoiseClr = addParam({ .key = "clapNoise", .label = "Clap Color", .unit = "%", .value = 70.0f, .target = NOISE, .onUpdate = [](void* ctx, float) { static_cast<DrumGeneric*>(ctx)->updateClapBiquad(); } }, false);
    Param& clapPunch = addParam({ .key = "clapPunch", .label = "Clap Punch", .unit = "%", .value = 40.0f, .target = NOISE });
    Param& clapTrans = addParam({ .key = "clapTrans", .label = "Clap Click", .unit = "%", .value = 15.0f, .target = NOISE });

    // HiHat-Specific Parameters
    Param& hiInharmonic = addParam({ .key = "hiInharm", .label = "Hat Inharm", .unit = "%", .value = 40.0f, .target = NOISE });
    Param& hiDetune = addParam({ .key = "hiDetune", .label = "Hat Detune", .unit = "%", .value = 20.0f, .target = NOISE });
    Param& hiFmAmt = addParam({ .key = "hiFmAmt", .label = "Hat FM Amt", .unit = "%", .value = 25.0f, .target = NOISE });
    Param& hiTightness = addParam({ .key = "hiTight", .label = "Hat Tight", .unit = "%", .value = 40.0f, .target = NOISE });

    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 10.0f, .target = GLOBAL });
    Param& cutoff = addParam({ .key = "cutoff", .label = "Cutoff", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .target = GLOBAL });
    Param& resonance = addParam({ .key = "resonance", .label = "Resonance", .unit = "%", .value = 0.0f, .target = GLOBAL });
    Param& reverb = addParam({ .key = "reverb", .label = "Reverb", .unit = "%", .value = 0.0f, .target = GLOBAL });

    DrumGeneric(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "Drum", params)
        , sampleRate(sampleRate)
        , reverbBuffer(rvBuffer)
    {
        updateClapBiquad();
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        ampEnv = 1.0f;

        float masterDurSamples = std::max(1.0f, sampleRate * (duration.value * 0.001f));
        ampStep = 1.0f / masterDurSamples;

        // Reset Tonal elements
        bodyEnvelope = 1.0f;
        tonalPhase = 0.0f;
        ringPhase = 0.0f;

        // Reset HiHat internal spaces
        bp1Lp = bp1Bp = bp2Lp = bp2Bp = hatDcState = 0.0f;
        for (int i = 0; i < 6; ++i) {
            osc1Phases[i] = 0.0f;
            osc2Phases[i] = 0.0f;
        }

        // Reset Clap internal spaces
        clapTime = 0.0f;
        burstTimer = 0.0f;
        burstIndex = 0;
        burstEnv = 1.0f;
        pinkNoiseState = 0.0f;
        clapLpState = 0.0f;
        bp_x1 = bp_x2 = bp_y1 = bp_y2 = 0.0f;
    }

    float sampleImpl()
    {
        if (ampEnv <= 0.001f) return applyRvb(0.0f);

        float currentAmp = ampEnv;
        ampEnv -= ampStep;

        // ─────────────────────────────────────────────────────────────────────
        // 1. COMPUTE INTERNAL ENVELOPES
        // ─────────────────────────────────────────────────────────────────────
        // Tonal Envelope decay scaling
        float bodyTimeSec = 0.005f + (bodyDecay.value * 0.002f);
        bodyEnvelope *= Math::exp(-1.0f / (sampleRate * bodyTimeSec));

        // Noise Envelope linear execution trace context
        float noiseTimeSec = 0.005f + (noiseDecay.value * 0.005f);
        float noiseEnvDecrease = 1.0f / (sampleRate * noiseTimeSec);
        float currentNoiseEnv = std::max(0.0f, 1.0f - (sampleCounter() * noiseEnvDecrease));

        // ─────────────────────────────────────────────────────────────────────
        // 2. TONAL CORE LAYER
        // ─────────────────────────────────────────────────────────────────────
        float bendDepthHz = bodyBend.value * 5.0f;
        float bendExponent = 1.0f + bendShape.value * 0.06f;
        float bodyShapedEnv = Math::pow(bodyEnvelope, bendExponent);
        float operationalFundFreq = baseFrequency.value + (bodyShapedEnv * bendDepthHz);

        tonalPhase += operationalFundFreq / sampleRate;
        if (tonalPhase > 1.0f) tonalPhase -= 1.0f;
        float fundamentalOsc = Math::sin(PI_X2 * tonalPhase);

        if (bodyShape.value > 0.0f) {
            float shapeValue = 100.0f - bodyShape.value;
            fundamentalOsc = CLAMP(fundamentalOsc * shapeValue * 0.02f, -1.0f, 1.0f);
            fundamentalOsc *= (1.0f - (shapeValue * 0.003f));
        }

        ringPhase += (operationalFundFreq * 1.61f) / sampleRate;
        if (ringPhase > 1.0f) ringPhase -= 1.0f;
        float bodyRingOsc = Math::sin(PI_X2 * ringPhase) * ringAmount.value * 0.01f;

        float totalTonalPart = (fundamentalOsc + bodyRingOsc) * bodyEnvelope;

        // ─────────────────────────────────────────────────────────────────────
        // 3. NOISE / TRANSIENT GENERATION MATRIX
        // ─────────────────────────────────────────────────────────────────────
        float noiseBlendFactor = pct(character); // 0 = Hat, 1 = Clap
        float combinedNoiseLayerSignal = 0.0f;

        // A. Hi-Hat Component Logic
        if (noiseBlendFactor < 0.999f) {
            static const float metallicRatios[6] = { 1.0f, 1.413f, 1.854f, 2.278f, 3.014f, 4.127f };
            float referenceMetalFreq = 3500.0f;
            float inharmonicSpread = hiInharmonic.value * 12.0f;
            float detuningOffsetHz = referenceMetalFreq * hiDetune.value * 0.0005f;
            float fmIndexFactor = hiFmAmt.value * 0.04f;

            float metallicSum1 = 0.0f, metallicSum2 = 0.0f;
            float feedbackTrace1 = 0.0f, feedbackTrace2 = 0.0f;

            for (int i = 0; i < 6; ++i) {
                float targetFreq1 = referenceMetalFreq * metallicRatios[i] + i * inharmonicSpread;
                targetFreq1 = std::min(targetFreq1, sampleRate * 0.47f);
                osc1Phases[i] += targetFreq1 / sampleRate + feedbackTrace1 * fmIndexFactor;
                if (osc1Phases[i] > 1.0f) osc1Phases[i] -= 1.0f;
                feedbackTrace1 = osc1Phases[i] > 0.5f ? 1.0f : -1.0f;
                metallicSum1 += (i % 2 == 0) ? feedbackTrace1 : -feedbackTrace1 * 0.8f;

                float targetFreq2 = (referenceMetalFreq + detuningOffsetHz) * metallicRatios[i] + i * inharmonicSpread;
                targetFreq2 = std::min(targetFreq2, sampleRate * 0.47f);
                osc2Phases[i] += targetFreq2 / sampleRate + feedbackTrace2 * fmIndexFactor;
                if (osc2Phases[i] > 1.0f) osc2Phases[i] -= 1.0f;
                feedbackTrace2 = osc2Phases[i] > 0.5f ? 1.0f : -1.0f;
                metallicSum2 += (i % 2 == 0) ? feedbackTrace2 : -feedbackTrace2 * 0.8f;
            }

            float primaryHatNoise = (metallicSum1 + metallicSum2) * (1.0f / 12.0f);

            // Fixed internal metallic-to-white noise blend (25%) to preserve hat sharpness
            primaryHatNoise = primaryHatNoise * 0.75f + Noise::sample() * 0.25f;

            // Cascaded Fixed SVF Bandpass filtering tailored for High-Hats (~7kHz focus)
            {
                float qCoeff = 0.58f;
                float feedbackFactor = 1.0f / qCoeff;
                float cutoffF1 = std::min(2.0f * Math::sin((float)M_PI * 7000.0f / sampleRate), 0.49f);
                float highpassNode1 = primaryHatNoise - bp1Bp * feedbackFactor - bp1Lp;
                bp1Bp = std::clamp(bp1Bp + cutoffF1 * highpassNode1, -4.0f, 4.0f);
                bp1Lp = std::clamp(bp1Lp + cutoffF1 * bp1Bp, -4.0f, 4.0f);

                float cutoffF2 = std::min(2.0f * Math::sin((float)M_PI * 8540.0f / sampleRate), 0.49f);
                float highpassNode2 = primaryHatNoise - bp2Bp * feedbackFactor - bp2Lp;
                bp2Bp = std::clamp(bp2Bp + cutoffF2 * highpassNode2, -4.0f, 4.0f);
                bp2Lp = std::clamp(bp2Lp + cutoffF2 * bp2Bp, -4.0f, 4.0f);
                primaryHatNoise = primaryHatNoise * 0.3f + (bp1Bp + bp2Bp) * 0.7f * 0.5f;
            }

            // Fixed High-Pass Decoupler
            hatDcState += 0.45f * (primaryHatNoise - hatDcState);
            primaryHatNoise = (primaryHatNoise - hatDcState) * 1.4f;

            // Amplitude shaping based on hat tightly closed modifiers
            float tightnessPow = 1.0f + hiTightness.value * 0.03f;
            primaryHatNoise *= Math::pow(currentNoiseEnv, tightnessPow);

            combinedNoiseLayerSignal += primaryHatNoise * (1.0f - noiseBlendFactor);
        }

        // B. Handclap Component Logic
        if (noiseBlendFactor > 0.001f) {
            float clapVoiceOutput = 0.0f;
            clapTime += 1.0f / sampleRate;

            // Fixed internal spacing settings (30ms spacing, 35ms burst decay durations)
            float fixedSpacingSec = 0.019f;
            float fixedDecaySec = 0.025f;

            // Execute 5 discrete cluster micro-burst triggers
            if (burstIndex < 5) {
                burstTimer += 1.0f / sampleRate;
                if (burstTimer >= fixedSpacingSec) {
                    burstTimer -= fixedSpacingSec;
                    burstIndex++;
                    burstEnv = 1.0f;
                }
            }

            if (burstEnv > 0.0f) {
                float whiteNoiseVal = Noise::sample();
                pinkNoiseState = 0.98f * pinkNoiseState + 0.02f * whiteNoiseVal;
                float filteredNoiseMix = pinkNoiseState * (1.0f - pct(clapNoiseClr)) + whiteNoiseVal * pct(clapNoiseClr);
                clapVoiceOutput += filteredNoiseMix * burstEnv;
                burstEnv *= Math::exp(-1.0f / (sampleRate * fixedDecaySec));
            }

            clapVoiceOutput = applyClapBiquadBP(clapVoiceOutput);

            // Punch processing block
            float punchFactor = pct(clapPunch);
            if (clapTime < 0.02f) {
                float trackingHighpassed = clapVoiceOutput - clapLpState;
                clapLpState += 0.01f * (clapVoiceOutput - clapLpState);
                clapVoiceOutput += trackingHighpassed * punchFactor * 2.0f;
            }

            // Transient Click Injector
            if (clapTime < 0.001f) {
                clapVoiceOutput += Noise::sample() * pct(clapTrans) * 5.0f;
            }

            // Scale against main structural decaying noise window
            clapVoiceOutput *= currentNoiseEnv;

            combinedNoiseLayerSignal += clapVoiceOutput * noiseBlendFactor;
        }

        // ─────────────────────────────────────────────────────────────────────
        // 4. MAIN SUMMING MATRIX AND DOWNSTREAM PROCESSORS
        // ─────────────────────────────────────────────────────────────────────
        float crossfadeBalance = pct(mixBlend); // 0 = Body Only, 1 = Noise Only
        float mixedOutputSignal = (totalTonalPart * 0.6f * (1.0f - crossfadeBalance)) + (combinedNoiseLayerSignal * 0.7f * crossfadeBalance);

        mixedOutputSignal *= currentAmp; // Apply master cross-duration window boundaries

        // Apply Saturating Overdrive Processor Node
        if (drive.value > 0.0f) {
            mixedOutputSignal = applyDrive(mixedOutputSignal, pct(drive) * 4.0f);
        }

        // Apply Central Multi-Mode Filtering Unit
        mixedOutputSignal = CLAMP(mixedOutputSignal, -1.0f, 1.0f);
        mixedOutputSignal = applyMorphFilter(mixedOutputSignal, cutoff.value, resonance.value * 0.01f);

        // Commit execution vector calculations to algorithmic room reflections
        return applyRvb(mixedOutputSignal * velocity);
    }

    float applyRvb(float out)
    {
        return applyReverb(out, reverb.value * 0.01f, reverbBuffer, reverbIndex);
    }

private:
    // Tracking active runtime context duration frame iteration increments
    int sampleCounter() const
    {
        return (int)((1.0f - ampEnv) / ampStep);
    }
};