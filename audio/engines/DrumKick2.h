#pragma once

#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/noise.h"
#include <cmath>

class DrumKick2 : public EngineBase<DrumKick2> {
protected:
    float velocity = 1.0f;
    float oscillatorPhase = 0.0f;
    float pitchEnvelopeState = 0.0f;
    float clickEnvelopeState = 0.0f;
    float lowPassState = 0.0f;

    const float sampleRate = 48000.0f;

public:
    Param params[10] = {
        { .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 80.0f, .step = 0.1f },
        { .label = "Pitch", .unit = nullptr, .value = 0.0f, .min = -12.0f, .max = 12.0f },
        { .label = "Sweep", .unit = "%", .value = 70.0f },
        { .label = "Punch", .unit = "%", .value = 30.0f },
        { .label = "Symmetry", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Click", .unit = "%", .value = 20.0f },
        { .label = "Air", .unit = "%", .value = 5.0f },
        { .label = "Drive", .unit = "%", .value = 30.0f },
        { .label = "Glue", .unit = "%", .value = 20.0f },
        { .label = "Tone", .unit = "%", .value = 50.0f }
    };

    Param& subFreq = params[0];
    Param& pitch = params[1];
    Param& sweepDepth = params[2];
    Param& sweepSpeed = params[3];
    Param& symmetry = params[4];
    Param& click = params[5];
    Param& noise = params[6];
    Param& drive = params[7];
    Param& glue = params[8];
    Param& tone = params[9];

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
    }

    float sampleImpl()
    {
        // 1. Pitch Envelope (Punch)
        float sweepDecayTime = 0.005f + (1.0f - getPct(sweepSpeed)) * 0.05f;
        pitchEnvelopeState *= expf(-1.0f / (sampleRate * sweepDecayTime));

        // 2. Frequency Logic
        float rootFreq = subFreq.value * powf(2.0f, (pitch.value) / 12.0f);
        float currentFrequency = rootFreq + (getPct(sweepDepth) * 600.0f * pitchEnvelopeState);

        // 3. Oscillator & Symmetry
        oscillatorPhase += currentFrequency / sampleRate;
        if (oscillatorPhase > 1.0f) oscillatorPhase -= 1.0f;

        float rawSine = sinf(2.0f * M_PI * oscillatorPhase);
        float shapeAmount = (getPct(symmetry) * 2.0f - 1.0f) * 0.9f;
        float shapedSine = (rawSine + shapeAmount * (rawSine * rawSine * rawSine)) / (1.0f + shapeAmount);

        // 4. Transient / Click
        // (Placeholder noise - replace with your actual noise generator/LUT)
        float noiseSample = makeNoise() * 2.0f - 1.0f;
        float clickPart = noiseSample * clickEnvelopeState * getPct(click);
        clickEnvelopeState *= expf(-1.0f / (sampleRate * 0.002f));

        // 5. Signal Summing
        float finalOutput = shapedSine + clickPart;

        // 6. Processing Chain (Drive -> Filter -> Glue)
        if (drive.value > 0.0f) {
            // finalOutput = applyDrive(finalOutput, getPct(drive) * 2.5f); // FIXME
        }

        float filterCoefficient = 0.05f + getPct(tone) * 0.7f;
        lowPassState += filterCoefficient * (finalOutput - lowPassState);
        finalOutput = lowPassState;

        if (glue.value > 0.0f) {
            finalOutput = applyCompression(finalOutput, getPct(glue));
        }

        return finalOutput * velocity;
    }
};