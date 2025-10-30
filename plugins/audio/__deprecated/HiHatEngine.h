#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"

class HiHatEngine : public DrumEngine {
public:
    Val& openAmt = val(0.4f, "OPEN", { "Open", .unit = "%" });
    Val& tone = val(8000.f, "TONE", { "Tone", .min = 1000.f, .max = 15000.f, .step = 10.f, .unit = "Hz" });
    Val& noiseMix = val(0.6f, "NOISE", { "Noise", .unit = "%" });
    Val& decay = val(0.35f, "DECAY", { "Decay", .min = 0.01f, .max = 1.f, .step = 0.01f });
    Val& hpCut = val(1000.f, "HP", { "HP", .min = 20.f, .max = 8000.f, .step = 10.f, .unit = "Hz" });
    Val& lpCut = val(12000.f, "LP", { "LP", .min = 8000.f, .max = 20000.f, .step = 100.f, .unit = "Hz" });
    Val& pitchEnv = val(0.3f, "P_ENV", { "P.Env", .unit = "%" });
    Val& reverbAmt = val(0.25f, "REVERB", { "Reverb", .unit = "%" });
    Val& driveAmt = val(0.f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& volume = val(0.8f, "VOL", { "Vol", .min = 0.f, .max = 1.f, .unit = "%" });

    HiHatEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "HiHat")
    {
        initValues();
    }

    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    // --- tiny SVF state
    float lp1 = 0.f, bp1 = 0.f, hp1 = 0.f;
    float lp2 = 0.f, bp2 = 0.f, hp2 = 0.f;

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        const float t = float(sc) / ts;
        const float env = expf(-t * (1.f + openAmt.pct() * 3.f));
        const float pitch = tone.get() * (1.f + pitchEnv.pct() * (1.f - t));

        // --- 6 detuned oscillators
        float osc = 0.f;
        for (int i = 0; i < 6; ++i)
            osc += sinf(2.f * M_PI * (pitch * (1.f + 0.01f * i)) * t * props.sampleRate);
        osc *= 0.2f * envAmp * env;

        // --- Noise
        float noise = (whiteNoise() * 2.f - 1.f) * 0.3f * envAmp * env * noiseMix.pct();

        // --- inline 2-pole SVF: HP first, then LP
        float in = noise;
        float hp = in - (bp1 + lp1);
        float bp = bp1 + hp * (2.f * M_PI * hpCut.get() / props.sampleRate);
        float lp = lp1 + bp * (2.f * M_PI * hpCut.get() / props.sampleRate);
        lp1 = lp;
        bp1 = bp;
        in = lp;

        bp = bp2 + in * (2.f * M_PI * lpCut.get() / props.sampleRate);
        lp = lp2 + bp * (2.f * M_PI * lpCut.get() / props.sampleRate);
        hp2 = in - (bp + lp);
        bp2 = bp;
        lp2 = lp;
        noise = lp;

        float out = (osc + noise) * volume.pct();
        out = applyReverb(out, reverbAmt.pct(), reverbBuf, rIdx, REVBUF);
        out = applyDrive(out, fabsf(driveAmt.pct()), props.lookupTable);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(0.f, reverbAmt.pct(), reverbBuf, rIdx, REVBUF);
    }

    void noteOn(uint8_t, float, void* = nullptr) override
    {
        lp1 = bp1 = hp1 = lp2 = bp2 = hp2 = 0.f;
    }

    float whiteNoise()
    {
        return props.lookupTable->getNoise();
    }
};