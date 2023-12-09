#ifndef _HELPER_RANDOM_H_
#define _HELPER_RANDOM_H_

#include <stdlib.h>
#include <time.h>

class Random {
protected:
    int randCounter = 0;

public:
    int get()
    {
        // could create a lookup table ?
        srand(time(NULL) + randCounter++);
        return rand();
    }

    float pct()
    {
        return toPct(get());
    }

    float toPct(int value)
    {
        return value / (float)RAND_MAX;
    }
};

#endif