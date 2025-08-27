#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/EnvelopeTableGenerator.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/TransientGenerator.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class KickEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    MMfilter filter;
    MultiFx multiFx;

    WavetableInterface* wave = nullptr;
    WavetableGenerator waveform;
    EnvelopeTableGenerator envelope;
    TransientGenerator transient;

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

    Val& waveformType = val(250.0f, "WAVEFORM_TYPE", { "Waveform", VALUE_STRING, .max = WAVEFORMS_COUNT * 100 - 1 }, [&](auto p) {
        float current = p.val.get();
        int currentWave = (int)p.val.get() / 100;
        p.val.setFloat(p.value);
        int newWave = (int)p.val.get() / 100;
        if (!wave || currentWave != newWave) {
            WaveformType type = waveformTypes[newWave];
            p.val.props().unit = type.name;
            wave = type.wave;
            waveform.setType((WavetableGenerator::Type)type.indexType);
        }
        int morph = p.val.get() - newWave * 100;
        waveform.setMorph(morph / 100.0f);
        p.val.setString(std::to_string(morph) + "%");
    });

    Val& envelopeType = val(0.0f, "ENVELOPE_TYPE", { "Freq. Env.", VALUE_STRING, .max = ENVELOP_COUNT - 1 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);
        WaveformType type = envelopeTypes[(int)p.val.get()];
        p.val.setString(type.name);
        envelope.setType((EnvelopeTableGenerator::Type)type.indexType);
    });
    Val& envelopeShape = val(0.0f, "ENVELOPE_SHAPE", { "Env. Shape", VALUE_BASIC, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envelope.setMorph(p.val.pct());
    });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& transientMorph = val(100.0, "TRANSIENT", { "Transient", VALUE_STRING, .step = 0.1f, .floatingPoint = 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        transient.morphType(p.val.pct());
        p.val.setString(std::to_string((int)(transient.getMorph() * 100)) + "%");
        p.val.props().unit = transient.getTypeName();
    });

    Val& boost = val(0.0f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    KickEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Kick")
        , waveform(props.lookupTable, props.sampleRate)
        , multiFx(props.sampleRate, props.lookupTable)
        , envelope(props.lookupTable)
        , transient(props.sampleRate, 50)
    {
        initValues();
    }

    float scaledClipping = 0.0f;
    float freq = 1.0f;

    float sampleIndex = 0.0f;
    float lpState = 0.f;
    float transientIndex = 0.0f;
    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {

        float t = float(sampleCounter) / totalSamples;
        float envFreq = envelope.next(t);
        float modulatedFreq = freq + envFreq;
        float out = wave->sample(&sampleIndex, modulatedFreq) * envAmp * 0.5f;

        if (t < 0.01f && transientMorph.get() > 0.0f) {
            out = out + transient.next(&transientIndex) * envAmp;
        }

        out = applyBoostOrCompression(out);
        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out * velocity;
    }

    void sampleOff(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
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
        lpState = 0.f;
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
        transientIndex = 0.0f;
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
