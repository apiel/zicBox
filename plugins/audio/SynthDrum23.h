#pragma once

#include <vector>

#include "./utils/Wavetable.h"
#include "./utils/WavetableGenerator.h"
#include "audioPlugin.h"
#include "filter.h"
#include "mapping.h"
#include "utils/EnvelopRelative.h"
#include "utils/FastWaveform.h"

/*md
## SynthDrum23

Synth engine to generate drum sounds using wavetables.
*/
class SynthDrum23 : public Mapping {
protected:
    uint8_t baseNote = 60;
    uint64_t sampleRate;

    WavetableInterface* wave = nullptr;
    WavetableGenerator waveform;
    Wavetable wavetable;
    FastWaveform fastWaveform;

    unsigned int sampleCountDuration = 0;
    unsigned int sampleDurationCounter = 0;

    float velocity = 1.0f;
    float noteMult = 1.0f;

    // https://codesandbox.io/p/sandbox/green-platform-tzl4pn?file=%2Fsrc%2Findex.js
    EnvelopRelative envelopAmp = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.0f, 1.0f } }, 1);
    EnvelopRelative envelopFreq = EnvelopRelative({
        { "Kick", [](EnvelopRelative* env, bool init = true) {
            env->useMacro = false;
            env->data.push_back({ 1.0f, 0.0f });
            env->data.push_back({ 0.5f, 0.03f });
            env->data.push_back({ 0.3f, 0.07f });
            env->data.push_back({ 0.09f, 0.19f });
            env->data.push_back({ 0.0f, 1.0f }); } },
        { "Down hills", [](EnvelopRelative* env, bool init = true) {
             env->useMacro = true;
             if (init) {
                 env->macro.a = 0.5;
                 env->macro.b = 0.5;
                 env->macro.c = 0.01;
             }
             for (float x = 0.0f; x <= 1.0f; x += 0.01f) {
                 float a = 100 * env->macro.a + 5;
                 float b = env->macro.b;
                 float c = 100 * env->macro.c;
                 float y = range(1 * exp(-a * x) + b * sin(x) - pow(x, c), 0.0f, 1.0f);
                 env->data.push_back({ y, x });
             }
         } },
        { "Expo decay", [](EnvelopRelative* env, bool init = true) {
             env->useMacro = true;
             if (init) {
                 env->macro.a = 0.5;
                 env->macro.b = 0.5;
                 env->macro.c = 0.01;
             }
             for (float x = 0.0f; x <= 1.0f; x += 0.01f) {
                 float a = 70 * env->macro.a;
                 float b = 0.5 * env->macro.b;
                 float c = 100 * env->macro.c;
                 float y = range(1 * exp(-a * x) + (b - b * pow(x, c)), 0.0f, 1.0f);
                 env->data.push_back({ y, x });
             }
         } },
        { "Multi decay", [](EnvelopRelative* env, bool init = true) {
             env->useMacro = true;
             if (init) {
                 env->macro.a = 0.5;
                 env->macro.b = 0.5;
                 env->macro.c = 0.5;
             }
             for (float x = 0.0f; x <= 1.0f; x += 0.01f) {
                 float decay = 0.7f * exp(-60.0f * (env->macro.a + 0.05f) * x);
                 // printf("decay: %f macro.a: %f\n", decay, macro.a);
                 float tail = 0.2f * exp(-5.0f * env->macro.b * x);
                 float release = tail - tail * pow(x, env->macro.c);

                 float y = range(decay + release, 0.0f, 1.0f);
                 env->data.push_back({ y, x });
             }
         } },
        { "Sin pow", [](EnvelopRelative* env, bool init = true) {
             env->useMacro = true;
             if (init) {
                 env->macro.a = 0.1;
                 env->macro.b = 0.2;
                 env->macro.c = 0.00;
             }
             for (float x = 0.0f; x <= 1.0f; x += 0.01f) {
                 float a = env->macro.b;
                 int b = env->macro.a * 100;
                 b = b * 2 + 4;
                 float c = 0.5 * env->macro.c;

                 float y = range(-a * sin(-1 + x) + pow(-1 + x, b) + c * acos(x), 0.0f, 1.0f);
                 env->data.push_back({ y, x });
             }
         } },
    });

    EffectFilterData clickFilter;
    // float addSecondLayer(float time, float out)
    // {
    //     // Add a click at the beginning
    //     float duration = clickDuration.pct(); // Duration of the click in seconds
    //     float clickAmplitude = click.pct();

    //     if (clickAmplitude && time < duration && clickCutoff.pct() > 0.0f) {
    //         float noise = fastWaveform.process();
    //         float clickEnv = 1.0f - (time / duration); // Linear fade-out for the click

    //         // Apply the envelope to the noise
    //         float rawClick = noise * clickAmplitude * clickEnv;

    //         clickFilter.setSampleData(rawClick);
    //         out += clickFilter.lp;

    //         // out += rawClick;
    //     }
    //     return out;
    // }

    EnvelopRelative envelopAmpLayer2 = EnvelopRelative({ { 0.0f, 0.0f }, { 0.0f, 0.01f }, { 0.0f, 1.0f } }, 1);
    float addSecondLayer(float time, float out)
    {
        float envAmp = envelopAmpLayer2.next(time);
        float out2 = fastWaveform.process() * envAmp;
        if (clickCutoff.pct() > 0.0f) {
            clickFilter.setSampleData(out2);
            out2 = clickFilter.lp;
        }
        out += out2;
        return out;
    }

