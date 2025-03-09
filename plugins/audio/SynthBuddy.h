#pragma once

#include "audioPlugin.h"
#include "helpers/range.h"
#include "mapping.h"
#include "plugins/audio/filter.h"
#include "./utils/Wavetable.h"

/*md
## SynthBuddy

A simple synth engine that can generate a wide range of sounds but still quiet simple to use and cpu efficient.

*/
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

    enum FilterType {
        LP,
        BP,
        HP,
        FILTER_COUNT
    };

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

    /*md - `FILTER_TYPE` Select filter type.*/
    Val& filterType = val(0, "FILTER_TYPE", { "Filter", VALUE_STRING, .max = SynthBuddy::FilterType::FILTER_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == SynthBuddy::FilterType::LP) {
            p.val.setString("LPF");
        } else if (p.val.get() == SynthBuddy::FilterType::BP) {
            p.val.setString("BPF");
        } else if (p.val.get() == SynthBuddy::FilterType::HP) {
            p.val.setString("HPF");
        }
    });

    /*md - `FILTER_CUTOFF` set the filter cutoff frequency.*/
    Val& filterCutoff = val(0, "FILTER_CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        if (filterType.get() == SynthBuddy::FilterType::LP) {
            filter.setLpCutoff(p.val.pct());
        } else if (filterType.get() == SynthBuddy::FilterType::BP) {
            filter.setBpCutoff(p.val.pct());
        } else if (filterType.get() == SynthBuddy::FilterType::HP) {
            filter.setHpCutoff(p.val.pct());
        }
    });

    /*md - `FILTER_RESONANCE` set the filter resonance.*/
    Val& filterResonance = val(0, "FILTER_RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    SynthBuddy(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config) // clang-format on
    {
        initValues();
    }

    void sample(float* buf)
    {
        if (env >= 0.0f) {
            if (released) {
                env -= releaseStepInc;
            } else if (env < 1.0f) {
                env += attackStepInc;
            }
            float modulatedFreq = freq;
            if (frequencyMod.pct() != 0.5f) {
                // modulatedFreq = modulatedFreq * pow(2.0f, env * (frequencyMod.pct() - 0.5f));
                // modulatedFreq = modulatedFreq + (modulatedFreq * env * (frequencyMod.pct() - 0.5f));
                modulatedFreq += env * (frequencyMod.pct() - 0.5f);
                // modulatedFreq += env * (pow(2.0f, env * (frequencyMod.pct() - 0.5f)) - 1.0f);
            }
            float out = wavetable.sample(&wavetable.sampleIndex, modulatedFreq);

            if (filterCutoff.get() > 0.0f) {
                filter.setSampleData(out);
                if (filterType.get() == SynthBuddy::FilterType::LP) {
                    out = filter.lp;
                } else if (filterType.get() == SynthBuddy::FilterType::BP) {
                    out = filter.bp;
                } else if (filterType.get() == SynthBuddy::FilterType::HP) {
                    out = filter.hp;
                }
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

    // std::vector<float> waveformData;
    // DataFn dataFunctions[1] = {
    //     { "WAVEFORM", [this](void* userdata) {
    //          if (!wave) {
    //              return (void*)NULL;
    //          }
    //          float* index = (float*)userdata;
    //          return (void*)wave->sample(index);
    //      } },
    // };
    // DEFINE_GETDATAID_AND_DATA
};
