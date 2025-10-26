#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class Additive2Engine : public Engine {
protected:
    MultiFx multiFx;
    MMfilter filter;

    float velocity = 1.0f;
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float phase3 = 0.0f;
    float phase4 = 0.0f;

    // params
    float harm1Level = 0.0f;
    float harm2Level = 0.0f;
    float harm3Level = 0.0f;
    float brightness = 0.5f;
    float noiseMix = 0.0f;

public:
    // --- 10 parameters ---
    Val& body = val(0.0f, "BODY", { .label = "Body", .type = VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    Val& harm1 = val(50.0f, "HARM1", { .label = "Harm2", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        harm1Level = p.val.pct();
    });

    Val& harm2 = val(50.0f, "HARM2", { .label = "Harm3", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        harm2Level = p.val.pct();
    });

    Val& harm3 = val(50.0f, "HARM3", { .label = "Harm4", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        harm3Level = p.val.pct();
    });

    Val& bright = val(50.0f, "BRIGHTNESS", { .label = "Brightness", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        brightness = p.val.pct();
    });

    Val& noise = val(0.0f, "NOISE", { .label = "Noise", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        noiseMix = p.val.pct();
    });

    Val& cutoff = val(0.0f, "CUTOFF", { .label = "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });

    Val& resonance = val(0.0f, "RESONANCE", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    // --- constructor ---
    Additive2Engine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Aditiv2")
        , multiFx(props.sampleRate, props.lookupTable)
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

        // Base frequency
        float freq = baseFreq;

        // harmonic frequencies
        float f1 = freq;
        float f2 = freq * 2.0f;
        float f3 = freq * 3.0f;
        float f4 = freq * 4.0f;

        // step per sample
        float step1 = f1 / props.sampleRate;
        float step2 = f2 / props.sampleRate;
        float step3 = f3 / props.sampleRate;
        float step4 = f4 / props.sampleRate;

        // advance phases
        phase1 = fmodf(phase1 + step1, 1.0f);
        phase2 = fmodf(phase2 + step2, 1.0f);
        phase3 = fmodf(phase3 + step3, 1.0f);
        phase4 = fmodf(phase4 + step4, 1.0f);

        // sine waves
        float s1 = sinf(phase1 * 2.0f * M_PI);
        float s2 = sinf(phase2 * 2.0f * M_PI);
        float s3 = sinf(phase3 * 2.0f * M_PI);
        float s4 = sinf(phase4 * 2.0f * M_PI);

        // combine with levels
        float harmonicMix = s1 * (1.0f - brightness)
            + s2 * harm1Level
            + s3 * harm2Level
            + s4 * harm3Level * brightness;

        // noise
        float n = props.lookupTable->getNoise();
        float out = harmonicMix * (1.0f - noiseMix) + n * noiseMix;

        // filter + FX
        out = filter.process(out);
        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        phase1 = phase2 = phase3 = phase4 = 0.0f;
        setBaseFreq(body.get(), note);
    }
};
