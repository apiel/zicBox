#pragma once
#include <cstdint>
#include <cmath>
#include <stdlib.h>

class FastWaveform {
public:
    enum Type {
        TRIANGLE,
        SQUARE,
        SAWTOOTH,
        REVERSE_SAWTOOTH,
        NOISE,
        SAMPLE_AND_HOLD,
        TYPE_COUNT,
    };

    FastWaveform(float sampleRate, float rate = 1.0f, Type wave = TRIANGLE)
        : sampleRate(sampleRate)
        , phase(0.0f)
        , sampleHoldValue(0.0f)
    {
        setType(wave);
        setRate(rate);
    }

    void setRate(float r)
    {
        rate = r;
        phaseIncrement = rate / sampleRate;
    }

    void setType(Type wave)
    {
        type = wave;
        switch (wave) {
        case SQUARE:
            processFunc = &FastWaveform::processSquare;
            break;
        case TRIANGLE:
            processFunc = &FastWaveform::processTriangle;
            break;
        case SAWTOOTH:
            processFunc = &FastWaveform::processSawtooth;
            break;
        case REVERSE_SAWTOOTH:
            processFunc = &FastWaveform::processReverseSawtooth;
            break;
        case NOISE:
            processFunc = &FastWaveform::processNoise;
            break;
        case SAMPLE_AND_HOLD:
            processFunc = &FastWaveform::processSampleAndHold;
            break;
        }
    }

    void setType(uint8_t wave)
    {
        if (wave < TYPE_COUNT) {
            setType(static_cast<Type>(wave));
        }
    }

    float process()
    {
        return (this->*processFunc)(); // Call the selected function
    }

    const char* toString()
    {
        switch (type) {
        case SQUARE:
            return "Square";
        case TRIANGLE:
            return "Triangle";
        case SAWTOOTH:
            return "Sawtooth";
        case REVERSE_SAWTOOTH:
            return "Ramp Down";
        case NOISE:
            return "Noise";
        case SAMPLE_AND_HOLD:
            return "Sample & Hold";
        }
        return "Unknown";
    }

private:
    float sampleRate;
    float rate;
    Type type;
    float phase;
    float sampleHoldValue;
    float phaseIncrement;
    float (FastWaveform::*processFunc)() = nullptr;

    float processSquare()
    {
        float value = (phase < 0.5f) ? 1.0f : -1.0f;
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        return value;
    }

    float processTriangle()
    {
        // float value = 2.0f * fabs(2.0f * phase - 1.0f) - 1.0f;
        float value = (phase < 0.5f) ? (4.0f * phase - 1.0f) : (3.0f - 4.0f * phase); // Alternate formula to fabs for efficiency
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        return value;
    }

    float processSawtooth()
    {
        float value = 2.0f * phase - 1.0f;
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        return value;
    }

    float processReverseSawtooth()
    {
        float value = 1.0f - 2.0f * phase;
        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;
        return value;
    }

    uint32_t seed = 123456789;
    float processNoise() // Linear Congruential Generator (LCG): An LCG is a very lightweight and efficient pseudo-random number generator (PRNG) commonly used in audio synthesis. Also, compare to LUT, LGG should be more efficient.
    {
        seed = (1664525u * seed + 1013904223u);
        return ((seed >> 16) & 0x7FFF) / 32767.0f * 2.0f - 1.0f;
    }
    // float processNoise() // Xorshift Algorithm: A slightly more advanced PRNG, slower than LCG, but still faster than rand().
    // {
    //     seed ^= seed << 13;
    //     seed ^= seed >> 17;
    //     seed ^= seed << 5;
    //     return (seed / 4294967295.0f) * 2.0f - 1.0f;
    // }

    float processSampleAndHold()
    {
        if (phase >= 1.0f) {
            phase -= 1.0f;
            sampleHoldValue = processNoise();
        }
        phase += phaseIncrement;
        return sampleHoldValue;
    }
};
