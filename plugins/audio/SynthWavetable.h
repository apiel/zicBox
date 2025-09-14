#pragma once

#include "./utils/FastWaveform.h"
#include "./utils/Wavetable.h"
#include "audioPlugin.h"
#include "helpers/clamp.h"
#include "mapping.h"
// #include "plugins/audio/utils/filter.h"
#include "plugins/audio/utils/filter3.h"

/*md
## SynthWavetable

A simple synth engine that can generate a wide range of sounds but still quiet simple to use and cpu efficient.

*/
// TODO lfo modulation/fm/mix
// TODO envlope curve!!

class SynthWavetable : public Mapping {
protected:
    uint8_t baseNote = 60;
    float velocity = 1.0f;
    float freq = 1.0f;

    Wavetable wavetable;
    // EffectFilterData filter;
    // EffectFilter2 filter;
    EffectFilter3 filter;
    FastWaveform lfo;

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
        LP2,
        BP2,
        HP2,
        LP3,
        BP3,
        HP3,
        FILTER_COUNT
    };
    Val& filterType = val(1, "FILTER_TYPE", { "Filter", VALUE_STRING, .max = SynthWavetable::FilterType::FILTER_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == SynthWavetable::FilterType::FILTER_OFF) {
            p.val.setString("OFF");
        } else if (p.val.get() == SynthWavetable::FilterType::LP) {
            p.val.setString("LPF");
            // filter.setType(EffectFilterData::Type::LP);
            // filter.setType(EffectFilter2::Type::LP);
            filter.setType(EffectFilter3::Type::LP);
        } else if (p.val.get() == SynthWavetable::FilterType::BP) {
            p.val.setString("BPF");
            // filter.setType(EffectFilterData::Type::BP);
            // filter.setType(EffectFilter2::Type::BP);
            filter.setType(EffectFilter3::Type::BP);
        } else if (p.val.get() == SynthWavetable::FilterType::HP) {
            p.val.setString("HPF");
            // filter.setType(EffectFilterData::Type::HP);
            // filter.setType(EffectFilter2::Type::HP);
            filter.setType(EffectFilter3::Type::HP);
        } else if (p.val.get() == SynthWavetable::FilterType::LP2) {
            p.val.setString("LPF2");
            // filter.setType(EffectFilter2::Type::LP2);
            filter.setType(EffectFilter3::Type::LP);
        } else if (p.val.get() == SynthWavetable::FilterType::BP2) {
            p.val.setString("BPF2");
            // filter.setType(EffectFilter2::Type::BP2);
            filter.setType(EffectFilter3::Type::BP);
        } else if (p.val.get() == SynthWavetable::FilterType::HP2) {
            p.val.setString("HPF2");
            // filter.setType(EffectFilter2::Type::HP2);
            filter.setType(EffectFilter3::Type::HP);
        } else if (p.val.get() == SynthWavetable::FilterType::LP3) {
            p.val.setString("LPF3");
            filter.setType(EffectFilter3::Type::LP3);
        } else if (p.val.get() == SynthWavetable::FilterType::BP3) {
            p.val.setString("BPF3");
            filter.setType(EffectFilter3::Type::BP3);
        } else if (p.val.get() == SynthWavetable::FilterType::HP3) {
            p.val.setString("HPF3");
            filter.setType(EffectFilter3::Type::HP3);
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
    Val& lfoRate = val(1.0f, "LFO_RATE", { "LFO Rate", .min = 0.1f, .max = 100.0f, .step = 0.1f, .floatingPoint = 1, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        lfo.setRate(p.val.get());
    });

    /*md - `LFO_WAVEFORM` set the LFO waveform.*/
    Val& lfoWaveform = val(0, "LFO_WAVEFORM", { "LFO", VALUE_STRING, .max = FastWaveform::TYPE_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        lfo.setType((int)p.val.get());
        p.val.setString(lfo.toString());
    });

    /*md - `LFO_FREQ_MOD` set the LFO depth for frequency modulation.*/
    Val& lfoFreq = val(0.0f, "LFO_FREQ_MOD", { "Freq. Mod.", VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    // / *md - `LFO_AMPLITUDE_MOD` set the LFO depth for amplitude modulation.*/
    // Val& lfoAmplitude = val(0.0f, "LFO_AMPLITUDE_MOD", { "Amp. Mod.", .unit = "%" });

    /*md - `LFO_WAVE_MOD` set the LFO depth for wave modulation.*/
    Val& lfoWave = val(0.0f, "LFO_WAVE_MOD", { "Wave Mod.", VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    /*md - `LFO_CUTOFF_MOD` set the LFO depth for cutoff modulation.*/
    Val& lfoCutoff = val(0.0f, "LFO_CUTOFF_MOD", { "Cutoff Mod.", VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    /*md - `LFO_RESONANCE_MOD` set the LFO depth for resonance modulation.*/
    Val& lfoResonance = val(0.0f, "LFO_RESONANCE_MOD", { "Resonance Mod.", VALUE_CENTERED, .min = -100.0f, .unit = "%" });

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
            float lfoValue = lfo.process();

            if (frequencyMod.pct() != 0.5f) {
                modulatedFreq += invEnv * (frequencyMod.pct() - 0.5f);
            }
            if (lfoFreq.pct() != 0.5f) {
                modulatedFreq += (lfoFreq.pct() - 0.5f) * lfoValue;
            }
            if (modulatedFreq < 0.0f) {
                modulatedFreq = 0.000001f;
            }
            // float out = wavetable.sample(&wavetable.sampleIndex, modulatedFreq, (lfoValue - 0.5f) * 2);
            float out = lfoWave.get() != 0.0f
                ? wavetable.sample(&wavetable.sampleIndex, modulatedFreq, (lfoWave.pct() - 0.5f) * 2 * lfoValue)
                : wavetable.sample(&wavetable.sampleIndex, modulatedFreq);

            if (filterType.get() != SynthWavetable::FilterType::FILTER_OFF) {
                // if (lastInvEnv != invEnv && (cutoffMod.pct() != 0.5f || resonanceMod.pct() != 0.5f)) {
                //     filter.set(
                //         CLAMP(filterCutoff.pct() + invEnv * (cutoffMod.pct() - 0.5f), 0.0f, 1.0f),
                //         CLAMP(filterResonance.pct() + invEnv * (resonanceMod.pct() - 0.5f), 0.0f, 1.0f));
                // }
                float cutoff = filterCutoff.pct();
                float reso = filterResonance.pct();
                if (lastInvEnv != invEnv) {
                    if (cutoffMod.pct() != 0.5f) {
                        cutoff += invEnv * (cutoffMod.pct() - 0.5f);
                    }
                    if (resonanceMod.pct() != 0.5f) {
                        reso += invEnv * (resonanceMod.pct() - 0.5f);
                    }
                }
                if (lfoCutoff.pct() != 0.5f) {
                    cutoff += (lfoCutoff.pct() - 0.5f) * lfoValue;
                }
                if (lfoResonance.pct() != 0.5f) {
                    reso += (lfoResonance.pct() - 0.5f) * lfoValue;
                }
                if (cutoff != filterCutoff.pct() || reso != filterResonance.pct()) {
                    filter.set(range(cutoff, 0.0f, 1.0f), CLAMP(reso, 0.0f, 1.0f));
                }
                lastInvEnv = invEnv;
                out = filter.process(out);
            }

            out = out * velocity * env;
            // if (lfoAmplitude.pct() != 0.0f) {
            //     out = out * (1.0f - lfoAmplitude.pct()) + out * lfoAmplitude.pct() * lfoValue;
            // }
            buf[track] = out;
        }
    }

    uint8_t playingNote = 0;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        playingNote = note;
        env = 0.0f;
        released = false;
        velocity = _velocity;
        wavetable.sampleIndex = 0;
        lfo.reset();
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (note == playingNote) {
            released = true;            
        }
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
