#ifndef _UI_PIXEL_COMPONENT_DRUM_ENV_H_
#define _UI_PIXEL_COMPONENT_DRUM_ENV_H_

#include "../component.h"

class DrumEnvelopComponent : public Component {
public:
    DrumEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
    {
    }

public:
    void render() override
    {
    }

    void onEncoder(int id, int8_t direction) override
    {
        printf("DrumEnvelopComponent onEncoder: %d %d\n", id, direction);

        ValueInterface* value = getPlugin("SynthDrum23", 1).getValue("DURATION");
        value->increment(direction);
    }
};

#endif
