/** Description:
This technical blueprint defines a versatile digital audio component called the `WavetableGenerator`. Its core purpose is to efficiently create a wide range of sound waves for music synthesis.

### How it Works

The generator operates by calculating and storing one complete cycle of a sound wave (the "wavetable") in an internal list of numbers. When the audio system needs sound, it quickly reads from this stored table, using high-speed techniques like linear interpolation to ensure a smooth, high-quality signal at the correct pitch.

### Key Features and Controls

1.  **Wave Types:** It supports seven fundamental sound shapes, including classic waves like Sine, Sawtooth, Square, and Triangle, as well as more complex sounds generated using Frequency Modulation (FM).
2.  **The Morph Control:** This is a crucial feature that acts like a continuous adjustment knob (ranging from 0 to 1). The Morph setting determines the complexity or character of the wave. For example, on a Square wave, morphing changes the pulse width; on a Saw wave, it might introduce distortion or wave folding.
3.  **Sound Output:** You can request the sound wave using either the specific name (like "Saw") or by selecting its digital identifier. Once the type and morph are set, the generator recalculates and updates the internal wavetable immediately.

In summary, this component provides a flexible and efficient foundation for generating diverse, highly controllable waveforms essential for creating complex digital music textures.

sha: cfd23b97977d6f69a957b10895a84bb463f7e7d544d313afc0ea07f2ad4f7599 
*/
#pragma once

#include <algorithm>
#include <cmath>
#include <string>


#include "audio/WavetableInterface.h"
#include "audio/lookupTable.h"
#include "audio/utils/linearInterpolation.h"

class WavetableGenerator : public WavetableInterface {
public:
    enum class Type {
        Sine,
        Saw,
        Square,
        Triangle,
        Pulse,
        FM,
        FMSquare,
        COUNT,
    };

    WavetableGenerator(LookupTable* sharedLut, uint64_t sampleRate)
        : WavetableInterface(LOOKUP_TABLE_SIZE)
        , sharedLut(sharedLut)
        , sampleRate(sampleRate)
    {
        setType(Type::Sine);
    }

    void setType(Type t)
    {
        type = t;
        updateTable();
    }

    void setType(std::string t)
    {
        if (t == "Sine")
            setType(Type::Sine);
        else if (t == "Saw")
            setType(Type::Saw);
        else if (t == "Square")
            setType(Type::Square);
        else if (t == "Triangle")
            setType(Type::Triangle);
        else if (t == "Pulse")
            setType(Type::Pulse);
        else if (t == "FM")
            setType(Type::FM);
        else if (t == "FMSquare")
            setType(Type::FMSquare);
    }

    Type getType() const { return type; }
    std::string getTypeName() const
    {
        switch (type) {
        case Type::Sine:
            return "Sine";
        case Type::Saw:
            return "Saw";
        case Type::Square:
            return "Square";
        case Type::Triangle:
            return "Triangle";
        case Type::Pulse:
            return "Pulse";
        case Type::FM:
            return "FM";
        case Type::FMSquare:
            return "FMSquare";
        default:
            return "Unknown";
        }
    }

    void setMorph(float m)
    {
        morph = std::clamp(m, 0.0f, 1.0f);
        updateTable();
    }

    float getMorph() const { return morph; }

    float sample(float* index, float freq) override
    {
        float phaseIncrement = (110.0f * freq) / static_cast<float>(sampleRate);
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
    uint64_t sampleRate;
    Type type = Type::Sine;
    float morph = 0.0f;
    float lut[LOOKUP_TABLE_SIZE];

    // static float linearInterpolation(float index, uint64_t size, const float* table)
    // {
    //     float fIndex = index * size;
    //     int i0 = static_cast<int>(fIndex) % size;
    //     int i1 = (i0 + 1) % size;
    //     float frac = fIndex - static_cast<float>(i0);
    //     return table[i0] * (1.0f - frac) + table[i1] * frac;
    // }

    void updateTable()
    {
        for (uint64_t i = 0; i < sampleCount; ++i) {
            float phase = static_cast<float>(i) / sampleCount;
            lut[i] = generateSample(phase);
        }
    }

    float generateSample(float phase) const
    {
        const float pi = 3.14159265358979f;
        const float twopi = 2.0f * pi;

        switch (type) {
            // case Type::Sine: {
            //     float base = sinf(phase * 2.0f * M_PI);
            //     float harmonic2 = 0.5f * sinf(phase * 2.0f * M_PI * 2.0f);
            //     float harmonic3 = 0.25f * sinf(phase * 2.0f * M_PI * 3.0f);

            //     return base + morph * (harmonic2 + harmonic3);
            // }

        case Type::Sine: {
            // base sine phase
            float basePhase = phase * 2.0f * M_PI;

            // modulator frequency (fixed low-frequency modulator)
            float modFreq = 5.0f;

            // morph controls modulation index from 0 (no FM) to 2 (strong FM)
            float modIndex = morph * 2.0f;

            // frequency modulation with modulator sine wave
            float modulator = sinf(basePhase * modFreq);

            // frequency compensation: lower base frequency as morph increases
            float compensatedPhase = basePhase * (1.0f - morph * 0.5f);

            // apply FM by modulating the compensated phase
            float fmPhase = compensatedPhase + modIndex * modulator;

            // final waveform
            return sinf(fmPhase);
        }

        case Type::Saw: {
            float x = 2.0f * (phase - 0.5f); // Clean saw in [-1, 1]

            // Apply morph-based folding amount (0 = clean, 1 = heavy wavefolding)
            float foldAmount = 1.0f + morph * 4.0f; // Morph affects folding intensity
            float folded = fmodf(x * foldAmount + 3.0f, 4.0f) - 2.0f;
            folded = 2.0f - fabsf(folded);

            // Optional nonlinear shaping for extra grit at high morph
            if (morph > 0.7f) {
                float shapeAmt = (morph - 0.7f) / 0.3f;
                folded = std::tanh(folded * (1.0f + shapeAmt * 10.0f));
            }

            return folded;
        }

        case Type::Square: {
            float pw = 0.5f - morph * 0.49f;
            return (phase < pw) ? 1.0f : -1.0f;
        }

        case Type::Triangle: {
            float t = fmodf(phase, 1.0f);
            float skew = morph * 0.499f + 0.001f; // prevents division by 0

            if (t < 0.5f) {
                return (t < skew) ? t / skew : (1.0f - t) / (0.5f - skew);
            } else {
                float t2 = t - 0.5f;
                return (t2 < skew) ? -t2 / skew : -(0.5f - t2) / (0.5f - skew);
            }
        }

        case Type::Pulse: {
            float edge = 0.5f * morph + 0.01f;
            float t = std::fmod(phase, 1.0f);
            if (t < 0.5f - edge)
                return 1.0f;
            if (t > 0.5f + edge)
                return -1.0f;
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
