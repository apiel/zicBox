#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/EnvelopDrumAmp.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/effects/applyReverb.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class VolcanEngine : public DrumEngine {
protected:
#define WAVEFORMS_COUNT 6
    struct WaveformType {
        std::string name;
        WavetableInterface* wave;
        uint8_t indexType = 0;
    };

    struct Layer {
        WavetableGenerator osc;
        EnvelopDrumAmp envAmp;
        EnvelopDrumAmp envPitch;
        WavetableInterface* wave = nullptr;
        WaveformType waveformTypes[WAVEFORMS_COUNT] = {
            { "Sine", &osc, (uint8_t)WavetableGenerator::Type::Sine },
            { "Sawtooth", &osc, (uint8_t)WavetableGenerator::Type::Saw },
            { "Square", &osc, (uint8_t)WavetableGenerator::Type::Square },
            { "Pulse", &osc, (uint8_t)WavetableGenerator::Type::Pulse },
            { "Triangle", &osc, (uint8_t)WavetableGenerator::Type::Triangle },
            { "FMSquare", &osc, (uint8_t)WavetableGenerator::Type::FMSquare },
        };

        float sampleIndex = 0.0f;
        float baseFreq = 100.0f;
        float decayMorph = 0.0f;

        Layer(LookupTable* lut, float sampleRate)
            : osc(lut, sampleRate)
        {
        }

        float render(float t, float mainEnvAmp)
        {
            float ampVal = 1.0f - envAmp.next(t);
            float envFreq = envPitch.next(t);
            float modulatedFreq = baseFreq + envFreq;
            float s = wave ? wave->sample(&sampleIndex, modulatedFreq) : 0.0f;
            return s * ampVal * mainEnvAmp;
        }
    };

    Layer layerA;
    Layer layerB;

    MultiFx multiFx;

    float velocity = 1.0f;
    uint8_t baseNote = 60;
    float mixAB = 0.5f;
    float fxAmount = 0.0f; // -1 → drive, +1 → compression

public:
    // --- 10 parameters ---
    Val& mix = val(0.5f, "MIX", { "Mix A/B", VALUE_CENTERED, .min = -100.0f, .max = 100.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        mixAB = p.val.pct();
    });

    Val& fx = val(0.0f, "FX", { "FX Blend", VALUE_CENTERED, .min = -100.0f, .max = 100.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        fxAmount = p.val.pct() * 2 - 1; // -1 to +1
    });

    // --- Layer A ---
    Val& layerAPitch = val(0, "LAYER_A_PITCH", { "A Pitch", VALUE_CENTERED, .min = -24, .max = 24, .skipJumpIncrements = true });
    Val& layerADecay = val(0.0f, "LAYER_A_DECAY", { "A Decay", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layerA.envAmp.morph(p.val.pct());
    });
    Val& layerAWaveform = val(250.0f, "LAYER_A_WAVEFORM", { "A Wave", VALUE_STRING, .max = WAVEFORMS_COUNT * 100 - 1 }, [&](auto p) {
        int currentWave = (int)p.val.get() / 100;
        p.val.setFloat(p.value);
        int newWave = (int)p.val.get() / 100;
        if (!layerA.wave || currentWave != newWave) {
            auto type = layerA.waveformTypes[newWave];
            p.val.props().unit = type.name;
            layerA.wave = type.wave;
            layerA.osc.setType((WavetableGenerator::Type)type.indexType);
        }
        int morph = p.val.get() - newWave * 100;
        layerA.osc.setMorph(morph / 100.0f);
        p.val.setString(std::to_string(morph) + "%");
    });
    Val& layerAMod = val(0.0f, "LAYER_A_MOD", { "A Mod", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layerA.envPitch.morph(p.val.pct());
    });

    // --- Layer B ---
    Val& layerBPitch = val(0, "LAYER_B_PITCH", { "B Pitch", VALUE_CENTERED, .min = -24, .max = 24, .skipJumpIncrements = true });
    Val& layerBDecay = val(0.0f, "LAYER_B_DECAY", { "B Decay", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layerB.envAmp.morph(p.val.pct());
    });
    Val& layerBWaveform = val(250.0f, "LAYER_B_WAVEFORM", { "B Wave", VALUE_STRING, .max = WAVEFORMS_COUNT * 100 - 1 }, [&](auto p) {
        int currentWave = (int)p.val.get() / 100;
        p.val.setFloat(p.value);
        int newWave = (int)p.val.get() / 100;
        if (!layerB.wave || currentWave != newWave) {
            auto type = layerB.waveformTypes[newWave];
            p.val.props().unit = type.name;
            layerB.wave = type.wave;
            layerB.osc.setType((WavetableGenerator::Type)type.indexType);
        }
        int morph = p.val.get() - newWave * 100;
        layerB.osc.setMorph(morph / 100.0f);
        p.val.setString(std::to_string(morph) + "%");
    });
    Val& layerBMod = val(0.0f, "LAYER_B_MOD", { "B Mod", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layerB.envPitch.morph(p.val.pct());
    });

    // --- constructor ---
    VolcanEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Volcan")
        , layerA(props.lookupTable, props.sampleRate)
        , layerB(props.lookupTable, props.sampleRate)
        , multiFx(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;

    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {
        float t = (float)sampleCounter / totalSamples;

        float outA = layerA.render(t, envAmp);
        float outB = layerB.render(t, envAmp);

        float mixOut = outA * (1.0f - mixAB) + outB * mixAB;

        // Apply FX depending on bipolar param
        float fxOut = mixOut;
        if (fxAmount < 0.0f)
            fxOut = applyDrive(fxOut, -fxAmount * 2.0f, props.lookupTable); // stronger drive as it goes negative
        else if (fxAmount > 0.0f)
            fxOut = applyReverb(fxOut, fxAmount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);

        buf[track] = fxOut * velocity;
    }

    void sampleOff(float* buf) override
    {
        if (fxAmount > 0.0f) {
            buf[track] = applyReverb(buf[track], fxAmount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE);
        }
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        layerA.sampleIndex = 0.0f;
        layerB.sampleIndex = 0.0f;

        // use 220 instead 440 to have a more percussive sound
        layerA.baseFreq = 220.0f * powf(2.0f, (note - baseNote + layerAPitch.get()) / 12.0f);
        layerB.baseFreq = 220.0f * powf(2.0f, (note - baseNote + layerBPitch.get()) / 12.0f);
    }
};
