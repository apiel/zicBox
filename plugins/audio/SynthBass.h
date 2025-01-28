#ifndef _SYNTH_BASS_H_
#define _SYNTH_BASS_H_

#include "../../helpers/range.h"
#include "audioPlugin.h"
#include "filter.h"
#include "mapping.h"
#include "utils/EnvelopRelative.h"

#include "./utils/Waveform.h"
#include "./utils/Wavetable.h"

#define ZIC_BASS_UI 1000

/*md
## SynthBass

Synth engine to generate bass sounds.

*/
class SynthBass : public Mapping {
protected:
    uint8_t baseNote = 60;

    EffectFilterData filter;
    EffectFilterData filter2;

    // Envelop might need a bit of curve??
    EnvelopRelative envelop = EnvelopRelative({ { 0.0f, 0.0f }, { 1.0f, 0.01f }, { 0.3f, 0.4f }, { 0.0f, 1.0f } });

    unsigned int sampleCountDuration = 0;
    unsigned int sampleIndex = 0;
    float velocity = 1.0f;

    WaveformInterface* wave = nullptr;
    Waveform waveform;
    Wavetable wavetable;
#define BASS_WAVEFORMS_COUNT 4
    struct WaveformType {
        std::string name;
        WaveformInterface* wave;
        uint8_t indexType = 0;
    } waveformTypes[BASS_WAVEFORMS_COUNT] = {
        { "Wavetable", &wavetable },
        { "Sawtooth", &waveform, Waveform::Type::Sawtooth },
        { "Square", &waveform, Waveform::Type::Square },
        { "Pulse", &waveform, Waveform::Type::Pulse },
    };

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;
    float applyReverb(float signal)
    {
        float reverbAmount = reverb.pct();
        if (reverbAmount == 0.0f) {
            return signal;
        }
        int reverbSamples = static_cast<int>((reverbAmount * 0.5f) * props.sampleRate); // Reverb duration scaled
        float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
        float mix = reverbAmount * 0.5f; // Mix scaled proportionally

        if (reverbSamples > REVERB_BUFFER_SIZE) {
            reverbSamples = REVERB_BUFFER_SIZE; // Cap the reverb duration to buffer size
        }

        float reverbSignal = reverbBuffer[reverbIndex];
        reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;
        reverbIndex = (reverbIndex + 1) % reverbSamples;

        return signal * (1.0f - mix) + reverbSignal * mix;
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
        filter2.setCutoff(cutoffValue);
    });
    /*md - `RESONANCE` to set resonance. */
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 3));
        float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 2));
        filter.setResonance(res);
        filter2.setResonance(res);
    });
    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" });
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    // /*/md - FREQ sets the frequency of the bass. */
    // Val& freq = val(30.0f, "FREQ", { "Frequency", .min = 10.0, .max = 200.0, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });
    /*md - FREQ_RATIO sets the frequency of the bass. */
    Val& freqRatio = val(25.0f, "FREQ_RATIO", { "Freq. ratio", .step = 0.1, .floatingPoint = 1, .unit = "%" });

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
            waveform.setWaveformType((Waveform::Type)type.indexType);
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
            float value = range(p.value, 1.0f, ZIC_WAVETABLE_WAVEFORMS_COUNT);
            p.val.setFloat(value);
            wavetable.morph((int)p.val.get() - 1);
            p.val.setString(std::to_string((int)p.val.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
        }
    });

    /*md - `BOOST` to set compression or bass boost. */
    Val& boost = val(0.0f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });

    SynthBass(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name) // clang-format on
        , waveform(props.lookupTable, props.sampleRate)
    {
        initValues();
    }

    float prevInput = 0.0f, prevOutput = 0.0f;
    float applyBoost(float input)
    {
        if (boost.pct() == 0.5f) {
            return input;
        }
        if (boost.pct() > 0.5f) {
            float amount = boost.pct() * 2 - 1.0f;
            if (input > 0.0f) {
                return std::pow(input, 1.0f - amount * 0.8f);
            }
            return -std::pow(-input, 1.0f - amount * 0.8f);
        }
        float amount = 1 - boost.pct() * 2;
        float bassFreq = 0.2f + 0.8f * amount;
        float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
        prevInput = input;
        prevOutput = bassBoosted;
        bassBoosted *= 1.0f + amount * 2.0f;

        return bassBoosted;
    }

    // TODO improve filter perf by adding multiple pass... so cutoff and resonance is only calculated once..

    float scaledClipping = 0.0f;
    float freq = 1.0f;
    void sample(float* buf)
    {
        if (sampleIndex < sampleCountDuration) {
            float time = (float)sampleIndex / (float)sampleCountDuration;
            float env = envelop.next(time);
            float out = wave->sample(&wavetable.sampleIndex, freq);
            out = out * velocity * env;
            filter.setCutoff(0.85 * cutoff.pct() * env + 0.1);
            filter.setSampleData(out);
            out = filter.lp;
            filter2.setCutoff(0.85 * cutoff.pct() * env + 0.1);
            filter2.setSampleData(out);
            out = filter2.lp;
            out = range(out + out * clipping.pct() * 8, -1.0f, 1.0f);
            out = applyReverb(out);
            out = applyBoost(out);
            buf[track] = out;
            sampleIndex++;
        } else {
            buf[track] = applyReverb(buf[track]);
        }
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        velocity = _velocity;
        // printf("bass noteOn: %d %f\n", note, velocity);

        sampleIndex = 0;
        envelop.reset();
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

#endif
