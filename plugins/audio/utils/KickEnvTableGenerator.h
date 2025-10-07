#pragma once

#include "helpers/clamp.h"
#include "plugins/audio/utils/WavetableInterface.h"
#include "plugins/audio/utils/lookupTable.h"
#include <cmath>

class KickEnvTableGenerator : public WavetableInterface {
public:
    KickEnvTableGenerator()
        : WavetableInterface(LOOKUP_TABLE_SIZE)
    {
        setMorph(0.0f);
    }

    // Morph is now normalized: 0.0 → 1.0
    void setMorph(float m)
    {
        morph = CLAMP(m, 0.0f, 0.9999f);
        updateTable();
    }

    float next(float index)
    {
        return linearInterpolation(index, sampleCount, lut);
    }

    float sample(float* index, float freq = 1.f) override
    {
        float phaseIncrement = freq / (float)sampleCount;
        *index += phaseIncrement;
        while (*index >= 1.0f)
            *index -= 1.0f;
        return linearInterpolation(*index, sampleCount, lut);
    }

    float* sample(float* index) override
    {
        return &lut[(uint16_t)(*index * sampleCount)];
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

    // --- Envelope shape helpers ---
    inline float shapeExp(float x, float sharp) const { return std::exp(-sharp * x); }
    inline float shapePow(float x, float p) const { return std::pow(1.0f - x, p); }
    inline float shapeInverse(float x, float p) const { return 1.0f / (1.0f + std::pow(x * 4.0f, p)); }
    inline float shapeBounce(float x, float f, float d) const
    {
        return std::fabs(std::sin((1.0f - x) * M_PI * f)) * std::exp(-d * x);
    }
    inline float shapeGlitch(float x, float f, float a) const
    {
        float y = std::exp(-8.0f * x) + std::sin(f * x) * a;
        return CLAMP(y, 0.0f, 1.0f);
    }

    static inline float smoothstep(float a, float b, float x)
    {
        float t = CLAMP((x - a) / (b - a), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    void updateTable()
    {
        for (uint64_t i = 0; i < sampleCount; ++i) {
            float x = static_cast<float>(i) / (float)sampleCount;
            lut[i] = generateEnvelopeValue(x);
        }
    }

    float generateEnvelopeValue(float x) const
    {
        // non-linear morph curve for expressive control
        float m = std::pow(morph, 1.2f);

        // asymmetric zones for smooth expressive transitions
        const float zones[5] = { 0.0f, 0.45f, 0.6f, 0.75f, 1.0f };

        int idx = 0;
        for (int z = 0; z < 4; ++z) {
            if (m < zones[z + 1]) {
                idx = z;
                break;
            }
        }

        float prev = zones[idx];
        float next = zones[idx + 1];
        float f = smoothstep(prev, next, m);

        // base envelope shapes
        float e1 = shapeExp(x, 5.0f + std::pow(1.0f - morph, 4.0f) * 150.0f);
        float e2 = shapePow(x, 1.5f + morph * 5.0f);
        float e3 = shapeInverse(x, 1.0f + morph * 8.0f);
        float e4 = shapeBounce(x, 1.0f + morph * 2.0f, 2.0f + morph * 6.0f);
        float e5 = shapeGlitch(x, 10.0f + morph * 70.0f, 0.1f + morph * 0.1f);

        const float shapes[5] = { e1, e2, e3, e4, e5 };
        float result = shapes[idx];
        if (idx < 4)
            result = shapes[idx] * (1.0f - f) + shapes[idx + 1] * f;

        return result;
    }
};
