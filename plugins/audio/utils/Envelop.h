#ifndef _ZIC_ENVELOP_H_
#define _ZIC_ENVELOP_H_

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
        return ms * sampleRate * 0.001f;
    }

    float next(unsigned int& sampleCountRef, unsigned int& indexRef)
    {
        sampleCountRef++;
        if (indexRef > data.size() - 1) {
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

        float timeRatio = sampleCount / data[indexRef].sampleCount;
        return (data[indexRef + 1].modulation - data[indexRef].modulation) * timeRatio + data[indexRef].modulation;

        return 0.0f;
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
            (indexRef)++;
            setNextPhase(sampleCountRef, indexRef);
        } else {
            for (int i = indexRef; i < data.size(); i++) {
                if (isSustain(i)) {
                    (indexRef) = i + 1;
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

#endif
