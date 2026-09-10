#pragma once

#include <algorithm>
#include <cmath>
#include <cstring>

#ifndef AUDIO_FOLDER
#include "host/constants.h"
#endif

#include "audio/Wavetable.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/linearInterpolation.h"
#include "audio/utils/math.h"
#include "helpers/clamp.h"

class KickWavetable : public EngineBase<KickWavetable> {
private:
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float envPhase = 0.0f;
    bool isTriggered = false;

public:
    Wavetable wavetable;
    char wtName[64] = "---";

    Param params[6];

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
            int pos = (int)val;
            self->wavetable.open(pos, false);
            strncpy(self->wtName,
                self->wavetable.fileBrowser.getFileWithoutExtension(pos).c_str(),
                sizeof(self->wtName) - 1);
        }
    });

    Param& morph = addParam({
        .key = "morph",
        .label = "Morph",
        .unit = "wave",
        .value = 1.0f,
        .min = 1.0f,
        .max = 64.0f,
        .step = 1.0f
    });

    Param& pitchModShape = addParam({
        .key = "pitchModShape",
        .label = "Pitch Shape",
        .unit = "%",
        .value = 50.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f
    });

    Param& duration = addParam({
        .key = "duration",
        .label = "Duration",
        .unit = "ms",
        .value = 400.0f,
        .min = 50.0f,
        .max = 2000.0f,
        .step = 10.0f
    });

    Param& frequency = addParam({
        .key = "frequency",
        .label = "Frequency",
        .unit = "Hz",
        .value = 50.0f,
        .min = 20.0f,
        .max = 200.0f,
        .step = 1.0f
    });

    Param& drive = addParam({
        .key = "drive",
        .label = "Drive",
        .unit = "%",
        .value = 0.0f,
        .min = 0.0f,
        .max = 100.0f,
        .step = 1.0f
    });

    KickWavetable(const float sampleRate = 44100.0f)
        : EngineBase(Drum, "KickWavetable", params)
        , sampleRate(sampleRate)
    {
        wavetableParam.max = std::max(0, wavetable.fileBrowser.count - 1);
        if (wavetable.fileBrowser.count > 0) {
            wavetable.open(0, true);
            strncpy(wtName,
                wavetable.fileBrowser.getFileWithoutExtension(0).c_str(),
                sizeof(wtName) - 1);
        }
    }

    void trigger(float vel = 1.0f)
    {
        noteOnImpl(60, vel);
    }

    void noteOnImpl(uint8_t note, float velocity)
    {
        (void)note;
        (void)velocity;
        phase = 0.0f;
        envPhase = 0.0f;
        isTriggered = true;
    }

    void noteOffImpl(uint8_t note)
    {
        (void)note;
    }

    float sampleImpl()
    {
        if (!isTriggered) return 0.0f;

        float durMs = std::max(10.0f, duration.value);
        float totalSamples = (durMs * 0.001f) * sampleRate;

        envPhase += 1.0f / totalSamples;
        if (envPhase >= 1.0f) {
            isTriggered = false;
            return 0.0f;
        }

        // Exponential amplitude decay envelope
        float ampEnv = (1.0f - envPhase) * (1.0f - envPhase);

        // Pitch sweep curve shaped by pitchModShape
        float rawPitchEnv = 1.0f - envPhase;
        float shapeNorm = pitchModShape.value * 0.01f;
        float pitchEnv = std::pow(rawPitchEnv, 1.0f + shapeNorm * 5.0f);

        // Frequency drop from high start pitch to base frequency
        float currentFreq = frequency.value * (1.0f + pitchEnv * 3.5f);

        // Advance wavetable phase
        float inc = (currentFreq / sampleRate) * wavetable.sampleCount;
        phase += inc;
        while (phase >= wavetable.sampleCount) {
            phase -= wavetable.sampleCount;
        }

        // Morphing read from loaded wavetable file
        float s = wavetable.readMorph(morph.value, phase);

        // Apply amplitude envelope
        s *= ampEnv;

        // Apply drive effect
        float driveAmount = drive.value * 0.01f;
        if (driveAmount > 0.0f) {
            s = applyDrive(s, driveAmount);
        }

        return s;
    }
};
