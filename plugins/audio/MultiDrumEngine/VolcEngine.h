/** Description:
This file defines the `VolcEngine`, an advanced sound generation module designed specifically for creating percussive or drum sounds. It builds upon a general `DrumEngine` framework, adding complex layering and effect capabilities.

**Core Architecture: Layered Synthesis**
The engine operates using two independent sound generators, Layer A and Layer B. Each layer is a complete synthesizer voice responsible for creating a basic tone.

**Sound Generation Mechanics**
Each layer features an oscillator that generates the fundamental tone using various selectable and morphable waveforms (such as Sine, Sawtooth, or Square). The sound is then dynamically shaped by two critical components:
1.  **Amplitude Envelope:** Controls how the volume starts loud and quickly fades out (decay) over time, crucial for percussive hits.
2.  **Pitch Envelope:** Modulates the frequency at the start of the sound, allowing for characteristic pitch drops common in kick drums.

When a note is triggered, the engine calculates the initial frequency for both Layer A and B based on the musical note and specified pitch offsets.

**Mixing and Effects**
The outputs of Layer A and Layer B are combined using a central "Mix" control. This blended signal is then routed through a unique effect system controlled by a single "FX Blend" parameter. This parameter acts bipolarly:
*   If set negatively, it applies subtle distortion (Drive).
*   If set positively, it applies spatial Reverb.

**User Controls**
The engine exposes ten main parameters for shaping the sound, allowing users to adjust the Pitch, Decay rate, Waveform type, and pitch Modulation intensity independently for both Layer A and Layer B, alongside the central Mix and FX Blend controls.

sha: d44801499168fe31974f48b2facb0631854c82d8de1709538774560606463293 
*/
#pragma once

#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/EnvelopDrumAmp.h"
#include "audio/MMfilter.h"
#include "audio/WavetableGenerator2.h"
#include "audio/effects/applyReverb.h"
#include "audio/effects/applyDrive.h"

class VolcEngine : public DrumEngine {
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

    float velocity = 1.0f;
    uint8_t baseNote = 60;
    float mixAB = 0.5f;
    float fxAmount = 0.0f; // -1 → drive, +1 → compression

public:
    // --- 10 parameters ---
    Val& mix = val(0.5f, "MIX", { .label = "Mix A/B", .type = VALUE_CENTERED, .min = -100.0f, .max = 100.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        mixAB = p.val.pct();
    });

    Val& fx = val(0.0f, "FX", { .label = "FX Blend", .type = VALUE_CENTERED, .min = -100.0f, .max = 100.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        fxAmount = p.val.pct() * 2 - 1; // -1 to +1
    });

    // --- Layer A ---
    Val& layerAPitch = val(0, "LAYER_A_PITCH", { .label = "A Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24, .incType = INC_ONE_BY_ONE });
    Val& layerADecay = val(0.0f, "LAYER_A_DECAY", { .label = "A Decay", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layerA.envAmp.morph(p.val.pct());
    });
    Val& layerAWaveform = val(250.0f, "LAYER_A_WAVEFORM", { .label = "A Wave", .type = VALUE_STRING, .max = WAVEFORMS_COUNT * 100 - 1 }, [&](auto p) {
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
    Val& layerAMod = val(0.0f, "LAYER_A_MOD", { .label = "A Mod", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layerA.envPitch.morph(p.val.pct());
    });

    // --- Layer B ---
    Val& layerBPitch = val(0, "LAYER_B_PITCH", { .label = "B Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24, .incType = INC_ONE_BY_ONE });
    Val& layerBDecay = val(0.0f, "LAYER_B_DECAY", { .label = "B Decay", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layerB.envAmp.morph(p.val.pct());
    });
    Val& layerBWaveform = val(250.0f, "LAYER_B_WAVEFORM", { .label = "B Wave", .type = VALUE_STRING, .max = WAVEFORMS_COUNT * 100 - 1 }, [&](auto p) {
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
    Val& layerBMod = val(0.0f, "LAYER_B_MOD", { .label = "B Mod", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        layerB.envPitch.morph(p.val.pct());
    });

    // --- constructor ---
    VolcEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Volc")
        , layerA(props.lookupTable, props.sampleRate)
        , layerB(props.lookupTable, props.sampleRate)
    {
        initValues();
    }

    FX_BUFFER
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
            fxOut = applyDrive(fxOut, -fxAmount * 2.0f); // stronger drive as it goes negative
        else if (fxAmount > 0.0f)
            fxOut = applyReverb(fxOut, fxAmount, buffer, reverbIndex);

        buf[track] = fxOut * velocity;
    }

    void sampleOff(float* buf) override
    {
        if (fxAmount > 0.0f) {
            buf[track] = applyReverb(buf[track], fxAmount, buffer, reverbIndex);
        }
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        layerA.sampleIndex = 0.0f;
        layerB.sampleIndex = 0.0f;

        // use 220 instead 440 to have a more percussive sound
        layerA.baseFreq = 220.0f * powf(2.0f, (note - baseNote + layerAPitch.get()) / 12.0f);
        layerB.baseFreq = 220.0f * powf(2.0f, (note - baseNote + layerBPitch.get()) / 12.0f);
    }
};
