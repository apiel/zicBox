#pragma once

#include "audio/effects/applyBoost.h"
#include "audio/effects/applyClipping.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyWaveshape.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include <cmath>

class DrumKickSeg : public EngineBase<DrumKickSeg> {
public:
protected:
    const float sampleRate;
    const float sampleRateDiv;
    float velocity = 1.0f;
    float phase = 0.0f;
    float modPhase = 0.0f;
    int sampleCounter = 0;
    int totalSamples = 0; // For linear decay calculation

    float fxData1 = 0.0f;
    float fxData2 = 0.0f;

public:
    // Reduced to 4 segments + terminal value to save param space
    float pitchSegments[5] = { 1.0f, 0.3f, 0.2f, 0.15f, 0.0f };

    Param params[12];
    Param& duration = addParam({ .label = "Duration", .unit = "ms", .value = 400.0f, .min = 50.0f, .max = 2000.0f });
    Param& seg0 = addParam({ .label = "Seg 0ms", .unit = "%", .value = 100.0f, .onUpdate = [](void* ctx, float val) { ((DrumKickSeg*)ctx)->pitchSegments[0] = val * 0.01f; } });
    Param& seg20 = addParam({ .label = "Seg 20ms", .unit = "%", .value = 35.0f, .onUpdate = [](void* ctx, float val) { ((DrumKickSeg*)ctx)->pitchSegments[1] = val * 0.01f; } });
    Param& seg40 = addParam({ .label = "Seg 40ms", .unit = "%", .value = 15.0f, .onUpdate = [](void* ctx, float val) { ((DrumKickSeg*)ctx)->pitchSegments[2] = val * 0.01f; } });
    Param& seg60 = addParam({ .label = "Seg 60ms", .unit = "%", .value = 5.0f, .onUpdate = [](void* ctx, float val) { ((DrumKickSeg*)ctx)->pitchSegments[3] = val * 0.01f; } });

    Param& baseFrequency = addParam({ .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 80.0f });
    Param& pitchRange = addParam({ .label = "Env Range", .unit = "Hz", .value = 600.0f, .min = 0.0f, .max = 2000.0f });

    Param& fmDirt = addParam({ .label = "FM Dirt", .unit = "%", .value = 5.0f, .min = -100.0f, .onUpdate = [](void* ctx, float val) { if (val <= 0.0f) ((DrumKickSeg*)ctx)->pitchSegments[4] = val * -0.01f; } });

    Param& punchDrive = addParam({ .label = "Punch", .unit = "%", .value = 30.0f });
    Param& driveAmount = addParam({ .label = "Drive", .unit = "%", .value = 15.0f, .min = -100.0f });
    Param& compressionAmount = addParam({ .label = "Compress", .unit = "%", .value = 20.0f, .min = -100.0f });
    Param& waveshapeAmount = addParam({ .label = "Waveshape", .unit = "%", .value = 5.0f, .min = -100.0f });

    DrumKickSeg(const float sampleRate)
        : EngineBase(Drum, "KickSeg", params)
        , sampleRate(sampleRate)
        , sampleRateDiv(1.0f / sampleRate)
    {
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        phase = 0.0f;
        modPhase = 0.0f;
        sampleCounter = 0;
        totalSamples = (int)(sampleRate * (duration.value * 0.001f));
    }

    float sampleImpl()
    {
        if (sampleCounter >= totalSamples) return 0.0f;

        // 1. Linear Amplitude Decay
        float envAmp = 1.0f - ((float)sampleCounter / (float)totalSamples);
        if (envAmp <= 0.0f) return 0.0f;

        // 2. Pitch Segment Interpolation
        float timeInSeconds = (float)sampleCounter * sampleRateDiv;
        float currentPos = timeInSeconds * 50.0f; // 20ms steps

        int i0 = (int)std::floor(currentPos);
        float fraction = currentPos - (float)i0;
        float envValue = 0.0f;

        if (i0 < 3) {
            envValue = pitchSegments[i0] * (1.0f - fraction) + pitchSegments[i0 + 1] * fraction;
        } else {
            envValue = pitchSegments[4] * (1.0f - fraction);
        }

        float rootFreq = baseFrequency.value + (envValue * pitchRange.value);
        float dirtPct = fmDirt.value > 0.0f ? fmDirt.value * 0.01f : 0.0f;
        float modFreq = rootFreq * (3.5f + dirtPct * 5.0f);

        modPhase += (modFreq * sampleRateDiv);
        if (modPhase > 1.0f) modPhase -= 1.0f;

        float modulator = 4.0f * (modPhase < 0.5f ? modPhase : 1.0f - modPhase) - 1.0f;
        float modDepth = dirtPct * envValue * 0.25f;
        phase += (rootFreq * sampleRateDiv) + (modulator * modDepth);
        if (phase > 1.0f) phase -= 1.0f;

        float out = Math::sin(PI_X2 * phase);

        if (punchDrive.value > 0.0f) {
            float intensity = (punchDrive.value * 0.01f) * 24.0f * (envValue + 0.1f);
            out = (out * (1.0f + intensity)) / (1.0f + intensity * std::abs(out));
        }

        if (driveAmount.value > 0.0f) out = applyDrive(out, driveAmount.value * 0.01f);
        else if (driveAmount.value < 0.0f) out = applyBoost(out, -driveAmount.value * 0.01f, fxData1, fxData2);
        if (compressionAmount.value > 0.0f) out = applyCompression(out, compressionAmount.value * 0.01f);
        else if (compressionAmount.value < 0.0f) out = applyClipping(out, -compressionAmount.value * 0.01f);
        if (waveshapeAmount.value > 0.0f) out = applyWaveshape2(out, waveshapeAmount.value * 0.01f);
        else if (waveshapeAmount.value < 0.0f) out = applyWaveshape(out, -waveshapeAmount.value * 0.01f);

        sampleCounter++;
        return out * envAmp * velocity;
    }
};