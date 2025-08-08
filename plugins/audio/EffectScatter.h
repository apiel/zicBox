#pragma once

#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/effects/applySample.h"

#define MAX_SCATTER_EFFECTS 10

class EffectScatter : public Mapping {
protected:
    float velocity = 0.0f;
    bool effectActive[MAX_SCATTER_EFFECTS] = { false };

public:
    EffectScatter(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float out = buf[track];
        if (effectActive[0]) {
            out = fxSampleReducer(out, 0.5f * velocity);
        }
        if (effectActive[1]) {
            out = fxBitcrusher(out, 0.25 * velocity);
        }
        if (effectActive[2]) {
            out = fxTremolo(out, velocity);
        }
        if (effectActive[3]) {
            out = fxRingMod(out, velocity);
        }
        if (effectActive[4]) {
            out = fxLowPass(out, 0.5 * velocity);
        }
        if (effectActive[5]) {
            out = fxHighPass(out, 0.5 * velocity);
        }
        if (effectActive[6]) {
            out = fxWavefold(out, velocity);
        }
        if (effectActive[7]) {
            out = fxOverdrive(out, velocity);
        }
        if (effectActive[8]) {
            out = fxDecimator(out, velocity);
        }
        if (effectActive[9]) {
            out = fxSlapback(out, velocity);
        }
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (_velocity == 0.0f) {
            return noteOff(note, _velocity);
        }
        // logDebug("EffectScatter noteOn: %d %f\n", note, _velocity);
        effectActive[note % MAX_SCATTER_EFFECTS] = true;
        velocity = _velocity;
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        // logDebug("EffectScatter noteOff: %d %f\n", note, _velocity);
        effectActive[note % MAX_SCATTER_EFFECTS] = false;
    }

protected:
    float sampleSqueeze;
    int samplePosition = 0;
    float fxSampleReducer(float input, float amount)
    {
        return applySampleReducer(input, amount, sampleSqueeze, samplePosition);
    }

    float sampleHold = 0.0f;
    int sampleCounter = 0;
    float fxBitcrusher(float input, float amount)
    {
        return applyBitcrusher(input, amount, sampleHold, sampleCounter);
    }

    float tremoloPhase = 0.0f;
    float fxTremolo(float input, float amount)
    {
        return applyTremolo(input, amount, tremoloPhase);
    }

    float ringPhase = 0.0f; // Phase for the sine wave oscillator
    float fxRingMod(float input, float amount)
    {
        return applyRingMod(input, amount, ringPhase, props.sampleRate);
    }

    // Filter state
    float fBuf = 0.0f;
    float fHP = 0.0f;
    float fBP = 0.0f;
    float fLP = 0.0f;
    // Parameter
    float cutoff = 0.05f; // 0..1
    inline void setSampleData(float inputValue, float& _buf, float& _hp, float& _bp, float& _lp)
    {
        _hp = inputValue - _buf;
        _bp = _buf - _lp;
        _buf = _buf + cutoff * _hp;
        _lp = _lp + cutoff * (_buf - _lp);
    }
    // Low-pass FX
    float fxLowPass(float input, float amount)
    {
        cutoff = amount * 0.25f; // map velocity/knob to cutoff speed
        setSampleData(input, fBuf, fHP, fBP, fLP);
        return fLP;
    }
    // High-pass FX
    float fxHighPass(float input, float amount)
    {
        cutoff = amount * 0.25f;
        setSampleData(input, fBuf, fHP, fBP, fLP);
        return fHP;
    }

    float fxWavefold(float input, float amount)
    {
        float threshold = 1.0f - amount;
        if (fabs(input) > threshold) {
            input = threshold - fabs(input - threshold);
        }
        return input;
    }

    float fxOverdrive(float input, float amount)
    {
        // Scale amount to reasonable gain
        float drive = 1.0f + amount * 20.0f;
        float x = input * drive;
        // Soft clip using tanh
        return tanh(x) / tanh(drive);
    }

    float decimHold = 0.0f;
    int decimCounter = 0;
    float fxDecimator(float input, float amount)
    {
        int interval = 1 + (int)(amount * 30.0f); // Downsample ratio
        if (decimCounter % interval == 0) {
            decimHold = input;
        }
        decimCounter++;
        return decimHold;
    }

    static const int delaySize = 1024;
    float delayBuffer[delaySize] = { 0 };
    int delayIndex = 0;

    float fxSlapback(float input, float amount)
    {
        int delaySamples = 50 + (int)(amount * 400); // 50-450 samples
        int readIndex = (delayIndex - delaySamples + delaySize) % delaySize;
        float delayed = delayBuffer[readIndex];

        delayBuffer[delayIndex] = input + delayed * 0.3f; // light feedback
        delayIndex = (delayIndex + 1) % delaySize;

        return input * 0.7f + delayed * 0.7f;
    }
};
