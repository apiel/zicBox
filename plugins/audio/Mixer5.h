#ifndef _MIXER5_H_
#define _MIXER5_H_

#include "Mixer.h"

class Mixer5 : public Mixer<5> {
public:
    Mixer5(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<5>(props, config)
    {
    }
};

#endif
