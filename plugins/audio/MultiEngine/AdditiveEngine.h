#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MultiFx.h"

class AdditiveEngine : public Engine {
protected:
    static constexpr int MAX_PARTIALS = 8;
    MultiFx multiFx;

    float velocity = 1.0f;
    float harmonicDecay = 1.0f;

    float phases[MAX_PARTIALS] = { 0.0f };

    // LFO state
    float lfoPhase = 0.0f;

    // odd/even bias
    float oddEvenBias = 0.5f;

public:
    // --- 10 parameters ---
    Val& body = val(0.0f, "BODY", { "Body", VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    Val& harmonics = val(50.0f, "HARMONICS", { "Harmonics", .unit = "%" });

    Val& spread = val(0.0f, "SPREAD", { "Spread", .unit = "%" });

    Val& inharm = val(0.0f, "INHARM", { "Inharm", .unit = "%" });

    Val& harmonicDecayVal = val(50.0f, "HARM_DECAY", { "Decay", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        harmonicDecay = 0.2f + p.val.pct() * 2.0f; // rolloff
    });

    Val& oddEven = val(50.0f, "ODDEVEN", { "Odd/Even", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        oddEvenBias = p.val.pct(); // 0 = only even, 1 = only odd, 0.5 = neutral
    });

    Val& lfoRate = val(0.0f, "LFO_RATE", { "LFO Rate", .min = 0.0f, .max = 100.0f, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });

    Val& lfoDepth = val(0.0f, "LFO_DEPTH", { "LFO Depth", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

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

            // --- Odd/Even bias ---
            bool isOdd = ((i + 1) % 2 == 1);
            float biasAmp = isOdd ? oddEvenBias : (1.0f - oddEvenBias);

            float amp = powf(1.0f / (i + 1), harmonicDecay) * biasAmp;
            sampleSum += fastSin2(phases[i]) * amp;
        }

        float out = sampleSum / numPartials;

        // --- Apply LFO (tremolo) ---
        if (lfoRate.get() > 0.0f) {
            float lfoInc = 2.0f * M_PI * lfoRate.get() / props.sampleRate;
            lfoPhase += lfoInc;
            if (lfoPhase > 2.0f * M_PI)
                lfoPhase -= 2.0f * M_PI;
            float lfoVal = 0.5f * (1.0f + fastSin2(lfoPhase)); // [0..1]
            out *= (1.0f - lfoDepth.pct()) + lfoVal * lfoDepth.pct();
        }

        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setBaseFreq(body.get(), note);
        for (int i = 0; i < MAX_PARTIALS; i++) {
            phases[i] = 0.0f;
        }
        lfoPhase = 0.0f;
    }
};
