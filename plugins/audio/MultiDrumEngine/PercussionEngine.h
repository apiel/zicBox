#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"

class PercussionEngine : public DrumEngine {
public:
    /* 10 parameters ------------------------------------------------------ */
    Val& tone = val(150.f, "TONE", { "Tone", .min = 30.f, .max = 800.f, .step = 1.f, .unit = "Hz" });
    Val& toneDecay = val(0.6f, "T_DEC", { "T.Dec", .min = 0.01f, .max = 2.f, .step = 0.01f });
    Val& toneMix = val(0.5f, "T_MIX", { "T.Mix", .unit = "%" });

    Val& noiseLvl = val(0.5f, "N_LVL", { "Noise", .unit = "%" });
    Val& noiseDecay = val(0.25f, "N_DEC", { "N.Dec", .min = 0.01f, .max = 1.f, .step = 0.01f });
    Val& hpCut = val(800.f, "HP", { "HP", .min = 20.f, .max = 8000.f, .step = 10.f, .unit = "Hz" });
    Val& lpCut = val(12000.f, "LP", { "LP", .min = 2000.f, .max = 20000.f, .step = 100.f, .unit = "Hz" });

    Val& snap = val(0.3f, "SNAP", { "Snap", .unit = "%" });
    Val& drive = val(0.f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverbAmt = val(0.2f, "REVERB", { "Reverb", .unit = "%" });
    /* --------------------------------------------------------------------- */

    PercussionEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Perc")
    {
        initValues();
    }

    /* --------------------------------------------------------------------- */
    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    /* states for tiny SVF and resonators */
    float phase = 0.f;
    float toneState = 0.f, noiseState = 0.f;
    float lp1 = 0.f, bp1 = 0.f, hp1 = 0.f, lp2 = 0.f, bp2 = 0.f, hp2 = 0.f;

    /* --------------------------------------------------------------------- */
    void sampleOn(float* buf, float envAmp, int sc, int totalSamples) override
    {
        const float t = float(sc) / totalSamples;

        /* ---- Tone layer (membrane/pitch) -------------------------------- */
        const float toneEnv = expf(-t * 3.f / toneDecay.get());
        const float snapEnv = 1.f - t;
        const float snapCurve = powf(snapEnv, 1.f + snap.pct() * 3.f);

        float toneOut = sinf(phase) * envAmp * toneEnv * snapCurve;
        phase += 2.f * M_PI * tone.get() / props.sampleRate;

        /* 2-pole resonator for body */
        toneOut = resonator(toneOut * toneMix.pct(), tone.get(), toneDecay.get(), toneState);

        /* ---- Noise layer ------------------------------------------------- */
        const float noiseEnv = expf(-t * 4.f / noiseDecay.get());
        float n = whiteNoise() * envAmp * noiseEnv * noiseLvl.pct();

        /* inline SVF: HP then LP */
        // float hp = n - (bp1 + lp1);
        // float bp = bp1 + hp * (2.f * M_PI * hpCut.get() / props.sampleRate);
        // float lp = lp1 + bp * (2.f * M_PI * hpCut.get() / props.sampleRate);
        // lp1 = lp;
        // bp1 = bp;
        // n = lp;

        // bp = bp2 + n * (2.f * M_PI * lpCut.get() / props.sampleRate);
        // lp = lp2 + bp * (2.f * M_PI * lpCut.get() / props.sampleRate);
        // hp2 = n - (bp + lp);
        // bp2 = bp;
        // lp2 = lp;

        // n = lp; // final filtered noise

        /* ---- Mix & FX ---------------------------------------------------- */
        float out = toneOut + n;
        out = applyEffects(out);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(0.f, reverbAmt.pct(), reverbBuf, rIdx, REVBUF);
    }

    void noteOn(uint8_t note, float /*vel*/, void* = nullptr) override
    {
        phase = toneState = noiseState = 0.f;
        lp1 = bp1 = hp1 = lp2 = bp2 = hp2 = 0.f;
        /* optional chromatic tracking */
        float f = tone.get() * powf(2.f, (note - 60) / 12.f);
        tone.set(f);
    }

private:
    /* 1-pole resonator (low-pass ring) */
    float resonator(float in, float f, float d, float& s)
    {
        s += f / props.sampleRate;
        return in * expf(-d * s) * sinf(2.f * M_PI * f * s);
    }

    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float applyEffects(float x)
    {
        float d = drive.pct() * 2.f - 1.f;
        if (d > 0.f)
            x = applyDrive(x, d, props.lookupTable);
        else
            x = applyBoost(x, -d, prevInput, prevOutput);
        return applyReverb(x, reverbAmt.pct(), reverbBuf, rIdx, REVBUF);
    }

    float whiteNoise()
    {
        return props.lookupTable->getNoise();
    }
};