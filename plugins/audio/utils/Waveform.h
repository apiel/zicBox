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

#include "plugins/audio/lookupTable.h"
#include "plugins/audio/utils/utils.h"

#include <cstdint>

class Waveform {
protected:
    LookupTable* sharedLut;
    uint64_t sampleRate;

    void loadSineType()
    {
        for (uint16_t i = 0; i < LOOKUP_TABLE_SIZE; i++) {
            lut[i] = sharedLut->sine[i];
        }
    }

    void loadTriangleType()
    {
        for (uint16_t i = 0; i < LOOKUP_TABLE_SIZE; i++) {
            lut[i] = 4.0f * std::abs((i / (float)LOOKUP_TABLE_SIZE) - 0.5f) - 1.0f;
        }
    }

public:
    float lut[LOOKUP_TABLE_SIZE];

    Waveform(LookupTable* sharedLut, uint64_t sampleRate)
        : sharedLut(sharedLut)
        , sampleRate(sampleRate)
    {
        loadSineType();
        // loadTriangleType();
    }

    float sample(float* index, float freq)
    {
        float phaseIncrement = freq / sampleRate;

        *index += phaseIncrement;
        while (*index >= 1.0f) {
            *index -= 1.0f;
        }

        // return lut[(uint16_t)(*index * LOOKUP_TABLE_SIZE)];
        return linerInterpolation(*index, LOOKUP_TABLE_SIZE, lut);
    }


};

#endif
