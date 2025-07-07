#pragma once

#include <vector>

#include "./utils/Wavetable.h"
#include "./utils/WavetableGenerator.h"
#include "audioPlugin.h"
#include "mapping.h"
#include "utils/EnvelopDrumAmp.h"
#include "utils/EnvelopRelative.h"
#include "utils/FastWaveform.h"
#include "utils/MMfilter.h"


// NOTE layer2 need complex amp envelop!
// to solve limited amount of params
// TODO could get rid of resonance...
// or filter could be part of OSC morph...
// or resonance could be use only when osc2 is noise, instead of freq...


/*md
## SynthKick23

Synth engine to generate drum sounds using wavetables (copy of SynthDrum23 slighlty simpler but optimized).
*/
class SynthKick23 : public Mapping {
protected:
    uint8_t baseNote = 60;
    uint64_t sampleRate;

    WavetableInterface* wave = nullptr;
    WavetableGenerator waveform;
    Wavetable wavetable;
    FastWaveform fastWaveform;
    MMfilter layer2Filter;

    unsigned int sampleCountDuration = 0;
    unsigned int sampleDurationCounter = 0;

    float velocity = 1.0f;
    float noteMult = 1.0f;

    // https://codesandbox.io/p/sandbox/green-platform-tzl4pn?file=%2Fsrc%2Findex.js
    EnvelopDrumAmp envelopAmp;
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

    EnvelopRelative envelopAmpLayer2 = EnvelopRelative({ { 0.0f, 0.0f }, { 0.0f, 0.01f }, { 0.0f, 1.0f } }, 1);
    float addSecondLayer(float out)
    {
        float envAmp = envelopAmpLayer2.next();
        if (envAmp > 0.0f) {
            float out2 = fastWaveform.process() * envAmp;
            out2 = layer2Filter.process(out2);
            out += out2;
        }
        return out;
    }

    void updateBoostTimeInc()
    {
        boostTimeInc = (1.0f / (float)sampleCountDuration) * highBoost.get();
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
        updateBoostTimeInc();
        if (isOff) {
            sampleDurationCounter = sampleCountDuration;
        }
    });
    /*md - `AMP_MORPH` morph on the shape of the envelop of the amplitude.*/
    Val& ampMorph = val(0.0f, "AMP_MORPH", { "Amp. Morph", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envelopAmp.morph(p.val.pct());
    });

    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        scaledClipping = p.val.pct() * p.val.pct() * 20;
    });

    /*md - `HIGH_FREQ_BOOST` set the high boost level.*/
    Val& highBoost = val(0.0, "HIGH_FREQ_BOOST", { "High Boost", .min = 0.0, .max = 20.0, .step = 0.1, .floatingPoint = 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        updateBoostTimeInc();
    });

    /*md - `OSC2` second oscillator.*/
    Val& osc2Waveform = val(0, "OSC2", { "Osc.2", VALUE_STRING, .max = FastWaveform::TYPE_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        fastWaveform.setType((int)p.val.get());
        p.val.setString(fastWaveform.toString());
    });

    /*md - `OSC2_FREQ` sets the base frequency of the percussive tone. */
    Val& osc2Freq = val(220.0f, "OSC2_FREQ", { "Osc.2 Freq", .min = 10.0, .max = 2000.0, .step = 10.0, .unit = "Hz" });

    /*md - `LAYER2_CUTOFF` to set cutoff frequency and switch between low and high pass filter. */
    Val& osc2Cutoff = val(0.0, "LAYER2_CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0 }, [&](auto p) {
        p.val.setFloat(p.value);
        float amount = p.val.pct() * 2 - 1.0f;

        char strBuf[128];
        layer2Filter.setCutoff(amount);
        if (amount > 0.0) {
            sprintf(strBuf, "HP %d%%", (int)(amount * 100));
        } else {
            sprintf(strBuf, "LP %d%%", (int)((-amount) * 100));
        }
        p.val.setString(strBuf);
    });

    /*md - `LAYER2_RESONANCE` to set resonance. */
    Val& Osc2Resonance = val(0.0, "LAYER2_RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layer2Filter.setResonance(p.val.pct());
    });

    SynthKick23(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , waveform(props.lookupTable, props.sampleRate)
        , sampleRate(props.sampleRate)
        , fastWaveform(props.sampleRate, FastWaveform::Type::NOISE, 100.0f)
    {
        initValues();
    }

    double boostTimeInc = 0.0f;
    double boostTime = 0.0f;
    float highFreqBoost(float input)
    {
        if (highBoost.get() == 0) {
            return input;
        }
        // Simple high-shelf boost logic
        // TODO optimize precalculate: highBoost.get() * time
        // float highFreqComponent = input * (highBoost.get() * boostTime); // Emphasize high frequencies
        float highFreqComponent = input * boostTime; // Emphasize high frequencies
        boostTime += boostTimeInc;
        return input + highFreqComponent;
    }

    float scaledClipping = 0.0f;
    void sample(float* buf)
    {
        if (sampleDurationCounter < sampleCountDuration) {
            float envAmp = envelopAmp.next();
            float envFreq = envelopFreq.next();

            float freq = envFreq + noteMult;
            float out = wave->sample(&wavetable.sampleIndex, freq) * envAmp;
            out = addSecondLayer(out);
            out = highFreqBoost(out);
            out = range(out + out * scaledClipping, -1.0f, 1.0f);
            buf[track] = out * velocity;

            sampleDurationCounter++;
            // printf("[%d] sample: %d of %d=%f\n", track, sampleDurationCounter, sampleCountDuration, buf[track]);
        }
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        boostTime = 0.0f;
        wavetable.sampleIndex = 0;
        sampleDurationCounter = 0;
        envelopAmp.reset(sampleCountDuration);
        envelopFreq.reset(sampleCountDuration);
        envelopAmpLayer2.reset(sampleCountDuration);
        velocity = range(_velocity, 0.0f, 1.0f);

        noteMult = pow(2, ((note - baseNote + pitch.get()) / 12.0));

        fastWaveform.setRate(osc2Freq.get() * noteMult);
    }

    void serialize(FILE* file, std::string separator) override
    {
        Mapping::serialize(file, separator);
        envelopFreq.serialize(file, separator, "ENV_FREQ");
        envelopAmpLayer2.serialize(file, separator, "ENV_AMP_OSC2");
    }

    void hydrate(std::string value) override
    {
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
    DataFn dataFunctions[16] = {
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
        { "ENV_AMP_FORM", [this](void* userdata) {
             float* index = (float*)userdata;
             return (void*)envelopAmp.getMorphShape(*index);
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
