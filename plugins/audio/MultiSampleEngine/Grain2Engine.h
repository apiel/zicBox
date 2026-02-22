/** Description:
This document describes the design of an advanced audio component called the **Grain Engine**. Its primary function is to perform *granular synthesis*, a sophisticated technique that breaks down a source audio sample into hundreds of tiny, overlapping segments—or "grains"—to create dense, evolving sound textures.

The engine is built on top of a foundational looping structure, allowing it to continuously read and manipulate an underlying sound buffer.

**How It Works:**
The core of the system relies on two main internal modules: the `Grains` module for sound generation and the `MultiFx` module for post-processing effects. When the engine is asked to produce sound, the `Grains` module calculates the audio by selecting, shaping, and combining these tiny audio snippets based on the current settings. This generated audio is then passed to the `MultiFx` module, which applies user-defined effects like delays or modulation before the sound is outputted.

**Key User Controls:**
The engine exposes several critical parameters that control the texture and behavior of the sound:
1. **Grain Length:** Adjusts the duration of the individual audio snippets (measured in milliseconds).
2. **Density/Delay:** Controls how frequently new grains are started, affecting the thickness of the sound.
3. **Detune:** Shifts the pitch of the grains relative to the original audio source.
4. **Randomization:** Introduces intentional unpredictability in grain timing or pitch for complex, unstable textures.
5. **Effects:** Selects and controls the strength of the built-in effects processor.

This modular architecture allows the engine to create everything from subtle textural overlays to dramatic, sweeping soundscapes based on the provided sample audio.

sha: 199a96abaca3f3b3d9180b08a1422fa1f110f87595402b817e8313b79d29b6f2
*/
#pragma once

#include "audio/EnvelopRelative.h"
#include "audio/Grains.h"
#include "plugins/audio/utils/valMultiFx.h"
#include "plugins/audio/MultiSampleEngine/LoopedEngine.h"
#include "plugins/audio/mapping.h"

class Grain2Engine : public LoopedEngine {
protected:
    MultiFx multiFx;
    Grains grains;

    float envSteps = 0.00001f;

    float getDataSample(uint64_t idx)
    {
        return sampleBuffer.data[idx];
    }

public:
    Val& length = val(100.0f, "GRAIN_LENGTH", { "Grain Length", .min = 5.0, .max = 500.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setGrainDuration(props.sampleRate * length.get() * 0.001f);
    });

    Val& densityDelay = val(100.0f, "GRAIN_DELAY", { "Grain Delay", .min = 1.0, .max = 1000, .unit = "ms" }, [&](auto p) {
        densityDelay.setFloat(p.value);
        grains.setGrainDelay(props.sampleRate * p.val.get() * 0.001f);
    });

    Val& density = val(4.0f, "DENSITY", { "Density", .min = 1.0, .max = MAX_GRAINS }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setDensity(p.val.get());
    });

    Val& pitchRand = val(0.0f, "RANDOMIZE_PITCH", { "Pitch Rand", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setPitchRandomize(p.val.pct());
    });

    Val& delayRand = val(0.0f, "RANDOMIZE_DELAY", { "Delay Rand", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setDelayRandomize(p.val.pct());
    });

    Val& page = val(1, "PAGE", { "Page", .min = 1, .max = 2, .unit = "/2" }, [&](auto p) {
        float current = p.val.get();
        p.val.setFloat(p.value);

        if (current != p.val.get()) {
            std::swap(mapping[8], mapping[12]);
            std::swap(mapping[9], mapping[13]);
            std::swap(mapping[10], mapping[14]);
            needCopyValues = true;
        }
    });

    Val& detune = val(0.0f, "DETUNE", { "Detune", VALUE_CENTERED, .min = -12.0f, .max = 12.0f, .step = 0.1f, .unit = "st" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setDetune(p.val.get());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx));
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Grain2Engine(AudioPlugin::Props& props, AudioPlugin::Config& config, SampleBuffer& sampleBuffer, float& index, float& stepMultiplier, Val* browser, float* fxBuffer)
        : LoopedEngine(props, config, sampleBuffer, index, stepMultiplier, "Grain", browser)
        , multiFx(props.sampleRate, fxBuffer)
        , grains(props.lookupTable, [this](uint64_t idx) -> float { return getDataSample(idx); })
    {
    }

    void postProcess(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    float getSample(float stepIncrement) override
    {
        return grains.getGrainSample(stepIncrement, index, sampleBuffer.count);
    }

    // Actually one or the other should be enough...
    uint16_t getLoopCountRelease() override { return 0; }
    void postIncrement() override
    {
        if (index >= loopEnd && sustainedNote) {
            index = loopStart;
        }
    }
};
