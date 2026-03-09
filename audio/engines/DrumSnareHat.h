#pragma once

#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <algorithm>
#include <cmath>

class DrumSnareHat : public EngineBase<DrumSnareHat> {

protected:
    const float sampleRate;

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

public:
    Param params[17] = {
        { .label = "Duration", .unit = "ms", .value = 400.0f, .min = 10.0f, .max = 2000.0f, .step = 10.0f },
        { .label = "Body Freq", .unit = "Hz", .value = 180.0f, .min = 100.0f, .max = 400.0f },
        { .label = "Body", .unit = "%", .value = 30.0f },
        { .label = "Ring", .unit = "%", .value = 25.0f },
        { .label = "Snappy", .unit = "%", .value = 50.0f },
        { .label = "Snap Tail", .unit = "%", .value = 40.0f },
        { .label = "Snap Tone", .unit = "%", .value = 50.0f },
        { .label = "Punch", .unit = "%", .value = 20.0f },
        { .label = "Impact", .unit = "%", .value = 30.0f },
        { .label = "Drive", .unit = "%", .value = 15.0f },
        { .label = "Tightness", .unit = "%", .value = 50.0f },
        { .label = "LPF", .unit = "%", .value = 100.0f },
        { .label = "Grit", .unit = "%", .value = 0.0f },
        { .label = "Metal Ring", .unit = "%", .value = 0.0f },
        { .label = "Metal Freq", .unit = "Hz", .value = 400.0f, .min = 100.0f, .max = 400.0f },
        { .label = "Metal FM", .unit = "%", .value = 0.0f },
        { .label = "Noise - Metal", .unit = "%", .value = 50.0f }
    };

    Param& duration = params[0];
    Param& baseFrequency = params[1];
    Param& bodyDecay = params[2];
    Param& ringAmount = params[3];
    Param& snappyLevel = params[4];
    Param& snappyDecay = params[5];
    Param& snapTone = params[6];
    Param& pitchDrop = params[7];
    Param& impact = params[8];
    Param& drive = params[9];
    Param& tightness = params[10];
    Param& lpf = params[11];
    Param& grit = params[12];
    Param& metalRing = params[13];
    Param& metalFreq = params[14];
    Param& metalFm = params[15];
    Param& noiseMix = params[16];

    DrumSnareHat(const float sampleRate)
        : EngineBase(Drum, "SnareHat", params)
        , sampleRate(sampleRate)
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

        ampEnv = 1.0f;
        float durSamples = std::max(1.0f, sampleRate * (duration.value * 0.001f));
        ampStep = 1.0f / durSamples;
    }

    float sampleImpl()
    {
        if (ampEnv <= 0.0f) return 0.0f;
        float currentAmp = ampEnv;
        ampEnv -= ampStep;

        // 1. Envelopes
        // Body time scaled by duration to keep it relative
        float bodyTime = 0.005f + (pct(bodyDecay) * 0.2f);
        bodyEnvelope *= Math::exp(-1.0f / (sampleRate * bodyTime));

        float noiseTime = 0.01f + (pct(snappyDecay) * 0.5f);
        noiseEnvelope *= Math::exp(-1.0f / (sampleRate * noiseTime));

        // 2. Tonal Part (Fundamental + Harmonic Ring)
        float pitchEnv = bodyEnvelope * pct(pitchDrop) * 150.0f;
        float fundFreq = baseFrequency.value + pitchEnv;

        // Primary hit
        tonalPhase += fundFreq / sampleRate;
        if (tonalPhase > 1.0f) tonalPhase -= 1.0f;
        float fundamental = Math::sin(PI_X2 * tonalPhase);

        // Secondary "Ring" (approx 1.6x fundamental for metallic resonance)
        ringPhase += (fundFreq * 1.61f) / sampleRate;
        if (ringPhase > 1.0f) ringPhase -= 1.0f;
        float ring = Math::sin(PI_X2 * ringPhase) * pct(ringAmount);

        float tonalPart = (fundamental + ring) * bodyEnvelope;

        // 3. Noise Part (The "Wires")
        float rawNoise = Noise::sample();

        // 3.2. Metallic Oscillator Bank with FM
        float ratios[6] = { 1.0f, 1.523f, 1.965f, 2.381f, 3.121f, 4.451f };
        float metalMix = 0.0f;
        float inharmonicity = metalRing.value * 5.0f;
        
        float fmIntensity = (1.0f - metalFm.value * 0.01f) * 2.0f;
        float lastSig = 0.0f; // Used for cross-modulation

        for (int i = 0; i < 6; ++i) {
            float freq = (metalFreq.value * ratios[i]) + (i * inharmonicity);
            
            // The FM Magic: Each oscillator's phase is pushed by the previous oscillator
            float fmOffset = (lastSig * fmIntensity);
            oscillatorPhases[i] += (freq / sampleRate) + (fmOffset * 0.01f);
            
            if (oscillatorPhases[i] > 1.0f) oscillatorPhases[i] -= 1.0f;

            float sig = oscillatorPhases[i] > 0.5f ? 1.0f : -1.0f;
            lastSig = sig; // Feed this into the next oscillator in the loop

            if (i % 2 == 0) metalMix += sig;
            else metalMix *= sig;
        }

        float noise = metalMix * noiseMix.value * 0.01f + rawNoise * (1.0f - noiseMix.value * 0.01f);

        // High pass logic
        float hpCutoff = 0.01f + (snapTone.value * 0.006f);
        noiseHpState += hpCutoff * (noise - noiseHpState);
        float snappyPart = (noise - noiseHpState) * noiseEnvelope * pct(snappyLevel);

        // 4. Impact Transient
        float impactPart = 0.0f;
        if (sampleCounter < 200) { // Slightly longer transient window (~4ms)
            impactPart = noise * pct(impact) * 1.5f;
        }

        // 5. Mix and Saturation
        float mixedSignal = (tonalPart * 0.5f) + (snappyPart * 0.8f) + impactPart;

        if (drive.value > 0.0f) {
            mixedSignal = applyDrive(mixedSignal, pct(drive) * 4.0f);
        }

        // Sample Reduction
        if (grit.value > 0.0f) {
            int holdSamples = (int)(grit.value * 0.4f);
            if (++sampleHoldCounter >= holdSamples) {
                sampleHoldState = mixedSignal;
                sampleHoldCounter = 0;
            }
            mixedSignal = sampleHoldState;
        }

        // Master Tone (Low Pass Filter)
        float masterCutoff = 0.05f + lpf.value * 0.0094f;
        lowPassState += masterCutoff * (mixedSignal - lowPassState);
        mixedSignal = lowPassState;

        // 6. Tightness & Velocity
        // Higher tightness makes the decay curve more "concave"
        float tightFactor = Math::pow(currentAmp, 1.0f + pct(tightness) * 4.0f);

        sampleCounter++;
        return mixedSignal * tightFactor * velocity;
    }
};