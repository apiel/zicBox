#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/utils.h"
#include "utils/effects/applyBoost.h"
#include "utils/effects/applyCompression.h"
#include "utils/effects/applyDrive.h"
#include "utils/effects/applyReverb.h"
#include "utils/effects/applyWaveshape.h"

/*md
## EffectVolumeMultiFx

EffectVolumeMultiFx plugin is used to apply gain and volume on audio buffer, as well to apply a selected effect.
*/
class EffectVolumeMultiFx : public Mapping {
protected:
    typedef float (EffectVolumeMultiFx::*FnPtr)(float);
    FnPtr fxFn = &EffectVolumeMultiFx::fxOff;

    float fxOff(float input) { return input; }

    float fxShimmerReverb(float input)
    {
        return applyShimmerReverb(input, fxAmount.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, ReverbVoiceCount, voices);
    }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;
    float fxReverb(float signal)
    {
        float reverbAmount = fxAmount.pct();
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

    float fxReverb2(float signal)
    {
        float reverbAmount = fxAmount.pct();
        if (reverbAmount == 0.0f) {
            return signal;
        }

        // Store input signal in buffer
        reverbBuffer[reverbIndex] = signal;

        float reverbOut = 0.0f;

        // Process only 4 key delay taps
        for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
            int readIndex = (reverbIndex + REVERB_BUFFER_SIZE - voices[i].delay) % REVERB_BUFFER_SIZE;
            reverbOut += reverbBuffer[readIndex] * voices[i].gain;
        }

        // Apply global feedback (reduces CPU by avoiding per-voice feedback writes)
        float globalFeedback = reverbAmount * 0.6f;
        reverbBuffer[reverbIndex] = signal + reverbOut * globalFeedback;

        // Advance buffer index
        reverbIndex = (reverbIndex + 1) % REVERB_BUFFER_SIZE;

        // Final wet/dry mix
        float mix = reverbAmount * 0.8f;
        return signal * (1.0f - mix) + reverbOut * mix;
    }

    float tanhLookup(float x)
    {
        x = range(x, -1.0f, 1.0f);
        int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1));
        return props.lookupTable->tanh[index];
    }

    float sineLookupInterpolated(float x)
    {
        x -= std::floor(x);
        return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
    }

    float prevInput = 0;
    float prevOutput = 0;
    float fxBoost(float input)
    {
        return applyBoost(input, fxAmount.pct(), prevInput, prevOutput);
    }

    float fxDrive(float input)
    {
        return applyDrive(input, fxAmount.pct(), props.lookupTable);
    }

    float fxCompressor(float input)
    {
        return applyCompression(input, fxAmount.pct());
    }

    float fxWaveshaper(float input)
    {
        return applyWaveshape(input, fxAmount.pct());
    }

    float fxWaveshaper2(float input)
    {
        return applyWaveshapeLut(input, fxAmount.pct(), props.lookupTable);
    }

    float fxClipping(float input)
    {
        if (fxAmount.pct() == 0.0f) {
            return input;
        }
        float scaledClipping = fxAmount.pct() * fxAmount.pct() * 20;
        return range(input + input * scaledClipping, -1.0f, 1.0f);
    }

    float sampleSqueeze;
    int samplePosition = 0;
    float fxSampleReducer(float input)
    {
        if (fxAmount.get() == 0.0f) {
            return input;
        }
        if (samplePosition < fxAmount.get() * 2) {
            samplePosition++;
        } else {
            samplePosition = 0;
            sampleSqueeze = input;
        }

        return sampleSqueeze;
    }

    float sampleHold = 0.0f;
    int sampleCounter = 0;
    float fxBitcrusher(float input)
    {
        if (fxAmount.pct() == 0.0f) {
            return input;
        }

        // Reduce Bit Depth
        int bitDepth = 2 + fxAmount.pct() * 10; // Stronger effect
        float step = 1.0f / (1 << bitDepth); // Quantization step
        float crushed = round(input / step) * step; // Apply bit reduction

        // Reduce Sample Rate
        int sampleRateDivider = 1 + fxAmount.pct() * 20; // Reduces update rate
        if (sampleCounter % sampleRateDivider == 0) {
            sampleHold = crushed; // Hold the value for "stepping" effect
        }
        sampleCounter++;

        if (fxAmount.pct() < 0.1f) {
            // mix with original signal
            return sampleHold * (fxAmount.pct() * 10) + input * (1.0f - (fxAmount.pct() * 10));
        }

        return sampleHold;
    }

    float fxInverter(float input)
    {
        if (input > fxAmount.pct() || input < -fxAmount.pct()) {
            return -input;
        }
        return input;
    }

    float tremoloPhase = 0.0f;
    float fxTremolo(float input)
    {
        if (fxAmount.pct() == 0.0f) {
            return input;
        }

        float speed = 1.0f; // Tremolo speed in Hz
        tremoloPhase += 0.05f * speed;
        float mod = (sin(tremoloPhase) + 1.0f) / 2.0f; // Modulation between 0-1

        return input * (1.0f - fxAmount.pct() + fxAmount.pct() * mod);
    }

    float ringPhase = 0.0f; // Phase for the sine wave oscillator
    float fxRingMod(float input)
    {
        if (fxAmount.pct() == 0.0f) {
            return input;
        }

        float ringFreq = 200.0f + fxAmount.pct() * 800.0f; // Modulation frequency (200Hz - 1000Hz)
        ringPhase += 2.0f * M_PI * ringFreq / props.sampleRate;

        // Keep phase in the [0, 2π] range
        if (ringPhase > 2.0f * M_PI) {
            ringPhase -= 2.0f * M_PI;
        }

        float modulator = sin(ringPhase); // Sine wave oscillator
        float modulated = input * modulator; // Apply ring modulation

        return (1.0f - fxAmount.pct()) * input + fxAmount.pct() * modulated;
    }

    // static const int delaySamples = 88; // Fixed delay for ~500 Hz resonance
    // float buffer[delaySamples] = {0.0f}; // Circular buffer
    // int writeIndex = 0;
    // float feedback = 0.95f;
    // float damping = 0.8f;    // Low-pass filter coefficient
    // float fxResonator(float input) {
    //     if (fxAmount.pct() == 0.0f) {
    //         return input;
    //     }

    //     // Circular buffer read index
    //     int readIndex = (writeIndex - delaySamples + delaySamples) % delaySamples;
    //     float delayedSample = buffer[readIndex];

    //     // Simple low-pass filter on feedback path
    //     static float lastSample = 0.0f;
    //     float filteredSample = damping * delayedSample + (1.0f - damping) * lastSample;
    //     lastSample = filteredSample;

    //     // Apply stronger feedback loop
    //     float resonated = filteredSample * feedback + input;

    //     // Store in delay buffer
    //     buffer[writeIndex] = resonated;
    //     writeIndex = (writeIndex + 1) % delaySamples;

    //     // Mix dry & wet signal
    //     return (1.0f - fxAmount.pct()) * input + fxAmount.pct() * resonated;
    // }

