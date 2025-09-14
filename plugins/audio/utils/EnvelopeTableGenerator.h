#pragma once

#include <cmath>

#include "helpers/range.h"
#include "plugins/audio/utils/WavetableInterface.h"
#include "plugins/audio/utils/lookupTable.h"

class EnvelopeTableGenerator : public WavetableInterface {
public:
    enum class Type {
        ExpoDecay,
        DownHills,
        MultiDecay,
        SinPow,
        ShortPunch,
        LongBoom,
        SnappyFall,
        SmoothSlope,
        RubberDrop,
        TiltedArc,
        BassPluck,
        BreakPoint,
        ClickSpike,
        InversePow,
        AnalogSnap,
        CubicDrop,
        GlitchFall,
        HyperCurve,
        Foldback,
        GhostTail,
    };

    EnvelopeTableGenerator(LookupTable* sharedLut)
        : WavetableInterface(LOOKUP_TABLE_SIZE)
        , sharedLut(sharedLut)
    {
        setType(Type::ExpoDecay);
    }

    void setType(Type t)
    {
        type = t;
        updateTable();
    }

    void setMorph(float m)
    {
        morph = CLAMP(m, 0.0f, 1.0f);
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
    LookupTable* sharedLut;
    Type type = Type::ExpoDecay;
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
            float x = static_cast<float>(i) / (float)sampleCount;
            lut[i] = generateEnvelopeValue(x);
        }
    }

    float generateEnvelopeValue(float x) const
    {
        switch (type) {
        case Type::ExpoDecay: {
            float amount = 1.0f - morph;
            float a = 10.0f + 100.0f * amount * amount * amount * amount; // Decay speed
            return std::exp(-a * x);
        }

        case Type::DownHills: {
            float a = 20.f * morph + 5.f;
            float b = morph;
            float c = 2.f * morph + 1.f;
            return CLAMP(std::exp(-a * x) + b * std::sin(x) - std::pow(x, c), 0.0f, 1.0f);
        }

        case Type::MultiDecay: {
            float amount = 1.0f - morph;
            float decay = 0.7f * std::exp(-30.0f * (amount + 0.05f) * x);
            float tail = 0.2f * std::exp(-5.0f * amount * x);
            float release = tail - tail * std::pow(x, amount + 0.1f);
            return CLAMP(decay + release, 0.0f, 1.0f);
        }

        case Type::SinPow: {
            float a = morph * 0.1f;
            int b = morph * 10;
            b = b * 2 + 4;
            float c = 0.1 * morph;
            return CLAMP(-a * sin(-1 + x) + pow(-1 + x, b) + c * acos(x), 0.0f, 1.0f);
            ;
        }

        case Type::ShortPunch: {
            float a = 40.0f + 100.0f * (1.0f - morph); // Sharpness
            return std::exp(-a * x);
        }

        case Type::LongBoom: {
            float a = 1.0f + 10.0f * morph;
            float b = 1.0f + 5.0f * morph;
            return std::pow(1.0f - x, a) * std::exp(-b * x);
        }

        case Type::SnappyFall: {
            float snap = 1.0f - morph;
            return std::exp(-50.0f * x) + 0.3f * std::exp(-10.0f * snap * x);
        }

        case Type::SmoothSlope: {
            float curve = 2.0f + 6.0f * morph;
            return std::pow(1.0f - x, curve);
        }

        case Type::RubberDrop: {
            float damping = 4.0f + 12.0f * (1.0f - morph);
            return std::abs(std::sin((1.0f - x) * M_PI * morph * 0.6f + 0.3f)) * std::exp(-damping * x);
        }

        case Type::TiltedArc: {
            float skew = 0.3f + 0.7f * morph;
            float curved = std::pow(1.0f - std::pow(x, skew), 2.0f);
            return curved;
        }

        case Type::BassPluck: {
            float fastDecay = std::exp(-80.0f * x);
            float softTail = 0.2f * std::exp(-4.0f * morph * x);
            return fastDecay + softTail;
        }

        case Type::BreakPoint: {
            float pivot = 0.2f + morph * 0.3f;
            if (x < pivot)
                return 1.0f - x / pivot;
            else
                return std::exp(-5.0f * (x - pivot));
        }

        case Type::ClickSpike: {
            float t = 0.01f + 0.05f * (1.0f - morph);
            return std::exp(-x * x / (2.0f * t * t));
        }

        case Type::InversePow: {
            float powVal = 2.0f + morph * 5.0f;
            return 1.0f / (1.0f + std::pow(x * 4.0f, powVal));
        }

        case Type::AnalogSnap: {
            float strength = 1.0f + 4.0f * morph;
            float shape = std::tanh((1.0f - x) * strength);
            return CLAMP(shape, 0.0f, 1.0f);
        }

        case Type::CubicDrop: {
            return std::pow(1.0f - x, 3.0f + 3.0f * morph);
        }

        case Type::GlitchFall: {
            float glitch = std::sin(20.0f * x * morph) * 0.1f;
            float base = std::exp(-10.0f * x);
            return CLAMP(base + glitch, 0.0f, 1.0f);
        }

        case Type::HyperCurve: {
            float m = 1.0f + 9.0f * morph;
            return 1.0f / (1.0f + m * x * x);
        }

        case Type::Foldback: {
            float folded = std::abs(1.0f - 2.0f * x);
            return std::pow(folded, 1.0f + morph * 4.0f);
        }
        case Type::GhostTail: {
            float a = 80.0f;
            float b = 3.0f + 10.0f * morph;
            float sharp = std::exp(-a * x);
            float longTail = std::exp(-b * x);
            return CLAMP(sharp + 0.3f * longTail, 0.0f, 1.0f);
        }

        default:
            return 0.0f;
        }
    }
};
