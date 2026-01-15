#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/Wavetable.h"
#include "audio/KickEnvTableGenerator.h"
#include "audio/MMfilter.h"
#include "audio/MultiFx.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

class KickWaveEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    MMfilter filter;
    MultiFx multiFx;
    MultiFx multiFx2;

    Wavetable wavetable;
    KickEnvTableGenerator kickEnv;

public:
    // --- Pitch & Envelope Logic ---
    GraphPointFn frequencyGraph = [&](float index) { return *kickEnv.sample(&index); };
    Val& frequencyModulation = val(10.0f, "ENVELOPE_SHAPE", { .label = "Freq. mod.", .type = VALUE_BASIC, .step = 0.05f, .floatingPoint = 2, .unit = "%", .graph = frequencyGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        kickEnv.setMorph(p.val.pct());
    });

    Val& pitchOffset = val(0, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24, .incType = INC_ONE_BY_ONE });

    // --- Wavetable Logic (Replacing Waveform and Transient) ---
    Val& wave = val(0, "WAVE", { .label = "Wave", VALUE_STRING }, [&](auto p) {
        p.val.setFloat(p.value);
        int position = (int)p.val.get();
        wavetable.open(position, false);
        p.val.setString(wavetable.fileBrowser.getFileWithoutExtension(position));
        // Reset morph when changing files
        setVal("WAVE_EDIT", 1.0f); 
    });

    GraphPointFn graphWave = [&](auto index) { return *wavetable.sample(&index); };
    Val& waveEdit = val(1.0f, "WAVE_EDIT", { .label = "Wave Morph", .type = VALUE_STRING, .min = 1.0, .max = ZIC_WAVETABLE_WAVEFORMS_COUNT, .graph = graphWave }, [&](auto p) {
        p.val.setFloat(p.value);
        wavetable.morph((int)p.val.get() - 1);
        p.val.setString(std::to_string((int)p.val.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
    });

    // --- Shaping & FX ---
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

    KickWaveEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "KickWave")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
    {
        initValues();
    }

    float baseFrequency = 1.0f;

    void sampleOn(float* buffer, float envelopeAmplitude, int sampleCounter, int totalSamples) override
    {
        float normalizedTime = (float)sampleCounter / totalSamples;
        
        // 1. Frequency Modulation from the KickEnvTable
        float envelopeFrequency = kickEnv.next(normalizedTime);
        float modulatedFrequency = baseFrequency + envelopeFrequency;

        // 2. Generate Wavetable Sample
        // Note: We use the &wavetable.sampleIndex to let the wavetable handle its own phase tracking
        float output = wavetable.sample(&wavetable.sampleIndex, modulatedFrequency) * envelopeAmplitude;

        // 3. Filters and FX
        output = filter.process(output);
        output = multiFx.apply(output, fxAmount.pct());
        output = multiFx2.apply(output, fx2Amount.pct());

        buffer[track] = output * velocity;
    }

    void sampleOff(float* buffer) override
    {
        float output = buffer[track];
        output = multiFx.apply(output, fxAmount.pct());
        buffer[track] = output;
    }

    uint8_t baseNote = 60 + 12; // Adjusted for kick range
    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        
        wavetable.sampleIndex = 0.0f;
        // Calculate the base frequency based on the MIDI note and pitch offset
        baseFrequency = powf(2.0f, ((float)note - (float)baseNote + pitchOffset.get()) / 12.0f);
    }
};