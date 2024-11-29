#ifndef _UI_PIXEL_COMPONENT_DRUM_ENV_H_
#define _UI_PIXEL_COMPONENT_DRUM_ENV_H_

#include "../../../helpers/range.h"
#include "../component.h"
#include "../utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## Drum envelop

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/DrumEnvelop.png" />

Show a representation of a drum envelop (envelop with relative time and modulation, without sustain).
*/

class DrumEnvelopComponent : public GroupColorComponent {
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
    Point pos = { 0, 0 };

    bool filled = true;
    bool outline = true;

    Color bgColor;

    ToggleColor fillColor;
    ToggleColor outlineColor;
    ToggleColor textColor;
    ToggleColor cursorColor;

    int encoderPhase = -1;
    int encoderTime = -1;
    int encoderModulation = -1;

    void renderEnvelop()
    {
        Data data = envData->at(0);
        std::vector<Point> points;
        points.push_back({ envPosition.x, envPosition.y + envelopHeight });
        if (data.modulation != 0.0f) {
            points.push_back({ envPosition.x, (int)(envPosition.y + envelopHeight - envelopHeight * data.modulation) });
        }
        for (int i = 1; i < envData->size(); i++) {
            data = envData->at(i);
            points.push_back({ (int)(envPosition.x + size.w * data.time), (int)(envPosition.y + envelopHeight - envelopHeight * data.modulation) });
        }

        if (data.modulation != 0.0f) {
            points.push_back({ (int)(envPosition.x + size.w), envPosition.y + envelopHeight });
        }

        if (filled) {
            draw.filledPolygon(points, { fillColor.color });
        }
        if (outline) {
            draw.lines(points, { outlineColor.color });
        }
    }

    void renderEditStep()
    {
        if (currentstep < envData->size() - 1) {
            float currentTime = envData->at(currentstep).time;
            float nextTime = envData->at(currentstep + 1).time;
            draw.line({ (int)(pos.x + size.w * currentTime), cursorY }, { (int)(pos.x + size.w * currentTime), cursorY - 3 }, { cursorColor.color });
            draw.line({ (int)(pos.x + size.w * currentTime), cursorY - 1 }, { (int)(pos.x + size.w * nextTime), cursorY - 1 }, { cursorColor.color });
            draw.line({ (int)(pos.x + size.w * nextTime), cursorY }, { (int)(pos.x + size.w * nextTime), cursorY - 3 }, { cursorColor.color });
        } else {
            draw.line({ pos.x + size.w, cursorY }, { pos.x + size.w, cursorY - 3 }, { cursorColor.color });
        }
    }

    void renderTitles()
    {
        int cellWidth = size.w / 3;
        int x = pos.x + cellWidth * 0.5;
        draw.textCentered({ x, pos.y }, std::to_string(currentstep + 1) + "/" + std::to_string(envData->size()), 8, { textColor.color });
        draw.textCentered({ x + cellWidth, pos.y }, std::to_string(currentTimeMs) + "ms", 8, { textColor.color });
        draw.textCentered({ x + cellWidth * 2, pos.y }, std::to_string((int)(currentMod * 100)) + "%", 8, { textColor.color });
    }

public:
    DrumEnvelopComponent(ComponentInterface::Props props)
        : GroupColorComponent(props, { { "FILL_COLOR", &fillColor }, { "OUTLINE_COLOR", &outlineColor }, { "TEXT_COLOR", &textColor }, { "CURSOR_COLOR", &cursorColor } })
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text, inactiveColorRatio)
        , cursorColor(styles.colors.text, inactiveColorRatio)
        , fillColor(styles.colors.primary, inactiveColorRatio)
        , outlineColor(lighten(styles.colors.primary, 0.5), inactiveColorRatio)
    {
        updateColors();

        envelopHeight = size.h - 6 - 10;
    }

    void render() override
    {
        pos = getPosition();
        envPosition = { pos.x, pos.y + 10 };
        cursorY = pos.y + size.h - 1;
        draw.filledRect(pos, size, { bgColor });

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
        if (isActive) {
            if (id == encoderPhase) {
                plugin->data(currentStepDataId, &direction);
                renderNext();
            } else if (id == encoderTime) {
                plugin->data(timeDataId, &direction);
                renderNext();
            } else if (id == encoderModulation) {
                plugin->data(modDataId, &direction);
                renderNext();
            } else {
                // printf("DrumEnvelopComponent onEncoder: %d %d\n", id, direction);
                // ValueInterface* value = plugin->getValue("DURATION");
                // value->increment(direction);

                if (id > 4) { // just for testing rpi speaker, to be removed
                    printf("play SynthDrum23 note on (encoder %d)\n", id);
                    AudioPlugin* plugin = &getPlugin("SynthDrum23", 1);
                    plugin->noteOn(60, 1.0f);
                }
            }
        }
    }

    /*md **Config**: */
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

        /*md - `OUTLINE: true/false` is if the envelop should be outlined (default: true). */
        if (strcmp(key, "OUTLINE") == 0) {
            outline = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `FILLED: true/false` is if the envelop should be filled (default: true). */
        if (strcmp(key, "FILLED") == 0) {
            filled = strcmp(value, "true") == 0;
            return true;
        }

        /*md - `ENCODER_PHASE: id` is the id of the encoder to control the phase. */
        if (strcmp(key, "ENCODER_PHASE") == 0) {
            encoderPhase = atoi(value);
            return true;
        }

        /*md - `ENCODER_TIME: id` is the id of the encoder to control the time. */
        if (strcmp(key, "ENCODER_TIME") == 0) {
            encoderTime = atoi(value);
            return true;
        }

        /*md - `ENCODER_MODULATION: id` is the id of the encoder to control the modulation. */
        if (strcmp(key, "ENCODER_MODULATION") == 0) {
            encoderModulation = atoi(value);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `FILL_COLOR: color` is the color of the envelop. */
        /*md - `OUTLINE_COLOR: color` is the color of the envelop outline. */
        /*md - `TEXT_COLOR: color` is the color of the text. */
        /*md - `CURSOR_COLOR: color` is the color of the cursor. */
        /*md - `INACTIVE_COLOR_RATIO: 0.0 - 1.0` is the ratio of darkness for the inactive color (default: 0.5). */
        return GroupColorComponent::config(key, value);
    }
};

#endif
