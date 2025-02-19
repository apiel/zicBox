#ifndef _MIXER4_H_
#define _MIXER4_H_

#include "Mixer.h"

class Mixer4 : public Mixer<4> {
public:
    Mixer4(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<4>(props, config)
    {
    }
};

#endif
