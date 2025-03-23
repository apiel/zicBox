#pragma once

#include "./utils/Wavetable.h"
#include "./utils/Lfo.h"
#include "audioPlugin.h"
#include "helpers/range.h"
#include "mapping.h"
#include "plugins/audio/filter.h"
// #include "plugins/audio/filter2.h"

/*md
## SynthWavetable

A simple synth engine that can generate a wide range of sounds but still quiet simple to use and cpu efficient.

*/
// TODO use filter8
// TODO lfo modulation/fm/mix
// TODO envlope curve!!

class SynthWavetable : public Mapping {
protected:
    uint8_t baseNote = 60;
    float velocity = 1.0f;
    float freq = 1.0f;

    Wavetable wavetable;
    EffectFilterData filter;
    LFO lfo;

    float attackStepInc = 0.0f;
    float releaseStepInc = 0.0f;
    float env = -1.0f;
    bool released = false;
    float freqMod = 1.0f;


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
    Val& filterType = val(1, "FILTER_TYPE", { "Filter", VALUE_STRING, .max = SynthWavetable::FilterType::FILTER_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == SynthWavetable::FilterType::FILTER_OFF) {
            p.val.setString("OFF");
        } else if (p.val.get() == SynthWavetable::FilterType::LP) {
            p.val.setString("LPF");
            filter.setType(EffectFilterData::Type::LP);
        } else if (p.val.get() == SynthWavetable::FilterType::BP) {
            p.val.setString("BPF");
            filter.setType(EffectFilterData::Type::BP);
        } else if (p.val.get() == SynthWavetable::FilterType::HP) {
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

    /*md - `LFO_RATE` set the LFO rate.*/
    Val& lfoRate = val(1.0f, "LFO_RATE", { "LFO Rate", .min = 0.1f, .max = 20.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        lfo.setRate(p.val.get());
    });

    /*md - `LFO_DEPTH` set the LFO depth.*/
    Val& lfoDepth = val(0.5f, "LFO_DEPTH", { "LFO Depth", .min = 0.0f, .max = 1.0f });

    /*md - `LFO_WAVEFORM` set the LFO waveform.*/
    Val& lfoWaveform = val(0, "LFO_WAVEFORM", { "LFO Waveform", VALUE_STRING, .max = LFO::WAVEFORM_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        lfo.setWaveform((int)p.val.get());
        p.val.setString(lfo.toString());
    });

    SynthWavetable(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config) // clang-format on
        , lfo(props.sampleRate)
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
            // float lfoValue = lfo.process();

            if (frequencyMod.pct() != 0.5f) {
                modulatedFreq += invEnv * (frequencyMod.pct() - 0.5f);
                if (modulatedFreq < 0.0f) {
                    modulatedFreq = 0.000001f;
                }
            }
            float out = wavetable.sample(&wavetable.sampleIndex, modulatedFreq);

            if (filterType.get() != SynthWavetable::FilterType::FILTER_OFF) {
                if (lastInvEnv != invEnv && (cutoffMod.pct() != 0.5f || resonanceMod.pct() != 0.5f)) {
                    filter.set(
                        range(filterCutoff.pct() + invEnv * (cutoffMod.pct() - 0.5f), 0.0f, 1.0f),
                        range(filterResonance.pct() + invEnv * (resonanceMod.pct() - 0.5f), 0.0f, 1.0f));
                }
                lastInvEnv = invEnv;
                out = filter.process(out);
            }

            out = out * velocity * env;
            buf[track] = out;
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
