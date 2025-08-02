#pragma once

#include "Mixer.h"

class Mixer10 : public Mixer<10> {
public:
    Mixer10(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mixer<10>(props, config)
    {
    }
};
