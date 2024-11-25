#ifndef _UI_PIXEL_COMPONENT_DRUM_ENV_H_
#define _UI_PIXEL_COMPONENT_DRUM_ENV_H_

#include "../../../helpers/range.h"
#include "../component.h"
#include "../utils/color.h"
#include "utils/ToggleColor.h"

/*md
## Drum envelop

Show a representation of a drum envelop (envelop with relative time and modulation, without sustain).
*/

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

    bool filled = true;
    bool outline = true;


    Color bgColor;

    const float inactiveColorRatio = 0.5f; // Must be define before ToggleColor, could also use #define
    ToggleColor fillColor;
    ToggleColor outlineColor;
    ToggleColor textColor;
    ToggleColor cursorColor;


    bool encoderActive = true;
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
            draw.line({ (int)(position.x + size.w * currentTime), cursorY }, { (int)(position.x + size.w * currentTime), cursorY - 3 }, { cursorColor.color });
            draw.line({ (int)(position.x + size.w * currentTime), cursorY - 1 }, { (int)(position.x + size.w * nextTime), cursorY - 1 }, { cursorColor.color });
            draw.line({ (int)(position.x + size.w * nextTime), cursorY }, { (int)(position.x + size.w * nextTime), cursorY - 3 }, { cursorColor.color });
        } else {
            draw.line({ position.x + size.w, cursorY }, { position.x + size.w, cursorY - 3 }, { cursorColor.color });
        }
    }

    void renderTitles()
    {
        int cellWidth = size.w / 3;
        int x = position.x + cellWidth * 0.5;
        draw.textCentered({ x, position.y }, std::to_string(currentstep + 1) + "/" + std::to_string(envData->size()), 8, { textColor.color });
        draw.textCentered({ x + cellWidth, position.y }, std::to_string(currentTimeMs) + "ms", 8, { textColor.color });
        draw.textCentered({ x + cellWidth * 2, position.y }, std::to_string((int)(currentMod * 100)) + "%", 8, { textColor.color });
    }

    void updateColors() {
        fillColor.toggle(encoderActive);
        outlineColor.toggle(encoderActive);
        textColor.toggle(encoderActive);
        cursorColor.toggle(encoderActive);
    }

public:
    DrumEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text, inactiveColorRatio)
        , cursorColor(styles.colors.text, inactiveColorRatio)
        , fillColor(styles.colors.primary, inactiveColorRatio)
        , outlineColor(lighten(styles.colors.primary, 0.5), inactiveColorRatio)
    {
        updateColors();

        envPosition = { position.x, position.y + 10 };
        envelopHeight = size.h - 6 - 10;
        cursorY = position.y + size.h - 1;
    }

    void render() override
    {
        draw.filledRect(position, size, { bgColor });

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
        if (encoderActive) {
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
            }
        }
    }

    void onGroupChanged(int8_t index) override
    {
        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != encoderActive) {
            encoderActive = shouldActivate;
            updateColors();
            renderNext();
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

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `FILL_COLOR: color` is the color of the envelop. */
        if (strcmp(key, "FILL_COLOR") == 0) {
            fillColor.setColor(draw.getColor(value), inactiveColorRatio);
            return true;
        }

        /*md - `OUTLINE_COLOR: color` is the color of the envelop outline. */
        if (strcmp(key, "OUTLINE_COLOR") == 0) {
            outlineColor.setColor(draw.getColor(value), inactiveColorRatio);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor.setColor(draw.getColor(value), inactiveColorRatio);
            return true;
        }

        /*md - `CURSOR_COLOR: color` is the color of the cursor. */
        if (strcmp(key, "CURSOR_COLOR") == 0) {
            cursorColor.setColor(draw.getColor(value), inactiveColorRatio);
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

        return false;
    }
};

#endif
