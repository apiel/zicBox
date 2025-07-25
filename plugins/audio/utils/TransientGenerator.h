#pragma once

#include "helpers/range.h"
#include "plugins/audio/utils/WavetableInterface.h"

#include <cmath>

class TransientGenerator : public WavetableInterface {
public:
    enum class Type {
        Click,
        Thump,
        Zap,
        Snap,
        Pop,
        Knock,
        ReverseClick,
        PulsePop,
        Buzz,
        COUNT,
    };

    TransientGenerator(uint64_t sampleRate, float durationMs = 200.0f)
        : WavetableInterface(static_cast<uint64_t>((sampleRate * durationMs) / 1000.0f))
        , sampleRate(sampleRate)
    {
        setType(Type::Click);
    }

    void setType(Type t)
    {
        type = t;
        updateTable();
    }

    void setMorph(float m)
    {
        morph = range(m, 0.0f, 1.0f);
        updateTable();
    }

    // New morphing between types using a single parameter t in [0, 1]
    void morphType(float t)
    {
        float normalized = range(t, 0.0f, 1.0f);
        float scaled = normalized * (static_cast<int>(Type::COUNT) - 1); // e.g., 0.0 → Click, 1.0 → Reverse
        int typeIndex = static_cast<int>(scaled);
        type = static_cast<Type>(typeIndex);
        morph = scaled - static_cast<float>(typeIndex);
        updateTable();
    }

    float next(float* index)
    {
        return sample(index, 0.0f);
    }

    float sample(float* index, float /*freq*/) override
    {
        *index += 1.0f / static_cast<float>(sampleCount);
        if (*index >= 1.0f) {
            *index = 1.0f; // lock to end
        }
        return linearInterpolation(*index, sampleCount, lut);
    }

    float* sample(float* index) override
    {
        return &lut[static_cast<uint64_t>(*index * sampleCount)];
    }

    float* samples() override
    {
        return lut;
    }

private:
    uint64_t sampleRate;
    Type type = Type::Click;
    float morph = 0.0f;
    float* lut = new float[sampleCount]; // you might prefer static allocation

    static float linearInterpolation(float index, uint64_t size, const float* table)
    {
        float fIndex = index * size;
        int i0 = static_cast<int>(fIndex) % size;
        int i1 = (i0 + 1) % size;
        float frac = fIndex - static_cast<float>(i0);
        return table[i0] * (1.0f - frac) + table[i1] * frac;
    }

    void updateTable()
    {
        for (uint64_t i = 0; i < sampleCount; ++i) {
            float x = static_cast<float>(i) / sampleCount;
            lut[i] = generateSample(type, x, morph);
        }
    }

    static float generateSample(Type t, float x, float shapeMorph)
    {
        const float pi = 3.14159265358979f;
        const float twopi = 2.0f * pi;

        // Envelope helpers
        auto exponentialDecay = [](float t, float a) {
            return std::exp(-a * t);
        };

        auto reverseDecay = [](float t, float a) {
            return 1.0f - std::exp(-a * (1.0f - t));
        };

        auto saturate = [](float v, float drive) {
            return std::tanh(v * drive);
        };

        auto whiteNoise = []() {
            return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        };

        switch (t) {
        case Type::Click: {
            float amp = exponentialDecay(x * 10.0f, 25.0f + 75.0f * shapeMorph); // ~1–10ms
            float noise = whiteNoise();
            return amp * noise;
        }

        case Type::Thump: {
            float env = exponentialDecay(x, 12.0f + shapeMorph * 40.0f);
            float sine = sinf(twopi * 80.0f * x * (1.0f + shapeMorph * 2.0f));
            return saturate(env * sine, 1.0f + shapeMorph * 4.0f);
        }

        case Type::Zap: {
            float freqStart = 800.0f;
            float freqEnd = 120.0f + shapeMorph * 100.0f;
            float freq = freqStart + (freqEnd - freqStart) * x;
            float env = exponentialDecay(x, 15.0f);
            return env * sinf(twopi * freq * x);
        }

        case Type::Snap: {
            float centerFreq = 1000.0f + shapeMorph * 3000.0f;
            float bw = 1.0f - shapeMorph * 0.9f;
            float filteredNoise = sinf(twopi * centerFreq * x) * whiteNoise() * bw;
            float env = exponentialDecay(x, 20.0f);
            return filteredNoise * env;
        }

        case Type::Pop: {
            float env = exponentialDecay(x, 20.0f);
            float n = whiteNoise();
            return saturate(env * n, 1.0f + shapeMorph * 3.0f);
        }

        case Type::Knock: {
            float f = 150.0f + shapeMorph * 80.0f;
            float env = exponentialDecay(x, 8.0f);
            float sine = sinf(twopi * f * x);
            return sine * env;
        }

        case Type::ReverseClick: {
            float env = reverseDecay(x, 20.0f);
            float noise = whiteNoise();
            return noise * env;
        }

        case Type::PulsePop: {
            float pw = 0.2f + shapeMorph * 0.5f;
            float val = (fmod(x, 1.0f) < pw) ? 1.0f : -1.0f;
            float env = exponentialDecay(x, 30.0f);
            return env * val;
        }

        case Type::Buzz: {
            float drive = 5.0f + shapeMorph * 15.0f;
            float noise = whiteNoise();
            float env = exponentialDecay(x, 10.0f);
            return saturate(env * noise, drive);
        }

        default:
            return 0.0f;
        }
    }
};
