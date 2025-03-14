#pragma once

#include "./utils/Wavetable.h"
#include "audioPlugin.h"
#include "helpers/range.h"
#include "mapping.h"
#include "plugins/audio/filter.h"
// #include "plugins/audio/filter2.h"

#include <functional>

/*md
## SynthBuddy

A simple synth engine that can generate a wide range of sounds but still quiet simple to use and cpu efficient.

*/
// TODO use filter8
// TODO lfo modulation/fm/mix
// TODO fx sample reducer
// TODO envlope curve!!

class SynthBuddy : public Mapping {
protected:
    uint8_t baseNote = 60;
    float velocity = 1.0f;
    float freq = 1.0f;

    Wavetable wavetable;
    EffectFilterData filter;

    float attackStepInc = 0.0f;
    float releaseStepInc = 0.0f;
    float env = -1.0f;
    bool released = false;
    float freqMod = 1.0f;

    typedef float (SynthBuddy::*FnPtr)(float);
    FnPtr fxFn = &SynthBuddy::fxOff;

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

public:
    /*md **Values**: */
    /*md - `PITCH` set the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -36, .max = 36 });
    /*md - `ATTACK` set the attack time.*/
    Val& attack = val(0, "ATTACK", { "Attack", .min = 10, .max = 5000, .step = 10, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        int sampleCount = p.val.get() * props.sampleRate * 0.001f * props.channels;
        attackStepInc = 1.0f / sampleCount;
    });
    /*md - `RELEASE` set the release time.*/
    Val& release = val(100, "RELEASE", { "Release", .min = 10, .max = 10000, .step = 10, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        int sampleCount = p.val.get() * props.sampleRate * 0.001f * props.channels;
        releaseStepInc = 1.0f / sampleCount;
    });
    /*md - `FREQUENCY_MOD` set the frequency modulation using the envelope output.*/
    Val& frequencyMod = val(0, "FREQUENCY_MOD", { "Freq. Mod.", VALUE_CENTERED, .min = -100.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        freqMod = pow(2.0f, p.val.pct());
    });
    // /* md - `OSC_MOD` set the oscillator modulation using the envelope output (morph on wavetable and change start point on sample).*/
    // Val& oscMod = val(0, "OSC_MOD", { "Osc. Mod." });

    /*md - `CUTOFF_MOD` set the cutoff modulation using the envelope output. */
    Val& cutoffMod = val(0, "CUTOFF_MOD", { "Cutoff Mod.", VALUE_CENTERED, .min = -100.0f });
    /*md - `RESONANCE_MOD` set the resonance modulation using the envelope output. */
    Val& resonanceMod = val(0, "RESONANCE_MOD", { "Resonance Mod.", VALUE_CENTERED, .min = -100.0f });