#define DRUM23_WAVEFORMS_COUNT 7
    struct WaveformType {
        std::string name;
        WavetableInterface* wave;
        uint8_t indexType = 0;
    } waveformTypes[DRUM23_WAVEFORMS_COUNT] = {
        { "Wavetable", &wavetable },
        { "Sine", &waveform, WavetableGenerator::Type::Sine },
        { "Triangle", &waveform, WavetableGenerator::Type::Triangle },
        { "Square", &waveform, WavetableGenerator::Type::Square },
        { "Pulse", &waveform, WavetableGenerator::Type::Pulse },
        { "FM", &waveform, WavetableGenerator::Type::Fm },
        { "FMsquare", &waveform, WavetableGenerator::Type::FmSquare },
    };

public:
    /*md **Values**: */
    /*md - `WAVEFORM_TYPE` Select waveform type (wavetable, sine, triangle...).*/
    Val& waveformType = val(0.0f, "WAVEFORM_TYPE", { "Waveform", VALUE_STRING, .max = DRUM23_WAVEFORMS_COUNT - 1 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);
        if (wave && current == p.val.get()) {
            return;
        }
        WaveformType type = waveformTypes[(int)p.val.get()];
        p.val.setString(type.name);
        wave = type.wave;
        if (p.val.get() != 0.0f) {
            waveform.setType((WavetableGenerator::Type)type.indexType);
            printf("Waveform type: %s shape: %f macro: %f\n", type.name.c_str(), waveform.shape * 100.0f, waveform.macro * 100.0f);
            shape.set(waveform.shape * 1000.0f);
            macro.set(waveform.macro * 100.0f);
        } else {
            shape.set(wavetable.fileBrowser.position);
            macro.set(wavetable.getIndex());
        }
    });
    /*md - `SHAPE` Morhp over the waveform shape.*/
    Val& shape = val(0.0f, "SHAPE", { "Shape", VALUE_STRING, .max = 1000 }, [&](auto p) {
        if (waveformType.get() != 0.0f) {
            int direction = p.value - p.val.get();
            int value = p.val.get() + direction * 10;
            // printf("val: %f, direction: %d, value: %d\n", p.value, direction, value);
            p.val.setFloat(value);
            waveform.setShape(p.val.pct());
            p.val.setString(std::to_string((int)(p.val.get() * 0.1)) + "%");
        } else {
            int value = range(p.value, 1.0f, wavetable.fileBrowser.count);
            p.val.setFloat(value);

            int position = p.val.get();
            wavetable.open(position, false);
            p.val.setString(wavetable.fileBrowser.getFileWithoutExtension(position));

            sampleDurationCounter = -1; // set counter to the maximum
            sampleDurationCounter = sampleCountDuration;
        }
    });

    /*md - `MACRO` Macro is arbitrary parameter depending of selected waveform type. */
    Val& macro = val(0.0f, "MACRO", { "Macro", VALUE_STRING }, [&](auto p) {
        if (waveformType.get() != 0.0f) {
            p.val.setFloat(p.value);
            waveform.setMacro(p.val.pct());
            p.val.setString(std::to_string((int)p.val.get()) + "%");
        } else {
            float value = range(p.value, 1.0f, ZIC_WAVETABLE_WAVEFORMS_COUNT);
            p.val.setFloat(value);
            wavetable.morph((int)p.val.get() - 1);
            p.val.setString(std::to_string((int)p.val.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
        }
    });

    /*md - `PITCH` Modulate the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -36, .max = 36 });
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        bool isOff = sampleCountDuration == sampleDurationCounter;
        sampleCountDuration = p.val.get() * (sampleRate * 0.001f);
        timeIncrement = 1.0f / (float)sampleCountDuration;
        if (isOff) {
            sampleDurationCounter = sampleCountDuration;
        }
    });

    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        scaledClipping = p.val.pct() * p.val.pct() * 20;
    });

    /*md - `HIGH_FREQ_BOOST` set the high boost level.*/
    Val& highBoost = val(0.0, "HIGH_FREQ_BOOST", { "High boost", .min = 0.0, .max = 20.0, .step = 0.1, .floatingPoint = 1 });

    /*md - `OSC2` second oscillator.*/
    Val& lfoWaveform = val(0, "OSC2", { "Osc.2", VALUE_STRING, .max = FastWaveform::TYPE_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        fastWaveform.setType((int)p.val.get());
        p.val.setString(fastWaveform.toString());
    });

    /*md - `OSC2_FREQ` sets the base frequency of the percussive tone. */
    Val& osc2Freq = val(220.0f, "OSC2_FREQ", { "Osc.2 Freq", .min = 10.0, .max = 2000.0, .step = 10.0, .unit = "Hz" });

    /*md - `CLICK` set the click level.*/
    Val& click = val(0, "CLICK", { "Click" });

    /*md - `CLICK_DURATION` set the duration of the click.*/
    Val& clickDuration = val(0.1f, "CLICK_DURATION", { "Click Dur.", .step = 0.1, .floatingPoint = 1, .unit = "%" });

    /*md - `CLICK_CUTOFF` set the cutoff frequency of the click.*/
    Val& clickCutoff = val(50.0f, "CLICK_CUTOFF", { "Click Cutoff", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.pct() < 0.3f) {
            clickFilter.setRawCutoff(p.val.pct());
        } else {
            clickFilter.setResonance(p.val.pct() - 0.3f);
        }
    });

    /*md - `LAYER2_FILTER_TYPE` Select filter type.*/
    enum FilterType {
        LP,
        BP,
        HP,
        FILTER_COUNT
    };
    Val& layer2FilterType = val(1, "LAYER2_FILTER_TYPE", { "Filter", VALUE_STRING, .max = FilterType::FILTER_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == FilterType::LP) {
            p.val.setString("LPF");
            clickFilter.setType(EffectFilterData::Type::LP);
        } else if (p.val.get() == FilterType::BP) {
            p.val.setString("BPF");
            clickFilter.setType(EffectFilterData::Type::BP);
        } else if (p.val.get() == FilterType::HP) {
            p.val.setString("HPF");
            clickFilter.setType(EffectFilterData::Type::HP);
        }
        clickCutoff.set(clickCutoff.get());
        layer2FilterResonance.set(layer2FilterResonance.get());
    });

    /*md - `LAYER2_FILTER_RESONANCE` set the filter resonance.*/
    Val& layer2FilterResonance = val(0, "LAYER2_FILTER_RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        clickFilter.setResonance(p.val.pct());
    });

    SynthDrum23(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , waveform(props.lookupTable, props.sampleRate)
        , sampleRate(props.sampleRate)
        , fastWaveform(props.sampleRate, FastWaveform::Type::NOISE, 100.0f)
    {
        initValues();

        clickFilter.setResonance(0.85);
        clickFilter.setRawCutoff(0.10);
    }

    float highFreqBoost(float input, float time)
    {
        if (highBoost.get() == 0) {
            return input;
        }
        // Simple high-shelf boost logic
        float highFreqComponent = input * (highBoost.get() * time); // Emphasize high frequencies
        return input + highFreqComponent;
    }

    double time = 0.0f;
    double timeIncrement = 0.0f;
    float scaledClipping = 0.0f;
    void sample(float* buf)
    {
        if (sampleDurationCounter < sampleCountDuration) {
            // float time = (float)sampleDurationCounter / (float)sampleCountDuration;
            float envAmp = envelopAmp.next(time);
            float envFreq = envelopFreq.next(time);

            float freq = envFreq + noteMult;
            float out = wave->sample(&wavetable.sampleIndex, freq) * envAmp;
            out = addSecondLayer(time, out);
            out = highFreqBoost(out, time);
            out = range(out + out * scaledClipping, -1.0f, 1.0f);
            buf[track] = out * velocity;

            sampleDurationCounter++;
            time += timeIncrement;
            // printf("[%d] sample: %d of %d=%f\n", track, sampleDurationCounter, sampleCountDuration, buf[track]);
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        time = 0.0f;
        wavetable.sampleIndex = 0;
        sampleDurationCounter = 0;
        envelopAmp.reset();
        envelopFreq.reset();
        envelopAmpLayer2.reset();
        velocity = range(_velocity, 0.0f, 1.0f);

        noteMult = pow(2, ((note - baseNote + pitch.get()) / 12.0));

        fastWaveform.setRate(osc2Freq.get() * noteMult);
    }

    void serialize(FILE* file, std::string separator) override
    {
        Mapping::serialize(file, separator);
        envelopAmp.serialize(file, separator, "ENV_AMP");
        envelopFreq.serialize(file, separator, "ENV_FREQ");
        envelopAmpLayer2.serialize(file, separator, "ENV_AMP_OSC2");
    }

    void hydrate(std::string value) override
    {
        if (value.find("ENV_AMP ") != std::string::npos) {
            envelopAmp.hydrate(value.substr(8));
            return;
        }
        if (value.find("ENV_FREQ ") != std::string::npos) {
            envelopFreq.hydrate(value.substr(9));
            return;
        }
        if (value.find("ENV_AMP_OSC2 ") != std::string::npos) {
            envelopAmpLayer2.hydrate(value.substr(13));
            return;
        }
        Mapping::hydrate(value);
    }

protected:
    uint16_t msEnv = 0;
    float fMsEnv = 0.0f;

public:
    DataFn dataFunctions[19] = {
        { "ENV_AMP", [this](void* userdata) {
             return &envelopAmp.data;
         } },
        { "ENV_AMP_EDIT", [this](void* userdata) {
             return envelopAmp.updateEditPhase((int8_t*)userdata);
         } },
        { "ENV_AMP_TIME", [this](void* userdata) {
             float* timePct = envelopAmp.updatePhaseTime((int8_t*)userdata);
             msEnv = (uint16_t)(*timePct * duration.get());
             return &msEnv;
         } },
        { "ENV_AMP_MOD", [this](void* userdata) {
             return envelopAmp.updatePhaseModulation((int8_t*)userdata);
         } },
        { "ENV_FREQ", [this](void* userdata) {
             return &envelopFreq.data;
         } },
        { "ENV_FREQ_EDIT", [this](void* userdata) {
             return envelopFreq.updateEditPhase((int8_t*)userdata);
         } },
        { "ENV_FREQ_TIME", [this](void* userdata) {
             float* timePct = envelopFreq.updatePhaseTime((int8_t*)userdata);
             msEnv = (uint16_t)(*timePct * duration.get());
             return &msEnv;
         } },
        { "ENV_FREQ_MOD", [this](void* userdata) {
             return envelopFreq.updatePhaseModulation((int8_t*)userdata);
         } },
        { "ENV_FREQ2", [this](void* userdata) {
             return &envelopFreq.data;
         } },
        { "ENV_FREQ_MODE", [this](void* userdata) {
             return envelopFreq.updateMode((int8_t*)userdata);
         } },
        { "ENV_FREQ_IS_MACRO", [this](void* userdata) {
             return &envelopFreq.useMacro;
         } },
        { "ENV_FREQ_MACRO1", [this](void* userdata) {
             return envelopFreq.updateMacro1((int8_t*)userdata);
         } },
        { "ENV_FREQ_MACRO2", [this](void* userdata) {
             return envelopFreq.updateMacro2((int8_t*)userdata);
         } },
        { "ENV_FREQ_MACRO3", [this](void* userdata) {
             float* macro3 = envelopFreq.updateMacro3((int8_t*)userdata);
             if (!envelopFreq.useMacro) {
                 fMsEnv = *macro3 * duration.get();
                 return &fMsEnv;
             }
             return macro3;
         } },
        { "WAVEFORM", [this](void* userdata) {
             if (!wave) {
                 return (void*)NULL;
             }
             float* index = (float*)userdata;
             return (void*)wave->sample(index);
         } },
        { "ENV_AMP_OSC2", [this](void* userdata) {
             return &envelopAmpLayer2.data;
         } },
        { "ENV_AMP_OSC2_EDIT", [this](void* userdata) {
             return envelopAmpLayer2.updateEditPhase((int8_t*)userdata);
         } },
        { "ENV_AMP_OSC2_TIME", [this](void* userdata) {
             float* timePct = envelopAmpLayer2.updatePhaseTime((int8_t*)userdata);
             msEnv = (uint16_t)(*timePct * duration.get());
             return &msEnv;
         } },
        { "ENV_AMP_OSC2_MOD", [this](void* userdata) {
             return envelopAmpLayer2.updatePhaseModulation((int8_t*)userdata);
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
