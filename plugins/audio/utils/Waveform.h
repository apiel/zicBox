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
        float pulse = range(macro, 0.05f, 1.0f);
        for (uint16_t i = 0; i < sampleCount; i++) {
            float phase = i / (float)sampleCount; // Normalized phase [0, 1)

            // Adjust phase to fit within the pulse width
            if (phase < pulse) {
                phase /= pulse; // Scale phase within the active pulse region

                // Compute the waveform value based on the shape parameter
                float y = phase < shape
                    ? (1.0f / shape) * phase // Rising edge
                    : (1.0f - (1.0f / (1.0f - shape)) * (phase - shape)); // Falling edge

                // Scale and store in the lookup table
                lut[i] = -(2.0f * y - 1.0f);
            } else {
                // Outside the pulse range, set to 0
                lut[i] = 0.0f;
            }
        }
    }

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

    void setWaveformType(Type waveformType, bool reset = true)
    {
        selectedType = waveformType;
        switch (waveformType) {
        case Type::Sine: {
            loadSineType();
            break;
        }
        case Type::Triangle: {
            if (reset) {
                shape = 0.5f; // triangle
                macro = 1.0f; // no pulse
            }
            loadTriangleType();
            break;
        }
        case Type::Square: {
            if (reset) {
                shape = 0.0f; // square
                macro = 0.5f; // centered
            }
            loadSquareType();
            break;
        }
        case Type::Flame:
            loadFlameType();
            break;
        }
    }

    void setShape(float value)
    {
        shape = range(value, 0.0f, 1.0f);
        setWaveformType(selectedType, false);
    }

    void setMacro(float value)
    {
        macro = range(value, 0.0f, 1.0f);
        setWaveformType(selectedType, false);
    }
};

#endif
