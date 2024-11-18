#ifndef _UI_PIXEL_COMPONENT_DRUM_ENV_H_
#define _UI_PIXEL_COMPONENT_DRUM_ENV_H_

#include "../../../helpers/range.h"
#include "../component.h"

class DrumEnvelopComponent : public Component {
protected:
    struct Data {
        float modulation;
        float time;
    };

    AudioPlugin* plugin = NULL;
    std::vector<Data>* envData = NULL;

    int envelopHeight = 30;
    int cursorY = 0;

    void renderEnvelop()
    {
        Data& data = envData->at(0);
        Point lastPosition = { (int)(position.x + size.w * data.time), (int)(position.y + envelopHeight - envelopHeight * data.modulation) };
        // printf("[0] %f %f => %dpx x %dpx\n", data.modulation, data.time, lastPosition.x, lastPosition.y);
        for (int i = 1; i < envData->size(); i++) {
            Data& data2 = envData->at(i);
            Point nextPosition = { (int)(position.x + size.w * data2.time), (int)(position.y + envelopHeight - envelopHeight * data2.modulation) };
            draw.line(lastPosition, nextPosition);
            // printf("[%d] %f %f => %dpx x %dpx\n", i, data2.modulation, data2.time, nextPosition.x, nextPosition.y);
            lastPosition = nextPosition;
        }
    }

    void renderEditStep()
    {
        int8_t currentstep = *(int8_t*)plugin->data(1);
        if (currentstep < envData->size() - 1) {
            float currentTime = envData->at(currentstep).time;
            float nextTime = envData->at(currentstep + 1).time;
            draw.line({ (int)(position.x + size.w * currentTime), cursorY }, { (int)(position.x + size.w * currentTime), cursorY - 3 });
            draw.line({ (int)(position.x + size.w * currentTime), cursorY - 1 }, { (int)(position.x + size.w * nextTime), cursorY - 1 });
            draw.line({ (int)(position.x + size.w * nextTime), cursorY }, { (int)(position.x + size.w * nextTime), cursorY - 3 });
        } else {
            draw.line({ position.x + size.w, cursorY }, { position.x + size.w, cursorY - 3 });
        }
    }

public:
    DrumEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
    {
        envelopHeight = size.h - 6;
        cursorY = position.y + size.h - 1;
    }

    void render() override
    {
        draw.filledRect(position, { size.w, size.h }, { .color = { SSD1306_BLACK } });
        if (envData) {
            renderEnvelop();
            renderEditStep();
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (id == 0) {
            plugin->data(1, &direction);
            renderNext();
        } else if (id == 1) {
            plugin->data(2, &direction);
            renderNext();
        } else if (id == 2) {
            plugin->data(3, &direction);
            renderNext();
        } else {
            printf("DrumEnvelopComponent onEncoder: %d %d\n", id, direction);
            ValueInterface* value = plugin->getValue("DURATION");
            value->increment(direction);
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `PLUGIN: plugin_name` set plugin target */
        if (strcmp(key, "PLUGIN") == 0) {
            plugin = &getPlugin(value, track);
            return true;
        }

        /*md - `ENVELOP_DATA_ID: id` is the id of the envelope data.*/
        if (strcmp(key, "ENVELOP_DATA_ID") == 0) {
            envData = (std::vector<Data>*)plugin->data(atoi(value));
            return true;
        }

        return false;
    }
};

#endif
