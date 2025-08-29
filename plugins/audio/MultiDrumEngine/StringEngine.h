#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "plugins/audio/utils/EnvelopDrumAmp.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/WavetableGenerator2.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

#include <cmath>
#include <vector>

class StringEngine : public DrumEngine {
protected:
    MMfilter filter;
    MultiFx multiFx;

    // Resonator
    std::vector<float> delayLine;
    uint32_t delayLen = 0;
    uint32_t writePos = 0;
    uint32_t readPos = 0;
    float onePoleState = 0.0f;
    bool resonatorActive = false;

    float velocity = 1.0f;

    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536

    inline float noteToFreq(int note) const
    {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

    uint32_t ticksPlayed = 0;
    float resonatorTick()
    {
        if (!resonatorActive || delayLen == 0)
            return 0.0f;

        uint32_t rp = readPos % delayLen;
        uint32_t rp1 = (rp + 1) % delayLen;
        float s0 = delayLine[rp];
        float s1 = delayLine[rp1];
        float out = 0.5f * (s0 + s1);

        float cutoff = std::max(0.001f, tone.pct());
        onePoleState += cutoff * (out - onePoleState);
        float filtered = onePoleState;

        float fb = decay.get();
        delayLine[writePos % delayLen] = filtered * fb;

        writePos = (writePos + 1) % delayLen;
        readPos = (readPos + 1) % delayLen;

        ticksPlayed++;
        if (ticksPlayed >= delayLen * 100) { // arbitrary safety: allow multiple feedback passes
            resonatorActive = false;
        }

        return filtered;
    }

public:
    // --- 10 parameters ---
    // --- Parameters (exactly 10) ---
    Val& pitch = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 });

    Val& decay = val(0.98f, "DECAY", { "Decay", .min = 0.80f, .max = 0.99f, .step = 0.01f, .floatingPoint = 2 });
    Val& tone = val(50.0f, "TONE", { "Tone", .unit = "%" });
    Val& pluckNoise = val(50.0f, "PLUCK_NOISE", { "Pluck Noise", .unit = "%" });
    Val& damping = val(0.5f, "DAMPING", { "Damping", .unit = "%" });
    Val& stereo = val(0.0f, "STEREO", { "Stereo", .unit = "%" });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    // --- constructor ---
    StringEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "String")
        , multiFx(props.sampleRate, props.lookupTable)
    {
        delayLen = (uint32_t)std::min<uint64_t>((uint64_t)(props.sampleRate * 0.02f), (uint64_t)MAX_DELAY);
        delayLine.assign(delayLen + 4, 0.0f);
        initValues();
    }

    void sampleOn(float* buf, float envAmpVal, int sampleCounter, int totalSamples) override
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
        velocity = _velocity;
        float freq = powf(2.0f, (note - 60 + pitch.get()) / 12.0f);
        if (freq < 20.0f)
            freq = 20.0f;
        if (freq > props.sampleRate * 0.45f)
            freq = props.sampleRate * 0.45f;

        uint32_t len = (uint32_t)std::max<int>(2, (int)std::round(props.sampleRate / freq));
        len = std::min<uint32_t>(len, MAX_DELAY);

        if (len != delayLen) {
            delayLen = len;
            delayLine.assign(delayLen + 4, 0.0f);
        }

        // Excite buffer with noise
        for (uint32_t i = 0; i < delayLen; ++i) {
            float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            n = filter.process(n);
            delayLine[i] = n * velocity * pluckNoise.pct();
        }

        writePos = 0;
        readPos = 0;
        onePoleState = 0.0f;
        resonatorActive = true;
        ticksPlayed = 0;
    }
};
