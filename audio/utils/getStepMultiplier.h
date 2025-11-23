/** Description:
This code snippet serves as a small utility tool crucial for ensuring proper playback speed when loading audio samples, such as those found in wave files.

In digital music creation, audio tracks can have different numbers of channels (usually Mono, meaning one channel, or Stereo, meaning two channels). When you load a song file, the system must harmonize the file's channel count with the music player’s intended output structure.

The core function of this code is to calculate a "step multiplier," which acts as an adjustment factor. This factor tells the audio loader how many data points to read, skip, or duplicate from the source file to maintain the correct playback speed and pitch.

The logic is split into two main scenarios:
1. **Mono Track Processing:** Calculates the required adjustment if the application is internally treating the audio as a single, centralized track.
2. **Stereo Track Processing:** Calculates the adjustment if the application is handling the audio as a full dual-channel track.

By comparing the channel count of the source file to the desired output channels, the code returns a fractional or whole number (like 0.5, 1.0, 2.0, or 4.0). This number then scales the reading process, ensuring that audio loaded from a stereo file plays correctly even if the system is currently outputting mono sound, and vice-versa, achieving accurate and synchronized playback.

sha: c0e982dc82681ab9daf3ee6befa535a08ebca23607cf5471ab7c4e1491a922eb 
*/
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
