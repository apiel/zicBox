/** Description:
This C++ component acts as a versatile digital effects rack for processing audio signals. It is designed as a centralized control unit, housing over 25 distinct sound modifications, ranging from atmospheric effects like various Reverbs and Delays, to aggressive distortions like Drive, Bitcrusher, and Clipping, as well as utility tools such as Compression and advanced frequency Filters.

The key efficiency of this design lies in its dynamic switching capability. Instead of checking which effect is active for every single audio sample, the class uses an internal dynamic selector (a specialized pointer) that always points directly to the function of the currently chosen effect.

When a user selects a new effect (e.g., moving from ‘Tremolo’ to ‘Flanger’), a dedicated control routine immediately redirects this internal selector to the proper processing logic. The main "apply" function then simply executes whatever code the selector is currently pointing to, ensuring rapid, efficient processing.

For high-quality, real-time results, the class relies on essential background data, including the audio system's clock speed (sample rate) and pre-calculated mathematical tables (like sine waves or complex curves). This architecture makes the system quick to switch effects and robust enough to handle a wide spectrum of sound shaping tasks.

Tags: Multi-effects, Sound Processing, Reverb and Delay, Distortion, Audio Filtering
sha: 158e510ea6c33799bfaf38276413d9c2f1816d1f8f0c2cb7cece6a4fb06fc06c
*/
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
#include "audio/lookupTable.h"
#include "audio/utils/linearInterpolation.h"
#include "audio/utils/math.h"
#include "plugins/audio/mapping.h"

class MultiFx {
protected:
    uint64_t sampleRate;
    LookupTable* lookupTable;

    typedef float (MultiFx::*FnPtr)(float, float);
    FnPtr fxFn = &MultiFx::fxOff;

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
        return applyWaveshapeLut(input, amount, lookupTable);
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

public:
    enum FXType {
        FX_OFF,
        REVERB,
        REVERB2,
        REVERB3,
        DELAY,
        DELAY2,
        DELAY3,
        BASS_BOOST,
        DRIVE,
        COMPRESSION,
        WAVESHAPER,
        WAVESHAPER2,
        CLIPPING,
        SAMPLE_REDUCER,
        BITCRUSHER,
        INVERTER,
        TREMOLO,
        FLANGER,
        RING_MOD,
        FX_SHIMMER_REVERB,
        FX_SHIMMER2_REVERB,
        FX_FEEDBACK,
        DECIMATOR,
        LPF,
        HPF,
        HPF_DIST,
        FX_COUNT
    };
    Val::CallbackFn setFxType = [&](auto p) {
        p.val.setFloat(p.value);
        MultiFx::FXType type = (MultiFx::FXType)p.value;
        if (type == MultiFx::FXType::FX_OFF) {
            p.val.setString("OFF");
            fxFn = &MultiFx::fxOff;
        } else if (type == MultiFx::FXType::REVERB) {
            p.val.setString("Reverb");
            fxFn = &MultiFx::fxReverb;
        } else if (type == MultiFx::FXType::REVERB2) {
            p.val.setString("Reverb2");
            fxFn = &MultiFx::fxReverb2;
        } else if (type == MultiFx::FXType::REVERB3) {
            p.val.setString("Reverb3");
            fxFn = &MultiFx::fxReverb3;
        } else if (type == MultiFx::FXType::DELAY) {
            p.val.setString("Delay");
            fxFn = &MultiFx::fxDelay;
        } else if (type == MultiFx::FXType::DELAY2) {
            p.val.setString("Delay2");
            fxFn = &MultiFx::fxDelay2;
        } else if (type == MultiFx::FXType::DELAY3) {
            p.val.setString("Delay3");
            fxFn = &MultiFx::fxDelay3;
        } else if (type == MultiFx::FXType::BASS_BOOST) {
            p.val.setString("Bass boost");
            fxFn = &MultiFx::fxBoost;
        } else if (type == MultiFx::FXType::DRIVE) {
            p.val.setString("Drive");
            fxFn = &MultiFx::fxDrive;
        } else if (type == MultiFx::FXType::COMPRESSION) {
            p.val.setString("Compressor");
            fxFn = &MultiFx::fxCompressor;
        } else if (type == MultiFx::FXType::WAVESHAPER) {
            p.val.setString("Waveshap.");
            fxFn = &MultiFx::fxWaveshaper;
        } else if (type == MultiFx::FXType::WAVESHAPER2) {
            p.val.setString("Waveshap2");
            fxFn = &MultiFx::fxWaveshaper2;
        } else if (type == MultiFx::FXType::CLIPPING) {
            p.val.setString("Clipping");
            fxFn = &MultiFx::fxClipping;
        } else if (type == MultiFx::FXType::SAMPLE_REDUCER) {
            p.val.setString("Sample red.");
            fxFn = &MultiFx::fxSampleReducer;
        } else if (type == MultiFx::FXType::BITCRUSHER) {
            p.val.setString("Bitcrusher");
            fxFn = &MultiFx::fxBitcrusher;
        } else if (type == MultiFx::FXType::INVERTER) {
            p.val.setString("Inverter");
            fxFn = &MultiFx::fxInverter;
        } else if (type == MultiFx::FXType::TREMOLO) {
            p.val.setString("Tremolo");
            fxFn = &MultiFx::fxTremolo;
        } else if (type == MultiFx::FXType::FLANGER) {
            p.val.setString("Flanger");
            fxFn = &MultiFx::fxFlanger;
        } else if (type == MultiFx::FXType::RING_MOD) {
            p.val.setString("Ring mod.");
            fxFn = &MultiFx::fxRingMod;
        } else if (type == MultiFx::FXType::FX_SHIMMER_REVERB) {
            p.val.setString("Shimmer");
            fxFn = &MultiFx::fxShimmerReverb;
        } else if (type == MultiFx::FXType::FX_SHIMMER2_REVERB) {
            p.val.setString("Shimmer2");
            fxFn = &MultiFx::fxShimmer2Reverb;
        } else if (type == MultiFx::FXType::FX_FEEDBACK) {
            p.val.setString("Feedback");
            fxFn = &MultiFx::fxFeedback;
        } else if (type == MultiFx::FXType::DECIMATOR) {
            p.val.setString("Decimator");
            fxFn = &MultiFx::fxDecimator;
        } else if (type == MultiFx::FXType::LPF) {
            p.val.setString("LPF");
            fxFn = &MultiFx::fxLowPass;
        } else if (type == MultiFx::FXType::HPF) {
            p.val.setString("HPF");
            fxFn = &MultiFx::fxHighPass;
        } else if (type == MultiFx::FXType::HPF_DIST) {
            p.val.setString("HPF dist.");
            fxFn = &MultiFx::fxHighPassFilterDistorted;
        }
        // TODO: add fx sample reducer
    };

    MultiFx(uint64_t sampleRate, LookupTable* lookupTable)
        : sampleRate(sampleRate)
        , lookupTable(lookupTable)
    {
    }

    float apply(float in, float amount)
    {
        return (this->*fxFn)(in, amount);
    }
};
