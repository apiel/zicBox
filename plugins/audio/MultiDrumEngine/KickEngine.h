#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/KickEnvTableGenerator.h"
#include "plugins/audio/utils/KickTransientTableGenerator.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/effects/applyBoost.h"
#include "plugins/audio/utils/effects/applyCompression.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class KickEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    MMfilter filter;
    MultiFx multiFx;
    MultiFx multiFx2;

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
    GraphPointFn freqGraph = [&](float index) { return *kickEnv.sample(&index); };
    Val& freqModulation = val(10.0f, "ENVELOPE_SHAPE", { .label = "Freq. mod.", .type = VALUE_BASIC, .step = 0.05f, .floatingPoint = 2, .unit = "%", .graph = freqGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        kickEnv.setMorph(p.val.pct());
    });

    GraphPointFn waveGraph = [&](float index) { return wave == nullptr ? 0.0f : *wave->sample(&index); };
    Val& waveformType = val(250.0f, "WAVEFORM_TYPE", { .label = "Waveform", .type = VALUE_STRING, .max = WAVEFORMS_COUNT * 100 - 1, .graph = waveGraph }, [&](auto p) {
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

    Val& pitch = val(0, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24, .incType = INC_ONE_BY_ONE });

    GraphPointFn transientGraph = [&](float index) { return *transient.sample(&index); };
    Val& transientMorph = val(100.0, "TRANSIENT", { .label = "Transient", .step = 0.05f, .floatingPoint = 2, .unit = "%", .graph = transientGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        transient.setMorph(p.val.pct());
    });

    Val& cutoff = val(0.0, "CUTOFF", { .label = "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);
    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 edit", .unit = "%" });

    KickEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Kick")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
        , waveform(props.lookupTable, props.sampleRate)
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

        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());

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
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        sampleIndex = 0;
        lpState = 0.f;
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }
};
