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
    float shape = 0.5f;
    float macro = 0.5f;

    float lut[LOOKUP_TABLE_SIZE];

    Type selectedType = Type::Sine;

    void loadSineType()
    {
        for (uint16_t i = 0; i < sampleCount; i++) {
            lut[i] = sharedLut->sine[i];
        }
    }

    void loadTriangleType()
    {
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount;

            float y = phase < shape
                ? (1.0f / shape) * phase
                : (1.0f - (1.0f / (1.0f - shape)) * (phase - shape));

            lut[i] = -(2.0f * y - 1.0f);
        }
    }

    // void loadSquareType()
    // {
    //     for (uint16_t i = 0; i < sampleCount; i++) {
    //         lut[i] = i < sampleCount / 2 ? 1.0f : -1.0f;
    //     }
    // }

    // void loadSquareType()
    // {
    //     float pulse = range(macro, 0.1f, 0.9f);
    //     for (uint16_t i = 0; i < sampleCount; i++) {
    //         float phase = i / (float)sampleCount; // Normalized phase [0, 1)
    //         float dutyCycle = pulse; // Pulse width directly set by `pulse` parameter

    //         float value;
    //         if (phase < dutyCycle) {
    //             // High part of the pulse
    //             if (shape < 0.33f) {
    //                 // Square wave (sharp edges)
    //                 value = 1.0f;
    //             } else if (shape < 0.66f) {
    //                 // Rounded pulse (smooth transition to low)
    //                 float transition = (dutyCycle - phase) / (0.33f * dutyCycle);
    //                 value = 1.0f - std::exp(-transition * 5.0f); // Smooth transition down
    //             } else {
    //                 // Circular pulse (sinusoidal edge)
    //                 float normalizedPhase = phase / dutyCycle; // Normalize phase to [0, 1]
    //                 value = std::sin(normalizedPhase * M_PI_2); // Circular transition down
    //             }
    //         } else {
    //             // Low part of the pulse
    //             if (shape < 0.33f) {
    //                 // Square wave (sharp edges)
    //                 value = -1.0f;
    //             } else if (shape < 0.66f) {
    //                 // Rounded pulse (smooth transition to high)
    //                 float transition = (phase - dutyCycle) / (0.33f * (1.0f - dutyCycle));
    //                 value = -1.0f + std::exp(-transition * 5.0f); // Smooth transition up
    //             } else {
    //                 // Circular pulse (sinusoidal edge)
    //                 float normalizedPhase = (phase - dutyCycle) / (1.0f - dutyCycle); // Normalize phase to [0, 1]
    //                 value = -std::sin(normalizedPhase * M_PI_2); // Circular transition up
    //             }
    //         }

    //         // Store the computed value in the lookup table
    //         lut[i] = value;
    //     }
    // }

    void loadSquareType()
    {
        float pulse = range(macro, 0.1f, 0.9f);
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)
            float dutyCycle = pulse; // Pulse width directly set by `pulse` parameter

            float value;
            if (phase < dutyCycle) {
                // High part of the pulse
                float t = phase / dutyCycle; // Normalized phase within the high region
                if (shape < 0.5f) {
                    // Interpolate between sharp and rounded
                    float mix = shape * 2.0f; // Normalize shape to [0, 1]
                    float sharp = 1.0f; // Pure sharp edge
                    float rounded = std::tanh(5.0f * (1.0f - t)); // Smooth transition down
                    value = sharp * (1.0f - mix) + rounded * mix;
                } else {
                    // Interpolate between rounded and circular
                    float mix = (shape - 0.5f) * 2.0f; // Normalize shape to [0, 1]
                    float rounded = std::tanh(5.0f * (1.0f - t)); // Smooth transition down
                    float circular = std::sin(t * M_PI_2); // Circular transition down
                    value = rounded * (1.0f - mix) + circular * mix;
                }
            } else {
                // Low part of the pulse
                float t = (phase - dutyCycle) / (1.0f - dutyCycle); // Normalized phase within the low region
                if (shape < 0.5f) {
                    // Interpolate between sharp and rounded
                    float mix = shape * 2.0f; // Normalize shape to [0, 1]
                    float sharp = -1.0f; // Pure sharp edge
                    float rounded = -std::tanh(5.0f * t); // Smooth transition up
                    value = sharp * (1.0f - mix) + rounded * mix;
                } else {
                    // Interpolate between rounded and circular
                    float mix = (shape - 0.5f) * 2.0f; // Normalize shape to [0, 1]
                    float rounded = -std::tanh(5.0f * t); // Smooth transition up
                    float circular = -std::sin(t * M_PI_2); // Circular transition up
                    value = rounded * (1.0f - mix) + circular * mix;
                }
            }

            // Store the computed value in the lookup table
            lut[i] = value;
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
            lut[i] = (square1 - modulated) * 0.5f; // Combine waveforms
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

    void setMacro(float value)
    {
        macro = range(value, 0.0f, 1.0f);
        setWaveformType(selectedType);
    }
};

#endif
