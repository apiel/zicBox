#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"
#include <cmath>

class SuperSawEngine : public Engine {
protected:
    static constexpr int MAX_VOICES = 7; // number of saw layers
    MultiFx multiFx;
    MMfilter filter;

    float baseFreq = 100.0f;
    float velocity = 1.0f;

    float phases[MAX_VOICES] = { 0.0f };

public:
    // --- 10 parameters ---
    Val& body = val(0.0f, "BODY", { "Body", VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        baseFreq = 220.0f * powf(2.0f, p.val.get() / 12.0f);
    });

    Val& voices = val(5.0f, "VOICES", { "Voices", .min = 1, .max = MAX_VOICES }, [&](auto p) {});

    Val& detune = val(10.0f, "DETUNE", { "Detune", .unit = "%" }, [&](auto p) {});

    Val& stereoWidth = val(50.0f, "WIDTH", { "Stereo Width", .unit = "%" }, [&](auto p) {});

    Val& harmonics = val(0.0f, "HARMONICS", { "Extra Harmonics", .unit = "%" }, [&](auto p) {});

    Val& noiseMix = val(0.0f, "NOISE", { "Noise", .unit = "%" });

    Val& filterCutoff = val(0.0f, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });

    Val& filterRes = val(0.0f, "RES", { "Resonance", .unit = "%" }, [&](auto p) {
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    // --- constructor ---
    SuperSawEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "SuperSaw")
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

        int numVoices = 1 + int(voices.pct() * (MAX_VOICES - 1));
        float detuneAmt = detune.pct() * 0.05f;
        float widthAmt = stereoWidth.pct() * 0.5f;

        float sampleSum = 0.0f;
        for (int i = 0; i < numVoices; ++i) {
            float det = ((float)i - numVoices / 2.0f) * detuneAmt;
            float freq = baseFreq * (1.0f + det);

            float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
            phases[i] += phaseInc;
            if (phases[i] > 2.0f * M_PI) phases[i] -= 2.0f * M_PI;

            sampleSum += (2.0f * (phases[i] / (2.0f * M_PI)) - 1.0f); // saw wave approximation
        }

        float out = sampleSum / numVoices;

        // add subtle harmonics
        out += harmonics.pct() * (props.lookupTable->getNoise() * 0.1f);

        // noise
        float n = props.lookupTable->getNoise();
        out = out * (1.0f - noiseMix.pct()) + n * noiseMix.pct();

        out = filter.process(out);
        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        baseFreq = 220.0f * powf(2.0f, (note - 60) / 12.0f);
        for (int i = 0; i < MAX_VOICES; ++i)
            phases[i] = 0.0f;
    }
};
