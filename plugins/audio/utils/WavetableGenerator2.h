#pragma once

#include <cmath>
#include <algorithm>

#include "plugins/audio/utils/WavetableInterface.h"
#include "plugins/audio/utils/lookupTable.h"
class WavetableGenerator : public WavetableInterface {
public:
    enum class Type {
        Sine,
        Saw,
        Square,
        Triangle,
        Pulse,
        FM,
        FMSquare
    };

    WavetableGenerator(LookupTable* sharedLut, uint64_t sampleRate)
        : WavetableInterface(LOOKUP_TABLE_SIZE)
        , sharedLut(sharedLut)
        , sampleRate(sampleRate)
    {
        setType(Type::Sine);
    }

    void setType(Type t) {
        type = t;
        updateTable();
    }

    void setMorph(float m) {
        morph = std::clamp(m, 0.0f, 1.0f);
        updateTable();
    }

    float sample(float* index, float freq) override {
        float phaseIncrement = (110.0f * freq) / static_cast<float>(sampleRate);
        *index += phaseIncrement;
        while (*index >= 1.0f) *index -= 1.0f;

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
    uint64_t sampleRate;
    Type type = Type::Sine;
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
            float phase = static_cast<float>(i) / sampleCount;
            lut[i] = generateSample(phase);
        }
    }

    float generateSample(float phase) const {
        const float pi = 3.14159265358979f;
        const float twopi = 2.0f * pi;

        switch (type) {
            case Type::Sine: {
                int idx = static_cast<int>(phase * (LookupTable::size - 1));
                return sharedLut->sine[idx];
            }

            case Type::Saw: {
                float x = 2.0f * (phase - 0.5f);
                float linear = x;
                float nonlinear = std::tanh(x * (1.0f + morph * 4.0f));
                return (1.0f - morph) * linear + morph * nonlinear;
            }

            case Type::Square: {
                float pw = 0.5f - morph * 0.49f;
                return (phase < pw) ? 1.0f : -1.0f;
            }

            case Type::Triangle: {
                float skew = morph * 0.5f;
                float t = std::fmod(phase + skew, 1.0f);
                return 4.0f * std::abs(t - 0.5f) - 1.0f;
            }

            case Type::Pulse: {
                float edge = 0.5f * morph + 0.01f;
                float t = std::fmod(phase, 1.0f);
                if (t < 0.5f - edge) return 1.0f;
                if (t > 0.5f + edge) return -1.0f;
                float smooth = std::cos(((t - 0.5f) / edge) * pi);
                return smooth;
            }

            case Type::FM: {
                float mod = std::sin(twopi * phase * 2.0f);
                float modIndex = morph * 3.0f;
                return std::sin(twopi * (phase + modIndex * mod / twopi));
            }

            case Type::FMSquare: {
                float carrier = std::fmod(phase + morph * std::sin(twopi * phase), 1.0f);
                return (carrier < 0.5f) ? 1.0f : -1.0f;
            }

            default:
                return 0.0f;
        }
    }
};
