/** Description:
This module defines `SynthBass`, a specialized digital sound synthesizer designed exclusively for creating rich, low-frequency bass sounds within an audio system. It acts as a complete instrument engine, handling all aspects of sound creation from initial vibration to final effects.

### How It Works

The engine generates sound through a sequential process:

1.  **Waveform Generation:** It starts by creating the raw sonic material, which can be chosen from several fundamental types (like Sawtooth or Square wave) or loaded from customizable digital tables (Wavetables). Users can sculpt the texture of this source sound using the **Waveform Type** and **Shape** controls.
2.  **Timing and Volume (Envelope):** An internal mechanism, called the Envelope, controls the dynamics of the sound. When a note is triggered, this system determines how quickly the sound reaches full volume and how long it fades out, based on settings like **Duration** and **Decay**.
3.  **Tone Shaping (Filtering):** The raw sound is passed through a powerful filter system. The **Cutoff** control determines which high frequencies are removed (making the sound duller or brighter), while **Resonance** adds a distinctive peak near the cutoff point, giving the bass punch and character.
4.  **Final Effects:** Before the sound is output, optional effects are applied, including **Reverb** for adding spatial depth, and a **Boost** control that can switch between aggressive compression or traditional bass boosting to manage volume and add grit.

In summary, `SynthBass` combines a versatile sound source, precise volume timing, frequency filtering, and mastering effects to produce high-quality, customizable bass tones.

sha: 623c00e2da23c4152d91ecffc351a12d739d7ca77b5a576b93ef94660807dd7b 
*/
#pragma once

#include "audioPlugin.h"
#include "helpers/clamp.h"
#include "mapping.h"
#include "audio/EnvelopRelative.h"
#include "audio/Wavetable.h"
#include "audio/WavetableGenerator.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "audio/filterArray.h"

#define ZIC_BASS_UI 1000

/*md
## SynthBass

Synth engine to generate bass sounds.

*/
class SynthBass : public Mapping {
protected:
    uint8_t baseNote = 60;

    EffectFilterArray<2> filter;

    // Envelop might need a bit of curve??
    EnvelopRelative envelop = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.3f, 0.4f }, { 0.0f, 1.0f } });

    unsigned int sampleCountDuration = 0;
    unsigned int sampleIndex = 0;
    float velocity = 1.0f;

    WavetableInterface* wave = nullptr;
    WavetableGenerator waveform;
    Wavetable wavetable;
#define BASS_WAVEFORMS_COUNT 4
    struct WaveformType {
        std::string name;
        WavetableInterface* wave;
        uint8_t indexType = 0;
    } waveformTypes[BASS_WAVEFORMS_COUNT] = {
        { "Wavetable", &wavetable },
        { "Sawtooth", &waveform, WavetableGenerator::Type::Sawtooth },
        { "Square", &waveform, WavetableGenerator::Type::Square },
        { "Pulse", &waveform, WavetableGenerator::Type::Pulse },
    };

    REVERB_BUFFER
    int reverbIndex = 0;

    float prevInput = 0.0f, prevOutput = 0.0f;
    float applyEffects(float input)
    {
        float output = input;

        output = applyReverb(output, reverb.pct(), buffer, reverbIndex);

        if (boost.pct() == 0.5f) {
            return output;
        }
        if (boost.pct() > 0.5f) {
            float amount = boost.pct() * 2 - 1.0f;
            return applyCompression(output, amount);
        }
        float amount = 1 - boost.pct() * 2;
        return applyBoost(output, amount, prevInput, prevOutput);
    }

