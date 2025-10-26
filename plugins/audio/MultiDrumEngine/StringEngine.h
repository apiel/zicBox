#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

#include <cmath>
#include <vector>

class StringDrumEngine : public DrumEngine {
protected:
    MMfilter filter;
    MultiFx multiFx;

    std::vector<float> delayLine;
    uint32_t delayLen = 0;
    uint32_t writePos = 0;
    float onePoleState = 0.0f;

    float velocity = 1.0f;
    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536

    inline float noteToFreq(int note) const
    {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

    float resonatorTick()
    {
        if (delayLen == 0)
            return 0.0f;

        uint32_t rp = writePos % delayLen;
        uint32_t rp1 = (rp + 1) % delayLen;
        float s0 = delayLine[rp];
        float s1 = delayLine[rp1];
        float out = 0.5f * (s0 + s1);

        // one-pole lowpass
        float cutoff = std::max(0.001f, tone.pct() * (1.05f - damping.pct()));
        onePoleState += cutoff * (out - onePoleState);
        float filtered = onePoleState;

        // feedback
        float fb = decay.get();
        delayLine[writePos % delayLen] = filtered * fb;

        writePos = (writePos + 1) % delayLen;
        return filtered;
    }

public:
    // Parameters
    Val& pitch = val(0.0f, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24 });
    Val& decay = val(0.98f, "DECAY", { .label = "Decay", .min = 0.80f, .max = 0.99f, .step = 0.01f, .floatingPoint = 2 });
    Val& tone = val(50.0f, "TONE", { .label = "Tone", .unit = "%" });
    Val& pluckNoise = val(50.0f, "PLUCK_NOISE", { .label = "Pluck Noise", .unit = "%" });

    Val& exciteType = val(0.0f, "EXCITE_TYPE", { .label = "Excitation Type", .min = 0.0f, .max = 2.0f });
    Val& damping = val(0.5f, "DAMPING", { .label = "Damping", .unit = "%" });

    Val& cutoff = val(0.0, "CUTOFF", { .label = "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });
    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    // Constructor
    StringDrumEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "String")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        delayLen = (uint32_t)std::min<uint64_t>((uint64_t)(props.sampleRate * 0.02f), (uint64_t)MAX_DELAY);
        delayLine.assign(delayLen + 4, 0.0f);
        initValues();
    }

    void sampleOn(float* buf, float envAmpVal, int, int) override
    {
        float out = resonatorTick() * velocity * envAmpVal;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        float freq = noteToFreq(note + (int)pitch.get() - 24); // Let's remove 2 octaves
        if (freq < 20.0f)
            freq = 20.0f;
        if (freq > props.sampleRate * 0.45f)
            freq = props.sampleRate * 0.45f;

        delayLen = std::min<uint32_t>(MAX_DELAY, std::max<uint32_t>(2, (uint32_t)std::round(props.sampleRate / freq)));
        delayLine.assign(delayLen + 4, 0.0f);

        if (exciteType.get() == 0.0f) {
            // White noise
            for (uint32_t i = 0; i < delayLen; ++i) {
                float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
                delayLine[i] = filter.process(n) * velocity * pluckNoise.pct();
            }
        } else if (exciteType.get() == 1.0f) {
            // Square burst
            for (uint32_t i = 0; i < delayLen; ++i) {
                float n = sinf(2.0f * M_PI * i / delayLen) > 0.5f ? 1.0f : 0.0f;
                delayLine[i] = filter.process(n) * velocity * pluckNoise.pct();
            }
        } else {
            // Sine burst
            for (uint32_t i = 0; i < delayLen; ++i) {
                float n = sinf(2.0f * M_PI * i / delayLen);
                delayLine[i] = filter.process(n) * velocity * pluckNoise.pct();
            }
        }

        writePos = 0;
        onePoleState = 0.0f;
    }
};
