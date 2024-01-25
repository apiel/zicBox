#ifndef _MIXER8_H_
#define _MIXER8_H_

#include "Mixer.h"

class Mixer8 : public Mixer<8> {
public:
    Mixer8(AudioPlugin::Props& props, char* _name)
        : Mixer<8>(props, _name)
    {
    }
};

#endif