public:
    /*md **Values**: */
    /*md - `VOLUME` to set volume. */
    Val& volume = val(100.0f, "VOLUME", { "Volume", .unit = "%" }, [&](auto p) { setVolumeWithGain(p.value, gain.get()); });
    /*md - `GAIN` to set gain. */
    Val& gain = val(1.0f, "GAIN", { "Gain", .min = 1.0f, .max = 20.0f, .step = .1f }, [&](auto p) { setVolumeWithGain(volume.get(), p.value); });
    float volumeWithGain = volume.get();

    /*md - `FX_TYPE` select the effect.*/
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
        FX_COUNT
    };
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = EffectVolumeMultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == EffectVolumeMultiFx::FXType::FX_OFF) {
            p.val.setString("OFF");
            fxFn = &EffectVolumeMultiFx::fxOff;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::REVERB) {
            p.val.setString("Reverb");
            fxFn = &EffectVolumeMultiFx::fxReverb;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::REVERB2) {
            p.val.setString("Reverb2");
            fxFn = &EffectVolumeMultiFx::fxReverb2;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::BASS_BOOST) {
            p.val.setString("Bass boost");
            fxFn = &EffectVolumeMultiFx::fxBoost;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::DRIVE) {
            p.val.setString("Drive");
            fxFn = &EffectVolumeMultiFx::fxDrive;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::COMPRESSION) {
            p.val.setString("Compression");
            fxFn = &EffectVolumeMultiFx::fxCompressor;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::WAVESHAPER) {
            p.val.setString("Waveshaper");
            fxFn = &EffectVolumeMultiFx::fxWaveshaper;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::WAVESHAPER2) {
            p.val.setString("Waveshaper2");
            fxFn = &EffectVolumeMultiFx::fxWaveshaper2;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::CLIPPING) {
            p.val.setString("Clipping");
            fxFn = &EffectVolumeMultiFx::fxClipping;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::SAMPLE_REDUCER) {
            p.val.setString("Sample reducer");
            fxFn = &EffectVolumeMultiFx::fxSampleReducer;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::BITCRUSHER) {
            p.val.setString("Bitcrusher");
            fxFn = &EffectVolumeMultiFx::fxBitcrusher;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::INVERTER) {
            p.val.setString("Inverter");
            fxFn = &EffectVolumeMultiFx::fxInverter;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::TREMOLO) {
            p.val.setString("Tremolo");
            fxFn = &EffectVolumeMultiFx::fxTremolo;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::RING_MOD) {
            p.val.setString("Ring mod.");
            fxFn = &EffectVolumeMultiFx::fxRingMod;
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::FX_SHIMMER_REVERB) {
            p.val.setString("Shimmer");
            fxFn = &EffectVolumeMultiFx::fxShimmerReverb;
        }
        // TODO: add fx sample reducer
    });

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    EffectVolumeMultiFx(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float out = (this->*fxFn)(buf[track]);
        buf[track] = out * volumeWithGain;
    }

    EffectVolumeMultiFx& setVolumeWithGain(float vol, float _gain)
    {
        gain.setFloat(_gain);
        volume.setFloat(vol);
        volumeWithGain = (1.0 + (gain.get())) * volume.pct();

        return *this;
    }
};
