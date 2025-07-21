#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"

class PercussionEngine : public DrumEngine {
public:
public:
    Val& pitch = val(120.f, "PITCH", { "Pitch", .min = 40.f, .max = 400.f, .step = 1.f, .unit = "Hz" });
    Val& bend = val(0.4f, "BEND", { "Bend", .unit = "%" });
    Val& harmonics = val(0.3f, "HARMONICS", { "Harmonics", .unit = "%" });

    Val& snareTune = val(200.f, "NOISE_TUNE", { "Noise Tune", .min = 80.f, .max = 600.f, .step = 1.f, .unit = "Hz" });
    Val& snareDecay = val(0.15f, "NOISE_DEC", { "Noise Dec", .min = 0.01f, .max = 1.f, .step = 0.01f });
    Val& mix = val(0.5f, "MIX", { "Mix", .unit = "%" });

    Val& punch = val(0.6f, "PUNCH", { "Punch", .unit = "%" });
    Val& drive = val(0.f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(0.2f, "REVERB", { "Reverb", .unit = "%" });

    PercussionEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Perc")
    {
        initValues();
    }

    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    float phase = 0.f;
    // float memState[4] = {};

    float snareState = 0.f;

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        const float t = float(sc) / ts;
        // const float env = expf(-t * 4.f / decay.get());
        const float bendEnv = 1.f - bend.pct() * t;
        const float freq = pitch.get() * bendEnv;

        // --- Fundamental + 2 harmonics
        float out = 0.f;
        for (int h = 1; h <= 3; ++h) {
            float amp = (h == 1) ? 1.f : (harmonics.pct() / h);
            out += amp * sinf(phase * h);
        }
        phase += 2.f * M_PI * freq / props.sampleRate;
        out *= envAmp;

        float noise = (whiteNoise() * 2.f - 1.f) * envAmp;
        float snare = resonator(noise, snareTune.get(), snareDecay.get(), snareState);
        out = out * (1.f - mix.pct()) + snare * mix.pct();

        // --- Punch transient
        if (t < 0.02f) {
            out *= 1.f + punch.pct() * 2.f;
        }

        out = applyEffects(out);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuf, rIdx, REVBUF);
    }

    void noteOn(uint8_t note, float, void* = nullptr) override
    {
        phase = 0.f;
        // for (auto& s : memState) {
        //     s = 0.f;
        // }
        pitch.set(pitch.get() * powf(2.f, (note - 60) / 12.f));

        snareState = 0.f;
    }

private:
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float applyEffects(float x)
    {
        float d = drive.pct() * 2.f - 1.f;
        if (d > 0.f)
            x = applyDrive(x, d, props.lookupTable);
        else
            x = applyBoost(x, -d, prevInput, prevOutput);
        return applyReverb(x, reverb.pct(), reverbBuf, rIdx, REVBUF);
    }

    float whiteNoise()
    {
        return props.lookupTable->getNoise();
    }

    float resonator(float in, float f, float d, float& s)
    {
        s += f / props.sampleRate;
        return in * expf(-d * s) * sinf(2.f * M_PI * f * s);
    }
};