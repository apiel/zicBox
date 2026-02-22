#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/MultiFx.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include <cmath>

class DrumKickSeg : public EngineBase<DrumKickSeg> {
public:
    EnvelopDrumAmp envelopAmp;
    MultiFx multiFx;

protected:
    const float sampleRate;
    const float sampleRateDiv;
    float velocity = 1.0f;
    float phase = 0.0f;
    float modPhase = 0.0f;
    int sampleCounter = 0;

    char fxName[24] = "Off";

public:
    // Exposed for the Engine Wrapper to manipulate directly
    float pitchSegments[10] = { 1.0f, 0.3f, 0.2f, 0.15f, 0.12f, 0.11f, 0.08f, 0.07f, 0.06f, 0.0f };

    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 800.0f, .min = 50.0f, .max = 2000.0f, .step = 10.0f },
        { .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumKickSeg*>(ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "Seg Index", .value = 0.0f, .min = 0, .max = 9, .onUpdate = [](void* ctx, float val) {
             auto kick = (DrumKickSeg*)ctx;
             kick->activeSegVal.value = kick->pitchSegments[(int)val];
         } },
        { .label = "Seg Value", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) {
             auto kick = (DrumKickSeg*)ctx;
             int idx = (int)kick->activeSegIdx.value;
             kick->pitchSegments[idx] = val * 0.01f;
         } },
        { .label = "Sub Freq", .unit = "Hz", .value = 45.0f, .min = 30.0f, .max = 80.0f },
        { .label = "Env Range", .unit = "Hz", .value = 600.0f, .min = 0.0f, .max = 2000.0f },
        { .label = "Punch Drive", .unit = "%", .value = 20.0f },
        { .label = "FM Dirt", .unit = "%", .value = 0.0f },
        { .label = "Drive", .unit = "%", .value = 30.0f },
        { .label = "Compressor", .unit = "%", .value = 20.0f },
        { .label = "FX type", .string = fxName, .value = 0.0f, .max = MultiFx::FX_COUNT - 1, .onUpdate = [](void* ctx, float val) {
             auto kick = (DrumKickSeg*)ctx;
             kick->multiFx.setEffect(val);
             strcpy(kick->fxName, kick->multiFx.getEffectName());
         } },
        { .label = "FX amount", .unit = "%", .value = 0.0f },
    };

    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& activeSegIdx = params[2];
    Param& activeSegVal = params[3];
    Param& baseFrequency = params[4];
    Param& pitchRange = params[5];
    Param& punchDrive = params[6];
    Param& fmDirt = params[7];
    Param& driveAmount = params[8];
    Param& compressionAmount = params[9];
    Param& fxType = params[10];
    Param& fxAmount = params[11];

    DrumKickSeg(const float sampleRate, float* fxBuffer)
        : EngineBase(Drum, "KickSeg", params)
        , multiFx(sampleRate, fxBuffer)
        , sampleRate(sampleRate)
        , sampleRateDiv(1.0f / sampleRate)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        phase = 0.0f;
        modPhase = 0.0f;
        sampleCounter = 0;
        int totalSamples = (int)(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.001f) return 0.0f;

        // 1. Precise Segment Interpolation
        float timeInSeconds = (float)sampleCounter / sampleRate;
        float segmentDuration = 0.020f; // 20ms
        float currentPos = timeInSeconds / segmentDuration;

        int i0 = (int)std::floor(currentPos);
        float fraction = currentPos - (float)i0;
        float envValue = 0.0f;

        if (i0 < 9) {
            envValue = pitchSegments[i0] * (1.0f - fraction) + pitchSegments[i0 + 1] * fraction;
        } else if (i0 == 9) {
            envValue = pitchSegments[9] * (1.0f - fraction);
        }

        // 2. Dual Oscillator FM Logic
        float rootFreq = baseFrequency.value + (envValue * pitchRange.value);
        float dirtPct = fmDirt.value * 0.01f;
        float modFreq = rootFreq * (3.5f + dirtPct * 5.0f);

        modPhase += (modFreq / sampleRate);
        if (modPhase > 1.0f) modPhase -= 1.0f;

        // Triangle Modulator
        float modulator = 4.0f * (modPhase < 0.5f ? modPhase : 1.0f - modPhase) - 1.0f;

        // Phase Modulation
        float modDepth = dirtPct * envValue * 0.25f;
        phase += (rootFreq / sampleRate) + (modulator * modDepth);
        if (phase > 1.0f) phase -= 1.0f;
        if (phase < 0.0f) phase += 1.0f;

        float out = Math::sin(PI_X2 * phase);

        // 3. Hardcore Drive (Saturation)
        if (punchDrive.value > 0.0f) {
            float intensity = pct(punchDrive) * 24.0f * (envValue + 0.1f);
            out = (out * (1.0f + intensity)) / (1.0f + intensity * std::abs(out));
        }

        // 4. Wave Folding
        if (dirtPct > 0.7f) {
            float foldAmt = (dirtPct - 0.7f) * 4.0f;
            out = Math::sin(out * (1.0f + foldAmt));
        }

        // 5. Cleanup
        if (driveAmount.value > 0.0f) out = applyDrive(out, pct(driveAmount) * 2.5f);
        if (compressionAmount.value > 0.0f) out = applyCompression(out, pct(compressionAmount));

        out = multiFx.apply(out, fxAmount.value * 0.01f);

        sampleCounter++;
        return out * envAmp * velocity;
    }
};