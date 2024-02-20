#ifndef _LOOKUP_TABLE_H_
#define _LOOKUP_TABLE_H_

#include <math.h>

class LookupTable {
public:
    // const static int size = 2048;
    const static int size = 4096;
    float sine[size];
    float noise[size];

    LookupTable()
    {
        for (int i = 0; i < size; i++) {
            sine[i] = sinf((float)i / (float)size * 2.0f * M_PI);
            noise[i] = 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f;
        }
    }
};

#endif
