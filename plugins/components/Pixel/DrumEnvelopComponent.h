#ifndef _UI_PIXEL_COMPONENT_DRUM_ENV_H_
#define _UI_PIXEL_COMPONENT_DRUM_ENV_H_

#include "../component.h"

class DrumEnvelopComponent : public Component {
public:
    AudioPlugin* plugin = NULL;

    struct Data {
        float modulation;
        float time;
    };

    DrumEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
    {
        plugin = &getPlugin("SynthDrum23", 1);
    }

    void render() override
    {
        if (plugin) {
            std::vector<Data>* envData = (std::vector<Data>*)plugin->data(3);
            if (envData) {
                printf("envData size %lu\n", envData->size());

                Data& data = envData->at(0);
                Point lastPosition = { (int)(position.x + size.w * data.time), (int)(position.y + size.h - size.h * data.modulation) };
                for (int i = 1; i < envData->size() - 1; i++) {
                    data = envData->at(i);
                    Point nextPosition = { (int)(position.x + size.w * data.time), (int)(position.y + size.h - size.h * data.modulation) };
                    draw.line(lastPosition, nextPosition);
                    draw.rect(nextPosition, { 2, 2 });
                    printf("%f %f => %dpx x %dpx\n", data.modulation, data.time, nextPosition.x, nextPosition.y);
                    lastPosition = nextPosition;
                }
            }
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        printf("DrumEnvelopComponent onEncoder: %d %d\n", id, direction);

        ValueInterface* value = plugin->getValue("DURATION");
        value->increment(direction);
    }
};

#endif
