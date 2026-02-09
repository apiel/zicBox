#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include <cmath>

class DrumSnare : public EngineBase<DrumSnare> {
public:
    EnvelopDrumAmp envelopAmp;

protected:
    const float sampleRate;

    float velocity = 1.0f;
    float tonalPhase = 0.0f;
    float bodyEnvelope = 0.0f;
    float noiseEnvelope = 0.0f;
    float lowPassState = 0.0f;
    float noiseHpState = 0.0f;
    int sampleCounter = 0;

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 400.0f, .min = 50.0f, .max = 2000.0f, .step = 10.0f },
        { .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumSnare*>(ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "Body Freq", .unit = "Hz", .value = 180.0f, .min = 100.0f, .max = 400.0f },
        { .label = "Body", .unit = "%", .value = 30.0f },
        { .label = "Snappy", .unit = "%", .value = 50.0f },
        { .label = "Snap Tail", .unit = "%", .value = 40.0f },
        { .label = "Snap Tone", .unit = "%", .value = 50.0f },
        { .label = "Punch", .unit = "%", .value = 20.0f },
        { .label = "Impact", .unit = "%", .value = 30.0f },
        { .label = "Drive", .unit = "%", .value = 15.0f },
        { .label = "Tightness", .unit = "%", .value = 50.0f },
        { .label = "Tone", .unit = "%", .value = 100.0f }
    };

    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& baseFrequency = params[2];
    Param& bodyDecay = params[3];
    Param& snappyLevel = params[4];
    Param& snappyDecay = params[5];
    Param& snapTone = params[6];
    Param& pitchDrop = params[7];
    Param& impact = params[8];
    Param& drive = params[9];
    Param& tightness = params[10];
    Param& tone = params[11];

    DrumSnare(const float sampleRate)
        : EngineBase(Drum, "Snare", params)
        , sampleRate(sampleRate)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        tonalPhase = 0.0f;
        bodyEnvelope = 1.0f;
        noiseEnvelope = 1.0f;
        lowPassState = 0.0f;
        noiseHpState = 0.0f;
        sampleCounter = 0;

        int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.001f) return 0.0f;

        // 1. Envelopes
        float bodyTime = 0.01f + (pct(bodyDecay) * 0.2f);
        bodyEnvelope *= Math::exp(-1.0f / (sampleRate * bodyTime));

        float noiseTime = 0.02f + (pct(snappyDecay) * 0.4f);
        noiseEnvelope *= Math::exp(-1.0f / (sampleRate * noiseTime));

        // 2. Tonal Part
        float pitchEnv = bodyEnvelope * pct(pitchDrop) * 100.0f;
        tonalPhase += (baseFrequency.value + pitchEnv) / sampleRate;
        if (tonalPhase > 1.0f) tonalPhase -= 1.0f;
        float tonalPart = Math::sin(PI_X2 * tonalPhase) * bodyEnvelope;

        // 3. Noise Part
        float rawNoise = Noise::sample();
        float hpCutoff = 0.01f + (pct(snapTone) * 0.5f);
        noiseHpState += hpCutoff * (rawNoise - noiseHpState);
        float snappyPart = (rawNoise - noiseHpState) * noiseEnvelope * pct(snappyLevel);

        // 4. Impact Transient (First ~3ms @ 48k)
        float impactPart = 0.0f;
        if (sampleCounter < 150) {
            impactPart = rawNoise * pct(impact) * 2.0f;
        }

        // 5. Mix and Process
        float mixedSignal = (tonalPart * 0.6f) + snappyPart + impactPart;

        if (drive.value > 0.0f) {
            mixedSignal = applyDrive(mixedSignal, pct(drive) * 3.0f);
        }

        // Master Tone (LPF)
        float masterCutoff = 0.1f + pct(tone) * 0.9f;
        lowPassState += masterCutoff * (mixedSignal - lowPassState);
        mixedSignal = lowPassState;

        // 6. Tightness (Gate-like scaling)
        float tightFactor = Math::pow(envAmp, 1.0f + pct(tightness) * 3.0f);

        sampleCounter++;
        return mixedSignal * tightFactor * velocity;
    }
};