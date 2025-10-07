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

    // Morph is normalized 0.0 → 1.0
    void setMorph(float m)
    {
        morph = CLAMP(m, 0.0f, 0.9999f);
        updateTable();
    }

    float next(float index)
    {
        return linearInterpolation(index, sampleCount, lut);
    }

    float sample(float* index, float freq) override
    {
        float phaseIncrement = freq / static_cast<float>(sampleCount);
        *index += phaseIncrement;
        while (*index >= 1.0f)
            *index -= 1.0f;
        return linearInterpolation(*index, sampleCount, lut);
    }

    float* sample(float* index) override
    {
        return &lut[static_cast<uint16_t>(*index * sampleCount)];
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
    inline float shapeBounce(float x, float freq, float damp) const { return std::fabs(std::sin((1.0f - x) * M_PI * freq)) * std::exp(-damp * x); }
    inline float shapeGlitch(float x, float freq, float amp) const
    {
        float y = std::exp(-8.0f * x) + std::sin(freq * x) * amp;
        return CLAMP(y, 0.0f, 1.0f);
    }
    inline float shapeAnalogSnap(float x, float strength) const { return std::tanh((1.0f - x) * strength); }
    inline float shapeCubicDrop(float x, float power) const { return std::pow(1.0f - x, 3.0f + power); }
    inline float shapeFoldback(float x, float power) const { return std::pow(std::fabs(1.0f - 2.0f * x), 1.0f + power); }
    inline float shapeHyperCurve(float x, float m) const { return 1.0f / (1.0f + std::pow(m * x, 4.0f)); }
    inline float shapeBreakPoint(float x, float pivot) const { return x < pivot ? 1.0f - x / pivot : std::exp(-5.0f * (x - pivot)); }
    inline float shapeQuickSnap(float x) const { return std::exp(-100.0f * x); }
    inline float shapeSoftTail(float x) const { return std::pow(1.0f - x, 10.0f); }
    inline float shapeBounce2(float x) const { return std::fabs(std::sin((1.0f - x) * M_PI * 3.0f)) * std::exp(-4.0f * x); }
    inline float shapeHyperGlitch(float x) const { return std::exp(-12.0f * x) + std::sin(50.0f * x) * 0.05f; }
    inline float shapeDoubleSnap(float x) const { return std::exp(-80.0f * x) + 0.2f * std::exp(-8.0f * x); }
    inline float shapeAnalogPluck(float x) const { return std::pow(1.0f - x, 2.0f) * std::tanh((1.0f - x) * 5.0f); }

    static inline float smoothstep(float a, float b, float x)
    {
        float t = CLAMP((x - a) / (b - a), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    void updateTable()
    {
        for (uint64_t i = 0; i < sampleCount; ++i) {
            float x = static_cast<float>(i) / static_cast<float>(sampleCount);
            lut[i] = generateEnvelopeValue(x);
        }
    }

    float generateEnvelopeValue(float x) const
    {
        // non-linear morph curve for expressive control
        float m = std::pow(morph, 1.2f);

        // 15 asymmetric zones
        const float zones[15] = { 0.0f, 0.15f, 0.25f, 0.35f, 0.45f, 0.52f, 0.6f, 0.68f, 0.75f, 0.82f, 0.87f, 0.91f, 0.94f, 0.97f, 1.0f };
        int idx = 0;
        for (int z = 0; z < 14; z++) {
            if (m < zones[z + 1]) {
                idx = z;
                break;
            }
        }

        float prev = zones[idx];
        float next = zones[idx + 1];
        float f = smoothstep(prev, next, m);

        // Compute all 15 shapes
        const float shapes[15] = {
            shapeExp(x, 5 + std::pow(1 - morph, 4) * 150),
            shapePow(x, 1.5 + morph * 5),
            shapeInverse(x, 1 + morph * 8),
            shapeBounce(x, 1 + morph * 2, 2 + morph * 6),
            shapeGlitch(x, 10 + morph * 70, 0.1 + morph * 0.1),
            shapeAnalogSnap(x, 1 + 4 * morph),
            shapeCubicDrop(x, morph * 2),
            shapeFoldback(x, morph * 3),
            shapeHyperCurve(x, 1 + 9 * morph),
            shapeBreakPoint(x, 0.2 + morph * 0.3),
            shapeQuickSnap(x),
            shapeSoftTail(x),
            shapeBounce2(x),
            shapeHyperGlitch(x),
            shapeDoubleSnap(x)
        };

        float result = shapes[idx];
        if (idx < 14)
            result = shapes[idx] * (1.0f - f) + shapes[idx + 1] * f;

        return result;
    }
};
