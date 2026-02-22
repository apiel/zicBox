#pragma once

#include "audio/engines/DrumEdge.h"
#include "plugins/audio/MultiEngine.h"

class EdgeEngine : public MultiEngine {
protected:
    DrumEdge edge;

    // --- Global & Amp ---
    Val& duration = val("DURATION", edge.duration);
    GraphPointFn ampGraph = [&](float i) { return *edge.envelopAmp.getMorphShape(i); };
    Val& ampMorph = val("AMP_MORPH", edge.ampEnv, ampGraph);

    // --- VCO 1 ---
    Val& vco1Freq = val("VCO1_FREQ", edge.vco1Freq);
    Val& vco1Shape = val("VCO1_WAVE", edge.vco1Shape); // 0 = Triangle, 1 = Square
    Val& vco1Env = val("VCO1_ENV", edge.vco1EnvAmt);

    // --- VCO 2 ---
    Val& vco2Freq = val("VCO2_FREQ", edge.vco2Freq);
    Val& vco2Shape = val("VCO2_WAVE", edge.vco2Shape);
    Val& vco2Env = val("VCO2_ENV", edge.vco2EnvAmt);
    
    // --- FM & Mix ---
    Val& fmAmount = val("FM_AMT", edge.fmAmount);
    Val& vcoMix = val("VCO_MIX", edge.vcoMix); // Mix between VCO1 and VCO2

    // --- Global Filter/Drive ---
    // Val& drive = val("DRIVE", edge.drive);
    // Val& tone = val("TONE", edge.tone);

    Val& fxType = val("FX_TYPE", edge.fxType);
    Val& fxAmount = val("FX_AMOUNT", edge.fxAmount);

public:
    EdgeEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : MultiEngine(p, c, "Edge")
        , edge(p.sampleRate)
    {
        initValues();
    }

    void noteOn(uint8_t note, float velocity, void* = nullptr) override {
        edge.noteOn(note, velocity);
    }

    void sample(float* buf) override {
        buf[track] = edge.sample();
    }
};