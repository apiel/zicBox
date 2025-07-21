#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/Wavetable.h"
#include "plugins/audio/utils/WavetableGenerator.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/filterArray.h"

class BassEngine : public DrumEngine {
protected:
    uint8_t baseNote = 60;

    EffectFilterArray<2> filter;

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

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;

    float prevInput = 0.0f, prevOutput = 0.0f;
    float applyEffects(float input)
    {
        float output = input;

        output = applyReverb(output, reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);

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
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 });
    Val& cutoff = val(50.0, "CUTOFF", { "Cutoff", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        float cutoffValue = 0.85 * p.val.pct() + 0.1;
        filter.setCutoff(cutoffValue);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 3));
        float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 2));
        filter.setResonance(res);
    });
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Gain Clipping", .unit = "%" });
    Val& reverb = val(0.3f, "REVERB", { "Reverb", .unit = "%" });
    Val& freqRatio = val(25.0f, "FREQ_RATIO", { "Freq. ratio", .step = 0.1, .floatingPoint = 1, .unit = "%" });
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
    Val& shape = val(0.0f, "SHAPE", { "Shape", VALUE_STRING, .max = 1000 }, [&](auto p) {
        if (waveformType.get() != 0.0f) {
            int direction = p.value - p.val.get();
            int value = p.val.get() + direction * 10;
            p.val.setFloat(value);
            waveform.setShape(p.val.pct());
            p.val.setString(std::to_string((int)(p.val.get() * 0.1)) + "%");
        } else {
            int value = range(p.value, 1.0f, wavetable.fileBrowser.count);
            p.val.setFloat(value);

            int position = p.val.get();
            wavetable.open(position, false);
            p.val.setString(wavetable.fileBrowser.getFileWithoutExtension(position));
        }
    });
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
    Val& boost = val(0.0f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });

    BassEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Bass")
        , waveform(props.lookupTable, props.sampleRate)
    {
        initValues();
    }

    // static constexpr int REVBUF = 48000;
    // float reverbBuf[REVBUF] = {};
    // int rIdx = 0;

    // float phase = 0.f;
    // float filterState = 0.f;
    // float prevInput = 0.f;
    // float prevOutput = 0.f;

    float scaledClipping = 0.0f;
    float freq = 1.0f;

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        float out = wave->sample(&wavetable.sampleIndex, freq);
        out = out * velocity * envAmp;

        filter.setCutoff(0.85 * cutoff.pct() * envAmp + 0.1);
        filter.setSampleData(out, 0);
        filter.setSampleData(filter.lp[0], 1);
        out = filter.lp[1];

        out = range(out + out * clipping.pct() * 8, -1.0f, 1.0f);
        out = applyEffects(out);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        // buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuf, rIdx, REVBUF);
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);
    }

    void noteOn(uint8_t note, float, void* = nullptr) override
    {
        // phase = 0.f;
        pitch.set(pitch.get() * powf(2.f, (note - 60) / 12.f));
        // filterState = 0.f;
        prevOutput = 0.f;

        sampleIndex = 0;
        wavetable.sampleIndex = 0;
        freq = freqRatio.pct() * pow(2, ((note - baseNote + pitch.get()) / 12.0)) + 0.05;
        // printf("freq: %f\n", freq);
    }

private:
    // float applyEffects(float x)
    // {
    //     float d = drive.pct() * 2.f - 1.f;
    //     if (d > 0.f) {
    //         x = applyDrive(x, d, props.lookupTable);
    //     } else {
    //         x = applyBoost(x, -d, prevInput, prevOutput);
    //     }
    //     return applyReverb(x, reverb.pct(), reverbBuf, rIdx, REVBUF);
    // }

    // float applyEffects(float input)
    // {
    //     float output = input;

    //     output = applyReverb(output, reverb.pct(), reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);

    //     if (boost.pct() == 0.5f) {
    //         return output;
    //     }
    //     if (boost.pct() > 0.5f) {
    //         float amount = boost.pct() * 2 - 1.0f;
    //         return applyCompression(output, amount);
    //     }
    //     float amount = 1 - boost.pct() * 2;
    //     return applyBoost(output, amount, prevInput, prevOutput);
    // }
};
