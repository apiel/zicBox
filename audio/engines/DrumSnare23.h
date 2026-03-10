#pragma once

#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <algorithm>
#include <cmath>

class DrumSnare23 : public EngineBase<DrumSnare23> {

protected:
    const float sampleRate;

    float* reverbBuffer = nullptr;
    int reverbIndex = 0;

    float velocity = 1.0f;
    float tonalPhase = 0.0f;
    float ringPhase = 0.0f;
    float bodyEnvelope = 0.0f;
    float noiseEnvelope = 0.0f;
    float lowPassState = 0.0f;
    float noiseHpState = 0.0f;
    int sampleCounter = 0;
    float ampEnv = 0.0f;
    float ampStep = 0.0f;
    float sampleHoldState = 0.0f;
    int sampleHoldCounter = 0;
    float oscillatorPhases[6] = { 0.0f };

    float resLp1 = 0.0f, resBp1 = 0.0f;
    float resLp2 = 0.0f, resBp2 = 0.0f;

public:
    Param params[22] = {
        { .label = "Duration", .unit = "ms", .value = 400.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f },
        { .label = "Body Freq", .unit = "Hz", .value = 180.0f, .min = 100.0f, .max = 400.0f },
        { .label = "Body", .unit = "%", .value = 30.0f },
        { .label = "Body Ring", .unit = "%", .value = 25.0f },
        { .label = "Body Shape", .unit = "%", .value = 0.0f },
        { .label = "Body Bend", .unit = "%", .value = 20.0f },
        { .label = "Bend Shape", .unit = "%", .value = 0.0f },
        { .label = "Snappy", .unit = "%", .value = 50.0f },
        { .label = "Snap Tail", .unit = "%", .value = 40.0f },
        { .label = "Snap Tone", .unit = "%", .value = 50.0f },
        { .label = "Metal Ring", .unit = "%", .value = 0.0f },
        { .label = "Metal Freq", .unit = "Hz", .value = 400.0f, .min = 100.0f, .max = 2000.0f },
        { .label = "Metal FM", .unit = "%", .value = 0.0f },
        { .label = "Noise-Metal", .unit = "%", .value = 50.0f },
        { .label = "Impact", .unit = "%", .value = 30.0f },
        { .label = "Drive", .unit = "%", .value = 15.0f },
        { .label = "Tightness", .unit = "%", .value = 50.0f },
        { .label = "LPF", .unit = "%", .value = 100.0f },
        { .label = "Grit", .unit = "%", .value = 0.0f },
        { .label = "Res Freq", .unit = "Hz", .value = 3000.0f, .min = 500.0f, .max = 10000.0f, .step = 100.0f },
        { .label = "Resonance", .unit = "%", .value = 0.0f },
        { .label = "Reverb", .unit = "%", .value = 0.0f },
    };

    // ... (Reference bindings remain the same) ...
    Param& duration = params[0];
    Param& baseFrequency = params[1];
    Param& bodyDecay = params[2];
    Param& ringAmount = params[3];
    Param& bodyShape = params[4];
    Param& bodyBend = params[5];
    Param& bendShape = params[6];
    Param& snappyLevel = params[7];
    Param& snappyDecay = params[8];
    Param& snapTone = params[9];
    Param& metalRing = params[10];
    Param& metalFreq = params[11];
    Param& metalFm = params[12];
    Param& noiseMix = params[13];
    Param& impact = params[14];
    Param& drive = params[15];
    Param& tightness = params[16];
    Param& lpf = params[17];
    Param& grit = params[18];
    Param& resFreq = params[19];
    Param& resonance = params[20];
    Param& reverb = params[21];

