#ifndef _ZIC_ENVELOP_H_
#define _ZIC_ENVELOP_H_

#include <vector>

class Envelop {
public:
    unsigned int index = 0;

    struct Data {
        float modulation;
        float time;
    };

    std::vector<Data> data;

    Envelop(std::vector<Data> data)
        : data(data)
    {
        index = data.size() - 1;
    }

    float next(float time, unsigned int* indexPtr)
    {
        if (*indexPtr > data.size() - 1) {
            return 0.0f;
        }

        if (time >= data[*indexPtr + 1].time) {
            (*indexPtr)++;
        }
        float timeOffset = data[*indexPtr + 1].time - data[*indexPtr].time;
        // if (timeOffset == 0.0f) {
        //     return data[*indexPtr].modulation;
        // }
        float timeRatio = (time - data[*indexPtr].time) / timeOffset;
        return (data[*indexPtr + 1].modulation - data[*indexPtr].modulation) * timeRatio + data[*indexPtr].modulation;
    }

    float next(float time)
    {
        return next(time, &index);
    }

    void reset()
    {
        index = 0;
    }
};

#endif
