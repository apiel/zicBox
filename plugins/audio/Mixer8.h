#pragma once

#include "Mixer.h"

class Mixer8 : public Mixer<8> {
public:
    Mixer8(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<8>(props, config)
    {
    }
};
