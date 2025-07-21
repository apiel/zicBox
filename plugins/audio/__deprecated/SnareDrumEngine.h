#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"

class SnareDrumEngine : public DrumEngine {
public:
    // Val& noiseLevel = val(0.7f, "NOISE_LVL", { "Noise", .unit = "%" });
    Val& snareTune = val(200.f, "SNARE_TUNE", { "Tune", .min = 80.f, .max = 600.f, .step = 1.f, .unit = "Hz" });
    Val& snareDecay = val(0.15f, "SNARE_DEC", { "Sn.Dec", .min = 0.01f, .max = 1.f, .step = 0.01f });
    Val& bodyTone = val(180.f, "BODY_TONE", { "Body", .min = 60.f, .max = 400.f, .step = 1.f, .unit = "Hz" });
    Val& bodyDecay = val(0.30f, "BODY_DEC", { "Bdy Dec", .min = 0.01f, .max = 1.f, .step = 0.01f });
    Val& driveAmt = val(0.f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& boostAmt = val(0.f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverbAmt = val(0.2f, "REVERB", { "Reverb", .unit = "%" });
    // Val& mix = val(0.5f, "MIX", { "Mix", .unit = "%" });
    Val& snap = val(0.4f, "SNAP", { "Snap", .unit = "%" });

    SnareDrumEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Snare")
    {
        initValues();
    }

    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    float bodyState = 0.f, snareState = 0.f;

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        const float t = float(sc) / ts;
        const float snapEnv = 1.f - t;
        const float snapCurve = powf(snapEnv, 1.f + snap.pct() * 2.f);

        // --- Noise layer
        // float noise = (whiteNoise() * 2.f - 1.f) * envAmp * noiseLevel.pct() * snapCurve;
        float noise = (whiteNoise() * 2.f - 1.f) * envAmp * snapCurve;

        // --- Snare rattle (band-passed via resonator)
        float snare = resonator(noise, snareTune.get(), snareDecay.get(), snareState);

        // --- Body tone
        float body = sineWave(bodyTone.get(), t * props.sampleRate) * envAmp * (1.f - snapCurve);
        body = resonator(body, bodyTone.get(), bodyDecay.get(), bodyState);

        float out = (snare + body); // * mix.pct();
        out = applyEffects(out);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverbAmt.pct(), reverbBuf, rIdx, REVBUF);
    }

    void noteOn(uint8_t, float, void* = nullptr) override
    {
        bodyState = snareState = 0.f;
    }

private:
    float resonator(float in, float f, float d, float& s)
    {
        s += f / props.sampleRate;
        return in * expf(-d * s) * sinf(2.f * M_PI * f * s);
    }

    float sineWave(float f, float p) { return sinf(2.f * M_PI * f * p); }

    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float applyEffects(float input)
    {
        float output = input;
        float amount = driveAmt.pct() * 2 - 1.0f;
        if (amount > 0.0f) {
            output = applyBoost(input, amount, prevInput, prevOutput);
        } else if (amount < 0.0f) {
            output = applyDrive(input, -amount, props.lookupTable);
        }
        output = applyReverb(output, reverbAmt.pct(), reverbBuf, rIdx, REVBUF);
        return output;
    }

    float whiteNoise()
    {
        return props.lookupTable->getNoise();
    }
};