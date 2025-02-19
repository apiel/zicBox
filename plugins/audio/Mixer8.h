#ifndef _MIXER8_H_
#define _MIXER8_H_

#include "Mixer.h"

class Mixer8 : public Mixer<8> {
public:
    Mixer8(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<8>(props, config)
    {
    }
};

#endif
