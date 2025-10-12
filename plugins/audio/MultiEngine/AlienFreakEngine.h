#pragma once

#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MultiFx.h"
#include "helpers/math.h"
#include <cmath>
#include <cstdlib>

// FIXME

class AlienFreakEngine : public Engine {
protected:
    static constexpr int NUM_OSC = 3;

    MultiFx multiFx;

    float baseFreq = 100.0f;
    float velocity = 1.0f;

    float oscPhases[NUM_OSC] = {0.0f};
    float oscRatios[NUM_OSC] = {1.0f, 2.3f, 3.7f}; // inharmonic ratios
    float glitchState[NUM_OSC] = {0.0f};

    // LFO for sample-and-hold modulation
    float lfoPhase = 0.0f;
    float lfoRateHz = 1.0f;
    float lfoDepth = 0.0f;

    float noiseLow = 0.0f;
    float noiseHigh = 0.0f;
    float detune = 0.0f;
    float glitchAmount = 0.0f;

    float sampleOsc(int idx, float freq) {
        float phaseInc = 2.0f * M_PI * freq / props.sampleRate;
        oscPhases[idx] += phaseInc;
        if (oscPhases[idx] > 2.0f * M_PI) oscPhases[idx] -= 2.0f * M_PI;

        return fastSin2(oscPhases[idx]);
    }

public:
    // --- 10 parameters ---
    Val& basePitch = val(0.0f, "BASE_PITCH", {"Base Pitch", VALUE_CENTERED, .min=-24, .max=24}, [&](auto p){
        p.val.setFloat(p.value);
        baseFreq = 220.0f * powf(2.0f, p.val.get() / 12.0f);
    });

    Val& inharm = val(50.0f, "INHARM", {"Inharmonics", .unit="%"}, [&](auto p){
        float pct = p.val.pct();
        oscRatios[1] = 2.0f + pct * 2.0f; // 2..4.0
        oscRatios[2] = 3.0f + pct * 2.0f; // 3..5.0
    });

    Val& glitch = val(50.0f, "GLITCH", {"Glitch", .unit="%"}, [&](auto p){
        glitchAmount = p.val.pct();
    });

    Val& noiseLowParam = val(25.0f, "NOISE_LOW", {"Noise Low", .unit="%"}, [&](auto p){
        noiseLow = p.val.pct();
    });

    Val& noiseHighParam = val(25.0f, "NOISE_HIGH", {"Noise High", .unit="%"}, [&](auto p){
        noiseHigh = p.val.pct();
    });

    Val& lfoRateParam = val(1.0f, "LFO_RATE", {"LFO Rate", .min=0.1f, .max=20.0f, .unit="Hz"}, [&](auto p){
        lfoRateHz = p.value;
    });

    Val& lfoDepthParam = val(50.0f, "LFO_DEPTH", {"LFO Depth", .unit="%"}, [&](auto p){
        lfoDepth = p.val.pct();
    });

    Val& detuneParam = val(10.0f, "DETUNE", {"Detune", .unit="%"}, [&](auto p){
        detune = p.val.pct() * 0.05f;
    });

    Val& fxType = val(0, "FX_TYPE", {"FX Type", VALUE_STRING, .max=MultiFx::FXType::FX_COUNT-1}, multiFx.setFxType);

    Val& fxAmount = val(50.0f, "FX_AMOUNT", {"FX Amount", .unit="%"});

    // --- constructor ---
    AlienFreakEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "AlienFreak")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override {
        if(envAmpVal == 0.0f){
            float out = buf[track];
            out = multiFx.apply(out, fxAmount.pct());
            buf[track] = out;
            return;
        }

        // --- LFO update ---
        float lfoInc = 2.0f * M_PI * lfoRateHz / props.sampleRate;
        lfoPhase += lfoInc;
        if(lfoPhase > 2.0f * M_PI) lfoPhase -= 2.0f * M_PI;
        float lfoVal = fastSin2(lfoPhase) * lfoDepth; // ±LFO depth

        float sampleSum = 0.0f;
        for(int i=0;i<NUM_OSC;i++){
            float freq = baseFreq * oscRatios[i];
            // add detune
            freq *= 1.0f + ((float)i - NUM_OSC/2.0f)*detune;
            // apply LFO to pitch
            freq *= 1.0f + lfoVal*0.02f;
            // apply glitch randomly
            if(((float)rand()/RAND_MAX) < glitchAmount*0.01f){
                freq *= 0.5f + (float)rand()/RAND_MAX;
            }
            sampleSum += sampleOsc(i,freq);
        }

        float out = sampleSum / NUM_OSC;

        // // noise layers
        // float nLow = props.lookupTable->getNoise() * noiseLow;
        // float nHigh = props.lookupTable->getNoise() * noiseHigh;
        // out = out * (1.0f - noiseLow - noiseHigh) + nLow + nHigh;

        out *= envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override {
        velocity = _velocity;
        baseFreq = 220.0f * powf(2.0f,(note-60)/12.0f);
        for(int i=0;i<NUM_OSC;i++){
            oscPhases[i] = 0.0f;
        }
        lfoPhase = 0.0f;
    }
};
