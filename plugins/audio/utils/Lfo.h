#pragma once
#include <cstdint>

#include <cmath>
#include <stdlib.h>

class LFO {
public:
    enum Waveform {
        TRIANGLE,
        SQUARE,
        SAWTOOTH,
        REVERSE_SAWTOOTH,
        NOISE,
        WAVEFORM_COUNT,
    };

    LFO(float sampleRate, float rate = 1.0f, Waveform wave = TRIANGLE)
        : sampleRate(sampleRate)
        , rate(rate)
        , phase(0.0f)
    {
        setWaveform(wave);
        updatePhaseIncrement();
    }

    void setRate(float r)
    {
        rate = r;
        updatePhaseIncrement();
    }

    void setWaveform(Waveform wave)
    {
        waveform = wave;
        switch (wave) {
        case SQUARE:
            processFunc = &LFO::processSquare;
            break;
        case TRIANGLE:
            processFunc = &LFO::processTriangle;
            break;
        case SAWTOOTH:
            processFunc = &LFO::processSawtooth;
            break;
        case REVERSE_SAWTOOTH:
            processFunc = &LFO::processReverseSawtooth;
            break;
        case NOISE:
            processFunc = &LFO::processNoise;
            break;
        }
    }

    void setWaveform(uint8_t wave) {
        if (wave < WAVEFORM_COUNT) {
            setWaveform(static_cast<Waveform>(wave));
        }
    }

    float process()
    {
        return (this->*processFunc)(); // Call the selected function
    }

    const char* toString() {
        switch (waveform) {
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
        }
        return "Unknown";
    }

private:
    float sampleRate;
    float rate;
    Waveform waveform;
    float phase;
    float phaseIncrement;
    float (LFO::*processFunc)() = nullptr;

    void updatePhaseIncrement()
    {
        phaseIncrement = rate / sampleRate;
    }

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
        float value = 2.0f * fabs(2.0f * phase - 1.0f) - 1.0f;
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
};
