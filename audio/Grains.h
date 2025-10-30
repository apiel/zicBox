#pragma once

#include <cmath>
#include <cstdint>
#include <functional>

#include "helpers/clamp.h"
#include "audio/lookupTable.h"

#define MAX_GRAINS 16

class Grains {
protected:
    LookupTable* lookupTable;
    std::function<float(uint64_t)> sampleCallback;

    uint64_t grainDuration = 0;
    uint64_t grainDelay = 0;
    float densityDivider = 1.0f;

    float delayRandomize = 0.0f;
    float pitchRandomize = 0.0f;
    float detune = 0.0f;
    uint8_t density = 1;

    struct Grain {
        uint64_t index = 0;
        float position = 0.0f;
        float positionIncrement = 1.0f;
    } grains[MAX_GRAINS];

    void initGrain(uint8_t densityIndex, uint64_t sampleIndex, float stepIncrement, uint64_t sampleCount)
    {
        Grain& grain = grains[densityIndex];
        grain.index = 0;
        grain.position = CLAMP(sampleIndex + densityIndex * grainDelay + grainDelay * getRand() * delayRandomize, 0.0f, sampleCount - 1.0f);

        float dir = direction == FORWARD ? 1.0f : (direction == BACKWARD ? -1.0f : getRand());

        float pitchRand = pitchRandomize > 0.0f ? powf(2.0f, (getRand() * (6.0f * pitchRandomize)) / 12.0f) : 1.0f; // ±6 semitones at 100%
        float pitchDetune = 1.0f;
        if (detune != 0.0f) {
            if (density > 1.0f) {
                if (detuneMode == POSITIVE) {
                    float semitoneOffset = ((float)densityIndex / (density - 1.0f)) * detune;
                    pitchDetune = powf(2.0f, semitoneOffset / 12.0f);
                } else if (detuneMode == NEGATIVE) {
                    float semitoneOffset = ((float)densityIndex / (density - 1.0f)) * detune;
                    pitchDetune = powf(2.0f, -semitoneOffset / 12.0f);
                } else {
                    float offset = ((float)densityIndex / (density - 1.0f)) * 2.0f - 1.0f; // range [-1, +1]
                    float semitoneOffset = offset * (detune * 0.5f); // symmetric spread
                    pitchDetune = powf(2.0f, semitoneOffset / 12.0f);
                }
            } else {
                pitchDetune = powf(2.0f, detune / 12.0f);
            }
        }

        grain.positionIncrement = stepIncrement * pitchRand * pitchDetune * dir;
    }

    float getRand()
    {
        return lookupTable->getNoise(); // Random [-1.0, 1.0]
    }

public:
    enum DIRECTION {
        FORWARD = 0,
        BACKWARD = 1,
        RANDOM = 2
    } direction
        = FORWARD;

    enum DETUNE_MODE {
        POSITIVE = 0,
        SYMMETRIC = 1,
        NEGATIVE = 2
    } detuneMode
        = POSITIVE;

    Grains(LookupTable* lookupTable, std::function<float(uint64_t)> sampleCallback)
        : lookupTable(lookupTable)
        , sampleCallback(sampleCallback)
    {
    }

    float getGrainSample(float stepIncrement, uint64_t sampleIndex, uint64_t sampleCount)
    {
        float out = 0.0f;
        for (uint8_t i = 0; i < density; i++) {
            Grain& grain = grains[i];
            if (grain.index++ < grainDuration) {
                grain.position += grain.positionIncrement;
                if (grain.position <= 0 || grain.position >= sampleCount) {
                    initGrain(i, sampleIndex, stepIncrement, sampleCount);
                }
            } else {
                initGrain(i, sampleIndex, stepIncrement, sampleCount);
            }
            out += sampleCallback((uint64_t)grain.position);
        }
        out = out * densityDivider;
        return out;
    }

    void setDelayRandomize(float value) { delayRandomize = value; } // [0.0, 1.0f]
    void setPitchRandomize(float value) { pitchRandomize = value; } // [0.0, 1.0f]
    void setDetune(float value) { detune = value; } // [0.0, 12.0f]
    void setDetuneMode(enum DETUNE_MODE value) { detuneMode = value; }
    void setDirection(enum DIRECTION value) { direction = value; }
    void setDensity(uint8_t value) // [1, MAX_GRAINS]
    {
        density = value;
        densityDivider = 1.0f / sqrtf(density); // natural loudness
    }
    void setGrainDuration(uint64_t value) { grainDuration = value; }
    void setGrainDelay(uint64_t value) { grainDelay = value; }
};
