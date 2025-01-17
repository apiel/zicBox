#ifndef _UI_PIXEL_COMPONENT_DRUM_ENV_H_
#define _UI_PIXEL_COMPONENT_DRUM_ENV_H_

#include "../../../helpers/range.h"
#include "../component.h"
#include "../utils/color.h"
#include "utils/GroupColorComponent.h"
#include <stdexcept>

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
    Point envPosition = { 0, 0 };

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

    bool renderTitleOnTop = true;

    int fontSize = 8;

    void updateGraphHeight()
    {
        if (renderTitleOnTop) {
            envelopHeight = size.h - 6 - 10;
        } else {
            envelopHeight = size.h - 6 - 10 - 9;
        }
    }

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
        int cursorY = envPosition.y + envelopHeight + 5;
        if (currentstep < envData->size() - 1) {
            float currentTime = envData->at(currentstep).time;
            float nextTime = envData->at(currentstep + 1).time;
            draw.line({ (int)(relativePosition.x + size.w * currentTime), cursorY }, { (int)(relativePosition.x + size.w * currentTime), cursorY - 3 }, { cursorColor.color });
            draw.line({ (int)(relativePosition.x + size.w * currentTime), cursorY - 1 }, { (int)(relativePosition.x + size.w * nextTime), cursorY - 1 }, { cursorColor.color });
            draw.line({ (int)(relativePosition.x + size.w * nextTime), cursorY }, { (int)(relativePosition.x + size.w * nextTime), cursorY - 3 }, { cursorColor.color });
        } else {
            draw.line({ relativePosition.x + size.w, cursorY }, { relativePosition.x + size.w, cursorY - 3 }, { cursorColor.color });
        }
    }

    void renderTitles()
    {
        if (renderTitleOnTop) {
            int cellWidth = size.w / 3;
            int x = relativePosition.x + cellWidth * 0.5;
            draw.textCentered({ x, relativePosition.y }, std::to_string(currentstep + 1) + "/" + std::to_string(envData->size()), fontSize, { textColor.color });
            draw.textCentered({ x + cellWidth, relativePosition.y }, std::to_string(currentTimeMs) + "ms", fontSize, { textColor.color });
            draw.textCentered({ x + cellWidth * 2, relativePosition.y }, std::to_string((int)(currentMod * 100)) + "%", fontSize, { textColor.color });
        } else {
            int x = relativePosition.x;
            draw.text({ x + 2, relativePosition.y }, std::to_string(currentTimeMs) + "ms", fontSize, { textColor.color });
            draw.textRight({ x + size.w - 2, relativePosition.y }, std::to_string((int)(currentMod * 100)) + "%", fontSize, { textColor.color });
            draw.text({ x + 2, relativePosition.y + size.h - 8 }, std::to_string(currentstep + 1) + "/" + std::to_string(envData->size()), fontSize, { textColor.color });
        }
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
        updateGraphHeight();
    }

    void render() override
    {
        if (updatePosition()) {
            envPosition = { relativePosition.x, relativePosition.y + 10 };
            draw.filledRect(relativePosition, size, { bgColor });

            if (envData) {
                currentstep = *(int8_t*)plugin->data(currentStepDataId);
                currentMod = *(float*)plugin->data(modDataId);
                currentTimeMs = *(uint16_t*)plugin->data(timeDataId);

                renderEnvelop();
                renderEditStep();
                renderTitles();
            }
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
            if (plugin == NULL) {
                throw std::runtime_error("DrumEnvelopComponent cannot set ENVELOP_DATA_ID: plugin is not set");
            }
            uint8_t id = plugin->getDataId(value);
            envData = (std::vector<Data>*)plugin->data(id);
            currentStepDataId = id + 1;
            timeDataId = id + 2;
            modDataId = id + 3;
            return true;
        }

        /*md - `STEP_DATA_ID: id` is the data id to get/set the current step/phase to edit.*/
        if (strcmp(key, "STEP_DATA_ID") == 0) {
            if (plugin == NULL) {
                throw std::runtime_error("DrumEnvelopComponent cannot set STEP_DATA_ID: plugin is not set");
            }
            currentStepDataId = plugin->getDataId(value);
            return true;
        }

        /*md - `TIME_DATA_ID: id` is the data id to get/set the step to time.*/
        if (strcmp(key, "TIME_DATA_ID") == 0) {
            if (plugin == NULL) {
                throw std::runtime_error("DrumEnvelopComponent cannot set TIME_DATA_ID: plugin is not set");
            }
            timeDataId = plugin->getDataId(value);
            return true;
        }

        /*md - `MOD_DATA_ID: id` is the data id to get/set the step to mod.*/
        if (strcmp(key, "MOD_DATA_ID") == 0) {
            if (plugin == NULL) {
                throw std::runtime_error("DrumEnvelopComponent cannot set MOD_DATA_ID: plugin is not set");
            }
            modDataId = plugin->getDataId(value);
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

        /*md - `RENDER_TITLE_ON_TOP: true/false` is if the title should be rendered on top (default: true). */
        if (strcmp(key, "RENDER_TITLE_ON_TOP") == 0) {
            renderTitleOnTop = strcmp(value, "true") == 0;
            updateGraphHeight();
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
