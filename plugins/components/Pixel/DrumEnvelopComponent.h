#ifndef _UI_PIXEL_COMPONENT_DRUM_ENV_H_
#define _UI_PIXEL_COMPONENT_DRUM_ENV_H_

#include "../../../helpers/range.h"
#include "../component.h"

class DrumEnvelopComponent : public Component {
protected:
    AudioPlugin* plugin = NULL;

    struct Data {
        float modulation;
        float time;
    };

    int envelopHeight = 30;
    int cursorY = 0;

    int8_t currentstep = 0;
    int8_t editstepStart = 0;

    void renderEnvelop(std::vector<Data>* envData)
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

    void renderEditStep(std::vector<Data>* envData)
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

public:
    DrumEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
    {
        plugin = &getPlugin("SynthDrum23", 1);

        envelopHeight = size.h - 6;
        cursorY = position.y + size.h - 1;
    }

    void render() override
    {
        draw.filledRect(position, { size.w, size.h }, { .color = { SSD1306_BLACK } });
        if (plugin) {
            // TODO might want to set this globally
            std::vector<Data>* envData = (std::vector<Data>*)plugin->data(3);
            if (envData) {
                renderEnvelop(envData);
                renderEditStep(envData);
            }
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (id == 0) {
            if (direction > 0) {
                // TODO might want to set this globally
                std::vector<Data>* envData = (std::vector<Data>*)plugin->data(3);
                if (currentstep < envData->size() - 1) {
                    currentstep++;
                }
            } else if (currentstep > editstepStart) {
                currentstep--;
            }
            printf("update currentstep: %d\n", currentstep);
            renderNext();
        } else if (id == 1) {
            // TODO might want to set this globally
            std::vector<Data>* envData = (std::vector<Data>*)plugin->data(3);
            if (envData && currentstep > 0) {
                if (currentstep == envData->size() - 1) {
                    envData->push_back({ 0.0f, 1.0f });
                }
                Data& data = envData->at(currentstep);
                float value = data.time + (direction * 0.01f);
                data.time = range(value, 0.0f, 1.0f);
                renderNext();
            }
        } else if (id == 2) {
            // TODO might want to set this globally
            std::vector<Data>* envData = (std::vector<Data>*)plugin->data(3);
            if (envData) {
                Data& data = envData->at(currentstep);
                float value = data.modulation + (direction * 0.01f);
                data.modulation = range(value, 0.0f, 1.0f);
                renderNext();
            }
        } else {
            printf("DrumEnvelopComponent onEncoder: %d %d\n", id, direction);
            ValueInterface* value = plugin->getValue("DURATION");
            value->increment(direction);
        }
    }

    bool config(char* key, char* value)
    {
        /*md `EDIT_STEP_START: set the first modultation step that can be edited (by default 0). */
        if (strcmp(key, "EDIT_STEP_START") == 0) {
            editstepStart = atoi(value);
            // TODO might want to set this globally
            std::vector<Data>* envData = (std::vector<Data>*)plugin->data(3);
            currentstep = range(currentstep, editstepStart, envData->size() - 1);
            return true;
        }

        return false;
    }
};

#endif
