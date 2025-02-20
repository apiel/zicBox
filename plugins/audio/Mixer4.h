#pragma once

#include "Mixer.h"

class Mixer4 : public Mixer<4> {
public:
    Mixer4(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<4>(props, config)
    {
    }
};

