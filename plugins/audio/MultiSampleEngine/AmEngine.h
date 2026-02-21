/** Description:
This header file defines the `AmEngine`, a specialized audio synthesis module within a larger sound plugin framework. It is fundamentally an engine designed to create and shape sound, building upon existing functionality for playing and looping audio samples.

**Core Functionality: Amplitude Modulation (AM)**

The unique purpose of the `AmEngine` is to generate sound using Amplitude Modulation (AM). AM works by rapidly multiplying the volume (amplitude) of the primary sound by a second, internally generated wave (like a sine wave). This constant, rhythmic fluctuation in volume creates complex and often metallic or bell-like textures, resulting in a sound much richer than the original sample alone. The speed and intensity of this modulation are determined by the fundamental note played and controlled by parameters like "Ratio" and "Depth."

**Audio Shaping Components**

To refine the modulated sound, the engine incorporates two key processing stages:

1.  **Filtering (`MMfilter`):** This component acts like a sophisticated tone control, allowing the sound's frequency balance to be shaped. The "Cutoff" setting determines which parts of the sound spectrum (highs or lows) are emphasized, while "Resonance" controls the intensity of the filter’s effect.
2.  **Multi-Effects (`MultiFx`):** After the initial sound generation and filtering, a multi-effects unit applies additional sound enhancements, such as delay, reverb, or distortion, controlled by the "FX Type" and "FX Amount" parameters.

**User Control**

All aspects of the engine are exposed via customizable parameters (like Depth, Pitch Offset, Resonance, and Ratio). This allows the user to precisely sculpt the modulation intensity, tune the frequency steps, and adjust the post-processing effects. When a note is played, the engine immediately calculates the necessary internal speeds for the modulation wave, ensuring the AM effect is correctly synchronized to the musical pitch.

sha: 9ede694fb01a1baee5f8100f4ea4cacbbf170496f39b60e784e884dfd7419b94 
*/
#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/utils/valMultiFx.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

#include <cmath>

class AmEngine : public LoopedEngine {
protected:
    MultiFx multiFx;
    MMfilter filter;

    uint8_t playedNote = 0;

    // Parameters
    Val& depth = val(50.0f, "DEPTH", { "Depth", .unit = "%" });
    Val& pitchOffset = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24.0f, .max = 24.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        setModStep();
    });
    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, valMMfilterCutoff(filter));
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx));
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Val& ratio = val(1.0f, "RATIO", { "Ratio", .min = 0.25f, .max = 8.0f, .step = 0.25f }, [&](auto p) {
        p.val.setFloat(p.value);
        setModStep();
    });

    float modPhase = 0.0f;

    // Helper: simple sine wave increment
    float modStep = 0.0f;

    inline float midiToFreq(int note) const
    {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

    void setModStep()
    {
        float freq = midiToFreq(playedNote + pitchOffset.get());
        modStep = 2.0f * (float)M_PI * freq * ratio.get() / props.sampleRate;
    }

public:
    AmEngine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier, Val* browser)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "AM", browser)
        , multiFx(props.sampleRate)
    {
    }

    void engineNoteOn(uint8_t note, float velocity) override
    {
        playedNote = note;
        setModStep();
        modPhase = 0.0f;
    }

    float getSample(float stepIncrement) override
    {
        if (sampleBuffer.count == 0)
            return 0.0f;

        float modValue = sinf(modPhase);
        float depthFactor = depth.pct();

        modPhase += modStep;
        if (modPhase > 2.0f * (float)M_PI)
            modPhase -= 2.0f * (float)M_PI;

        return sampleBuffer.data[(int)index] * (1.0f + modValue * depthFactor) * 0.5f;
    }

    void postProcess(float* buf) override
    {
        float out = buf[track];

        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out;
    }
};
