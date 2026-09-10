#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/EnvelopDrumAmp.h"
#include "audio/Wavetable.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include <atomic>
#include <cstdint>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class KickWavetable : public EngineBase<KickWavetable> {
public:
    EnvelopDrumAmp envelopAmp;
    std::atomic<bool> isBodyMuted { false };
    float currentMorphVal = 1.0f;

protected:
    const float sampleRate;
    float velocity = 1.0f;

    float carrierPhase = 0.0f;
    float modulatorPhase = 0.0f;
    float modulationEnvelope = 0.0f;
    float compressionEnv = 0.0f;

    float lerp(float a, float b, float t) { return a + t * (b - a); }

public:
    Wavetable wavetable;
    char wtName[64] = "---";

    Param params[6];

    // Combined Wavetable File & Morph Position in one single parameter
    Param& wavetableParam = addParam({
        .key = "wavetable",
        .label = "Wavetable",
        .string = wtName,
        .value = 0.0f,
        .min = 0.0f,
        .max = 0.0f,
        .step = 1.0f,
        .onUpdate = [](void* ctx, float val) {
            auto* self = static_cast<KickWavetable*>(ctx);
            int fileCount = self->wavetable.fileBrowser.count;
            if (fileCount <= 0) return;

            int totalVal = (int)val;
            int totalMax = fileCount * 64;
            totalVal = (totalVal % totalMax + totalMax) % totalMax;

            int fileIdx = totalVal / 64;
            int morphIdx = (totalVal % 64) + 1;

            // FileBrowser uses 1-based indexing (1..fileCount)
            self->wavetable.open(fileIdx + 1, false);
            self->currentMorphVal = (float)morphIdx;

            std::string fname = self->wavetable.fileBrowser.getFileWithoutExtension(fileIdx + 1);
            snprintf(self->wtName, sizeof(self->wtName), "%s #%d", fname.c_str(), morphIdx);
        },
        .graph = [](void* ctx, float phase) {
            auto* self = static_cast<KickWavetable*>(ctx);
            if (self->wavetable.sampleCount <= 0.0f) return 0.0f;
            float phasePos = phase * self->wavetable.sampleCount;
            return self->wavetable.readMorph(self->currentMorphVal, phasePos);
        }
    });

    Param& pitchModShape = addParam({
        .key = "pitchModShape",
        .label = "Pitch Shape",
        .unit = "%",
        .value = 50.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f,
        .graph = [](void* ctx, float phase) {
            auto* self = static_cast<KickWavetable*>(ctx);
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

    KickWavetable(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "KickWavetable", params)
        , sampleRate(sampleRate)
    {
        int fileCount = wavetable.fileBrowser.count;
        wavetableParam.max = std::max(0.0f, (float)(fileCount * 64 - 1));
        if (fileCount > 0) {
            wavetableParam.set(0.0f);
        }
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
            float phaseInc = (currentFreq / sampleRate) * wavetable.sampleCount;
            carrierPhase += phaseInc + (modulatorSignal * fmIntensity * 20.0f);

            while (carrierPhase >= wavetable.sampleCount) carrierPhase -= wavetable.sampleCount;
            while (carrierPhase < 0.0f) carrierPhase += wavetable.sampleCount;

            // Read combined wavetable file + morph position
            float sig = wavetable.readMorph(currentMorphVal, carrierPhase);

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
