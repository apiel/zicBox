#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/utils.h"

/*md
## EffectVolumeMultiFx

EffectVolumeMultiFx plugin is used to apply gain and volume on audio buffer, as well to apply a selected effect.
*/
class EffectVolumeMultiFx : public Mapping {
protected:
    typedef float (EffectVolumeMultiFx::*FnPtr)(float);
    FnPtr fxFn = &EffectVolumeMultiFx::fxOff;

    float fxOff(float input) { return input; }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;
    float fxReverb(float signal)
    {
        float reverbAmount = fxAmount.pct();
        if (reverbAmount == 0.0f) {
            return signal;
        }

        float reverbSignal = reverbBuffer[reverbIndex];

        float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
        reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;

        int reverbSamples = reverbAmount * REVERB_BUFFER_SIZE; // Reverb duration scaled
        reverbIndex = (reverbIndex + 1) % reverbSamples;

        float mix = reverbAmount * 0.5f; // Mix scaled proportionally
        return signal * (1.0f - mix) + reverbSignal * mix;
    }

    static constexpr int ReverbVoiceCount = 8;
    struct ReverbVoice {
        uint64_t index; // voice sample postion to define delay time
        float amplitude;
        float feedback;
    } voices[ReverbVoiceCount] = {
        { 120, 0.05f, 0.9f },
        { 240, 0.10f, 0.85f },
        { 360, 0.15f, 0.8f },
        { 480, 0.20f, 0.75f },
        { 600, 0.25f, 0.7f },
        { 720, 0.30f, 0.65f },
        { 840, 0.35f, 0.6f },
        { 960, 0.40f, 0.55f },
    };
    float fxReverb2(float signal)
    {
        float reverbAmount = fxAmount.pct();
        if (reverbAmount == 0.0f) {
            return signal;
        }

        reverbBuffer[reverbIndex] = signal;

        float reverbOut = 0.0f;
        for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
            ReverbVoice& voice = voices[i];
            int readIndex = (reverbIndex + REVERB_BUFFER_SIZE - voice.index) % REVERB_BUFFER_SIZE;
            float delayedSample = reverbBuffer[readIndex];

            reverbOut += delayedSample * voice.amplitude;
            reverbBuffer[readIndex] += reverbOut * (voice.feedback * reverbAmount * 2.0f);
        }

        reverbIndex++;
        if (reverbIndex >= REVERB_BUFFER_SIZE) {
            reverbIndex = 0;
        }

        float mix = reverbAmount * 0.5f; // Prevents over-dominating effect
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
        if (fxAmount.pct() == 0.0f) {
            return input;
        }
        float bassFreq = 0.2f + 0.8f * fxAmount.pct();
        float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
        prevInput = input;
        prevOutput = bassBoosted;
        bassBoosted *= 1.0f + fxAmount.pct() * 2.0f;

        return bassBoosted;
    }

    float fxDrive(float input)
    {
        if (fxAmount.pct() == 0.0f) {
            return input;
        }
        return tanhLookup(input * (1.0f + fxAmount.pct() * 5.0f));
    }

    float fxCompressor(float input)
    {
        if (fxAmount.pct() == 0.0f) {
            return input;
        }
        return (input * (1 - fxAmount.pct())) + (range(std::pow(input, fxAmount.pct() * 0.8f), -1.0f, 1.0f) * fxAmount.pct());
    }

    float fxWaveshaper(float input)
    {
        if (fxAmount.pct() == 0.0f) {
            return input;
        }
        float sineValue = sinf(input);
        return input + fxAmount.pct() * sineValue * 2;
    }

    float fxWaveshaper2(float input)
    {
        if (fxAmount.pct() == 0.0f) {
            return input;
        }
        float sineValue = sineLookupInterpolated(input);
        return input + fxAmount.pct() * sineValue;
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
