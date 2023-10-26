#ifndef _AUDIO_BUFFER_H_
#define _AUDIO_BUFFER_H_

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

#endif