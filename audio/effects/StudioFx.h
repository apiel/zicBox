#pragma once

// Shared master FX bus for zicStudio: delay + reverb promoted from the
// per-instrument FX sections. Feed it a send bus, add the returned wet signal
// to the dry mix. The same header is used by the app and the offline harness.
#include <cstring>

#include "audio/effects/applyReverb.h"
#include "audio/effects/fxBuffer.h"

struct StudioFx {
    static constexpr int MAX_DELAY = 88200; // 2 s at 44.1 kHz
    float delayBuffer[MAX_DELAY] = {};
    int delayIndex = 0;
    int delaySamples = 11025; // ~250 ms at 44.1 kHz
    float delayFeedback = 0.35f;
    float delayLevel = 1.0f;
    float reverbBuffer[FX_BUFFER_SIZE] = {};
    int reverbIndex = 0;
    float reverbAmount = 0.0f;
    float reverbLevel = 1.0f;

    void reset()
    {
        std::memset(delayBuffer, 0, sizeof(delayBuffer));
        std::memset(reverbBuffer, 0, sizeof(reverbBuffer));
        delayIndex = 0;
        reverbIndex = 0;
    }

    // Returns the wet signal only (0 when the effect is bypassed).
    float process(float in)
    {
        int read = delayIndex - delaySamples;
        if (read < 0) read += MAX_DELAY;
        float delayed = delayBuffer[read];
        delayBuffer[delayIndex] = in + delayed * delayFeedback;
        delayIndex = (delayIndex + 1) % MAX_DELAY;

        float wet = delayed * delayLevel;

        if (reverbAmount > 0.001f) {
            float out = applyReverb2(in, reverbAmount, reverbBuffer, reverbIndex);
            wet += (out - in) * reverbLevel;
        }
        return wet;
    }
};
