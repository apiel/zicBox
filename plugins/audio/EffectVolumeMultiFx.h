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

    float fxInverter(float input)
    {
        if (input > fxAmount.pct() || input < -fxAmount.pct()) {
            return -input;
        }
        return input;
    }

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
        BASS_BOOST,
        DRIVE,
        COMPRESSION,
        WAVESHAPER,
        WAVESHAPER2,
        CLIPPING,
        SAMPLE_REDUCER,
        INVERTER,
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
        } else if (p.val.get() == EffectVolumeMultiFx::FXType::INVERTER) {
            p.val.setString("Inverter");
            fxFn = &EffectVolumeMultiFx::fxInverter;
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
