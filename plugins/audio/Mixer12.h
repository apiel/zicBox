#pragma once

#include "Mixer.h"

class Mixer12 : public Mixer<12> {
public:
    Mixer12(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<12>(props, config)
    {
    }
};
