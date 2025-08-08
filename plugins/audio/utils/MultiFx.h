#pragma once

#include "plugins/audio/mapping.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/effects/applySample.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include "plugins/audio/utils/lookupTable.h"
#include "plugins/audio/utils/utils.h"

class MultiFx {
protected:
    uint64_t sampleRate;
    LookupTable* lookupTable;

    typedef float (MultiFx::*FnPtr)(float, float);
    FnPtr fxFn = &MultiFx::fxOff;

    float fxOff(float input, float) { return input; }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    static constexpr int DELAY_BUFFER_SIZE = REVERB_BUFFER_SIZE * 3; // 3 second
    float buffer[DELAY_BUFFER_SIZE] = { 0.0f };
    int bufferIndex = 0;
    float fxReverb(float signal, float amount)
    {
        float reverbAmount = amount;
        return applyReverb(signal, reverbAmount, buffer, bufferIndex, REVERB_BUFFER_SIZE);
    }

    float fxShimmerReverb(float input, float amount)
    {
        return applyShimmerReverb(input, amount, buffer, bufferIndex, REVERB_BUFFER_SIZE);
    }

    int shimmerTime = 0;
    float fxShimmer2Reverb(float input, float amount)
    {
        return applyShimmer2Reverb(input, amount, buffer, bufferIndex, REVERB_BUFFER_SIZE, shimmerTime);
    }

    float fxReverb2(float signal, float amount)
    {
        return applyReverb2(signal, amount, buffer, bufferIndex, REVERB_BUFFER_SIZE);
    }

    float fxReverb3(float signal, float amount)
    {
        return applyReverb3(signal, amount, buffer, bufferIndex, REVERB_BUFFER_SIZE);
    }

    float fxDelay(float input, float amount)
    {
        return applyDelay(input, amount, buffer, bufferIndex, DELAY_BUFFER_SIZE);
    }

    float fxDelay2(float input, float amount)
    {
        return applyDelay2(input, amount, buffer, bufferIndex, DELAY_BUFFER_SIZE);
    }

    float fxDelay3(float input, float amount)
    {
        return applyDelay3(input, amount, buffer, bufferIndex, DELAY_BUFFER_SIZE);
    }

    float tanhLookup(float x)
    {
        x = range(x, -1.0f, 1.0f);
        int index = static_cast<int>((x + 1.0f) * 0.5f * (lookupTable->size - 1));
        return lookupTable->tanh[index];
    }

    float sineLookupInterpolated(float x)
    {
        x -= std::floor(x);
        return linearInterpolation(x, lookupTable->size, lookupTable->sine);
    }

    float prevInput = 0;
    float prevOutput = 0;
    float fxBoost(float input, float amount)
    {
        return applyBoost(input, amount, prevInput, prevOutput);
    }

    float fxDrive(float input, float amount)
    {
        return applyDrive(input, amount, lookupTable);
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
        return range(input + input * scaledClipping, -1.0f, 1.0f);
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
        if (amount == 0.0f) {
            return input;
        }

        float decay = 0.98f + 0.01f * (1.0f - amount); // decay rate based on amount
        float feedbackSample = buffer[bufferIndex]; // read from buffer

        // Simple one-pole lowpass to emphasize bass
        static float lowpassState = 0.0f;
        float cutoff = 80.0f + 100.0f * amount; // Low-pass around 80-180 Hz
        float alpha = cutoff / sampleRate;
        lowpassState += alpha * (feedbackSample - lowpassState);

        // Mix input with feedback and write to buffer
        float out = input + lowpassState * amount;
        buffer[bufferIndex] = out * decay;

        // Increment circular buffer index
        bufferIndex = (bufferIndex + 1) % REVERB_BUFFER_SIZE;

        return tanhLookup(out); // Add soft saturation
    }

    float revGateEnv = 0.0f;
    float fxReverseGate(float input, float amount)
    {
        return applyReverseGate(input, amount, revGateEnv);
    }

