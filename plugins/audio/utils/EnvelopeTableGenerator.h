#pragma once

#include <cmath>

#include "plugins/audio/utils/WavetableInterface.h"
#include "plugins/audio/utils/lookupTable.h"
#include "helpers/range.h"

class EnvelopeTableGenerator : public WavetableInterface {
public:
    enum class Type {
        ExpoDecay,
        DownHills,
        MultiDecay,
        SinPow
    };

    EnvelopeTableGenerator(LookupTable* sharedLut)
        : WavetableInterface(LOOKUP_TABLE_SIZE)
        , sharedLut(sharedLut)
    {
        setType(Type::ExpoDecay);
    }

    void setType(Type t) {
        type = t;
        updateTable();
    }

    void setMorph(float m) {
        morph = range(m, 0.0f, 1.0f);
        updateTable();
    }

    float next(float index) {
        return linearInterpolation(index, sampleCount, lut);
    }

    float sample(float* index, float freq = 1.f) override {
        float phaseIncrement = freq / (float)sampleCount;
        *index += phaseIncrement;
        while (*index >= 1.0f)
            *index -= 1.0f;

        return linearInterpolation(*index, sampleCount, lut);
    }

    float* sample(float* index) override {
        return &lut[(uint16_t)(*index * sampleCount)];
    }

    float* samples() override {
        return lut;
    }

private:
    LookupTable* sharedLut;
    Type type = Type::ExpoDecay;
    float morph = 0.0f;
    float lut[LOOKUP_TABLE_SIZE];

    static float linearInterpolation(float index, uint64_t size, const float* table) {
        float fIndex = index * size;
        int i0 = static_cast<int>(fIndex) % size;
        int i1 = (i0 + 1) % size;
        float frac = fIndex - static_cast<float>(i0);
        return table[i0] * (1.0f - frac) + table[i1] * frac;
    }

    void updateTable() {
        for (uint64_t i = 0; i < sampleCount; ++i) {
            float x = static_cast<float>(i) / (float)sampleCount;
            lut[i] = generateEnvelopeValue(x);
        }
    }

    float generateEnvelopeValue(float x) const {
        switch (type) {
        case Type::ExpoDecay: {
            float a = 20.0f + 80.0f * morph; // Decay speed
            return std::exp(-a * x);
        }

        case Type::DownHills: {
            float a = 100.f * morph + 5.f;
            float b = morph;
            float c = 20.f * morph + 1.f;
            return range(std::exp(-a * x) + b * std::sin(x) - std::pow(x, c), 0.0f, 1.0f);
        }

        case Type::MultiDecay: {
            float decay = 0.7f * std::exp(-60.0f * (morph + 0.05f) * x);
            float tail = 0.2f * std::exp(-5.0f * morph * x);
            float release = tail - tail * std::pow(x, morph + 0.1f);
            return range(decay + release, 0.0f, 1.0f);
        }

        case Type::SinPow: {
            float a = morph;
            int b = static_cast<int>(morph * 100) * 2 + 4;
            float c = 0.5f * morph;
            return range(-a * std::sin(-1 + x) + std::pow(std::max(0.f, -1 + x), b) + c * std::acos(std::min(x, 1.0f)), 0.0f, 1.0f);
        }

        default:
            return 0.0f;
        }
    }
};
