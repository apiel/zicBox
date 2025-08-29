#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MultiFx.h"
#include <cmath>
#include <vector>

class StringEngine : public Engine {
protected:
    MultiFx multiFx;
    MultiFx multiFx2;

    // Resonator
    std::vector<float> delayLine;
    uint32_t delayLen = 0;
    uint32_t writePos = 0;
    uint32_t readPos = 0;
    float onePoleState = 0.0f;
    bool resonatorActive = false;

    float sampleRate = 48000.0f;
    float velocity = 1.0f;

    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536

    inline float noteToFreq(int note) const
    {
        return 440.0f * powf(2.0f, (note - 69) / 12.0f);
    }

    // --- Resonator tick ---
    float resonatorTick()
    {
        if (!resonatorActive || delayLen == 0)
            return 0.0f;

        uint32_t rp = readPos % delayLen;
        uint32_t rp1 = (rp + 1) % delayLen;
        float s0 = delayLine[rp];
        float s1 = delayLine[rp1];
        float out = 0.5f * (s0 + s1); // linear interp

        // one-pole lowpass
        float cutoff = std::max(0.001f, tone.pct());
        onePoleState += cutoff * (out - onePoleState);
        float filtered = onePoleState;

        // feedback
        float fb = decay.get();
        delayLine[writePos % delayLen] = filtered * fb;

        // advance
        writePos = (writePos + 1) % delayLen;
        readPos = (readPos + 1) % delayLen;

        if (fabs(filtered) < 1e-6f) {
            resonatorActive = false;
        }

        return filtered;
    }

public:
    // --- Parameters (exactly 10) ---
    Val& pitch = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq((int)p.val.get());
    });

    Val& decay = val(0.98f, "DECAY", { "Decay", .min = 0.80f, .max = 0.999f }, [&](auto p) {
        p.val.setFloat(p.value);
    });

    Val& tone = val(50.0f, "TONE", { "Tone", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
    });

    Val& pluckNoise = val(50.0f, "PLUCK_NOISE", { "Pluck Noise", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
    });

    Val& damping = val(0.5f, "DAMPING", { "Damping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
    });

    Val& stereo = val(0.0f, "STEREO", { "Stereo", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { "FX2 type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx2.setFxType(p);
    });

    Val& fx2Amount = val(0, "FX2_AMOUNT", { "FX2 edit", .unit = "%" });

    // --- constructor ---
    StringEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "String")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
    {
        sampleRate = props.sampleRate;
        delayLen = (uint32_t)std::min<uint64_t>((uint64_t)(sampleRate * 0.02f), (uint64_t)MAX_DELAY);
        delayLine.assign(delayLen + 4, 0.0f);
        initValues();
    }

    // --- NoteOn: excite resonator ---
    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        setBaseFreq(pitch.get(), note);

        float freq = baseFreq;
        if (freq < 20.0f)
            freq = 20.0f;
        if (freq > sampleRate * 0.45f)
            freq = sampleRate * 0.45f;

        uint32_t len = (uint32_t)std::max<int>(2, (int)std::round(sampleRate / freq));
        len = std::min<uint32_t>(len, MAX_DELAY);

        if (len != delayLen) {
            delayLen = len;
            delayLine.assign(delayLen + 4, 0.0f);
        }

        // Excite buffer with noise
        for (uint32_t i = 0; i < delayLen; ++i) {
            float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            delayLine[i] = n * velocity * pluckNoise.pct();
        }

        writePos = 0;
        readPos = 0;
        onePoleState = 0.0f;
        resonatorActive = true;
    }

    // --- Sample ---
    void sample(float* buf, float envAmpVal) override
    {
        float out = resonatorTick();
        out *= velocity * envAmpVal;

        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());

        buf[track] = out;
    }
};
