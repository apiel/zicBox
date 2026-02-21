#pragma once

#include "audio/effects/applyBitcrusher.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyFlanger.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applyRingMod.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyTremolo.h"
#include "audio/effects/applyWaveshape.h"

#include "audio/utils/math.h"
#include "helpers/clamp.h"

class MFx {
public:
    typedef float (MFx::*FnPtr)(float, float);

    struct FxMapping {
        FnPtr function;
        const char* name;
    };

protected:
    uint64_t sampleRate;
    FnPtr fxFn = &MFx::fxOff;
    int currentIndex = 0;

    float fxOff(float input, float) { return input; }

    DELAY_BUFFER

    int bufferIndex = 0;
    float fxReverb(float signal, float amount)
    {
        float reverbAmount = amount;
        return applyReverb(signal, reverbAmount, buffer, bufferIndex);
    }

    float fxShimmerReverb(float input, float amount)
    {
        return applyShimmerReverb(input, amount, buffer, bufferIndex);
    }

    int shimmerTime = 0;
    float fxShimmer2Reverb(float input, float amount)
    {
        return applyShimmer2Reverb(input, amount, buffer, bufferIndex, shimmerTime);
    }

    float fxReverb2(float signal, float amount)
    {
        return applyReverb2(signal, amount, buffer, bufferIndex);
    }

    float fxReverb3(float signal, float amount)
    {
        return applyReverb3(signal, amount, buffer, bufferIndex);
    }

    float fxDelay(float input, float amount)
    {
        return applyDelay(input, amount, buffer, bufferIndex);
    }

    float fxDelay2(float input, float amount)
    {
        return applyDelay2(input, amount, buffer, bufferIndex);
    }

    float fxDelay3(float input, float amount)
    {
        return applyDelay3(input, amount, buffer, bufferIndex);
    }

    float prevInput = 0;
    float prevOutput = 0;
    float fxBoost(float input, float amount)
    {
        return applyBoost(input, amount, prevInput, prevOutput);
    }

    float fxDrive(float input, float amount)
    {
        return applyDrive(input, amount);
    }

    float fxCompressor(float input, float amount)
    {
        return applyCompression(input, amount);
    }

    float fxWaveshaper(float input, float amount)
    {
        return applyWaveshape(input, amount);
    }

    float fxWaveshaper2(float input, float amount)
    {
        return applyWaveshape2(input, amount);
    }

    float fxWaveshaper3(float input, float amount)
    {
        return applyWaveshape3(input, amount);
    }

    float fxWaveshaper4(float input, float amount)
    {
        return applyWaveshape4(input, amount);
    }

    float fxClipping(float input, float amount)
    {
        if (amount == 0.0f) {
            return input;
        }
        float scaledClipping = amount * amount * 20;
        return CLAMP(input + input * scaledClipping, -1.0f, 1.0f);
    }

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

    float fxInverter(float input, float amount)
    {
        if (input > amount || input < -amount) {
            return -input;
        }
        return input;
    }

    float tremoloPhase = 0.0f;
    float fxTremolo(float input, float amount)
    {
        return applyTremolo(input, amount, tremoloPhase);
    }

    float ringPhase = 0.0f; // Phase for the sine wave oscillator
    float fxRingMod(float input, float amount)
    {
        return applyRingMod(input, amount, ringPhase, sampleRate);
    }

    float fxFeedback(float input, float amount)
    {
        return applyFeedback(input, amount, buffer, bufferIndex, sampleRate);
    }

    float decimHold = 0.0f;
    int decimCounter = 0;
    float fxDecimator(float input, float amount)
    {
        return applyDecimator(input, amount, decimHold, decimCounter);
    }
    float flangerPhase = 0.0f;
    float fxFlanger(float input, float amount)
    {
        return applyFlanger(input, amount, buffer, bufferIndex, DELAY_BUFFER_SIZE, flangerPhase);
    }

    //--------
    // Filters
    //--------

