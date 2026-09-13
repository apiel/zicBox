#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include "samplesData.h"

struct SampleTrack {
    const char* name = "Track";
    uint8_t trackIdx = 0;

    // Parameters
    float volume = 1.0f;
    float pitch = 0.0f; // in semitones (-12 to +12)
    uint8_t sampleIdx = 0;
    bool muted = false;

    // Runtime state
    const int16_t* samplePtr = nullptr;
    uint32_t numSamples = 0;
    float playPos = 0.0f;
    float speed = 1.0f;
    bool active = false;

    void init(uint8_t idx, const char* trackName, uint8_t defaultSample)
    {
        trackIdx = idx;
        name = trackName;
        sampleIdx = defaultSample;
        setSample(defaultSample);
        updateSpeed();
    }

    void setSample(uint8_t sIdx)
    {
        sampleIdx = sIdx % (sizeof(g_presetSamples) / sizeof(g_presetSamples[0]));
        samplePtr = g_presetSamples[sampleIdx].pcm;
        numSamples = g_presetSamples[sampleIdx].numSamples;
    }

    void updateSpeed()
    {
        speed = std::pow(2.0f, pitch / 12.0f);
    }

    void trigger()
    {
        playPos = 0.0f;
        active = true;
    }

    inline float renderNextSample()
    {
        if (!active || muted || !samplePtr) return 0.0f;

        uint32_t idx = (uint32_t)playPos;
        if (idx >= numSamples) {
            active = false;
            return 0.0f;
        }

        int16_t raw = samplePtr[idx];
        playPos += speed;

        return (raw / 32768.0f) * volume;
    }
};
