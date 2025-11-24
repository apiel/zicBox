/** Description:
This header file defines a specialized data structure, called `AudioBuffer`, designed specifically for managing live sound recordings. Its primary purpose is to hold a fixed amount of recent audio data in memory, functioning much like a continuous recording loop or a "circular buffer."

By default, this structure is configured to store approximately five seconds of high-quality audio samples (based on a common sample rate of 48,000 measurements per second). However, the duration can be easily customized if needed.

The core mechanism relies on a single large internal storage area where audio measurements (samples) are kept. When the structure is initialized, it is automatically cleared, setting all initial values to zero.

The key operational feature is the method used to add new sound data. When a new audio measurement arrives, the structure uses an internal pointer to place the new measurement into the next available slot. Instead of stopping when the storage limit is reached, the system automatically resets its writing position back to the beginning, systematically overwriting the oldest data.

This wrap-around behavior ensures the buffer always contains the very latest data—in this case, the most recent five seconds of recorded sound—making it ideal for real-time audio processing, such as capturing a moment just after an event occurs.

sha: d95de664b8da60b2f4eb0af9e32d487650912edd034222649b66b271c302993b 
*/
#pragma once

#include <stdint.h>

#ifndef AUDIO_BUFFER_SIZE
#define AUDIO_BUFFER_SIZE 5 * 48000
#endif

template <uint64_t SIZE = AUDIO_BUFFER_SIZE>
class AudioBuffer {
public:
    const uint64_t size = SIZE;

    // keep in memory 5 seconds of samples
    float samples[SIZE];
    uint64_t index = 0;

    AudioBuffer()
    {
        for (uint64_t i = 0; i < size; i++) {
            samples[i] = 0.0;
        }
    }

    void addSample(float sample)
    {
        samples[index] = sample;
        index++;
        if (index >= size) {
            index = 0;
        }
    }
};
