#pragma once

#include "Mixer.h"

class Mixer6 : public Mixer<6> {
public:
    Mixer6(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<6>(props, config)
    {
    }
};