    float decimHold = 0.0f;
    int decimCounter = 0;
    float fxDecimator(float input, float amount)
    {
        return applyDecimator(input, amount, decimHold, decimCounter);
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
        RING_MOD,
        FX_SHIMMER_REVERB,
        FX_SHIMMER2_REVERB,
        FX_FEEDBACK,
        DECIMATOR,
        FX_COUNT
    };
    void setFxType(Val::CallbackProps& p)
    {
        p.val.setFloat(p.value);
        if (p.val.get() == MultiFx::FXType::FX_OFF) {
            p.val.setString("OFF");
            fxFn = &MultiFx::fxOff;
        } else if (p.val.get() == MultiFx::FXType::REVERB) {
            p.val.setString("Reverb");
            fxFn = &MultiFx::fxReverb;
        } else if (p.val.get() == MultiFx::FXType::REVERB2) {
            p.val.setString("Reverb2");
            fxFn = &MultiFx::fxReverb2;
        } else if (p.val.get() == MultiFx::FXType::REVERB3) {
            p.val.setString("Reverb3");
            fxFn = &MultiFx::fxReverb3;
        } else if (p.val.get() == MultiFx::FXType::DELAY) {
            p.val.setString("Delay");
            fxFn = &MultiFx::fxDelay;
        } else if (p.val.get() == MultiFx::FXType::DELAY2) {
            p.val.setString("Delay2");
            fxFn = &MultiFx::fxDelay2;
        } else if (p.val.get() == MultiFx::FXType::DELAY3) {
            p.val.setString("Delay3");
            fxFn = &MultiFx::fxDelay3;
        } else if (p.val.get() == MultiFx::FXType::BASS_BOOST) {
            p.val.setString("Bass boost");
            fxFn = &MultiFx::fxBoost;
        } else if (p.val.get() == MultiFx::FXType::DRIVE) {
            p.val.setString("Drive");
            fxFn = &MultiFx::fxDrive;
        } else if (p.val.get() == MultiFx::FXType::COMPRESSION) {
            p.val.setString("Compression");
            fxFn = &MultiFx::fxCompressor;
        } else if (p.val.get() == MultiFx::FXType::WAVESHAPER) {
            p.val.setString("Waveshaper");
            fxFn = &MultiFx::fxWaveshaper;
        } else if (p.val.get() == MultiFx::FXType::WAVESHAPER2) {
            p.val.setString("Waveshaper2");
            fxFn = &MultiFx::fxWaveshaper2;
        } else if (p.val.get() == MultiFx::FXType::CLIPPING) {
            p.val.setString("Clipping");
            fxFn = &MultiFx::fxClipping;
        } else if (p.val.get() == MultiFx::FXType::SAMPLE_REDUCER) {
            p.val.setString("Sample reducer");
            fxFn = &MultiFx::fxSampleReducer;
        } else if (p.val.get() == MultiFx::FXType::BITCRUSHER) {
            p.val.setString("Bitcrusher");
            fxFn = &MultiFx::fxBitcrusher;
        } else if (p.val.get() == MultiFx::FXType::INVERTER) {
            p.val.setString("Inverter");
            fxFn = &MultiFx::fxInverter;
        } else if (p.val.get() == MultiFx::FXType::TREMOLO) {
            p.val.setString("Tremolo");
            fxFn = &MultiFx::fxTremolo;
        } else if (p.val.get() == MultiFx::FXType::RING_MOD) {
            p.val.setString("Ring mod.");
            fxFn = &MultiFx::fxRingMod;
        } else if (p.val.get() == MultiFx::FXType::FX_SHIMMER_REVERB) {
            p.val.setString("Shimmer");
            fxFn = &MultiFx::fxShimmerReverb;
        } else if (p.val.get() == MultiFx::FXType::FX_SHIMMER2_REVERB) {
            p.val.setString("Shimmer2");
            fxFn = &MultiFx::fxShimmer2Reverb;
        } else if (p.val.get() == MultiFx::FXType::FX_FEEDBACK) {
            p.val.setString("Feedback");
            fxFn = &MultiFx::fxFeedback;
        } else if (p.val.get() == MultiFx::FXType::DECIMATOR) {
            p.val.setString("Decimator");
            fxFn = &MultiFx::fxDecimator;
        }
        // TODO: add fx sample reducer
    }

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
