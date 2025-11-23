/** Description:
This C++ header defines a utility class called `Envelop`, which is primarily designed for use in digital signal processing (DSP), such as in music synthesizers or audio effects. Its purpose is to generate an "envelope," which dictates how a certain value (like volume, brightness, or frequency) changes smoothly over time.

### Core Mechanism

The envelope works by defining a sequence of distinct stages. Each stage in the sequence specifies two crucial pieces of information:
1.  **Modulation:** The target output level or intensity the envelope should reach.
2.  **Sample Count:** The duration (measured in discrete time steps, or "samples") it takes to transition to that target level.

The central function is `next()`. This function is called repeatedly for every single time step the system processes. It performs an interpolation—a smooth calculation—to determine the exact output value between the start and end of the current stage, ensuring the transitions are gradual and natural.

### Key Features

*   **Time Management:** A static function is provided to convert real-world durations, measured in milliseconds, into the appropriate digital "sample count" required by the envelope timing system.
*   **Sustain Phase:** The envelope can handle a "Sustain" stage, which holds the output level indefinitely. This is crucial for sustained sounds, like holding a key on a synthesizer.
*   **Control Functions:**
    *   The `release()` function manually triggers the system to exit the Sustain phase and begin the final, usually decaying, stages of the envelope.
    *   The `reset()` function allows the user to immediately restart the entire envelope sequence from the very first stage.

sha: 2738f0f5c9f148e15e10c8124d7383433c1e8ae73ed394884e8c4d6cc7dd714f 
*/
#pragma once

#include <vector>

class Envelop {
protected:
    unsigned int index = -1;
    unsigned int sampleCount = 0;

    void setNextPhase(unsigned int& sampleCountRef, unsigned int& indexRef)
    {
        sampleCountRef = 0.0f;
        indexRef++;
    }

    bool isSustain(unsigned int indexRef)
    {
        return data[indexRef].sampleCount == 0.0f;
    }

public:
    struct Data {
        float modulation;
        unsigned int sampleCount;
    };

    std::vector<Data> data;

    Envelop(std::vector<Data> data)
        : data(data)
    {
    }

    static unsigned int msToSampleCount(unsigned int ms, unsigned int sampleRate)
    {
        unsigned int count = ms * sampleRate * 0.001f;
        // printf("msToSampleCount: %d\n", count);
        return count;
    }

    float next(unsigned int& sampleCountRef, unsigned int& indexRef)
    {
        sampleCountRef++;
        if (indexRef >= data.size() - 1) {
            return 0.0f;
        }

        if (isSustain(indexRef)) {
            return data[indexRef].modulation;
        }

        if (sampleCountRef >= data[indexRef].sampleCount) {
            setNextPhase(sampleCountRef, indexRef);
            if (isSustain(indexRef)) {
                return data[indexRef].modulation;
            }
        }

        float timeRatio = (float)sampleCount / (float)data[indexRef].sampleCount;
        float env = (data[indexRef + 1].modulation - data[indexRef].modulation) * timeRatio + data[indexRef].modulation;

        // if (indexRef == 3) {
        //     printf("[%d] sampleCount %d / %d, mod+1: %f, mod: %f, timeRatio: %f, env: %f yo %f\n", indexRef, sampleCount, data[indexRef].sampleCount, data[indexRef + 1].modulation, data[indexRef].modulation, timeRatio, env);
        // }
        return env;
    }

    float next()
    {
        return next(sampleCount, index);
    }

    void release()
    {
        release(sampleCount, index);
    }

    void release(unsigned int& sampleCountRef, unsigned int& indexRef)
    {
        if (isSustain(indexRef)) {
            setNextPhase(sampleCountRef, indexRef);
        } else {
            for (int i = indexRef; i < data.size() - 1; i++) {
                if (isSustain(i)) {
                    (indexRef) = i;
                    setNextPhase(sampleCountRef, indexRef);
                }
            }
        }
    }

    void reset()
    {
        reset(sampleCount, index);
    }

    void reset(unsigned int& sampleCountRef, unsigned int& indexRef)
    {
        indexRef = 0;
        sampleCountRef = 0;
    }
};
