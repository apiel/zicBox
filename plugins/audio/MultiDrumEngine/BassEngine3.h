#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyDrive.h"
// #include "plugins/audio/utils/effects/applyFilter.h" // we’ll inline a 1-pole LP/HP if not available
#include "plugins/audio/utils/effects/applyReverb.h"

class BassEngine : public DrumEngine {
public:
    /* 10 parameters ---------------------------------------------------- */
    Val& pitch = val(55.f, "PITCH", { "Pitch", .min = 20.f, .max = 400.f, .step = 1.f, .unit = "Hz" });
    Val& glide = val(0.f, "GLIDE", { "Glide", .min = 0.f, .max = 1.f, .step = 0.01f, .unit = "s" });
    Val& decay = val(0.6f, "DECAY", { "Decay", .min = 0.05f, .max = 2.f, .step = 0.01f });
    Val& accent = val(0.3f, "ACCENT", { "Accent", .unit = "%" });
    Val& wave = val(0.0f, "WAVE", { "Wave", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& resonance = val(0.2f, "REZ", { "Reso", .min = 0.f, .max = 1.f, .step = 0.01f });
    Val& cutoff = val(400.f, "CUTOFF", { "Cutoff", .min = 50.f, .max = 8000.f, .step = 10.f, .unit = "Hz" });
    Val& envMod = val(0.4f, "ENV", { "EnvMod", .unit = "%" });
    Val& drive = val(0.f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(0.1f, "REVERB", { "Reverb", .unit = "%" });
    /* ------------------------------------------------------------------ */

    BassEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Bass")
    {
        initValues();
    }

    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    /* state */
    float phase = 0.f;
    float glidePhase = 0.f;
    float filterZ = 0.f; // 1-pole state
    float noteFreq = 55.f;

    void sampleOn(float* buf, float envAmp, int sc, int totalSamples) override
    {
        const float t = float(sc) / totalSamples;
        const float env = expf(-t * 3.f / decay.get());
        const float target = noteFreq;
        /* glide */
        glidePhase = glidePhase + (target - glidePhase) / (1.f + glide.get() * props.sampleRate);
        const float f = glidePhase;

        /* oscillator: 303-style Saw↔Tri morph */
        float w = wave.pct() * 2.f - 1.f; // [-1,1]
        float tri = 2.f * asinf(sinf(2.f * M_PI * phase)) / M_PI;
        float saw = 2.f * (phase - floorf(phase)) - 1.f;
        float osc = (w <= 0.f) ? tri * (1.f + w) + saw * (-w)
                               : saw * (1.f - w) + tri * w;
        phase += f / props.sampleRate;

        /* 303-style filter envelope */
        float filtEnv = 1.f + (envMod.pct() * env);
        float cutoffHz = cutoff.get() * filtEnv;
        cutoffHz = fmaxf(cutoffHz, 20.f);
        float g = 2.f - cosf(2.f * M_PI * cutoffHz / props.sampleRate);
        g = g - sqrtf(g * g - 1.f); // 1-pole LP
        filterZ = filterZ + g * (osc - filterZ);

        /* resonance (simple feedback) */
        float res = resonance.get() * 0.95f;
        float sig = filterZ + res * (filterZ - filterZ);

        /* accent transient */
        float acc = (t < 0.01f) ? 1.f + accent.pct() * 2.f : 1.f;
        sig *= envAmp * env * acc;

        sig = applyEffects(sig);
        buf[track] = sig;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(0.f, reverb.pct(), reverbBuf, rIdx, REVBUF);
    }

    void noteOn(uint8_t note, float /*vel*/, void* = nullptr) override
    {
        phase = 0.f;
        noteFreq = pitch.get() * powf(2.f, (note - 36) / 12.f);
    }

private:
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float applyEffects(float x)
    {
        float d = drive.pct() * 2.f - 1.f;
        if (d > 0.f)
            x = applyDrive(x, d, props.lookupTable);
        else      x = applyBoost(x, -d, prevInput,prevOutput);
        return applyReverb(x, reverb.pct(), reverbBuf, rIdx, REVBUF);
    }
};