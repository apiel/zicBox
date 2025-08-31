#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

#include <cmath>
#include <cstdlib>

class StretchEngine : public SampleEngine {
protected:
    MultiFx multiFx;
    MMfilter filter;

    // Parameters
    Val& stretch = val(1.0f, "STRETCH", { "Stretch", .min = 1.0f, .max = 8.0f, .step = 0.1f, .floatingPoint = 2 });
    Val& grainSizeMs = val(100.0f, "GRAIN_SIZE", { "Grain Size", .min = 10.0f, .max = 500.0f, .unit = "ms" });
    Val& overlap = val(50.0f, "OVERLAP", { "Overlap", .unit = "%" });
    Val& pitchOffset = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24.0f, .max = 24.0f });
    Val& jitter = val(0.0f, "JITTER", { "Jitter", .unit = "%" });
    Val& stereoSpread = val(0.0f, "STEREO", { "Stereo Spread", .unit = "%" });
    Val& envShape = val(0.5f, "ENV_SHAPE", { "Envelope Shape", .min = 0.0f, .max = 1.0f });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 },
        [&](auto p) { multiFx.setFxType(p); });
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX amount", .unit = "%" });

    // State
    float sampleRate;
    float playhead = 0.0f;
    float grainPhase = 0.0f;
    float grainSizeSamples = 0.0f;
    float pitchRatio = 1.0f;
    bool activeNote = false;
    float noteVelocity = 1.0f;

public:
    StretchEngine(AudioPlugin::Props& props, AudioPlugin::Config& config,
        SampleBuffer& sampleBuffer, float& index, float& stepMultiplier)
        : SampleEngine(props, config, sampleBuffer, index, stepMultiplier, "Stretch")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        sampleRate = props.sampleRate;
    }

    void opened() override
    {
        playhead = 0.0f;
        grainPhase = 0.0f;
        activeNote = false;
    }

    void noteOn(uint8_t note, float velocity, void* userdata = nullptr) override
    {
        activeNote = true;
        noteVelocity = velocity;
        playhead = 0.0f;
        grainPhase = 0.0f;

        float semitones = pitchOffset.get();
        pitchRatio = powf(2.0f, semitones / 12.0f); // semitone shift applied to playhead increment
    }

    void noteOff(uint8_t note, float velocity, void* userdata = nullptr) override
    {
        activeNote = false;
    }
    void sample(float* buf) override
    {
        if (!activeNote || sampleBuffer.count == 0 || playhead >= sampleBuffer.count) {
            buf[track] = 0.0f;
            return;
        }

        // Grain envelope
        grainSizeSamples = (grainSizeMs.get() / 1000.0f) * sampleRate;
        float grainPos = fmodf(grainPhase, grainSizeSamples);
        float env = 0.5f * (1.0f - cosf(2.0f * M_PI * grainPos / grainSizeSamples));
        env = powf(env, envShape.get() * 4.0f + 0.1f);

        // Pure time-stretch read position
        float readPos = playhead;
        if (readPos >= sampleBuffer.count)
            readPos = sampleBuffer.count - 1;

        // Linear interpolation
        uint64_t i0 = (uint64_t)readPos;
        uint64_t i1 = (i0 + 1 < sampleBuffer.count) ? i0 + 1 : i0;
        float frac = readPos - (float)i0;
        float sampleVal = sampleBuffer.data[i0] * (1.0f - frac) + sampleBuffer.data[i1] * frac;

        // Envelope & velocity
        float out = sampleVal * env * noteVelocity;

        // Advance grain phase for envelope only
        grainPhase += 1.0f;
        if (grainPhase >= grainSizeSamples)
            grainPhase = 0.0f;

        // Advance playhead according to stretch
        playhead += 1.0f / stretch.get(); // <-- slow down/speed up
        if (playhead >= sampleBuffer.count) {
            activeNote = false; // stop playback at end
        }

        // Optional jitter
        if (jitter.get() > 0.0f) {
            float j = ((rand() / (float)RAND_MAX) - 0.5f) * jitter.get() * grainSizeSamples * 0.01f;
            playhead += j;
        }

        // Filter + FX
        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());

        buf[track] = out;
    }
};
