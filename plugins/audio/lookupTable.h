#ifndef _LOOKUP_TABLE_H_
#define _LOOKUP_TABLE_H_

#include <math.h>

#define LOOKUP_TABLE_SIZE 4096

class LookupTable {
protected:
    int noiseIndex = 0;

public:
    const static int size = LOOKUP_TABLE_SIZE;
    float sine[size];
    float noise[size];

    LookupTable()
    {
        for (int i = 0; i < size; i++) {
            sine[i] = sinf((float)i / (float)size * 2.0f * M_PI);
            noise[i] = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
        }
    }

    float getNoise()
    {
        if (noiseIndex >= size) {
            noiseIndex = 0;
        }
        return noise[noiseIndex++];
    }
};

#endif
