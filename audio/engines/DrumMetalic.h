#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include <cmath>

class DrumMetalic : public EngineBase<DrumMetalic> {
public:
    EnvelopDrumAmp envelopAmp;

protected:
    const float sampleRate;

    float* reverbBuffer = nullptr;
    int reverbIndex = 0;

    float velocity = 1.0f;
    float phase = 0.0f;
    float resonatorState = 0.0f;
    float noteFreq = 440.0f;
    int sampleCounter = 0;

    float time = 0.f;
    float timeRatio = 0.f;

    // Helper for generating the custom envelope curve shape
    float getShape(float t)
    {
        float shape = envShape.value;
        if (shape > 1.0f) {
            return Math::pow(1.0f - t, shape);
        } else {
            return Math::exp(-t * (shape > 0.0f ? shape : 1.0f));
        }
    }

public:
    Param params[12] = {
        { .label = "Duration", .unit = "ms", .value = 1000.0f, .min = 50.0f, .max = 3000.0f, .step = 10.0f },
        { .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumMetalic*>(ctx)->envelopAmp.morph(val * 0.01f); } },
        { .label = "Base Freq", .unit = "Hz", .value = 100.0f, .min = 10.0f, .max = 400.0f },
        { .label = "Resonator", .value = 0.8f, .min = 0.0f, .max = 1.5f, .step = 0.01f },
        { .label = "Tone Decay", .value = 0.02f, .min = 0.005f, .max = 1.0f, .step = 0.005f },
        { .label = "Timbre", .unit = "%", .value = 5.0f },
        { .label = "FM Freq", .unit = "Hz", .value = 50.0f, .min = 1.0f, .max = 500.0f },
        { .label = "FM Amp", .unit = "%", .value = 0.0f },
        { .label = "Env Mod", .unit = "%", .value = 0.0f },
        { .label = "Env Shape", .value = 0.5f, .min = 0.1f, .max = 5.0f },
        { .label = "Drive/Comp", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED },
        { .label = "Reverb", .unit = "%", .value = 30.0f, .min = -100.0f }
    };

    Param& duration = params[0];
    Param& ampEnv = params[1];
    Param& baseFreq = params[2];
    Param& bodyResonance = params[3];
    Param& toneDecay = params[4];
    Param& timbre = params[5];
    Param& fmFreq = params[6];
    Param& fmAmp = params[7];
    Param& envMod = params[8];
    Param& envShape = params[9];
    Param& driveComp = params[10];
    Param& reverb = params[11];

    DrumMetalic(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "Metalic", params)
        , sampleRate(sampleRate)
        , reverbBuffer(rvBuffer)
    {
        init();
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        phase = 0.0f;
        resonatorState = 0.0f;
        sampleCounter = 0;

        // Note tracking relative to BaseNote 60
        noteFreq = baseFreq.value * Math::pow(2.0f, (static_cast<float>(note) - 60.0f) / 12.0f);

        int totalSamples = static_cast<int>(sampleRate * (duration.value * 0.001f));
        timeRatio = 1 / sampleRate;
        time = 0.f;
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.001f) return applyRvb(0.0f);

        time += timeRatio;

        float currentEnvShape = getShape(time);

        // 1. Frequency calculation (with Env Mod)
        float freq = noteFreq;
        if (envMod.value > 0.0f) {
            freq += freq * pct(envMod) * currentEnvShape;
        }

        // 2. FM Modulation
        float fmModulator = 0.0f;
        if (fmAmp.value > 0.0f) {
            fmModulator = Math::sin(PI_X2 * fmFreq.value * phase) * pct(fmAmp) * 10.0f;
        }
        float tone = Math::sin(PI_X2 * (freq + fmModulator) * phase);

        // 3. Resonator logic
        if (bodyResonance.value > 0.0f) {
            resonatorState += 1.0f / sampleRate;
            float resFreq = freq * bodyResonance.value;
            float decay = Math::exp(-toneDecay.value * resonatorState);
            float ring = Math::sin(PI_X2 * resFreq * resonatorState);

            // Loudness compensation based on frequency
            float compensation = Math::sqrt(220.0f / (resFreq > 1.0f ? resFreq : 1.0f));
            tone = (tone * envAmp) * decay * ring * compensation;
        }

        // 4. Timbre Shaping
        if (timbre.value > 0.0f) {
            float harmonic = Math::sin(PI_X2 * freq * 0.5f * time);
            tone *= (1.0f - pct(timbre)) + pct(timbre) * harmonic;
        }

        // 5. FX Chain
        float output = tone * envAmp * velocity;

        // Centered Drive/Compression logic
        if (driveComp.value > 0.0f) {
            output = applyCompression(output, pct(driveComp));
        } else if (driveComp.value < 0.0f) {
            output = applyDrive(output, -pct(driveComp));
        }

        output = applyRvb(output);

        phase += 1.0f / sampleRate;
        sampleCounter++;

        return output;
    }

protected:
    float applyRvb(float output)
    {
        if (reverb.value == 0.0f) return output;

        if (reverb.value < 0.0f) {
            return applyMiniReverb(output, -reverb.value * 0.01f, reverbBuffer, reverbIndex);
        }

        return applyReverb(output, reverb.value * 0.01f, reverbBuffer, reverbIndex);
    }
};