    DrumSnare23(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "SnareHat", params)
        , sampleRate(sampleRate)
        , reverbBuffer(rvBuffer)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        tonalPhase = 0.0f;
        ringPhase = 0.0f;
        bodyEnvelope = 1.0f;
        noiseEnvelope = 1.0f;
        lowPassState = 0.0f;
        noiseHpState = 0.0f;
        sampleCounter = 0;
        resLp1 = resLp2 = resBp1 = resBp2 = 0.0f;
        ampEnv = 1.0f;
        float durSamples = std::max(1.0f, sampleRate * (duration.value * 0.001f));
        ampStep = 1.0f / durSamples;
    }

    float sampleImpl()
    {
        if (ampEnv < 0.001f) return applyRvb(0.0f);
        float currentAmp = ampEnv;
        ampEnv -= ampStep;

        // 1. Envelopes
        float bodyTime = 0.005f + (bodyDecay.value * 0.002f);
        bodyEnvelope *= Math::exp(-1.0f / (sampleRate * bodyTime));

        float noiseTime = 0.01f + (snappyDecay.value * 0.005f);
        noiseEnvelope *= Math::exp(-1.0f / (sampleRate * noiseTime));

        // 2. Tonal Part
        float bendDepth = bodyBend.value * 5.0f;
        float exponent = 1.0f + bendShape.value * 0.06f;
        float shapedEnv = Math::pow(bodyEnvelope, exponent);
        float fundFreq = baseFrequency.value + (shapedEnv * bendDepth);

        tonalPhase += fundFreq / sampleRate;
        if (tonalPhase > 1.0f) tonalPhase -= 1.0f;

        float fundamental = Math::sin(PI_X2 * tonalPhase);

        // // We push the gain of the sine wave and clip it to create a square-like shape
        // // At 0% morph, it's a pure sine. At 100%, it's quite square.
        // float driveAmount = 1.0f + (bodyShape.value * 0.01f);
        // fundamental = std::clamp(fundamental * driveAmount, -1.0f, 1.0f);
        // // Level compensation: Pure squares are louder than sines
        // fundamental *= (1.0f - (bodyShape.value * 0.003f));

        if (bodyShape.value > 0.0f) {
            fundamental = applyWaveshape2(fundamental, bodyShape.value * 0.01f);
            fundamental *= (1.0f - (bodyShape.value * 0.003f));
        }

        ringPhase += (fundFreq * 1.61f) / sampleRate;
        if (ringPhase > 1.0f) ringPhase -= 1.0f;
        float ring = Math::sin(PI_X2 * ringPhase) * ringAmount.value * 0.01f;

        float tonalPart = (fundamental + ring) * bodyEnvelope;

        // 3. Metallic Oscillator Bank
        float ratios[6] = { 1.0f, 1.523f, 1.965f, 2.381f, 3.121f, 4.451f };
        float metalMix = 0.0f;
        float inharmonicity = metalRing.value * 5.0f;
        float fmIntensity = (1.0f - metalFm.value * 0.01f) * 2.0f;
        float lastSig = 0.0f;

        for (int i = 0; i < 6; ++i) {
            float freq = (metalFreq.value * ratios[i]) + (i * inharmonicity);
            float fmOffset = (lastSig * fmIntensity);
            oscillatorPhases[i] += (freq / sampleRate) + (fmOffset * 0.01f);
            if (oscillatorPhases[i] > 1.0f) oscillatorPhases[i] -= 1.0f;

            float sig = oscillatorPhases[i] > 0.5f ? 1.0f : -1.0f;
            lastSig = sig;
            if (i % 2 == 0) metalMix += sig;
            else metalMix *= sig;
        }

        // 4. Resonator
        if (resonance.value > 0.0f) {
            float q = 0.5f + resonance.value * 0.175f;
            float fb = 1.0f / q;
            float f1 = std::min(2.0f * Math::sin(M_PI * resFreq.value / sampleRate), 0.49f);
            float hp1 = metalMix - resBp1 * fb - resLp1;
            resBp1 = std::clamp(resBp1 + f1 * hp1, -4.0f, 4.0f);
            resLp1 = std::clamp(resLp1 + f1 * resBp1, -4.0f, 4.0f);

            float f2 = std::min(2.0f * Math::sin(M_PI * resFreq.value * 1.17f / sampleRate), 0.49f);
            float hp2 = metalMix - resBp2 * fb - resLp2;
            resBp2 = std::clamp(resBp2 + f2 * hp2, -4.0f, 4.0f);
            resLp2 = std::clamp(resLp2 + f2 * resBp2, -4.0f, 4.0f);

            float wet = resonance.value * 0.01f;
            metalMix = metalMix * (1.0f - wet) + (resBp1 + resBp2) * wet * 0.5f;
        }

        // 5. Noise + metalMix blend
        float rawNoise = Noise::sample();
        float noise = metalMix * noiseMix.value * 0.01f
            + rawNoise * (1.0f - noiseMix.value * 0.01f);

        float hpCutoff = 0.01f + (snapTone.value * 0.006f);
        noiseHpState += hpCutoff * (noise - noiseHpState);
        float snappyPart = (noise - noiseHpState) * noiseEnvelope * pct(snappyLevel);

        // 6. Impact Transient
        float impactPart = 0.0f;
        if (sampleCounter < 200) {
            impactPart = noise * pct(impact) * 1.5f;
        }

        // 7. Mix and effects
        float mixedSignal = (tonalPart * 0.5f) + (snappyPart * 0.8f) + impactPart;

        if (drive.value > 0.0f) {
            mixedSignal = applyDrive(mixedSignal, pct(drive) * 4.0f);
        }

        if (grit.value > 0.0f) {
            int holdSamples = (int)(grit.value * 0.4f);
            if (++sampleHoldCounter >= holdSamples) {
                sampleHoldState = mixedSignal;
                sampleHoldCounter = 0;
            }
            mixedSignal = sampleHoldState;
        }

        float masterCutoff = 0.05f + lpf.value * 0.0094f;
        lowPassState += masterCutoff * (mixedSignal - lowPassState);
        mixedSignal = lowPassState;

        float tightFactor = Math::pow(currentAmp, 1.0f + pct(tightness) * 4.0f);

        sampleCounter++;
        return applyRvb(mixedSignal * tightFactor * velocity);
    }

    float applyRvb(float out)
    {
        return applyReverb(out, reverb.value * 0.01f, reverbBuffer, reverbIndex);
    }
};