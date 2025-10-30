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
