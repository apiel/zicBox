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
    float oscMorph[NUM_OSC] = { 0.0f }; // 0..1 morph between waveforms

    float lfoPhase = 0.0f;
    float lfoRateHz = 0.5f;
    float lfoDepth = 0.0f;

    inline float fastSin(float x)
    {
        if (x < -M_PI) x += 2.0f * M_PI;
        else if (x > M_PI) x -= 2.0f * M_PI;
        return (16.0f * x * (M_PI - fabsf(x))) / (5.0f * M_PI * M_PI - 4.0f * fabsf(x) * (M_PI - fabsf(x)));
    }

    // waveform morph: 0=sine, 0.33=triangle, 0.66=saw, 1.0=square
    float morphWave(float phase, float morph)
    {
        if (morph <= 0.33f) {
            float t = morph / 0.33f;
            return fastSin(phase) * (1.0f - t) + (2.0f / M_PI) * asin(sin(phase)) * t; // sine -> triangle
        } else if (morph <= 0.66f) {
            float t = (morph - 0.33f) / 0.33f;
            float tri = (2.0f / M_PI) * asin(sin(phase));
            float saw = 2.0f * (phase / (2.0f * M_PI)) - 1.0f;
            return tri * (1.0f - t) + saw * t; // triangle -> saw
        } else {
            float t = (morph - 0.66f) / 0.34f;
            float saw = 2.0f * (phase / (2.0f * M_PI)) - 1.0f;
            float sq = (phase < M_PI) ? 1.0f : -1.0f;
            return saw * (1.0f - t) + sq * t; // saw -> square
        }
    }

    float sampleOsc(int idx, float freq)
    {
        float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
        oscPhases[idx] += phaseInc;
        if (oscPhases[idx] > 2.0f * M_PI)
            oscPhases[idx] -= 2.0f * M_PI;

        return morphWave(oscPhases[idx], morphVal.pct());
    }

public:
    uint8_t pitchNote = 60;

    // --- 10 parameters ---
    Val& basePitch = val(0.0f, "BASE_PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        baseFreq = 50.0f * powf(2.0f, (pitchNote - 60 + p.val.get()) / 12.0f);
    });

    Val& morphVal = val(40.0f, "MORPH", { "Morph", .unit = "%" });

    Val& detune = val(10.0f, "DETUNE", { "Detune", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        float d = p.val.pct();
        oscMorph[0] = d * 0.5f;
        oscMorph[1] = 0.5f;
        oscMorph[2] = 1.0f - d * 0.5f;
    });

    Val& oscMix1 = val(40.0f, "OSC1", { "Osc1 Level", .unit = "%" });
    Val& oscMix2 = val(30.0f, "OSC2", { "Osc2 Level", .unit = "%" });
    Val& oscMix3 = val(30.0f, "OSC3", { "Osc3 Level", .unit = "%" });

    Val& lfoRate = val(0.5f, "LFO_RATE", { "LFO Rate", .min = 0.0f, .max = 10.0f, .step = 0.1f, .floatingPoint = 1, .unit = "Hz" });
    Val& lfoDepthVal = val(20.0f, "LFO_DEPTH", { "LFO Depth", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        lfoDepth = p.val.pct() * 0.2f; // morph modulation depth
    });

    Val& fxType = val(0, "FX_TYPE", { "FX Type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) { multiFx.setFxType(p); });
    Val& fxAmount = val(50.0f, "FX_AMOUNT", { "FX Amount", .unit = "%" });

    // --- constructor ---
    SpaceShipEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "SpaceShip")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            buf[track] = multiFx.apply(buf[track], fxAmount.pct());
            return;
        }

        // LFO modulates oscillator morph
        float lfoVal = 0.0f;
        if (lfoRate.get() > 0.0f) {
            float lfoInc = 2.0f * M_PI * lfoRate.get() / props.sampleRate;
            lfoPhase += lfoInc;
            if (lfoPhase > 2.0f * M_PI) lfoPhase -= 2.0f * M_PI;
            lfoVal = sinf(lfoPhase) * lfoDepth;
        }

        float sampleSum = 0.0f;
        float mixLevels[NUM_OSC] = { oscMix1.pct(), oscMix2.pct(), oscMix3.pct() };

        for (int i = 0; i < NUM_OSC; i++) {
            float morphVal = oscMorph[i] + lfoVal; // morph modulation
            if (morphVal < 0.0f) morphVal = 0.0f;
            if (morphVal > 1.0f) morphVal = 1.0f;

            float freq = baseFreq * (1.0f + i * 0.1f); // small internal detune
            sampleSum += sampleOsc(i, freq) * mixLevels[i];
        }

        float out = sampleSum;
        out *= envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        pitchNote = note;
        velocity = _velocity;
        baseFreq = 50.0f * powf(2.0f, (note - 60 + basePitch.get()) / 12.0f);
        for (int i = 0; i < NUM_OSC; i++) oscPhases[i] = 0.0f;
        lfoPhase = 0.0f;
    }
};
