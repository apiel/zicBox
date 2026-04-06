#pragma once

#include "audio/EnvelopDrumAmp.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"
#include "audio/engines/EngineBase.h"
#include "audio/utils/math.h"
#include "audio/utils/noise.h"
#include <cmath>

class DrumPercussion : public EngineBase<DrumPercussion> {
public:
    EnvelopDrumAmp envelopAmp;

protected:
    const float sampleRate;
    float* reverbBuffer = nullptr;
    int reverbIndex = 0;

    float velocity = 1.0f;
    float phase = 0.0f;
    float snareState = 0.0f;
    float pinkNoiseState = 0.0f;
    float crushedNoise = 0.0f;
    int bitcrushCounter = 0;

    float time = 0.0f;
    float timeRatio = 0.0f;
    int sampleCounter = 0;
    int totalSamples = 0;

    float prevInput = 0.0f;
    float prevOutput = 0.0f;

    float getBend(float t)
    {
        float bVal = pct(bend) * 2.0f - 1.0f;
        if (bVal == 0.0f) return 1.0f;
        if (bVal < 0.0f) return 1.0f + bVal * (1.0f - t);
        return 1.0f - bVal * t;
    }

    float customNoise(float input)
    {
        float amt = pct(noiseCharacter) * 2.0f - 1.0f;
        if (amt < 0.0f) {
            float blend = -amt;
            pinkNoiseState = blend * pinkNoiseState + (1.0f - blend) * input;
            return pinkNoiseState;
        } else if (amt > 0.0f) {
            float shapedAmt = Math::sqrt(amt);
            float depth = 1.0f - shapedAmt;
            int resolution = std::max(2, (int)(256 * depth));
            int crushRate = (int)(8 + (1.0f - depth) * 120);

            if (++bitcrushCounter >= crushRate) {
                bitcrushCounter = 0;
                crushedNoise = std::round(input * resolution) / (float)resolution;
            }
            return crushedNoise * shapedAmt + input * (1.0f - shapedAmt);
        }
        return input;
    }

public:
    Param params[12];

    Param& duration = addParam({ .key = "duration", .label = "Duration", .unit = "ms", .value = 600.0f, .min = 50.0f, .max = 3000.0f, .step = 10.0f });
    Param& ampEnv = addParam({ .key = "ampEnv", .label = "Amp. Env.", .unit = "%", .value = 0.0f, .onUpdate = [](void* ctx, float val) { static_cast<DrumPercussion*>(ctx)->envelopAmp.morph(val * 0.01f); } });

    Param& pitch = addParam({ .key = "pitch", .label = "Pitch", .unit = "Hz", .value = 120.0f, .min = 40.0f, .max = 400.0f });
    Param& bend = addParam({ .key = "bend", .label = "Bend", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& harmonics = addParam({ .key = "harmonics", .label = "Harmonics", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });

    Param& mix = addParam({ .key = "mix", .label = "Mix", .unit = "%", .value = 20.0f });
    Param& snareTune = addParam({ .key = "snareTune", .label = "Noise Tune", .unit = "Hz", .value = 200.0f, .min = 80.0f, .max = 600.0f });
    Param& noiseCharacter = addParam({ .key = "noiseCharacter", .label = "Noise Shape", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& snareDecay = addParam({ .key = "snareDecay", .label = "Noise Dec", .unit = "%", .value = 15.0f });

    Param& punch = addParam({ .key = "punch", .label = "Punch", .unit = "%", .value = 60.0f });
    Param& drive = addParam({ .key = "drive", .label = "Drive", .unit = "%", .value = 0.0f, .min = -100.0f, .max = 100.0f, .type = VALUE_CENTERED });
    Param& reverb = addParam({ .key = "reverb", .label = "Reverb", .unit = "%", .value = 20.0f });

    DrumPercussion(const float sampleRate, float* rvBuffer)
        : EngineBase(Drum, "Percussion", params)
        , sampleRate(sampleRate)
        , reverbBuffer(rvBuffer)
    {
    }

    void noteOnImpl(uint8_t note, float _velocity)
    {
        velocity = _velocity;
        phase = 0.0f;
        snareState = 0.0f;
        sampleCounter = 0;
        totalSamples = (int)(sampleRate * (duration.value * 0.001f));
        envelopAmp.reset(totalSamples);
    }

    float sampleImpl()
    {
        float envAmp = envelopAmp.next();
        if (envAmp < 0.001f) return applyRvb(0.0f);

        float t = (float)sampleCounter / totalSamples;
        float freq = pitch.value * getBend(t);

        // 1. Tonal Part
        float tonal = 0.0f;
        float hAmt = pct(harmonics) * 2.0f - 1.0f;
        if (hAmt >= 0.0f) {
            for (int h = 1; h <= 3; ++h) {
                float hAmp = (h == 1) ? 1.0f : (hAmt / h);
                tonal += hAmp * Math::sin(phase * h);
            }
        } else {
            tonal = Math::sin(phase) + hAmt * Math::sin(phase * 3.0f);
        }
        phase += PI_X2 * freq / sampleRate;
        if (phase > PI_X2) phase -= PI_X2;

        // 2. Noise Part
        float nRaw = (customNoise(Noise::sample()) * 2.0f - 1.0f) * envAmp;
        snareState += snareTune.value / sampleRate;
        float nEnv = Math::exp(-pct(snareDecay) * snareState);
        float snare = nRaw * nEnv * Math::sin(PI_X2 * snareTune.value * snareState);

        // 3. Mix & Punch
        float out = tonal * envAmp * (1.0f - pct(mix)) + snare * pct(mix);
        if (t < 0.02f) out *= 1.0f + pct(punch) * 2.0f;

        // 4. Effects
        float d = pct(drive) * 2.0f - 1.0f;
        if (d > 0.0f) out = applyDrive(out, d);
        else if (d < 0.0f) out = applyBoost(out, -d, prevInput, prevOutput);

        sampleCounter++;
        return applyRvb(out * velocity);
    }

    float applyRvb(float out)
    {
        return applyReverb(out, pct(reverb), reverbBuffer, reverbIndex);
    }
};