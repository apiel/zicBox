#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/filterArray.h"

class BassEngine : public Engine {
protected:
    MultiFx multiFx;
    MultiFx multiFx2;
    EffectFilterArray<2> filter;

    float velocity = 1.0f;
    float sampleIndex = 0.0f;

    WavetableInterface* wave = nullptr;
    WavetableGenerator waveform;
#define BASS_WAVEFORMS_COUNT 6
    struct WaveformType {
        std::string name;
        WavetableInterface* wave;
        uint8_t indexType = 0;
    } waveformTypes[BASS_WAVEFORMS_COUNT] = {
        { "Sine", &waveform, (uint8_t)WavetableGenerator::Type::Sine },
        { "Sawtooth", &waveform, (uint8_t)WavetableGenerator::Type::Saw },
        { "Square", &waveform, (uint8_t)WavetableGenerator::Type::Square },
        { "Pulse", &waveform, (uint8_t)WavetableGenerator::Type::Pulse },
        { "Triangle", &waveform, (uint8_t)WavetableGenerator::Type::Triangle },
        { "FMSquare", &waveform, (uint8_t)WavetableGenerator::Type::FMSquare },
    };

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a bass sound we want to remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    uint8_t baseNote = 60 + 12;
    float freq = 0.0f;
    uint8_t freqNote = 60;
    void setFreq(uint8_t note = 0)
    {
        if (note == 0) {
            note = freqNote;
        }
        freqNote = note;
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }

public:
    Val& pitch = val(0, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24, .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        setFreq();
    });
    Val& bend = val(0.4f, "BEND", { .label = "Bend", .unit = "%" });

    Val& waveformType = val(1.0f, "WAVEFORM_TYPE", { .label = "Waveform", .type =VALUE_STRING, .max = BASS_WAVEFORMS_COUNT - 1 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);
        if (wave && current == p.val.get()) {
            return;
        }
        WaveformType type = waveformTypes[(int)p.val.get()];
        p.val.setString(type.name);
        wave = type.wave;
        waveform.setType((WavetableGenerator::Type)type.indexType);
        // shape.set(waveform.modulation * 1000.0f);
    });
    Val& shape = val(0.0f, "SHAPE", { .label = "Shape", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        waveform.setMorph(p.val.pct());
    });

    Val& cutoff = val(50.0, "CUTOFF", { .label = "Cutoff", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        float cutoffValue = 0.85 * p.val.pct() + 0.1;
        filter.setCutoff(cutoffValue);
    });
    Val& resonance = val(0.0, "RESONANCE", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        // float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 3));
        float res = 0.95 * (1.0 - std::pow(1.0 - p.val.pct(), 2));
        filter.setResonance(res);
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);

    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 edit", .unit = "%" });

    // --- constructor ---
    BassEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Bass")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
        , waveform(props.lookupTable, props.sampleRate)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            float out = buf[track];
            out = multiFx.apply(out, fxAmount.pct());
            out = multiFx2.apply(out, fx2Amount.pct());
            buf[track] = out;
            return;
        }

        float bendedFreq = freq * (1.f - bend.pct() * envAmpVal);
        float out = wave->sample(&sampleIndex, bendedFreq);
        out = out * velocity * envAmpVal;

        filter.setCutoff(0.85 * cutoff.pct() * envAmpVal + 0.1);
        filter.setSampleData(out, 0);
        filter.setSampleData(filter.lp[0], 1);
        out = filter.lp[1];

        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        Engine::noteOn(note, _velocity);
        velocity = _velocity;
        setFreq(note);
    }
};