    /*md - `FILTER_TYPE` Select filter type.*/
    enum FilterType {
        FILTER_OFF,
        LP,
        BP,
        HP,
        FILTER_COUNT
    };
    Val& filterType = val(1, "FILTER_TYPE", { "Filter", VALUE_STRING, .max = SynthBuddy::FilterType::FILTER_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == SynthBuddy::FilterType::FILTER_OFF) {
            p.val.setString("OFF");
        } else if (p.val.get() == SynthBuddy::FilterType::LP) {
            p.val.setString("LPF");
            filter.setType(EffectFilterData::Type::LP);
        } else if (p.val.get() == SynthBuddy::FilterType::BP) {
            p.val.setString("BPF");
            filter.setType(EffectFilterData::Type::BP);
        } else if (p.val.get() == SynthBuddy::FilterType::HP) {
            p.val.setString("HPF");
            filter.setType(EffectFilterData::Type::HP);
        }
        filterCutoff.set(filterCutoff.get());
        filterResonance.set(filterResonance.get());
    });

    /*md - `FILTER_CUTOFF` set the filter cutoff frequency.*/
    Val& filterCutoff = val(0, "FILTER_CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.set(p.val.pct(), filterResonance.pct());
    });

    /*md - `FILTER_RESONANCE` set the filter resonance.*/
    Val& filterResonance = val(0, "FILTER_RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    /*md - `WAVE` select the wavetable.*/
    Val& wave = val(0, "WAVE", { "Wave", VALUE_STRING }, [&](auto p) {
        p.val.setFloat(p.value);
        int position = p.val.get();
        wavetable.open(position, false);
        p.val.setString(wavetable.fileBrowser.getFileWithoutExtension(position));
    });

    /*md - `WAVE_EDIT` morph the wavetable.*/
    Val& waveEdit = val(0, "WAVE_EDIT", { "Wave Edit", VALUE_STRING, .min = 1.0, .max = ZIC_WAVETABLE_WAVEFORMS_COUNT }, [&](auto p) {
        p.val.setFloat(p.value);
        wavetable.morph((int)p.val.get() - 1);
        p.val.setString(std::to_string((int)p.val.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
    });

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
        FX_COUNT
    };
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = SynthBuddy::FXType::FX_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == SynthBuddy::FXType::FX_OFF) {
            p.val.setString("OFF");
            fxFn = &SynthBuddy::fxOff;
        } else if (p.val.get() == SynthBuddy::FXType::REVERB) {
            p.val.setString("Reverb");
            fxFn = &SynthBuddy::fxReverb;
        } else if (p.val.get() == SynthBuddy::FXType::BASS_BOOST) {
            p.val.setString("Bass boost");
            fxFn = &SynthBuddy::fxBoost;
        } else if (p.val.get() == SynthBuddy::FXType::DRIVE) {
            p.val.setString("Drive");
            fxFn = &SynthBuddy::fxDrive;
        } else if (p.val.get() == SynthBuddy::FXType::COMPRESSION) {
            p.val.setString("Compression");
            fxFn = &SynthBuddy::fxCompressor;
        } else if (p.val.get() == SynthBuddy::FXType::WAVESHAPER) {
            p.val.setString("Waveshaper");
            fxFn = &SynthBuddy::fxWaveshaper;
        } else if (p.val.get() == SynthBuddy::FXType::WAVESHAPER2) {
            p.val.setString("Waveshaper2");
            fxFn = &SynthBuddy::fxWaveshaper2;
        } else if (p.val.get() == SynthBuddy::FXType::CLIPPING) {
            p.val.setString("Clipping");
            fxFn = &SynthBuddy::fxClipping;
        }
        // TODO: add fx sample reducer
        //       add fx sub osc
        //       add fx noise
        //       add fx transient --> click or whatever
    });

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    SynthBuddy(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config) // clang-format on
    // , filter2(props.sampleRate)
    {
        wave.props().max = wavetable.fileBrowser.count - 1;
        initValues();
    }

    float lastInvEnv = -111.0f;
    void sample(float* buf)
    {
        if (env >= 0.0f) {
            if (released) {
                env -= releaseStepInc;
            } else if (env < 1.0f) {
                env += attackStepInc;
            }
            float modulatedFreq = freq;
            float invEnv = 1.0f - env;
            if (frequencyMod.pct() != 0.5f) {
                modulatedFreq += invEnv * (frequencyMod.pct() - 0.5f);
            }
            float out = wavetable.sample(&wavetable.sampleIndex, modulatedFreq);

            if (filterType.get() != SynthBuddy::FilterType::FILTER_OFF) {
                if (lastInvEnv != invEnv && (cutoffMod.pct() != 0.5f || resonanceMod.pct() != 0.5f)) {
                    filter.set(
                        range(filterCutoff.pct() + invEnv * (cutoffMod.pct() - 0.5f), 0.0f, 1.0f),
                        range(filterResonance.pct() + invEnv * (resonanceMod.pct() - 0.5f), 0.0f, 1.0f));
                }
                lastInvEnv = invEnv;
                out = filter.process(out);
            }

            out = out * velocity * env;
            out = (this->*fxFn)(out);
            buf[track] = out;
        } else if (fxType.get() == SynthBuddy::FXType::REVERB) {
            buf[track] = (this->*fxFn)(buf[track]);
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        env = 0.0f;
        released = false;
        velocity = _velocity;
        wavetable.sampleIndex = 0;
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }

    void noteOff(uint8_t note, float _velocity) override
    {
        released = true;
    }

    std::vector<float> waveformData;
    DataFn dataFunctions[1] = {
        { "WAVEFORM", [this](void* userdata) {
             float* index = (float*)userdata;
             return (void*)wavetable.sample(index);
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
