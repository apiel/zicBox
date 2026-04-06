#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include <cmath>

class DrumKickFM : public EngineBase<DrumKickFM> {
public:
    EnvelopDrumAmp envelopAmp;

protected:
    const float sampleRate;

    float velocity = 1.0f;
    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float modulatorFeedbackState = 0.0f;

    float modulationEnvelope = 0.0f;
    float lowPassState = 0.0f;

public:
    Param params[12];

    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 800.0f, .min = 50.0f, .max = 2000.0f, .step = 10.0f });
    Param& ampEnv = addParam({ .key = "ampEnv", .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumKickFM*>(ctx)->envelopAmp.morph(val * 0.01f); } });

    Param& baseFrequency = addParam({ .key = "baseFrequency", .label = "Sub Freq", .unit = "Hz", .value = 50.0f, .min = 30.0f, .max = 100.0f });
    Param& pitchOffset = addParam({ .key = "pitchOffset", .label = "Pitch", .unit = "st", .value = 0.0f, .min = -12.0f, .max = 12.0f, .type = VALUE_CENTERED });

    Param& fmAmount = addParam({ .key = "fmAmount", .label = "FM Depth", .unit = "%", .value = 40.0f });
    Param& fmRatio = addParam({ .key = "fmRatio", .label = "FM Ratio", .value = 1.0f, .min = 0.5f, .max = 8.0f, .step = 0.1f });
    Param& fmDecay = addParam({ .key = "fmDecay", .label = "FM Speed", .unit = "%", .value = 20.0f });

    Param& fmFeedback = addParam({ .key = "fmFeedback", .label = "FM Grit", .unit = "%", .value = 0.0f });
    Param& fmCurve = addParam({ .key = "fmCurve", .label = "FM Curve", .unit = "%", .value = 50.0f });

    Param& punch = addParam({ .key = "punch", .label = "Punch", .unit = "%", .value = 50.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 25.0f });
    Param& tone = addParam({ .key = "tone", .label = "Tone", .unit = "%", .value = 60.0f });

    DrumKickFM(const float sampleRate)
        : EngineBase(Drum, "KickFM", params)
        , sampleRate(sampleRate)
    {
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        carrierPhase = 0.0f;
        modulatorPhase = 0.0f;
        modulatorFeedbackState = 0.0f;
        modulationEnvelope = 1.0f;
        lowPassState = 0.0f;

        int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.001f) return 0.0f;

        // 1. FM Envelope logic
        float curvePower = 1.0f + (pct(fmCurve) * 4.0f);
        float fmDecayTime = 0.002f + (pct(fmDecay) * 0.15f);
        modulationEnvelope *= Math::exp(-1.0f / (sampleRate * fmDecayTime));
        float shapedFmEnvelope = Math::pow(modulationEnvelope, curvePower);

        // 2. Frequencies
        float rootFreq = baseFrequency.value * Math::pow(2.0f, pitchOffset.value / 12.0f);
        float pitchSpike = (pct(punch) * 400.0f * shapedFmEnvelope);
        float carrierFreq = rootFreq + pitchSpike;
        float modulatorFreq = carrierFreq * fmRatio.value;

        // 3. Modulator with Feedback (Phase Modulation style)
        float feedbackAmt = pct(fmFeedback) * 0.25f;
        float modulatorLookup = modulatorPhase + (modulatorFeedbackState * feedbackAmt);

        float modulatorSignal = Math::sin(PI_X2 * modulatorLookup);
        modulatorFeedbackState = modulatorSignal;

        modulatorPhase += modulatorFreq / sampleRate;
        if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;

        // 4. Carrier Oscillator
        float fmIntensity = pct(fmAmount) * 1.5f * shapedFmEnvelope;
        carrierPhase += (carrierFreq / sampleRate) + (modulatorSignal * fmIntensity * 0.1f);
        if (carrierPhase > 1.0f) carrierPhase -= 1.0f;

        float finalSignal = Math::sin(PI_X2 * carrierPhase);

        // 5. Processing
        if (drive.value > 0.0f) {
            finalSignal = applyDrive(finalSignal, pct(drive) * 3.0f);
        }

        // Tone LPF
        float filterCut = 0.05f + pct(tone) * 0.8f;
        lowPassState += filterCut * (finalSignal - lowPassState);
        finalSignal = lowPassState;

        return finalSignal * envAmp * velocity;
    }
};
