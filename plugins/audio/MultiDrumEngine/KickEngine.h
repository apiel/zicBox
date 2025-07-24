#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/EnvelopeTableGenerator.h"
#include "plugins/audio/utils/WavetableGenerator2.h"

class KickEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    WavetableInterface* wave = nullptr;
    WavetableGenerator waveform;
    EnvelopeTableGenerator envelope;

#define WAVEFORMS_COUNT 6
#define ENVELOP_COUNT 4
    struct WaveformType {
        std::string name;
        WavetableInterface* wave;
        uint8_t indexType = 0;
    } waveformTypes[WAVEFORMS_COUNT] = {
        { "Sine", &waveform, (uint8_t)WavetableGenerator::Type::Sine },
        { "Sawtooth", &waveform, (uint8_t)WavetableGenerator::Type::Saw },
        { "Square", &waveform, (uint8_t)WavetableGenerator::Type::Square },
        { "Pulse", &waveform, (uint8_t)WavetableGenerator::Type::Pulse },
        { "Triangle", &waveform, (uint8_t)WavetableGenerator::Type::Triangle },
        { "FMSquare", &waveform, (uint8_t)WavetableGenerator::Type::FMSquare },
    },
      envelopeTypes[ENVELOP_COUNT] = {
          { "ExpoDecay", &envelope, (uint8_t)EnvelopeTableGenerator::Type::ExpoDecay },
          { "DownHills", &envelope, (uint8_t)EnvelopeTableGenerator::Type::DownHills },
          { "MultiDecay", &envelope, (uint8_t)EnvelopeTableGenerator::Type::MultiDecay },
          { "SinPow", &envelope, (uint8_t)EnvelopeTableGenerator::Type::SinPow },
      };

public:
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24, .skipJumpIncrements = true });
    Val& waveformType = val(1.0f, "WAVEFORM_TYPE", { "Waveform", VALUE_STRING, .max = WAVEFORMS_COUNT - 1 }, [&](auto p) {
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
    Val& shape = val(0.0f, "WAVEFORM_SHAPE", { "Waveform Shape", VALUE_BASIC, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        waveform.setMorph(p.val.pct());
    });

    Val& envelopeType = val(1.0f, "ENVELOPE_TYPE", { "Envelope", VALUE_STRING, .max = ENVELOP_COUNT - 1 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);
        if (current == p.val.get()) {
            return;
        }
        WaveformType type = envelopeTypes[(int)p.val.get()];
        p.val.setString(type.name);
        envelope.setType((EnvelopeTableGenerator::Type)type.indexType);
    });
    Val& envelopeShape = val(0.0f, "ENVELOPE_SHAPE", { "Envelope Shape", VALUE_BASIC, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envelope.setMorph(p.val.pct());
    });

    KickEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Kick")
        , waveform(props.lookupTable, props.sampleRate)
        , envelope(props.lookupTable)
    {
        initValues();
    }

    float scaledClipping = 0.0f;
    float freq = 1.0f;

    float sampleIndex = 0.0f;
    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {

        float t = float(sampleCounter) / totalSamples;
        float envFreq = envelope.next(t);
        float modulatedFreq = freq + envFreq;
        float out = wave->sample(&sampleIndex, modulatedFreq) * envAmp;
        buf[track] = out;
    }

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a kick sound we want bass and we remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    uint8_t baseNote = 60 + 12;
    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        sampleIndex = 0;
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }
};