    float lp_z1 = 0.0f; // 1-pole lowpass state
    float hp_z1 = 0.0f; // 1-pole highpass state
    float fxHighPassFilterDistorted(float input, float amount)
    {
        if (amount <= 0.0f)
            return input;

        amount = 1.0f - powf(amount, 0.25f);

        float cutoff = 200.0f + (sampleRate * 0.48f - 200.0f) * amount;
        float alpha = sampleRate / (cutoff + sampleRate);
        hp_z1 = alpha * (hp_z1 + input - (lp_z1 = lp_z1 + (cutoff / (cutoff + sampleRate)) * (input - lp_z1)));
        return hp_z1;
    }

    // Filter 2
    float filterBuf = 0.0;
    float lp = 0.0;
    float hp = 0.0;
    float bp = 0.0;
    void setSampleData(float inputValue, float cutoff, float& _buf, float& _hp, float& _bp, float& _lp)
    {
        _hp = inputValue - _buf;
        _bp = _buf - _lp;
        _buf = _buf + cutoff * _hp;
        _lp = _lp + cutoff * (_buf - _lp);
    }

    float fxLowPass(float input, float amount)
    {
        if (amount <= 0.0f)
            return input;

        amount = (1.0f - amount * 0.95f) + 0.05f;

        setSampleData(input, amount, filterBuf, hp, bp, lp);
        return lp;
    }

    float fxHighPass(float input, float amount)
    {
        if (amount <= 0.0f)
            return input;

        amount = amount * 0.95f;

        setSampleData(input, amount, filterBuf, hp, bp, lp);
        return hp;
    }

    // --- THE REGISTRY ---
    static const FxMapping registry[];

public:
    static const int FX_COUNT;

    MFx(uint64_t sampleRate)
        : sampleRate(sampleRate)
    {
        setEffect(0);
    }

    void setEffect(int index)
    {
        if (index >= 0 && index < FX_COUNT) {
            currentIndex = index;
            fxFn = registry[index].function;
        }
    }

    inline float apply(float in, float amount)
    {
        return (this->*fxFn)(in, amount);
    }

    const char* getEffectName() const
    {
        return registry[currentIndex].name;
    }

    static int getCount() { return FX_COUNT; }
};

const MFx::FxMapping MFx::registry[] = {
    { &MFx::fxOff, "OFF" },
    { &MFx::fxReverb, "REVERB" },
    { &MFx::fxReverb2, "REVERB2" },
    { &MFx::fxReverb3, "REVERB3" },
    { &MFx::fxDelay, "DELAY" },
    { &MFx::fxDelay2, "DELAY2" },
    { &MFx::fxDelay3, "DELAY3" },
    { &MFx::fxBoost, "BASS_BOOST" },
    { &MFx::fxDrive, "DRIVE" },
    { &MFx::fxCompressor, "COMPRESSION" },
    { &MFx::fxWaveshaper, "WAVESHAPER" },
    { &MFx::fxWaveshaper2, "WAVESHAPER2" },
    { &MFx::fxWaveshaper3, "WAVESHAPER3" },
    { &MFx::fxWaveshaper4, "WAVESHAPER4" },
    { &MFx::fxClipping, "CLIPPING" },
    { &MFx::fxSampleReducer, "SAMPLE_REDUCER" },
    { &MFx::fxBitcrusher, "BITCRUSHER" },
    { &MFx::fxInverter, "INVERTER" },
    { &MFx::fxTremolo, "TREMOLO" },
    { &MFx::fxFlanger, "FLANGER" },
    { &MFx::fxRingMod, "RING_MOD" },
    { &MFx::fxShimmerReverb, "FX_SHIMMER_REVERB" },
    { &MFx::fxShimmer2Reverb, "FX_SHIMMER2_REVERB" },
    { &MFx::fxFeedback, "FX_FEEDBACK" },
    { &MFx::fxDecimator, "DECIMATOR" },
    { &MFx::fxLowPass, "LPF" },
    { &MFx::fxHighPass, "HPF" },
    { &MFx::fxHighPassFilterDistorted, "HPF_DIST" }
};

const int MFx::FX_COUNT = sizeof(MFx::registry) / sizeof(MFx::FxMapping);
