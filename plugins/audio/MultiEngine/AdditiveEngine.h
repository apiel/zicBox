#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class AdditiveEngine : public Engine {
protected:
    static constexpr int MAX_PARTIALS = 8;
    MultiFx multiFx;
    MMfilter filter;

    float baseFreq = 100.0f;
    float velocity = 1.0f;
    float harmonicDecay = 1.0f;

    float noiseMix = 0.0f;

    float phases[MAX_PARTIALS] = { 0.0f };

    inline float fastSin(float x)
    {
        // sine approx: keep x in [-pi, pi]
        if (x < -M_PI)
            x += 2 * M_PI;
        else if (x > M_PI)
            x -= 2 * M_PI;

        // Bhaskara approximation, quite cheap & accurate
        float y = (16.0f * x * (M_PI - fabsf(x))) / (5.0f * M_PI * M_PI - 4.0f * fabsf(x) * (M_PI - fabsf(x)));
        return y;
    }

public:
    // --- 10 parameters ---
    Val& body = val(0.0f, "BODY", { "Body", VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        baseFreq = 220.0f * powf(2.0f, p.val.get() / 12.0f);
    });

    Val& harmonics = val(50.0f, "HARMONICS", { "Harmonics", .unit = "%" });

    Val& spread = val(0.0f, "SPREAD", { "Spread", .unit = "%" });

    Val& inharm = val(0.0f, "INHARM", { "Inharm", .unit = "%" });

    Val& harmonicDecayVal = val(50.0f, "HARM_DECAY", { "Decay", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        harmonicDecay = 0.2f + p.val.pct() * 2.0f; // rolloff
    });

    Val& noise = val(0.0f, "NOISE", { "Noise", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        noiseMix = p.val.pct();
    });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });

    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    // --- constructor ---
    AdditiveEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Aditiv")
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

        int numPartials = 1 + int(harmonics.pct() * (MAX_PARTIALS - 1));
        float spreadAmt = spread.pct() * 0.1f;
        float inharmAmt = inharm.pct() * 0.5f;

        float sampleSum = 0.0f;
        for (int i = 0; i < numPartials; i++) {
            float freq = baseFreq * (i + 1 + inharmAmt * i);
            freq *= (1.0f + (i - numPartials / 2) * spreadAmt);

            float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
            phases[i] += phaseInc;
            if (phases[i] > 2.0f * M_PI)
                phases[i] -= 2.0f * M_PI;

            float amp = powf(1.0f / (i + 1), harmonicDecay);
            sampleSum += fastSin(phases[i]) * amp;
        }

        float out = sampleSum / numPartials;

        // noise
        float n = props.lookupTable->getNoise();
        out = out * (1.0f - noiseMix) + n * noiseMix;

        out = filter.process(out);
        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        baseFreq = 220.0f * powf(2.0f, (note - 60) / 12.0f);
        for (int i = 0; i < MAX_PARTIALS; i++) {
            phases[i] = 0.0f;
        }
    }
};
