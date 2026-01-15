#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "audio/MultiFx.h"
#include "helpers/math.h"

class ScannerEngine : public LoopedEngine {
protected:
    MultiFx multiFx;
    
    float scanPos = 0.0f;
    float phaseA = 0.0f;
    float phaseB = 0.5f; 

public:
    // We use the existing 'start' and 'end' from LoopedEngine to define our range
    // We'll add 'Scrub' to move within that range
    Val& scrub = val(0.0f, "SCRUB", { "Scrub", .unit = "%" });

    Val& scanSpeed = val(0.0f, "SPEED", { "Speed", VALUE_CENTERED, .min = -200.0f, .max = 200.0f, .unit = "%" });

    Val& winSize = val(500.0f, "SIZE", { "Size", .min = 2.0f, .max = 500.0f, .unit = "ms" });

    Val& stretch = val(0.0f, "STRETCH", { "Stretch", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { "FX Type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX Edit", .unit = "%" });

    ScannerEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier, Val* browser)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "Scanner", browser)
        , multiFx(props.sampleRate, props.lookupTable)
    {
        // Hide LoopedEngine params that don't make sense for a Scanner
        // Or repurpose them. Here we just ensure they exist.
    }

    void engineNoteOn(uint8_t note, float velocity) override {
        // Start the scan at the 'scrub' position relative to the 'start' parameter
        float range = (float)(indexEnd - indexStart);
        scanPos = (float)indexStart + (scrub.pct() * range);
        
        phaseA = 0.0f;
        phaseB = 0.5f; 
    }

    // // This is the core fix: stop LoopedEngine from looping its own 'index'
    // void postIncrement() override {
    //     // Do nothing. We don't want the standard 'index' to jump back to 'loopStart'.
    //     // We let 'index' just run from indexStart to indexEnd like a standard envelope.
    // }

    float getSample(float stepInc) override {
        if (sampleBuffer.count == 0) return 0.0f;

        // 1. Movement Logic constrained by Start/End points
        scanPos += (scanSpeed.get() * 0.01f);
        
        // Bounce or Wrap scanPos within the LoopedEngine's boundaries
        if (scanPos > (float)indexEnd) scanPos = (float)indexStart;
        if (scanPos < (float)indexStart) scanPos = (float)indexEnd;

        // 2. Phase Processing
        float winSamples = winSize.get() * 0.001f * props.sampleRate;
        float phaseInc = stepInc / winSamples;

        phaseA += phaseInc;
        if (phaseA > 1.0f) phaseA -= 1.0f;
        phaseB += phaseInc;
        if (phaseB > 1.0f) phaseB -= 1.0f;

        // 3. Granular Read
        auto readSample = [&](float ph) {
            float phSpread = ph * (1.0f + stretch.pct() * 5.0f);
            float idx = scanPos + (phSpread * winSamples);
            
            // Wrap index globally within the buffer
            while (idx >= sampleBuffer.count) idx -= sampleBuffer.count;
            while (idx < 0) idx += sampleBuffer.count;
            
            float window = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * ph));
            return sampleBuffer.data[(int)idx] * window;
        };

        return readSample(phaseA) + readSample(phaseB);
    }

    void postProcess(float* buf) override {
        // The base class calls this. Apply FX here.
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }
};