public:
    /*md **Values**: */
    /*md - `PITCH` set the pitch.*/
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 });
    /*md - `DURATION` set the duration of the envelop.*/
    Val& duration = val(100.0f, "DURATION", { "Duration", .min = 10.0, .max = 5000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        bool isOff = sampleCountDuration == sampleIndex;
        sampleCountDuration = p.val.get() * (props.sampleRate * 0.001f);
        if (isOff) {
            sampleIndex = sampleCountDuration;
        }
    });
    /*md - `DECAY_LEVEL` set the decay level.*/
    Val& decayLevel = val(0.5f, "DECAY_LEVEL", { "Decay Level", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envelop.data[2].modulation = p.val.pct();
    });
    /*md - `DECAY_TIME` set the decay time percentage base on the total duration.*/
    Val& decayTime = val(100.0f, "DECAY_TIME", { "Decay Time", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envelop.data[2].time = p.val.pct();
    });
    /*md - `CUTOFF` to set cutoff frequency of low pass filter. */
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        float cutoffValue = 0.85 * p.val.pct() + 0.1;
        filter.setCutoff(cutoffValue);
    });
    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 3));
        float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 2));
        filter.setResonance(res);
    });
    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" });
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    // /*/md - FREQ sets the frequency of the bass. */
    // Val& freq = val(30.0f, "FREQ", { "Frequency", .min = 10.0, .max = 200.0, .step = 0.1, .unit = "Hz" });
    /*md - FREQ_RATIO sets the frequency of the bass. */
    Val& freqRatio = val(25.0f, "FREQ_RATIO", { "Freq. ratio", .step = 0.1, .unit = "%" });

    /*md - `WAVEFORM_TYPE` Select waveform type (wavetable, sawtooth, square...).*/
    Val& waveformType = val(1.0f, "WAVEFORM_TYPE", { "Waveform", VALUE_STRING, .max = BASS_WAVEFORMS_COUNT - 1 }, [&](auto p) {
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
            int value = CLAMP(p.value, 1.0f, wavetable.fileBrowser.count);
            p.val.setFloat(value);

            int position = p.val.get();
            wavetable.open(position, false);
            p.val.setString(wavetable.fileBrowser.getFileWithoutExtension(position));

            // sampleIndex = -1; // set counter to the maximum
            // sampleIndex = sampleCountDuration;
        }
    });

    /*md - `MACRO` Macro is arbitrary parameter depending of selected waveform type. */
    Val& macro = val(0.0f, "MACRO", { "Macro", VALUE_STRING }, [&](auto p) {
        if (waveformType.get() != 0.0f) {
            p.val.setFloat(p.value);
            waveform.setMacro(p.val.pct());
            p.val.setString(std::to_string((int)p.val.get()) + "%");
        } else {
            float value = CLAMP(p.value, 1.0f, ZIC_WAVETABLE_WAVEFORMS_COUNT);
            p.val.setFloat(value);
            wavetable.morph((int)p.val.get() - 1);
            p.val.setString(std::to_string((int)p.val.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
        }
    });

    /*md - `BOOST` to set compression or bass boost. */
    Val& boost = val(0.0f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });

    SynthBass(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config) // clang-format on
        , waveform(props.lookupTable, props.sampleRate)
    {
        initValues();
    }

    // TODO improve filter perf by adding multiple pass... so cutoff and resonance is only calculated once..

    float scaledClipping = 0.0f;
    float freq = 1.0f;
    void sample(float* buf)
    {
        if (sampleIndex < sampleCountDuration) {
            float env = envelop.next();
            float out = wave->sample(&wavetable.sampleIndex, freq);
            out = out * velocity * env;

            filter.setCutoff(0.85 * cutoff.pct() * env + 0.1);
            filter.setSampleData(out, 0);
            filter.setSampleData(filter.lp[0], 1);
            out = filter.lp[1];

            out = CLAMP(out + out * clipping.pct() * 8, -1.0f, 1.0f);
            out = applyEffects(out);
            buf[track] = out;
            sampleIndex++;
        } else {
            buf[track] = applyReverb(buf[track], reverb.pct(), buffer, reverbIndex);
        }
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        velocity = _velocity;
        // printf("bass noteOn: %d %f\n", note, velocity);

        sampleIndex = 0;
        envelop.reset(sampleCountDuration);
        wavetable.sampleIndex = 0;
        freq = freqRatio.pct() * pow(2, ((note - baseNote + pitch.get()) / 12.0)) + 0.05;
        // printf("freq: %f\n", freq);
    }

    std::vector<float> waveformData;
    DataFn dataFunctions[1] = {
        { "WAVEFORM", [this](void* userdata) {
             if (!wave) {
                 return (void*)NULL;
             }
             float* index = (float*)userdata;
             return (void*)wave->sample(index);
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
