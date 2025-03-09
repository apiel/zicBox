#pragma once

#include "audioPlugin.h"
#include "helpers/range.h"
#include "mapping.h"

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
