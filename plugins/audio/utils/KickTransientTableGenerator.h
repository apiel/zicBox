#pragma once

#include "helpers/clamp.h"
#include "plugins/audio/utils/WavetableInterface.h"
#include "plugins/audio/utils/lookupTable.h"

#include <array>
#include <cmath>
#include <cstdlib>

class KickTransientTableGenerator : public WavetableInterface {
public:
    KickTransientTableGenerator()
        : WavetableInterface(LOOKUP_TABLE_SIZE)
    {
        setMorph(0.0f);
    }

    void setMorph(float m)
    {
        morph = CLAMP(m, 0.0f, 1.0f);
        updateTable();
    }

    float getMorph() const { return morph; }

    float next(float index)
    {
        return linearInterpolation(index, sampleCount, lut);
    }

    float sample(float* index, float /*freq*/) override
    {
        *index += 1.0f / static_cast<float>(sampleCount);
        if (*index >= 1.0f)
            *index = 1.0f; // lock to end
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
    float morph = 0.0f;
    float lut[LOOKUP_TABLE_SIZE];

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
            float x = static_cast<float>(i) / static_cast<float>(sampleCount);
            lut[i] = generateTransient(x, morph);
        }
    }

    static float generateTransient(float x, float morph)
    {
        struct Family {
            float (*func)(float);
        };

        static const Family families[13] = {
            { [](float x) { float env = std::exp(-200.0f * 0.65f * x); float n = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; return n * env; } },
            { [](float x) { float env = std::exp(-1500.0f * 0.65f * x); float n = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; return n * env; } },
            { [](float x) { return (float)std::sin(2.0f * M_PI * 400.0f * x) * std::exp(-60.0f * x); } },
            { [](float x) { return (float)std::sin(2.0f * M_PI * (80.0f + 100.0f) * x) * std::exp(-40.0f * x); } },
            { [](float x) { return (float)std::sin(2.0f * M_PI * 60.0f * x) * std::exp(-15.0f * 0.65f * x); } },
            { [](float x) { float n = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; return n * 0.5f; } },
            { [](float x) { return (float)std::sin(2.0f * M_PI * 400.0f * x) * std::exp(-25.0f * x); } },
            { [](float x) { return (float)std::sin(2.0f * M_PI * 120.0f * x) * std::exp(-10.0f * x * 0.65f); } },
            { [](float x) { return (float)std::sin(2.0f * M_PI * 80.0f * x) * std::exp(-8.0f * x * 0.8f); } },
            { [](float x) { float n = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; return n * 0.5f; } },
            { [](float x) { return (float)std::tanh(std::sin(2.0f * M_PI * 300.0f * x) * 2.5f) * std::exp(-12.0f * x); } },
            { [](float x) { return (float)std::tanh(std::sin(2.0f * M_PI * 200.0f * x) * 3.0f) * std::exp(-15.0f * x); } },
            { [](float x) { return (float)std::tanh(std::sin(2.0f * M_PI * 150.0f * x) * 3.5f) * std::exp(-20.0f * x); } }
        };

        const float pos = morph * (13 - 1 + 0.9999f);
        const int i0 = static_cast<int>(pos);
        const int i1 = std::min(i0 + 1, 12);
        const float frac = pos - static_cast<float>(i0);

        float v0 = families[i0].func(x);
        float v1 = families[i1].func(x);

        return v0 * (1.0f - frac) + v1 * frac;
    }
};
