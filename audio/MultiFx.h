#pragma once

#include "audio/effects/applyBitcrusher.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyFlanger.h"
#include "audio/effects/applyHPFDistorded.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applyRingMod.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyTremolo.h"
#include "audio/effects/applyWaveshape.h"

#include "audio/utils/math.h"
#include "helpers/clamp.h"

class MultiFx {
public:
    typedef float (MultiFx::*FnPtr)(float, float);

    struct FxMapping {
        FnPtr function;
        const char* name;
        const char* shortName;
    };

protected:
    uint64_t sampleRate;
    FnPtr fxFn = &MultiFx::fxOff;
    int currentIndex = 0;

    float fxOff(float input, float) { return input; }

    FX_BUFFER

    int bufferIndex = 0;
    int iData1 = 0;
    float fData1 = 0.0f;
    float fData2 = 0.0f;

    float fxReverb(float signal, float amount) { return applyReverb(signal, amount, buffer, bufferIndex); }
    float fxShimmerReverb(float input, float amount) { return applyShimmerReverb(input, amount, buffer, bufferIndex); }
    float fxShimmer2Reverb(float input, float amount) { return applyShimmer2Reverb(input, amount, buffer, bufferIndex, iData1); }
    float fxReverb2(float signal, float amount) { return applyReverb2(signal, amount, buffer, bufferIndex); }
    float fxReverb3(float signal, float amount) { return applyReverb3(signal, amount, buffer, bufferIndex); }
    float fxDelay(float input, float amount) { return applyDelay(input, amount, buffer, bufferIndex); }
    float fxDelay2(float input, float amount) { return applyDelay2(input, amount, buffer, bufferIndex); }
    float fxDelay3(float input, float amount) { return applyDelay3(input, amount, buffer, bufferIndex); }
    float fxBoost(float input, float amount) { return applyBoost(input, amount, fData1, fData2); }
    float fxDrive(float input, float amount) { return applyDrive(input, amount); }
    float fxCompressor(float input, float amount) { return applyCompression(input, amount); }
    float fxWaveshaper(float input, float amount) { return applyWaveshape(input, amount); }
    float fxWaveshaper2(float input, float amount) { return applyWaveshape2(input, amount); }
    float fxWaveshaper3(float input, float amount) { return applyWaveshape3(input, amount); }
    float fxWaveshaper4(float input, float amount) { return applyWaveshape4(input, amount); }
    float fxSampleReducer(float input, float amount) { return applySampleReducer(input, amount, fData1, iData1); }
    float fxBitcrusher(float input, float amount) { return applyBitcrusher(input, amount, fData1, iData1); }
    float fxTremolo(float input, float amount) { return applyTremolo(input, amount, fData1); }
    float fxRingMod(float input, float amount) { return applyRingMod(input, amount, fData1, sampleRate); }
    float fxFeedback(float input, float amount) { return applyFeedback(input, amount, buffer, bufferIndex, sampleRate); }
    float fxDecimator(float input, float amount) { return applyDecimator(input, amount, fData1, iData1); }
    float fxFlanger(float input, float amount) { return applyFlanger(input, amount, buffer, bufferIndex, fData1); }
    float fxHighPassFilterDistorted(float input, float amount) { return applyHPFDistorded(input, amount, sampleRate, fData1, fData2); }

    float fxClipping(float input, float amount)
    {
        if (amount == 0.0f) {
            return input;
        }
        float scaledClipping = amount * amount * 20;
        return CLAMP(input + input * scaledClipping, -1.0f, 1.0f);
    }

    float fxInverter(float input, float amount)
    {
        if (input > amount || input < -amount) {
            return -input;
        }
        return input;
    }

    //--------
    // Filters
    //--------

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

    MultiFx(uint64_t sampleRate)
        : sampleRate(sampleRate)
    {
        setEffect(0);
    }

    void setEffect(int index)
    {
        if (index >= 0 && index < FX_COUNT) {
            currentIndex = index;
            fxFn = registry[index].function;
            fData1 = 0.0f;
            fData2 = 0.0f;
            iData1 = 0;
        }
    }

    void setEffect(const char* shortName)
    {
        for (int i = 0; i < FX_COUNT; i++) {
            if (strcmp(registry[i].shortName, shortName) == 0) {
                setEffect(i);
                return;
            }
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

    const char* getEffectShortName() const
    {
        return registry[currentIndex].shortName;
    }

    static int getCount() { return FX_COUNT; }
};

const MultiFx::FxMapping MultiFx::registry[] = {
    { &MultiFx::fxOff, "OFF", "Off" },
    { &MultiFx::fxReverb, "Reverb", "Rvb" },
    { &MultiFx::fxReverb2, "Reverb2", "Rvb2" },
    { &MultiFx::fxReverb3, "Reverb3", "Rvb3" },
    { &MultiFx::fxDelay, "Delay", "Dly" },
    { &MultiFx::fxDelay2, "Delay2", "Dly2" },
    { &MultiFx::fxDelay3, "Delay3", "Dly3" },
    { &MultiFx::fxBoost, "Bass boost", "B.Boost" },
    { &MultiFx::fxDrive, "Drive", "Drv" },
    { &MultiFx::fxCompressor, "Compressor", "Comp." },
    { &MultiFx::fxWaveshaper, "Waveshap.", "Shapr" },
    { &MultiFx::fxWaveshaper2, "Waveshap2", "Shapr2" },
    { &MultiFx::fxWaveshaper3, "Waveshap3", "Shapr3" },
    { &MultiFx::fxWaveshaper4, "Waveshap4", "Shapr4" },
    { &MultiFx::fxClipping, "Clipping", "Clip" },
    { &MultiFx::fxSampleReducer, "Sample red.", "Reduc" },
    { &MultiFx::fxBitcrusher, "Bitcrusher", "Crush" },
    { &MultiFx::fxInverter, "Inverter", "Inv" },
    { &MultiFx::fxTremolo, "Tremolo", "Trem" },
    { &MultiFx::fxFlanger, "Flanger", "Flng" },
    { &MultiFx::fxRingMod, "Ring mod.", "Ring" },
    { &MultiFx::fxShimmerReverb, "Shimmer", "Shmr" },
    { &MultiFx::fxShimmer2Reverb, "Shimmer2", "Shmr2" },
    { &MultiFx::fxFeedback, "Feedback", "Fb" },
    { &MultiFx::fxDecimator, "Decimator", "Decm" },
    { &MultiFx::fxLowPass, "LPF", "LP" },
    { &MultiFx::fxHighPass, "HPF", "HP" },
    { &MultiFx::fxHighPassFilterDistorted, "HPF dist.", "HPDist" }
};

const int MultiFx::FX_COUNT = sizeof(MultiFx::registry) / sizeof(MultiFx::FxMapping);
