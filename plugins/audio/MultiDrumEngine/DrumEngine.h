/** Description:
This blueprint defines the core engine for generating digital percussion sounds within an audio plugin environment. It acts as a sophisticated timing and volume manager for drum hits.

The engine is designed to handle fundamental aspects of sound control, presenting them as adjustable user parameters:
1. **Duration:** Defines the total length of the drum sound, typically set in milliseconds.
2. **Amplitude Envelope (Amp. Env.):** This component controls the volume shape—how quickly the sound fades from its initial impact to silence.
3. **Morphing:** Allows the user to smoothly blend between different preset volume decay patterns, offering extensive customization of the sound’s decay profile.

When a signal to start the sound ("Note On") is received, the engine calculates the precise number of internal steps required to match the set Duration. During playback, it cycles through these steps, asking the Envelope component for the exact volume level at that moment. This calculated volume level is then applied to the sound being generated.

Crucially, while this engine manages the *timing* and *loudness* of the drum hit, it relies on external, specialized components (other modules inheriting this structure) to define the actual *timbre* or tone of the drum itself, ensuring a highly modular and reusable system for sound design.

sha: 0636e7c0f12f7a68f27a4c4e89cc45a10feef5fa6706714a0cbc074ccf2417d7 
*/
#pragma once

#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"
#include "audio/EnvelopDrumAmp.h"
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
