#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include <algorithm>
#include <cmath>

class KickBody : public EngineBase<KickBody> {
public:
    EnvelopDrumAmp envelopAmp;

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float feedbackState = 0.0f;
    float modulationEnvelope = 0.0f;
    float lowPassState = 0.0f;

public:
    Param params[9];

    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 450.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .unit = "Hz", .value = 52.0f, .min = 30.0f, .max = 100.0f, .step = 1.0f });
    Param& punch = addParam({ .key = "punch", .label = "Punch", .unit = "%", .value = 50.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });

    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 40.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .unit = "x", .value = 1.0f, .min = 0.5f, .max = 8.0f, .step = 0.1f });
    Param& fmDecay = addParam({ .key = "fmDecay", .label = "FM Speed", .unit = "%", .value = 25.0f, .min = 1.0f, .max = 100.0f, .step = 2.0f });
    Param& fmGrit = addParam({ .key = "fmGrit", .label = "FM Grit", .unit = "%", .value = 20.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });

    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });
    Param& tone = addParam({ .key = "tone", .label = "Tone", .unit = "%", .value = 75.0f, .min = 0.0f, .max = 100.0f, .step = 2.0f });

    KickBody(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "KickFM", params)
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
        carrierPhase = 0.0f;
        modulatorPhase = 0.0f;
        feedbackState = 0.0f;
        modulationEnvelope = 1.0f;
        lowPassState = 0.0f;

        int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.0001f) return 0.0f;

        // 1. Envelope Decay for Punch & FM
        float fmDecayTime = 0.002f + (pct(fmDecay) * 0.12f);
        modulationEnvelope *= Math::exp(-1.0f / (sampleRate * fmDecayTime));

        // 2. Base Pitch Sweep (Punch)
        float rootFreq = baseFreq.value;
        float pitchSpike = (pct(punch) * 260.0f * modulationEnvelope);
        float carrierFreq = rootFreq + pitchSpike;

        // Modulator frequency anchored to fundamental pitch to prevent high-frequency aliasing noise
        float modulatorFreq = rootFreq * fmRatio.value;

        // 3. Modulator with Tanh-bounded Feedback (Prevents silence over 70% Grit)
        float feedbackAmt = pct(fmGrit) * 0.25f;
        float modPhaseLookup = modulatorPhase + (feedbackState * feedbackAmt);
        float modulatorSignal = Math::fastSin2(PI_X2 * modPhaseLookup);
        feedbackState = std::tanh(modulatorSignal); // Soft clip feedback loop

        modulatorPhase += modulatorFreq / sampleRate;
        if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;

        // 4. Carrier Phase Modulation
        float fmIntensity = pct(fmDepth) * 0.75f * modulationEnvelope;
        carrierPhase += (carrierFreq / sampleRate) + (modulatorSignal * fmIntensity * 0.04f);
        if (carrierPhase > 1.0f) carrierPhase -= 1.0f;

        float sig = Math::fastSin2(PI_X2 * carrierPhase);

        // 5. Overdrive Saturation
        if (drive.value > 0.0f) {
            sig = applyDrive(sig, pct(drive) * 3.0f);
        }

        // 6. LPF Tone Filter
        float filterCut = 0.05f + pct(tone) * 0.85f;
        lowPassState += filterCut * (sig - lowPassState);
        sig = lowPassState;

        return sig * envAmp * velocity;
    }
};
