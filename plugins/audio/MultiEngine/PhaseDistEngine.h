#pragma once

#include "audio/MultiFx.h"
#include "audio/MMfilter.h"
#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

class PhaseDistEngine : public Engine {
protected:
    MultiFx multiFx;
    MultiFx multiFx2;
    MMfilter filter;

    float velocity = 1.0f;
    float phase = 0.0f;
    float vibPhase = 0.0f;

    float pdOscillator(float phase, float distortion)
    {
        float warpedPhase = 0.0f;

        if (phase < distortion) {
            warpedPhase = (phase * 0.5f) / distortion;
        } else {
            warpedPhase = 0.5f + ((phase - distortion) * 0.5f) / (1.0f - distortion);
        }

        return sinf(warpedPhase * 2.0f * M_PI);
    }

public:
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    Val& distortion = val(0.0f, "DIST", { .label = "PD Amount", .unit = "%" });

    Val& detune = val(0.0f, "DETUNE", { .label = "Sub Detune", .unit = "%" });

    Val& envMod = val(50.0f, "ENV_MOD", { .label = "Env > PD", .unit = "%" });

    Val& cutoff = val(0.0f, "CUTOFF", { .label = "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });

    Val& resonance = val(0.0f, "RESONANCE", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX1 Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX1 Edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 Type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);
    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 Edit", .unit = "%" });

    PhaseDistEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "PhasDist")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            float out = buf[track];
            out = multiFx.apply(out, fxAmount.pct());
            buf[track] = out;
            return;
        }

        float currentDist = distortion.pct() + (envMod.pct() * envAmpVal);
        if (currentDist > 0.99f) currentDist = 0.99f;

        phase += (baseFreq) / props.sampleRate;
        if (phase > 1.0f) phase -= 1.0f;

        float out = pdOscillator(phase, 1.0f - currentDist);

        if (detune.pct() > 0.0f) {
            static float subPhase = 0.0f;
            subPhase += (baseFreq * (1.0f - detune.pct() * 0.02f)) / props.sampleRate;
            if (subPhase > 1.0f) subPhase -= 1.0f;
            out = (out + pdOscillator(subPhase, 1.0f - currentDist)) * 0.6f;
        }

        out = filter.process(out);
        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());

        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
        phase = 0.0f;
        vibPhase = 0.0f;
    }
};
