/** Description:
This code defines an audio processing function designed to implement the classic "Flanger" effect, a common sound manipulation technique used in music production.

### Purpose and Function

The core purpose of this code is to alter a stream of audio samples digitally. The Flanger effect is achieved by mixing the original sound with a slightly delayed copy of itself, where the amount of delay is continuously and smoothly changing. This constant fluctuation creates a distinctive sweeping, phase-shifted, or "whooshing" sound.

### How the Flanger Effect Works

The function operates on individual audio samples as they arrive. To create the effect, it requires temporary storage (a "buffer") to hold recent history of the audio stream.

1.  **Modulated Delay:** The key to flanging is modulation. The function uses a mathematical sine wave to calculate a precise, ever-changing delay time, which typically shifts between a very short delay (around 3 milliseconds) and a slightly longer one (around 25 milliseconds).
2.  **Mixing:** It fetches an old audio sample from the temporary storage corresponding to the calculated varying delay.
3.  **Output:** This retrieved delayed sample is then mixed with the current incoming audio sample, based on the strength of the effect specified.
4.  **Storage:** Finally, the new, combined output sample is written back into the temporary storage, preparing it to be read later for future delays.

The function manages the circular movement through the storage array to ensure efficient and continuous audio processing. If the effect strength is set to zero, the function bypasses the calculation and simply returns the original audio, allowing it to be deactivated easily.

Tags: Audio Effects, Sound Modulation, Digital Sound Processing, Time-Based Audio, Sound Generation
sha: 9781bf719f3d719c69aa73e4b05b455a73faf50b08ed800d37c45f93e033aed0 
*/
#pragma once

#include <cmath>

float applyFlanger(float input, float amount, float* buffer, int& bufferIndex, int bufferSize, float& flangerPhase)
{
    if (amount == 0.0f) {
        return input;
    }

    int min_delay = 144; // 3ms at 48000khz
    int max_delay = 1200; // 25ms at 48000khz
    float speed = 1.0f; // Flanger speed in Hz
    flangerPhase += 0.00002f * speed;
    float mod = (sinf(flangerPhase) + 1.0f) / 2.0f; // Modulation between 0-1

    int delay = mod * max_delay + min_delay;
    int readIndex = (bufferIndex + bufferSize - delay) % bufferSize;

    float out = input + buffer[readIndex] * amount;

    buffer[bufferIndex] = out;
    bufferIndex = (bufferIndex + 1) % bufferSize;

    return out;
}
