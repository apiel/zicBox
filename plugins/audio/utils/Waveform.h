#ifndef _WAVEFORM_H_
#define _WAVEFORM_H_

// https://potterdraw.sourceforge.io/Help/Modulation/Waveform.htm
// https://potterdraw.sourceforge.io/Help/Modulation/Slew.htm

// could make a LUT and do like envelop to get value between 2 points

// Param: edge sharpness for sine

// Param: stairs number?

// option1
// Param: point1 x
// Param: point1 y
// Param: point2 x
// Param: point2 y
// Param: width

// option4
// Param: angle from sawtooth to triangle
// Param: top width
// Param: bottom width

// option5
// Param: angle from sawtooth to triangle
// Param: stairs number: infinite triangle, 10, 9, to 1 became a square
// Param: pulse width: rectangular wave

// Type: sine, triangle, square,....
// Shape: sawtooth to triangle to soft edge triangle, square to circular...
// Pulse
// Stairs ?

#include "helpers/range.h"
#include "plugins/audio/lookupTable.h"
#include "plugins/audio/utils/WaveformInterface.h"
#include "plugins/audio/utils/utils.h"

#include <cstdint>

class Waveform : public WaveformInterface {
public:
    enum Type {
        Sine,
        Triangle,
        Square,
        Flame
    };

protected:
    LookupTable* sharedLut;
    uint64_t sampleRate;
    float shape = 0.9f;

    float lut[LOOKUP_TABLE_SIZE];

    Type selectedType = Type::Sine;

    void loadSineType()
    {
        for (uint16_t i = 0; i < sampleCount; i++) {
            lut[i] = sharedLut->sine[i];
        }
    }

    // void loadTriangleType()
    // {
    //     for (uint16_t i = 0; i < sampleCount; i++) {
    //         lut[i] = 4.0f * std::abs((i / (float)sampleCount) - 0.5f) - 1.0f;
    //     }
    // }

    void loadTriangleType()
    {
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)

            float y; // Output waveform value

            if (phase < shape) {
                // Ramp up for the rising segment
                y = (1.0f / shape) * phase;
            } else {
                // Ramp down for the falling segment
                y = 1.0f - (1.0f / (1.0f - shape)) * (phase - shape);
            }

            // Scale y to range [-1, 1]
            lut[i] = -(2.0f * y - 1.0f);
        }
    }

    void loadSquareType()
    {
        for (uint16_t i = 0; i < sampleCount; i++) {
            lut[i] = i < sampleCount / 2 ? 1.0f : -1.0f;
        }
    }

    void loadFlameType()
    {
        // A flame-like wave, similar to the Flame stitch in needlepoint, resulting from: squarewave(x) - squarewave(x * 2) * cos(x * PI * 2).
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)
            float square1 = phase < 0.5f ? 1.0f : -1.0f; // Square wave at base frequency
            float square2 = ((int)(phase * 2) % 2) == 0 ? 1.0f : -1.0f; // Square wave at double frequency
            float modulated = square2 * std::cos(phase * M_PI * 2); // Modulate square2 by cosine
            lut[i] = square1 - modulated; // Combine waveforms
        }
    }

public:
    Waveform(LookupTable* sharedLut, uint64_t sampleRate)
        : WaveformInterface(LOOKUP_TABLE_SIZE)
        , sharedLut(sharedLut)
        , sampleRate(sampleRate)
    {
        setWaveformType(Type::Sine);
    }

    float sample(float* index, float freq) override
    {
        float modulatedFreq = 110.0f * freq;
        float phaseIncrement = modulatedFreq / sampleRate;

        *index += phaseIncrement;
        while (*index >= 1.0f) {
            *index -= 1.0f;
        }

        // return lut[(uint16_t)(*index * sampleCount)];
        return linearInterpolation(*index, sampleCount, lut);
    }

    float* samples() override
    {
        return lut;
    }

    void setWaveformType(Type waveformType)
    {
        selectedType = waveformType;
        switch (waveformType) {
        case Type::Sine:
            loadSineType();
            break;
        case Type::Triangle:
            loadTriangleType();
            break;
        case Type::Square:
            loadSquareType();
            break;
        case Type::Flame:
            loadFlameType();
            break;
        }
    }

    void setShape(float value)
    {
        shape = range(value, 0.0f, 1.0f);
        setWaveformType(selectedType);
    }
};

#endif
