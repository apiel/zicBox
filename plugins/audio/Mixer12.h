#ifndef _MIXER12_H_
#define _MIXER12_H_

#include "Mixer.h"

class Mixer12 : public Mixer<12> {
public:
    Mixer12(AudioPlugin::Props& props, char* _name)
        : Mixer<12>(props, _name)
    {
    }
};

#endif
