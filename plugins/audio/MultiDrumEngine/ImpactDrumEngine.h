#pragma once

#include "audio/engines/DrumImpact.h"
#include "plugins/audio/MultiEngine.h"

class ImpactDrumEngine : public MultiEngine {
protected:
    DrumImpact impact;

    // --- Core Parameters ---
    Val& duration = val("DURATION", impact.duration);

    // Volume Envelope Morphing Visualizer
    GraphPointFn ampGraph = [&](float index) { return *impact.envelopAmp.getMorphShape(index); };
    Val& ampMorph = val("AMP_MORPH", impact.params[1], ampGraph);

    // --- Tonal & Sweep ---
    Val& baseFreq = val("BASE_FREQ", impact.params[2]);
    Val& sweepDepth = val("SWEEP_DEPTH", impact.sweepDepth);

    // Sweep Shape Visualizer (Exponential Decay)
    GraphPointFn sweepGraph = [&](float t) {
        float decayMod = 0.002f + (impact.sweepShape.value * 0.01f * 0.08f);
        return Math::exp(-t / (0.1f + decayMod * 10.0f));
    };
    Val& sweepShape = val("SWEEP_SHAPE", impact.sweepShape, sweepGraph);
    Val& vcoMorph = val("VCO_MORPH", impact.vcoMorph);
    Val& fmAmt = val("FM", impact.fmTexture);
    Val& clickFreq = val("CLICK_FREQ", impact.clickFreq);
    Val& clickAmt = val("CLICK_AMT", impact.clickAmt);
    Val& drive = val("DRIVE", impact.drive);

    Val& fxType = val("FX_TYPE", impact.fxType);
    Val& fxAmount = val("FX_AMOUNT", impact.fxAmount);

public:
    ImpactDrumEngine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer)
        : MultiEngine(p, c, "Impact")
        , impact(p.sampleRate, fxBuffer)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        impact.noteOn(note, _velocity);
    }

    void sample(float* buf) override
    {
        // Impact is a single-mono output engine, assigned to the track's buffer slot
        buf[track] = impact.sample();
    }
};