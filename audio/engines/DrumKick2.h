#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/noise.h"
#include "audio/utils/math.h"
#include <cmath>

class DrumKick2 : public EngineBase<DrumKick2> {
protected:
    EnvelopDrumAmp envelopAmp;

    float velocity = 1.0f;
    float oscillatorPhase = 0.0f;
    float pitchEnvelopeState = 0.0f;
    float clickEnvelopeState = 0.0f;
    float lowPassState = 0.0f;

    const float sampleRate = 48000.0f;

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 3000.0f, .step = 10.0f },
        { .label = "Amp. Env.", .unit = "%", .value = 0.0f },
        { .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 80.0f, .step = 0.1f },
        { .label = "Pitch", .unit = nullptr, .value = 0.0f, .min = -12.0f, .max = 12.0f },
        { .label = "Sweep", .unit = "%", .value = 70.0f },
        { .label = "Punch", .unit = "%", .value = 30.0f },
        { .label = "Symmetry", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Click", .unit = "%", .value = 20.0f },
        { .label = "Air", .unit = "%", .value = 5.0f },
        { .label = "Drive", .unit = "%", .value = 30.0f },
        { .label = "Comp.", .unit = "%", .value = 20.0f },
        { .label = "Tone", .unit = "%", .value = 50.0f }
    };

    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& subFreq = params[2];
    Param& pitch = params[3];
    Param& sweepDepth = params[4];
    Param& sweepSpeed = params[5];
    Param& symmetry = params[6];
    Param& click = params[7];
    Param& noise = params[8];
    Param& drive = params[9];
    Param& compression = params[10];
    Param& tone = params[11];

    DrumKick2()
        : EngineBase(Drum, "Kick2", params)
    {
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        oscillatorPhase = 0.0f;
        pitchEnvelopeState = 1.0f;
        clickEnvelopeState = 1.0f;
        lowPassState = 0.0f;

        totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
        i = 0;
    }

    int totalSamples = 0;
    int i = 0;
    float sampleImpl()
    {
        float envAmp = 0.0f;
        if (i < totalSamples) {
            envAmp = envelopAmp.next();
            i++;
        } else {
            return 0.0f;
        }

        float sweepDecayTime = 0.005f + (1.0f - pct(sweepSpeed)) * 0.05f;
        pitchEnvelopeState *= Math::exp(-1.0f / (sampleRate * sweepDecayTime));

        float rootFreq = subFreq.value * Math::pow(2.0f, (pitch.value) / 12.0f);
        float currentFrequency = rootFreq + (pct(sweepDepth) * 600.0f * pitchEnvelopeState);

        oscillatorPhase += currentFrequency / sampleRate;
        if (oscillatorPhase > 1.0f) oscillatorPhase -= 1.0f;

        float rawSine = Math::sin(2.0f * M_PI * oscillatorPhase);

        float shapeAmount = (pct(symmetry) * 2.0f - 1.0f) * 0.9f;
        float shapedSine = (rawSine + shapeAmount * (rawSine * rawSine * rawSine)) / (1.0f + shapeAmount);

        float noiseSample = (makeNoise() * 2.0f - 1.0f);
        float clickPart = noiseSample * clickEnvelopeState * pct(click);

        clickEnvelopeState *= Math::exp(-1.0f / (sampleRate * 0.002f));

        float finalOutput = shapedSine + clickPart;
        finalOutput += noiseSample * pct(noise) * 0.05f * envAmp;

        if (drive.value > 0.0f) {
            finalOutput = applyDrive(finalOutput, pct(drive) * 2.5f);
        }

        float filterCoefficient = 0.05f + pct(tone) * 0.7f;
        lowPassState += filterCoefficient * (finalOutput - lowPassState);
        finalOutput = lowPassState;

        if (compression.value > 0.0f) {
            finalOutput = applyCompression(finalOutput, pct(compression));
        }

        return finalOutput * envAmp * velocity;
    }
};