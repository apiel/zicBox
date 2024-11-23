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
    uint8_t currentStepDataId = 1;
    uint8_t timeDataId = 2;
    uint8_t modDataId = 3;

    int8_t currentstep = 0;
    float currentMod = 0.0f;
    uint16_t currentTimeMs = 0;

    int envelopHeight = 30;
    int cursorY = 0;
    Point envPosition = { 0, 0 };

    void renderEnvelop()
    {
        Data& data = envData->at(0);
        Point lastPosition = { (int)(envPosition.x + size.w * data.time), (int)(envPosition.y + envelopHeight - envelopHeight * data.modulation) };
        // printf("[0] %f %f => %dpx x %dpx\n", data.modulation, data.time, lastPosition.x, lastPosition.y);
        for (int i = 1; i < envData->size(); i++) {
            Data& data2 = envData->at(i);
            Point nextPosition = { (int)(envPosition.x + size.w * data2.time), (int)(envPosition.y + envelopHeight - envelopHeight * data2.modulation) };
            draw.line(lastPosition, nextPosition);
            // printf("[%d] %f %f => %dpx x %dpx\n", i, data2.modulation, data2.time, nextPosition.x, nextPosition.y);
            lastPosition = nextPosition;
        }
    }

    void renderEditStep()
    {
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

    void renderTitles()
    {
        draw.text({ position.x, position.y }, std::to_string(currentstep + 1) + "/" + std::to_string(envData->size()), 8);
        draw.textCentered({ position.x + size.w / 2, position.y }, std::to_string(currentTimeMs) + "ms", 8);
        draw.textRight({ position.x + size.w, position.y }, std::to_string((int)(currentMod * 100)) + "%", 8);
    }

public:
    DrumEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
    {
        envPosition = { position.x, position.y + 10 };
        envelopHeight = size.h - 6 - 10;
        cursorY = position.y + size.h - 1;
    }

    void render() override
    {
        draw.filledRect(position, { size.w, size.h }, { .color = { SSD1306_BLACK } });
        if (envData) {
            currentstep = *(int8_t*)plugin->data(currentStepDataId);
            currentMod = *(float*)plugin->data(modDataId);
            currentTimeMs = *(uint16_t*)plugin->data(timeDataId);

            renderEnvelop();
            renderEditStep();
            renderTitles();
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (id == 0) {
            plugin->data(currentStepDataId, &direction);
            renderNext();
        } else if (id == 1) {
            plugin->data(timeDataId, &direction);
            renderNext();
        } else if (id == 2) {
            plugin->data(modDataId, &direction);
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
            uint8_t id = atoi(value);
            envData = (std::vector<Data>*)plugin->data(id);
            currentStepDataId = id + 1;
            timeDataId = id + 2;
            modDataId = id + 3;
            return true;
        }

        /*md - `STEP_DATA_ID: id` is the data id to get/set the current step/phase to edit.*/
        if (strcmp(key, "STEP_DATA_ID") == 0) {
            currentStepDataId = atoi(value);
            return true;
        }

        /*md - `TIME_DATA_ID: id` is the data id to get/set the step to time.*/
        if (strcmp(key, "TIME_DATA_ID") == 0) {
            timeDataId = atoi(value);
            return true;
        }

        /*md - `MOD_DATA_ID: id` is the data id to get/set the step to mod.*/
        if (strcmp(key, "MOD_DATA_ID") == 0) {
            modDataId = atoi(value);
            return true;
        }

        return false;
    }
};

#endif
