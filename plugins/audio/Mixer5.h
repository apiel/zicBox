#ifndef _MIXER5_H_
#define _MIXER5_H_

#include "Mixer.h"

class Mixer5 : public Mixer<5> {
public:
    Mixer5(AudioPlugin::Props& props, char* _name)
        : Mixer<5>(props, _name)
    {
    }
};

#endif
