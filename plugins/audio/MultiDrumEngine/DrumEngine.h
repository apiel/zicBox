#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"
#include "plugins/audio/utils/EnvelopDrumAmp.h"
#include "plugins/audio/MultiEngine.h"

typedef std::function<void(std::string, float)> SetValFn;

class DrumEngine : public MultiEngine {
public:
    EnvelopDrumAmp envelopAmp;

    Val& duration = val(500.0f, "DURATION", { .label = "Duration", .min = 50.0, .max = 3000.0, .step = 10.0, .unit = "ms" });

    GraphPointFn ampGraph = [&](float index) { return *envelopAmp.getMorphShape(index); };
    Val& ampMorph = val(0.0f, "AMP_MORPH", { .label = "Amp. Env.", .unit = "%", .graph = ampGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        envelopAmp.morph(p.val.pct());
    });

    DrumEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : MultiEngine(props, config, name)
    {
    }

    int totalSamples = 0;
    int i = 0;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float envAmp = envelopAmp.next();
            sampleOn(buf, envAmp, i, totalSamples);
            i++;
        } else {
            sampleOff(buf);
        }
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        const float sampleRate = props.sampleRate;
        totalSamples = static_cast<int>(sampleRate * (duration.get() / 1000.0f));
        envelopAmp.reset(totalSamples);
        i = 0;
    }

    virtual void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) = 0;
    virtual void sampleOff(float* buf) { }
};
