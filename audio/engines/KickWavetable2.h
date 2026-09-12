#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>
#include <atomic>
#include <cstdint>

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class KickWavetable2 : public EngineBase<KickWavetable2> {
public:
    EnvelopDrumAmp envelopAmp;
    std::atomic<bool> isBodyMuted { false };

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float modulationEnvelope = 0.0f;
    float compressionEnv = 0.0f;

    float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    Param params[11];

    float synthesizeParametricSample(float phase)
    {
        float pOffset = phaseOffset.value * 0.01f;
        float p = phase + pOffset;
        p = p - std::floor(p);

        float skewNorm = std::clamp(skew.value * 0.01f, 0.05f, 0.95f);
        float pWarped = 0.0f;
        if (p < skewNorm) {
            pWarped = 0.5f * (p / skewNorm);
        } else {
            pWarped = 0.5f + 0.5f * ((p - skewNorm) / (1.0f - skewNorm));
        }

        float shapeVal = waveShape.value * 0.01f;
        float s = 0.0f;

        float sinVal = std::sin(pWarped * 2.0f * (float)M_PI);
        float triVal = (pWarped < 0.5f) ? (4.0f * pWarped - 1.0f) : (3.0f - 4.0f * pWarped);
        float sawVal = 1.0f - 2.0f * pWarped;
        float sqVal = (pWarped < 0.5f) ? 1.0f : -1.0f;

        if (shapeVal <= 0.3333f) {
            float t = shapeVal / 0.3333f;
            s = lerp(sinVal, triVal, t);
        } else if (shapeVal <= 0.6666f) {
            float t = (shapeVal - 0.3333f) / 0.3333f;
            s = lerp(triVal, sawVal, t);
        } else {
            float t = (shapeVal - 0.6666f) / 0.3333f;
            s = lerp(sawVal, sqVal, t);
        }

        float h2Val = harmonic2.value * 0.01f;
        if (std::abs(h2Val) > 0.001f) {
            s += h2Val * std::sin(pWarped * 4.0f * (float)M_PI);
        }

        float h3Val = harmonic3.value * 0.01f;
        if (std::abs(h3Val) > 0.001f) {
            s += h3Val * std::sin(pWarped * 6.0f * (float)M_PI);
        }

        float foldVal = fold.value * 0.01f;
        if (foldVal > 0.001f) {
            float foldDrive = 1.0f + foldVal * 3.5f;
            s = std::sin(s * foldDrive * ((float)M_PI * 0.5f));
        }

        return std::clamp(s, -1.0f, 1.0f);
    }

    Param& pitchModShape = addParam({
        .key = "pitchModShape",
        .label = "Pitch Shape",
        .unit = "%",
        .value = 50.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f,
        .graph = [](void* ctx, float phase) {
            auto* self = static_cast<KickWavetable2*>(ctx);
            float shapeNorm = self->pitchModShape.value * 0.01f;
            float rawEnv = std::exp(-phase * 4.0f);
            float pitchEnv = std::pow(rawEnv, 1.0f + shapeNorm * 5.0f);
            return pitchEnv * 2.0f - 1.0f;
        }
    });

    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Sub Freq", .unit = "Hz", .value = 52.0f, .min = 30.0f, .max = 100.0f, .step = 1.0f });
    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 350.0f, .min = 50.0f, .max = 1500.0f, .step = 10.0f });
    Param& fmDepth = addParam({ .key = "fmDepth", .label = "FM Depth", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 35.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f });

    Param& waveShape = addParam({
        .key = "waveShape",
        .label = "Wave Shape",
        .unit = "%",
        .value = 0.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f
    });

    Param& harmonic2 = addParam({
        .key = "harmonic2",
        .label = "Harmonic 2",
        .unit = "%",
        .value = 0.0f,
        .min = -100.0f,
        .max = 100.0f,
        .step = 1.0f,
        .type = VALUE_CENTERED
    });

    Param& harmonic3 = addParam({
        .key = "harmonic3",
        .label = "Harmonic 3",
        .unit = "%",
        .value = 0.0f,
        .min = -100.0f,
        .max = 100.0f,
        .step = 1.0f,
        .type = VALUE_CENTERED
    });

    Param& skew = addParam({
        .key = "skew",
        .label = "Wave Skew",
        .unit = "%",
        .value = 50.0f,
        .min = 5.0f,
        .max = 95.0f,
        .step = 1.0f
    });

    Param& fold = addParam({
        .key = "fold",
        .label = "Wave Fold",
        .unit = "%",
        .value = 0.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f
    });

    Param& phaseOffset = addParam({
        .key = "phaseOffset",
        .label = "Phase Shift",
        .unit = "%",
        .value = 0.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f
    });

    KickWavetable2(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "KickWavetable2", params)
        , sampleRate(sampleRate)
    {
    }

    void trigger(float vel = 1.0f)
    {
        noteOnImpl(60, vel);
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        (void)note;
        velocity = _velocity;

        if (!isBodyMuted) {
            carrierPhase = 0.0f;
            modulatorPhase = 0.0f;
            modulationEnvelope = 1.0f;
            compressionEnv = 0.0f;

            int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
            envelopAmp.reset(totalSamples);
        }
    }

    void noteOffImpl(uint8_t note)
    {
        (void)note;
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        float kickOut = 0.0f;

        // 1. Generate Main Kick Body Sample
        if (envAmp > 0.0001f) {
            modulationEnvelope *= Math::exp(-1.0f / (sampleRate * 0.025f));

            // Pitch mod shape curve
            float shapeNorm = pitchModShape.value * 0.01f;
            float pitchEnv = std::pow(modulationEnvelope, 1.0f + shapeNorm * 5.0f);

            float rootFreq = baseFreq.value;
            float currentFreq = rootFreq * (1.0f + pitchEnv * 2.5f);

            float modulatorFreq = currentFreq * 1.5f;
            float modulatorSignal = Math::fastSin2(PI_X2 * modulatorPhase);
            modulatorPhase += modulatorFreq / sampleRate;
            if (modulatorPhase > 1.0f) modulatorPhase -= 1.0f;

            float fmIntensity = pct(fmDepth) * 0.75f * pitchEnv;
            float cycleLen = 2048.0f;
            float phaseInc = (currentFreq / sampleRate) * cycleLen;
            carrierPhase += phaseInc + (modulatorSignal * fmIntensity * 20.0f);

            while (carrierPhase >= cycleLen) carrierPhase -= cycleLen;
            while (carrierPhase < 0.0f) carrierPhase += cycleLen;

            float phaseNorm = carrierPhase / cycleLen;
            float sig = synthesizeParametricSample(phaseNorm);

            kickOut = sig * envAmp;
        }

        // 2. Apply Drive & Internal Glue Compressor
        float out = kickOut;
        if (drive.value > 0.0f) {
            out = applyDrive(out, pct(drive) * 3.0f);
        }
        out = applyCompression2(out, 0.65f, compressionEnv);

        return out * velocity;
    }
};
