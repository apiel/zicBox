#ifndef _MIXER6_H_
#define _MIXER6_H_

#include "Mixer.h"

class Mixer6 : public Mixer<6> {
public:
    Mixer6(AudioPlugin::Props& props, char* _name)
        : Mixer<6>(props, _name)
    {
    }
};

#endif
