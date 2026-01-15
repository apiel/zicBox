#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "audio/MultiFx.h"
#include "helpers/math.h"

class ScannerEngine : public LoopedEngine {
protected:
    MultiFx multiFx;
    
    float phaseA = 0.0f;
    float phaseB = 0.5f; 
    float scanOffset = 0.0f;

public:
    // Scrub: Moves the playhead relative to the current loop position
    Val& scrub = val(0.0f, "SCRUB", { "Scrub", .unit = "%" });

    // Speed: 100% is normal, 0% is frozen. 
    // We achieve this by subtracting movement from the master index.
    Val& scanSpeed = val(0.0f, "SPEED", { "Speed", VALUE_CENTERED, .min = -200.0f, .max = 200.0f, .unit = "%" });

    Val& winSize = val(500.0f, "SIZE", { "Size", .min = 2.0f, .max = 500.0f, .unit = "ms" });
    Val& stretch = val(0.0f, "STRETCH", { "Stretch", .unit = "%" });

    Val& fxType = val(0, "FX_TYPE", { "FX Type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX Edit", .unit = "%" });

    ScannerEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier, Val* browser)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "Scanner", browser)
        , multiFx(props.sampleRate, props.lookupTable)
    {
    }

    void engineNoteOn(uint8_t note, float velocity) override {
        phaseA = 0.0f;
        phaseB = 0.5f; 
        scanOffset = 0.0f;
    }

    float getSample(float stepInc) override {
        if (sampleBuffer.count == 0) return 0.0f;

        // 1. Calculate how much we want to 'resist' the master index movement
        // speed 100% -> offsetInc = 0 (plays at master speed)
        // speed 0%   -> offsetInc = -stepInc (freezes the playhead)
        float speedPct = scanSpeed.get() * 0.01f;
        float offsetInc = stepInc * (speedPct - 1.0f);
        scanOffset += offsetInc;

        // 2. Grain Phase Logic
        float winSamples = winSize.get() * 0.001f * props.sampleRate;
        float phaseInc = stepInc / winSamples;

        phaseA += phaseInc;
        if (phaseA > 1.0f) phaseA -= 1.0f;
        phaseB += phaseInc;
        if (phaseB > 1.0f) phaseB -= 1.0f;

        // 3. Anchor Point
        // We take the LoopedEngine's index (which handles all loops/release) 
        // and apply our Scrub + Speed Offset.
        float range = (float)(indexEnd - indexStart);
        float anchor = index + scanOffset + (scrub.pct() * range);

        auto readSample = [&](float ph) {
            float phSpread = ph * (1.0f + stretch.pct() * 5.0f);
            float idx = anchor + (phSpread * winSamples);
            
            // Wrap safely within the actual buffer
            while (idx >= sampleBuffer.count) idx -= sampleBuffer.count;
            while (idx < 0) idx += sampleBuffer.count;
            
            float window = 0.5f * (1.0f - cosf(2.0f * (float)M_PI * ph));
            return sampleBuffer.data[(int)idx] * window;
        };

        return readSample(phaseA) + readSample(phaseB);
    }

    void postProcess(float* buf) override {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }
};