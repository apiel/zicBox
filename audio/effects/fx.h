#pragma once

#include "audio/effects/applyBitcrusher.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyClipping.h"
#include "audio/effects/applyCompression.h"
#include "audio/effects/applyDecimator.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyHighFreqBoost2.h"
#include "audio/effects/applyHPFDistorded.h"
#include "audio/effects/applyRingModFast.h"
#include "audio/effects/applySampleReducer.h"
#include "audio/effects/applyTremoloFast.h"
#include "audio/effects/applyWaveshape.h"
#include <string>

class Fx {
protected:
    int sampleRate;
    LookupTable* lookupTable;

    typedef float (Fx::*FnPtr)(float, float);
    FnPtr fxFn = &Fx::fxOff;

    float fxOff(float input, float) { return input; }

    int iData1 = 0;
    float fData1 = 0;
    float fData2 = 0;

    float fxHPFDistorded(float input, float amount)
    {
        return applyHPFDistorded(input, amount, sampleRate, fData1, fData2);
    }

    float fxHighFreqBoost(float input, float amount)
    {
        return applyHighFreqBoost2(input, amount, fData1, fData2);
    }

    float fxBoost(float input, float amount)
    {
        return applyBoost(input, amount, fData1, fData2);
    }

    float fxDrive(float input, float amount)
    {
        return applyDrive(input, amount, lookupTable);
    }

    float fxDecimator(float input, float amount)
    {
        return applyDecimator(input, amount, fData1, iData1);
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
        return applyWaveshapeLut(input, amount, lookupTable);
    }

    float fxClipping(float input, float amount)
    {
        return applyClipping(input, amount);
    }

    float fxSampleReducer(float input, float amount)
    {
        return applySampleReducer(input, amount, fData1, iData1);
    }

    float fxBitcrusher(float input, float amount)
    {
        return applyBitcrusher(input, amount, fData1, iData1);
    }

    float fxTremolo(float input, float amount)
    {
        return applyTremoloFast(input, amount, fData1);
    }

    float fxRingMod(float input, float amount)
    {
        return applyRingModFast(input, amount, fData1, fData2);
    }

public:
    Fx(int sampleRate, LookupTable* lookupTable)
        : sampleRate(sampleRate)
        , lookupTable(lookupTable)
    {
    }

    static const int8_t count = 14;

protected:
    int8_t index = 0;
    struct FxList {
        std::string name;
        std::string shortName;
        Fx::FnPtr fxFn;
    } list[count] = {
        { "Off", "Off", &Fx::fxOff },
        { "HPF Distorded", "HPF", &Fx::fxHPFDistorded },
        { "High Freq Boost", "HFB", &Fx::fxHighFreqBoost },
        { "Boost", "Boost", &Fx::fxBoost },
        { "Drive", "Driv", &Fx::fxDrive },
        { "Decimator", "Decim", &Fx::fxDecimator },
        { "Compressor", "Comp", &Fx::fxCompressor },
        { "Waveshaper", "Shapr", &Fx::fxWaveshaper },
        { "Waveshaper2", "Shap2", &Fx::fxWaveshaper2 },
        { "Clipping", "Clip", &Fx::fxClipping },
        { "Sample Reducer", "Reduc", &Fx::fxSampleReducer },
        { "Bitcrusher", "Crush", &Fx::fxBitcrusher },
        { "Tremolo", "Trem", &Fx::fxTremolo },
        { "Ring Mod", "RingM", &Fx::fxRingMod },
    };

public:
    int8_t getIndex() { return index; }
    std::string getName() { return list[index].name; }
    std::string getShortName() { return list[index].shortName; }
    void set(int8_t idx)
    {
        index = CLAMP(idx, 0, count - 1);
        fxFn = list[index].fxFn;
    }

    float apply(float in, float amount)
    {
        return (this->*fxFn)(in, amount);
    }

    void set(std::string shortName)
    {
        for (int8_t i = 0; i < count; i++) {
            if (list[i].shortName == shortName) {
                index = i;
                fxFn = list[i].fxFn;
                break;
            }
        }
    }
};