#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/KickEnvTableGenerator.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/KickTransientTableGenerator.h"
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
    KickTransientTableGenerator transient;
    KickEnvTableGenerator kickEnv;

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
    };

public:
    Val& freqModulation = val(10.0f, "ENVELOPE_SHAPE", { "Freq. mod.", VALUE_BASIC, .step = 0.05f, .floatingPoint = 2, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        kickEnv.setMorph(p.val.pct());
    });

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

    Val& pitch = val(0, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24, .incType = INC_ONE_BY_ONE });

    Val& envelopeType = val(0.0f, "ENVELOPE_TYPE", { "Freq. Env.", VALUE_STRING, .max = ENVELOP_COUNT - 1 }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);
        // WaveformType type = envelopeTypes[(int)p.val.get()];
        // p.val.setString(type.name);
        // envelope.setType((EnvelopeTableGenerator::Type)type.indexType);
    });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& transientMorph = val(100.0, "TRANSIENT", { "Transient", VALUE_BASIC, .step = 0.05f, .floatingPoint = 2, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        transient.setMorph(p.val.pct());
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
        float envFreq = kickEnv.next(t);
        float modulatedFreq = freq + envFreq;
        float out = wave->sample(&sampleIndex, modulatedFreq) * envAmp * 0.5f;

        if (t < 0.01f && transientMorph.get() > 0.0f) {
            out = out + transient.next(t) * envAmp;
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
    }

    float fallbackVal = 0.0f;
    DataFn dataFunctions[2] = {
        { "VAL_1_GRAPH", [this](void* userdata) {
             float* index = (float*)userdata;
             return (void*)kickEnv.sample(index);
         } },
        { "VAL_2_GRAPH", [this](void* userdata) {
             if (wave == nullptr) {
                 //  return (void*)nullptr;
                 return (void*)&fallbackVal;
             }
             float* index = (float*)userdata;
             return (void*)wave->sample(index);
         } },
    };
    DEFINE_GETDATAID_AND_DATA

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
