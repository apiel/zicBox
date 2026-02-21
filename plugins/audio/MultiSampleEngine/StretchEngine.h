/** Description:
This C++ header defines the `StretchEngine`, a specialized audio playback and processing module designed to work within a larger audio plugin environment.

### Core Function and Mechanism

The primary purpose of the `StretchEngine` is to perform **time-stretching** on audio samples without changing the fundamental pitch. It achieves this by using a granular looping technique:

1.  **Granular Looping:** The engine breaks the loaded audio sample into tiny segments, or "grains," defined by the **Grain Size** control.
2.  **Repetition:** The **Stretch** parameter dictates how many times each individual grain is replayed before the engine progresses to the next segment of the audio. By looping these small sections multiple times, the playback time is extended, achieving the desired stretching effect.

### Audio Signal Path

After the audio is retrieved and stretched, it passes through a structured modification pipeline:

1.  **Filtering:** The sound first enters a built-in digital filter (`MMfilter`). Users can adjust the tone using the **Cutoff** and **Resonance** parameters.
2.  **Effects:** The filtered signal is then processed by a comprehensive effects unit (`MultiFx`). Users select the specific effect using **FX Type** and control the intensity with **FX Amount**.

The entire engine provides a set of dynamically adjustable controls (like Stretch, Grain Size, Cutoff, and FX settings) that allow for real-time manipulation of the processed audio.

sha: d160c5bdea190800ac6cc4998f18b92050fe8736a023337e764a89b730e444e8 
*/
#pragma once

#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/utils/valMultiFx.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"

class StretchEngine : public LoopedEngine {
protected:
    MMfilter filter;
    MultiFx multiFx;

    Val& stretch = val(1.0f, "STRETCH", { "Stretch", .min = 1.0f, .max = 20.0f, .incType = INC_ONE_BY_ONE });
    Val& grainSize = val(100.0f, "GRAIN_SIZE", { "Grain Size", .min = 1.0f, .max = 5000.0f, .incType = INC_SCALED });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, valMMfilterCutoff(filter));
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx));
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX amount", .unit = "%" });

public:
    StretchEngine(AudioPlugin::Props& props, AudioPlugin::Config& config,
        SampleBuffer& sampleBuffer, float& index, float& stepMultiplier, Val* browser)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "Stretch", browser)
        , multiFx(props.sampleRate)
    {
    }

    int stretchLoopCount = 0;
    float grainStart = 0.0f;
    void postIncrement() override
    {
        if (index >= grainStart + grainSize.get()) {
            if (stretchLoopCount < stretch.get() - 1) {
                index = grainStart;
                stretchLoopCount++;
            } else {
                grainStart = index;
                stretchLoopCount = 0;
            }
        }

        if (index >= loopEnd && (sustainedNote || nbOfLoopBeforeRelease > 0)) {
            index = loopStart;
            grainStart = loopStart;
            nbOfLoopBeforeRelease--;
        }
    }

    void postProcess(float* buf) override
    {
        float out = buf[track];
        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void engineNoteOn(uint8_t note, float _velocity) override
    {
        grainStart = indexStart;
        stretchLoopCount = 0;
    }
};
