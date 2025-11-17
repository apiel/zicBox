#pragma once

#include <cstdint>

// Convert step increment when loading sample wave file with a different channel count

// By default we handle each track in mono and only make it stereo in the output plugin by copying the left channel to the right
// So if wavefile is stereo and app plays in stereo, we need to double the sample count.
float getStepMultiplierMonoTrack(uint8_t channels, uint8_t propsChannels)
{
    if (channels < propsChannels) {
        return 1.0f;
    } else if (channels > propsChannels) {
        return 4.0f;
    } else {
        return 2.0f;
    }
}

float getStepMultiplierStereoTrack(uint8_t channels, uint8_t propsChannels)
{
    if (channels < propsChannels) {
        return 0.5f;
    } else if (channels > propsChannels) {
        return 2.0f;
    } else {
        return 1.0f;
    }
}
