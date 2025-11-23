/** Description:
This C++ header defines a flexible utility class named `FastWaveform`, designed to generate various types of electronic signals or sound patterns, often referred to as an oscillator. It is optimized for speed and intended for systems that require frequent, repeatable calculations, such as digital audio synthesis or control systems.

**Core Functionality**

The generator operates by tracking its position within a single cycle, known as the "phase," which ranges from 0 to 1. At every step, the central `process()` function calculates the exact value (or amplitude) of the chosen waveform at the current phase, then increments the phase to prepare for the next step.

**Waveform Types**

This tool can create several fundamental shapes:
1.  **Periodic Waves:**
    *   **Square:** Alternating instantly between high and low values.
    *   **Triangle:** Moving smoothly up and down in a sharp V-shape.
    *   **Sawtooth/Reverse Sawtooth:** Ramping sharply up then instantly resetting (or vice-versa).
2.  **Random Waves:**
    *   **Noise:** Pure, completely random output.
    *   **Brown Noise:** A smoother, bass-heavy random sound (like a random walk).
    *   **Sample & Hold:** Holds a random value constant until the cycle finishes, then picks a new random value.

**Control and Operation**

You initialize the generator by providing the system's **Sample Rate** (how often values are calculated) and the desired **Rate** (the frequency of the waveform). The system uses an internal mechanism to efficiently select and run the correct mathematical formula for the chosen waveform type, allowing instantaneous switching between shapes without code clutter. The `process()` function is then called repeatedly to get the next calculated value in the sequence.

sha: f46c696ceec108fa1f59485cbb1aca71bfd0862e1529310f9d47c685fb556712 
*/
#pragma once
#include <cmath>
#include <cstdint>
#include <stdlib.h>

class FastWaveform {
public:
    enum Type {
        TRIANGLE,
        SQUARE,
        SAWTOOTH,
        REVERSE_SAWTOOTH,
        NOISE,
        BROWN_NOISE,
        // PINK_NOISE,
        SAMPLE_AND_HOLD,
        TYPE_COUNT,
    };

    FastWaveform(float sampleRate, Type wave = TRIANGLE, float rate = 1.0f)
        : sampleRate(sampleRate)
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
        case BROWN_NOISE:
            processFunc = &FastWaveform::processBrownNoise;
            break;
        // case PINK_NOISE:
        //     processFunc = &FastWaveform::processPinkNoise;
        //     break;
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
        case BROWN_NOISE:
            return "Brown Noise";
        // case PINK_NOISE:
        //     return "Pink Noise";
        case SAMPLE_AND_HOLD:
            return "S&H";
        }
        return "Unknown";
    }

    void reset()
    {
        phase = 0.0f;
    }

private:
    float sampleRate;
    float rate;
    Type type;
    float phase = 0.0f;
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

    float brown = 0.0f;
    float processBrownNoise()
    {
        float white = processNoise();
        // brown += white * 0.02f; // Adjust 0.02f to control the smoothness: 0.005f to 0.05f
        // brown *= 0.98f; // Damping factor for stability and realism: 0.95f to 0.995f
        brown += white * 0.04f;
        brown *= 0.96f;
        // brown += white * 0.01f;
        // brown *= 0.99f;
        return brown;
    }

    // float pink = 0.0f;
    // float processPinkNoise()
    // {
    //     float white = processNoise();
    //     pink = 0.95f * pink + 0.05f * white; // Simple low-pass filtering
    //     return pink;
    // }

    float sampleHoldValue = 0.0f;
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
