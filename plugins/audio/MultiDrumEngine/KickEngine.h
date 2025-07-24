#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/EnvelopeTableGenerator.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class KickEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    MMfilter filter;

    WavetableInterface* wave = nullptr;
    WavetableGenerator waveform;
    EnvelopeTableGenerator envelope;

#define WAVEFORMS_COUNT 6
#define ENVELOP_COUNT 20
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
          { "ShortPunch", &envelope, (uint8_t)EnvelopeTableGenerator::Type::ShortPunch },
          { "LongBoom", &envelope, (uint8_t)EnvelopeTableGenerator::Type::LongBoom },
          { "SnappyFall", &envelope, (uint8_t)EnvelopeTableGenerator::Type::SnappyFall },
          { "SmoothSlope", &envelope, (uint8_t)EnvelopeTableGenerator::Type::SmoothSlope },
          { "RubberDrop", &envelope, (uint8_t)EnvelopeTableGenerator::Type::RubberDrop },
          { "TiltedArc", &envelope, (uint8_t)EnvelopeTableGenerator::Type::TiltedArc },
          { "BassPluck", &envelope, (uint8_t)EnvelopeTableGenerator::Type::BassPluck },
          { "BreakPoint", &envelope, (uint8_t)EnvelopeTableGenerator::Type::BreakPoint },
          { "ClickSpike", &envelope, (uint8_t)EnvelopeTableGenerator::Type::ClickSpike },
          { "InversePow", &envelope, (uint8_t)EnvelopeTableGenerator::Type::InversePow },
          { "AnalogSnap", &envelope, (uint8_t)EnvelopeTableGenerator::Type::AnalogSnap },
          { "CubicDrop", &envelope, (uint8_t)EnvelopeTableGenerator::Type::CubicDrop },
          { "GlitchFall", &envelope, (uint8_t)EnvelopeTableGenerator::Type::GlitchFall },
          { "HyperCurve", &envelope, (uint8_t)EnvelopeTableGenerator::Type::HyperCurve },
          { "Foldback", &envelope, (uint8_t)EnvelopeTableGenerator::Type::Foldback },
          { "GhostTail", &envelope, (uint8_t)EnvelopeTableGenerator::Type::GhostTail },
      };

public:
    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24, .skipJumpIncrements = true });
    Val& transient = val(0.0, "TRANSIENT", { "Transient", VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
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
    });
    Val& shape = val(0.0f, "WAVEFORM_SHAPE", { "Waveform Shape", VALUE_BASIC, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        waveform.setMorph(p.val.pct());
    });

    Val& envelopeType = val(0.0f, "ENVELOPE_TYPE", { "Envelope", VALUE_STRING, .max = ENVELOP_COUNT - 1 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);
        WaveformType type = envelopeTypes[(int)p.val.get()];
        p.val.setString(type.name);
        envelope.setType((EnvelopeTableGenerator::Type)type.indexType);
    });
    Val& envelopeShape = val(0.0f, "ENVELOPE_SHAPE", { "Envelope Shape", VALUE_BASIC, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envelope.setMorph(p.val.pct());
    });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    float waveshapeAmount = 0.0f;
    Val& boost = val(0.0f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& waveshape = val(0.0, "WAVESHAPE", { "Waveshape", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        waveshapeAmount = p.val.pct() * 2 - 1.0f;
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
    float lpState = 0.f;
    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {

        float t = float(sampleCounter) / totalSamples;
        float envFreq = envelope.next(t);
        float modulatedFreq = freq + envFreq;
        float out = wave->sample(&sampleIndex, modulatedFreq) * envAmp;

        if (t < 0.01f) {
            float amt = transient.pct(); // 0.0 to 1.0

            if (amt < 0.5f) {
                if (t < 0.005f) {
                float weight = (0.5f - amt) * 2.0f;
                float noise = (props.lookupTable->getNoise() - 0.5f) * 2.0f;
                out += noise * weight * 6.0f;
                }
            } else if (amt > 0.5f) {
                float weight = (amt - 0.5f) * 2.0f;
                float highpassed = out - lpState;
                lpState += 0.01f * (out - lpState); // simple LPF
                out += highpassed * weight * 2.0f;
                if (t < 0.001f) {
                    float spike = (props.lookupTable->getNoise() - 0.5f) * 10.f;
                    out += spike * weight;
                }
            }
        }

        out = applyBoostOrCompression(out);
        out = applyWaveshape(out, waveshapeAmount, props.lookupTable);
        out = filter.process(out);

        buf[track] = out * velocity;
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
        lpState = 0.f;
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }

    float* getShape(float pct) override
    {
        shapeValue = -envelope.next(pct);
        return &shapeValue;
    }

protected:
    float prevInput = 0.f;
    float prevOutput = 0.f;

    float applyBoostOrCompression(float input)
    {
        if (boost.pct() == 0.5f)
            return input;
        if (boost.pct() > 0.5f) {
            float amt = (boost.pct() - 0.5f) * 2.f;
            return applyBoost(input, amt, prevInput, prevOutput);
        } else {
            float amt = (0.5f - boost.pct()) * 2.f;
            return applyCompression(input, amt);
        }
    }
};
