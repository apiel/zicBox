#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/MultiFx.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"

#include <cmath>

class DrumKick2 : public EngineBase<DrumKick2> {
public:
    EnvelopDrumAmp envelopAmp;
    MultiFx multiFx;

protected:
    const float sampleRate;

    float velocity = 1.0f;
    float oscillatorPhase = 0.0f;
    float pitchEnvelopeState = 0.0f;
    float lowPassState = 0.0f;

    // Tracks the frequency calculated from the MIDI note
    float noteBaseFrequency = 45.0f;

    char fxName[24] = "Off";

public:
    // TODO review Click and Air...

    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 500.0f, .min = 50.0f, .max = 3000.0f, .step = 10.0f },
        { .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumKick2*>(ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 80.0f, .step = 0.1f },
        { .label = "Pitch", .unit = nullptr, .value = 0.0f, .min = -12.0f, .max = 12.0f },
        { .label = "Sweep", .unit = "%", .value = 70.0f },
        { .label = "Punch", .unit = "%", .value = 30.0f },
        { .label = "Symmetry", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f },
        { .label = "Drive", .unit = "%", .value = 30.0f },
        { .label = "Comp.", .unit = "%", .value = 20.0f },
        { .label = "Filter", .unit = "%", .value = 50.0f },
        { .label = "FX type", .string = fxName, .value = 0.0f, .max = MultiFx::FX_COUNT - 1, .onUpdate = [](void* ctx, float val) { 
            auto edge = (DrumKick2*)ctx;
            edge->multiFx.setEffect(val);
            strcpy(edge->fxName, edge->multiFx.getEffectName());
        } },
        { .label = "FX amount", .unit = "%", .value = 0.0f },
    };

    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& subFreq = params[2];
    Param& pitch = params[3];
    Param& sweepDepth = params[4];
    Param& sweepSpeed = params[5];
    Param& symmetry = params[6];
    Param& drive = params[7];
    Param& compression = params[8];
    Param& tone = params[9];
    Param& fxType = params[10];
    Param& fxAmount = params[11];

    DrumKick2(const float sampleRate, float* fxBuffer)
        : EngineBase(Drum, "Kick2", params)
        , sampleRate(sampleRate)
        , multiFx(sampleRate, fxBuffer)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        oscillatorPhase = 0.0f;
        pitchEnvelopeState = 1.0f;
        lowPassState = 0.0f;

        // MIDI Note Tracking (Base note 60)
        // Offset is calculated relative to Middle C
        float semitoneOffset = static_cast<float>(note) - 60.0f;
        noteBaseFrequency = subFreq.value * Math::pow(2.0f, semitoneOffset / 12.0f);

        int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.001f) return multiFx.apply(0.0f, fxAmount.value * 0.01f);

        float sweepDecayTime = 0.005f + (1.0f - pct(sweepSpeed)) * 0.05f;
        pitchEnvelopeState *= Math::exp(-1.0f / (sampleRate * sweepDecayTime));

        float rootFreq = noteBaseFrequency * Math::pow(2.0f, (pitch.value) / 12.0f);
        float currentFrequency = rootFreq + (pct(sweepDepth) * 600.0f * pitchEnvelopeState);

        oscillatorPhase += currentFrequency / sampleRate;
        if (oscillatorPhase > 1.0f) oscillatorPhase -= 1.0f;

        float rawSine = Math::sin(PI_X2 * oscillatorPhase);

        float shapeAmount = symmetry.value * 0.009f;
        float finalOutput = (rawSine + shapeAmount * (rawSine * rawSine * rawSine)) / (1.0f + shapeAmount);

        if (drive.value > 0.0f) {
            finalOutput = applyDrive(finalOutput, drive.value * 0.01f);
        }

        float filterCoefficient = 0.05f + (100.0f - tone.value) * 0.007f;
        lowPassState += filterCoefficient * (finalOutput - lowPassState);
        finalOutput = lowPassState;

        if (compression.value > 0.0f) {
            finalOutput = applyCompression(finalOutput, compression.value * 0.01f);
        }

        finalOutput = multiFx.apply(finalOutput, fxAmount.value * 0.01f);
        return finalOutput * envAmp * velocity;
    }
};