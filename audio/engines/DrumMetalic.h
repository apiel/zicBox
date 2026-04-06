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
    float timeDelta = 0.f;

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
    Param params[12];

    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 1000.0f, .min = 50.0f, .max = 3000.0f, .step = 10.0f });
    Param& ampEnv = addParam({ .key = "ampEnv", .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumMetalic*>(ctx)->envelopAmp.morph(val * 0.01f); } });

    Param& baseFreq = addParam({ .key = "baseFreq", .label = "Base Freq", .unit = "Hz", .value = 100.0f, .min = 10.0f, .max = 400.0f });
    Param& bodyResonance = addParam({ .key = "bodyResonance", .label = "Resonator", .value = 0.8f, .min = 0.0f, .max = 1.5f, .step = 0.01f });
    Param& toneTension = addParam({ .key = "toneTension", .label = "Tone Tension", .value = 0.0f, .min = 0.0f, .max = 5.0f, .step = 0.01f });

    Param& timbre = addParam({ .key = "timbre", .label = "Timbre", .unit = "%", .value = 5.0f });
    Param& fmFreq = addParam({ .key = "fmFreq", .label = "FM Freq", .unit = "Hz", .value = 50.0f, .min = 1.0f, .max = 500.0f });
    Param& fmAmp = addParam({ .key = "fmAmp", .label = "FM Amp", .unit = "%", .value = 0.0f });

    Param& envMod = addParam({ .key = "envMod", .label = "Env Mod", .unit = "%", .value = 0.0f });
    Param& envShape = addParam({ .key = "envShape", .label = "Env Shape", .value = 0.5f, .min = 0.1f, .max = 5.0f, .step = 0.1f });

    Param& driveComp = addParam({ .key = "driveComp", .label = "Drive/Comp", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& reverb = addParam({ .key = "reverb", .label = "Reverb", .unit = "%", .value = 30.0f, .min = -100.0f });

    DrumMetalic(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "Metalic", params)
        , sampleRate(sampleRate)
        , reverbBuffer(rvBuffer)
    {
        timeDelta = 1 / sampleRate;
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
        time = 0.f;
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.001f) return applyRvb(0.0f);

        time += timeDelta;

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
            resonatorState += timeDelta;

            float tensionMod = Math::exp(-resonatorState * (5.0f - toneTension.value));
            float resFreq = freq * bodyResonance.value * (1.0f + tensionMod * toneTension.value * 0.5f);
            float ringDecay = Math::exp(-2.0f * resonatorState);
            float ring = Math::sin(PI_X2 * resFreq * resonatorState);

            if (toneTension.value > 2.0f) {
                ring = Math::sin(PI_X2 * resFreq * resonatorState + (ring * 0.5f));
            }

            float compensation = Math::sqrt(220.0f / (resFreq > 1.0f ? resFreq : 1.0f));
            tone = (tone * envAmp) * ringDecay * ring * compensation;
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
            output = applyCompression(output, driveComp.value * 0.01f);
        } else if (driveComp.value < 0.0f) {
            output = applyDrive(output, driveComp.value * -0.01f);
        }

        output = applyRvb(output);

        phase += timeDelta;
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