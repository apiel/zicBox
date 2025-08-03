#pragma once

#include "plugins/audio/mapping.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"
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
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;
    float fxReverb(float signal, float amount)
    {
        float reverbAmount = amount;
        // return applyReverb(signal, reverbAmount, reverbBuffer, reverbIndex, props.sampleRate, REVERB_BUFFER_SIZE);
        return applyReverb(signal, reverbAmount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);
    }

    static constexpr int ReverbVoiceCount = 4; // Reduced from 8 to 4 for efficiency
    ReverbVoice voices[ReverbVoiceCount] = {
        { 180 * 2, 0.6f }, // First early reflection
        { 420 * 2, 0.4f }, // Mid reflection
        { 690 * 2, 0.3f }, // Late reflection
        { 960 * 2, 0.2f }, // Very late tail
    };

    float fxShimmerReverb(float input, float amount)
    {
        return applyShimmerReverb(input, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, ReverbVoiceCount, voices);
    }

    int shimmerTime = 0;
    float fxShimmer2Reverb(float input, float amount)
    {
        return applyShimmer2Reverb(input, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, ReverbVoiceCount, voices, shimmerTime);
    }

    float fxReverb2(float signal, float amount)
    {
        return applyReverb2(signal, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, ReverbVoiceCount, voices);
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
        if (amount == 0.0f) {
            return input;
        }
        if (samplePosition < amount * 100 * 2) {
            samplePosition++;
        } else {
            samplePosition = 0;
            sampleSqueeze = input;
        }

        return sampleSqueeze;
    }

    float sampleHold = 0.0f;
    int sampleCounter = 0;
    float fxBitcrusher(float input, float amount)
    {
        if (amount == 0.0f) {
            return input;
        }

        // Reduce Bit Depth
        int bitDepth = 2 + amount * 10; // Stronger effect
        float step = 1.0f / (1 << bitDepth); // Quantization step
        float crushed = round(input / step) * step; // Apply bit reduction

        // Reduce Sample Rate
        int sampleRateDivider = 1 + amount * 20; // Reduces update rate
        if (sampleCounter % sampleRateDivider == 0) {
            sampleHold = crushed; // Hold the value for "stepping" effect
        }
        sampleCounter++;

        if (amount < 0.1f) {
            // mix with original signal
            return sampleHold * (amount * 10) + input * (1.0f - (amount * 10));
        }

        return sampleHold;
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
        if (amount == 0.0f) {
            return input;
        }

        float speed = 1.0f; // Tremolo speed in Hz
        tremoloPhase += 0.05f * speed;
        float mod = (sin(tremoloPhase) + 1.0f) / 2.0f; // Modulation between 0-1

        return input * (1.0f - amount + amount * mod);
    }

    float ringPhase = 0.0f; // Phase for the sine wave oscillator
    float fxRingMod(float input, float amount)
    {
        if (amount == 0.0f) {
            return input;
        }

        float ringFreq = 200.0f + amount * 800.0f; // Modulation frequency (200Hz - 1000Hz)
        ringPhase += 2.0f * M_PI * ringFreq / sampleRate;

        // Keep phase in the [0, 2π] range
        if (ringPhase > 2.0f * M_PI) {
            ringPhase -= 2.0f * M_PI;
        }

        float modulator = sin(ringPhase); // Sine wave oscillator
        float modulated = input * modulator; // Apply ring modulation

        return (1.0f - amount) * input + amount * modulated;
    }

    float fxFeedback(float input, float amount)
    {
        if (amount == 0.0f) {
            return input;
        }

        float decay = 0.98f + 0.01f * (1.0f - amount); // decay rate based on amount
        float feedbackSample = reverbBuffer[reverbIndex]; // read from buffer

        // Simple one-pole lowpass to emphasize bass
        static float lowpassState = 0.0f;
        float cutoff = 80.0f + 100.0f * amount; // Low-pass around 80-180 Hz
        float alpha = cutoff / sampleRate;
        lowpassState += alpha * (feedbackSample - lowpassState);

        // Mix input with feedback and write to buffer
        float out = input + lowpassState * amount;
        reverbBuffer[reverbIndex] = out * decay;

        // Increment circular buffer index
        reverbIndex = (reverbIndex + 1) % REVERB_BUFFER_SIZE;

        return tanhLookup(out); // Add soft saturation
    }

public:
    enum FXType {
        FX_OFF,
        REVERB,
        REVERB2,
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
