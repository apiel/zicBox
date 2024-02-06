#ifndef _ZIC_ENVELOP_RELATIVE_H_
#define _ZIC_ENVELOP_RELATIVE_H_

#include <vector>

class EnvelopRelative {
public:
    unsigned int index = -1;

    struct Data {
        float modulation;
        float time;
    };

    std::vector<Data> data;

    EnvelopRelative(std::vector<Data> data)
        : data(data)
    {
    }

    float next(float time, unsigned int& indexRef)
    {
        if (indexRef > data.size() - 1) {
            return 0.0f;
        }

        if (time >= data[indexRef + 1].time) {
            indexRef++;
        }
        float timeOffset = data[indexRef + 1].time - data[indexRef].time;
        float timeRatio = (time - data[indexRef].time) / timeOffset;
        return (data[indexRef + 1].modulation - data[indexRef].modulation) * timeRatio + data[indexRef].modulation;
    }

    float next(float time)
    {
        return next(time, index);
    }

    void reset()
    {
        reset(index);
    }

    void reset(unsigned int& indexRef)
    {
        indexRef = 0;
    }
};

#endif
