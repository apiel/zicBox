#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MultiFx.h"
#include <cmath>

class SpaceShipEngine : public Engine {
protected:
    static constexpr int NUM_OSC = 3;

    MultiFx multiFx;

    float baseFreq = 50.0f; // low engine hum
    float velocity = 1.0f;

    float oscPhases[NUM_OSC] = { 0.0f };
    float oscRatios[NUM_OSC] = { 1.0f, 1.5f, 2.2f };
    float glideTarget = 50.0f;
    float glideCurrent = 50.0f;
    float glideSpeed = 0.01f;

    float lfoPhase = 0.0f;
    float lfoRateHz = 0.5f;
    float lfoDepth = 0.0f;

    inline float fastSin(float x)
    {
        if (x < -M_PI)
            x += 2 * M_PI;
        else if (x > M_PI)
            x -= 2 * M_PI;
        return (16.0f * x * (M_PI - fabsf(x))) / (5.0f * M_PI * M_PI - 4.0f * fabsf(x) * (M_PI - fabsf(x)));
    }

    float sampleOsc(int idx, float freq)
    {
        float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
        oscPhases[idx] += phaseInc;
        if (oscPhases[idx] > 2.0f * M_PI)
            oscPhases[idx] -= 2.0f * M_PI;
        return fastSin(oscPhases[idx]);
    }

public:
    // --- 10 parameters ---
    Val& basePitch = val(0.0f, "BASE_PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 });

    Val& glide = val(50.0f, "GLIDE", { "Glide Speed", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        glideSpeed = 0.001f + p.val.pct() * 0.05f;
    });

    Val& detune = val(10.0f, "DETUNE", { "Detune", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        oscRatios[0] = 1.0f - p.val.pct() * 0.5f;
        oscRatios[1] = 1.5f;
        oscRatios[2] = 2.2f + p.val.pct() * 0.5f;
    });

    Val& oscMix1 = val(40.0f, "OSC1", { "Osc1 Level", .unit = "%" });
    Val& oscMix2 = val(30.0f, "OSC2", { "Osc2 Level", .unit = "%" });
    Val& oscMix3 = val(30.0f, "OSC3", { "Osc3 Level", .unit = "%" });

    Val& lfoRate = val(0.5f, "LFO_RATE", { "LFO Rate", .min = 0.0f, .max = 10.0f, .step = 0.1f, .floatingPoint = 1, .unit = "Hz" });
    Val& lfoDepthVal = val(20.0f, "LFO_DEPTH", { "LFO Depth", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        lfoDepth = p.val.pct() * 0.05f;
    });

    Val& fxType = val(0, "FX_TYPE", { "FX Type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) { multiFx.setFxType(p); });
    Val& fxAmount = val(50.0f, "FX_AMOUNT", { "FX Amount", .unit = "%" });

    // --- constructor ---
    SpaceShipEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Space")
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

        // glide
        glideCurrent += (glideTarget - glideCurrent) * glideSpeed;

        // LFO
        float lfoVal = 0.0f;
        if (lfoRate.get() > 0.0f) {
            float lfoInc = 2.0f * M_PI * lfoRate.get() / props.sampleRate;
            lfoPhase += lfoInc;
            if (lfoPhase > 2.0f * M_PI)
                lfoPhase -= 2.0f * M_PI;
            lfoVal = fastSin(lfoPhase) * lfoDepth;
        }

        float sampleSum = 0.0f;
        float mixLevels[NUM_OSC] = { oscMix1.pct(), oscMix2.pct(), oscMix3.pct() };

        for (int i = 0; i < NUM_OSC; i++) {
            float freq = glideCurrent * oscRatios[i] * (1.0f + lfoVal);
            sampleSum += sampleOsc(i, freq) * mixLevels[i];
        }

        float out = sampleSum;
        out *= envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        baseFreq = 50.0f * powf(2.0f, (note - 60 + basePitch.get()) / 12.0f);
        glideTarget = baseFreq;
        glideCurrent = baseFreq;
        for (int i = 0; i < NUM_OSC; i++)
            oscPhases[i] = 0.0f;
        lfoPhase = 0.0f;
    }
};
