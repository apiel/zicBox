#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"
#include "plugins/audio/utils/MultiFx.h"
#include <cmath>

class FmEngine : public SampleEngine {
protected:
    MultiFx multiFx;
    float freq = 0.0f;

    Val& ratio = val(1.0f, "RATIO", { "Ratio", .min = 0.25f, .max = 8.0f, .step = 0.25f, .floatingPoint = 2 }, [&](auto p) { 
        p.val.setFloat(p.value); 
        modStep = 2.0f * (float)M_PI * freq * p.val.get() / sampleRate;
    });
    Val& depth = val(50.0f, "DEPTH", { "Depth", .unit = "%" });
    Val& pitchOffset = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24.0f, .max = 24.0f });
    Val& feedback = val(0.0f, "FEEDBACK", { "Feedback", .unit = "%" });
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) { multiFx.setFxType(p); });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    float sampleRate;

    float modPhase = 0.0f;
    float lastOutput = 0.0f;

    // Helper: simple sine wave increment
    float modStep = 0.0f;

    inline float midiToFreq(int note) const
    {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

public:
    FmEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer)
        : SampleEngine(props, config, sampleBuffer, "FM")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        sampleRate = props.sampleRate;
    }

    void noteOn(uint8_t note, float velocity, void*) override
    {
        (void)velocity;
        freq = midiToFreq(note + pitchOffset.get());
        modStep = 2.0f * (float)M_PI * freq * ratio.get() / sampleRate;
        modPhase = 0.0f;
        lastOutput = 0.0f;
    }

    float getSample(int index, float stepIncrement) override
    {
        float modValue = sinf(modPhase + lastOutput * feedback.pct() * 0.5f);
        float depthFactor = depth.pct() * 0.01f;

        float offset = modValue * depthFactor * stepIncrement;
        float idx = index + offset;
        uint64_t i0 = (uint64_t)idx;
        uint64_t i1 = (i0 + 1 < sampleBuffer.count) ? i0 + 1 : i0;
        float frac = idx - (float)i0;
        float s0 = sampleBuffer.data[i0];
        float s1 = sampleBuffer.data[i1];
        float out = s0 + (s1 - s0) * frac;

        lastOutput = out;

        modPhase += modStep;
        if (modPhase > 2.0f * (float)M_PI)
            modPhase -= 2.0f * (float)M_PI;

        return out;
    }

    void sample(float* buf, int index) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }
};
