#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"

class TomEngine : public DrumEngine {
public:
    Val& pitch = val(120.f, "PITCH", { "Pitch", .min = 40.f, .max = 400.f, .step = 1.f, .unit = "Hz" });
    Val& decay = val(0.5f, "DECAY", { "Decay", .min = 0.05f, .max = 1.5f, .step = 0.01f });
    Val& punch = val(0.6f, "PUNCH", { "Punch", .unit = "%" });
    Val& bend = val(0.4f, "BEND", { "Bend", .unit = "%" });
    Val& bodyMix = val(0.7f, "BODY", { "Body", .unit = "%" });
    Val& harmonics = val(0.3f, "HARM", { "Harm", .unit = "%" });
    Val& drive = val(0.f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& boost = val(0.f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(0.2f, "REVERB", { "Reverb", .unit = "%" });
    Val& volume = val(0.9f, "VOL", { "Vol", .unit = "%" });

    TomEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Tom")
    {
        initValues();
    }

    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    float phase = 0.f;
    float memState[4] = {};

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        const float t = float(sc) / ts;
        const float env = expf(-t * 4.f / decay.get());
        const float bendEnv = 1.f - bend.pct() * t;
        const float freq = pitch.get() * bendEnv;

        // --- Fundamental + 2 harmonics
        float out = 0.f;
        for (int h = 1; h <= 3; ++h) {
            float amp = (h == 1) ? 1.f : (harmonics.pct() / h);
            out += amp * sinf(phase * h);
        }
        phase += 2.f * M_PI * freq / props.sampleRate;

        // --- Membrane resonance (2-pole)
        for (int i = 0; i < 2; ++i) {
            memState[i * 2] += freq * (i + 1) / props.sampleRate;
            out += sinf(memState[i * 2]) * expf(-t * (i + 1) / decay.get()) * bodyMix.pct() * 0.3f;
        }

        out *= envAmp * env * volume.pct();

        // --- Punch transient
        if (t < 0.02f)
            out *= 1.f + punch.pct() * 2.f;

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
        for (auto& s : memState)
            s = 0.f;
        pitch.set(pitch.get() * powf(2.f, (note - 60) / 12.f));
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
};