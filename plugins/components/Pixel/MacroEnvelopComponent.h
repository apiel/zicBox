#pragma once

#include "helpers/range.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include <stdexcept>

/*md
## Macro envelop

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/MacroEnvelop.png" />

Show a representation of a macro envelop (envelop with relative time and modulation, without sustain).
*/

class MacroEnvelopComponent : public Component {
protected:
    struct Data {
        float modulation;
        float time;
    };

    AudioPlugin* plugin = NULL;
    std::vector<Data>* envData = NULL;
    uint8_t modeDataId = -1;
    uint8_t isMacroDataId = -1;
    uint8_t macro1DataId = -1;
    uint8_t macro2DataId = -1;
    uint8_t macro3DataId = -1;
    std::string* modePtr = NULL;
    float macro1 = 0.0f;
    float macro2 = 0.0f;
    float macro3 = 0.0f;
    bool* isMacro = NULL;

    int envelopHeight = 30;
    Point envPosition = { 0, 0 };

    bool filled = true;
    bool outline = true;

    Color bgColor;

    Color fillColor;
    Color outlineColor;
    Color textColor;
    Color cursorColor;

    int encoders[4] = { 0, 1, 2, 3 };

    int fontSize = 8;

    void updateGraphHeight()
    {
        envelopHeight = size.h - 6 - 10 - 9;
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
        // points.push_back({ envPosition.x, envPosition.y + envelopHeight });

        if (data.modulation != 0.0f) {
            points.push_back({ (int)(envPosition.x + size.w), envPosition.y + envelopHeight });
        }

        if (filled) {
            draw.filledPolygon(points, { fillColor });
        }
        if (outline) {
            draw.lines(points, { outlineColor });
        }
    }

    void renderEditStep()
    {
        int cursorY = envPosition.y + envelopHeight + 5;
        int8_t currentstep = macro1;
        if (currentstep < envData->size() - 1) {
            float currentTime = envData->at(currentstep).time;
            float nextTime = envData->at(currentstep + 1).time;
            draw.line({ (int)(relativePosition.x + size.w * currentTime), cursorY }, { (int)(relativePosition.x + size.w * currentTime), cursorY - 3 }, { cursorColor });
            draw.line({ (int)(relativePosition.x + size.w * currentTime), cursorY - 1 }, { (int)(relativePosition.x + size.w * nextTime), cursorY - 1 }, { cursorColor });
            draw.line({ (int)(relativePosition.x + size.w * nextTime), cursorY }, { (int)(relativePosition.x + size.w * nextTime), cursorY - 3 }, { cursorColor });
        } else {
            draw.line({ relativePosition.x + size.w, cursorY }, { relativePosition.x + size.w, cursorY - 3 }, { cursorColor });
        }
    }

    void renderTitles()
    {
        int x = relativePosition.x;
        if (modePtr != NULL) {
            draw.text({ x + 2, relativePosition.y }, modePtr->c_str(), fontSize, { textColor });
        }

        if (*isMacro) {
            draw.textRight({ x + size.w - 2, relativePosition.y }, std::to_string((int)(macro1 * 100)) + "%", fontSize, { textColor });
            draw.text({ x + 2, relativePosition.y + size.h - 8 }, std::to_string((int)(macro2 * 100)) + "%", fontSize, { textColor });
            draw.textRight({ x + size.w - 2, relativePosition.y + size.h - 8 }, std::to_string((int)(macro3 * 100)) + "%", fontSize, { textColor });
        } else {
            draw.text({ x + size.w - 2, relativePosition.y }, std::to_string((int)macro1 + 1) + "/" + std::to_string(envData->size()), fontSize, { textColor });
            draw.textRight({ x + 2, relativePosition.y + size.h - 8 }, std::to_string((int)(macro2 * 100)) + "%", fontSize, { textColor });
            draw.textRight({ x + size.w - 2, relativePosition.y + size.h - 8 }, std::to_string((int)macro3) + "ms", fontSize, { textColor });
        }
    }

public:
    MacroEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , cursorColor(styles.colors.text)
        , fillColor(styles.colors.primary)
        , outlineColor(lighten(styles.colors.primary, 0.5))
    {
        updateGraphHeight();
    }

    void render() override
    {
        if (updatePosition()) {
            envPosition = { relativePosition.x, relativePosition.y + 10 };
            draw.filledRect(relativePosition, size, { bgColor });

            if (envData) {
                renderEnvelop();
                renderTitles();

                if (!(*isMacro)) {
                    renderEditStep();
                }
            }
        }
    }

    bool isActive = true;
    void onGroupChanged(int8_t index) override
    {
        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != isActive) {
            isActive = shouldActivate;
            renderNext();
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
        if (isActive) {
            if (id == encoders[0]) {
                modePtr = (std::string*)plugin->data(modeDataId, &direction);
                macro1 = *(float*)plugin->data(macro1DataId);
                macro2 = *(float*)plugin->data(macro2DataId);
                macro3 = *(float*)plugin->data(macro3DataId);
                renderNext();
            } else if (id == encoders[1]) {
                macro1 = *(float*)plugin->data(macro1DataId, &direction);
                renderNext();
            } else if (id == encoders[2]) {
                macro2 = *(float*)plugin->data(macro2DataId, &direction);
                renderNext();
            } else if (id == encoders[3]) {
                macro3 = *(float*)plugin->data(macro3DataId, &direction);
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
                throw std::runtime_error("MacroEnvelopComponent cannot set ENVELOP_DATA_ID: plugin is not set");
            }
            uint8_t id = plugin->getDataId(value);
            envData = (std::vector<Data>*)plugin->data(id);

            modeDataId = id + 1;
            isMacroDataId = id + 2;
            macro1DataId = id + 3;
            macro2DataId = id + 4;
            macro3DataId = id + 5;

            isMacro = (bool*)plugin->data(isMacroDataId);

            modePtr = (std::string*)plugin->data(modeDataId);
            macro1 = *(float*)plugin->data(macro1DataId);
            macro2 = *(float*)plugin->data(macro2DataId);
            macro3 = *(float*)plugin->data(macro3DataId);

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

        /*md - `ENCODER_PHASE: mode macro1 macro2 macro3` is the id of the encoders. */
        if (strcmp(key, "ENCODERS") == 0) {
            encoders[0] = atoi(strtok(value, " "));
            encoders[1] = atoi(strtok(NULL, " "));
            encoders[2] = atoi(strtok(NULL, " "));
            encoders[3] = atoi(strtok(NULL, " "));
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(value);
            return true;
        }

        /*md - `FILL_COLOR: color` is the color of the envelop. */
        if (strcmp(key, "FILL_COLOR") == 0) {
            fillColor = draw.getColor(value);
            return true;
        }

        /*md - `OUTLINE_COLOR: color` is the color of the envelop outline. */
        if (strcmp(key, "OUTLINE_COLOR") == 0) {
            outlineColor = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: color` is the color of the text. */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            textColor = draw.getColor(value);
            return true;
        }

        /*md - `CURSOR_COLOR: color` is the color of the cursor. */
        if (strcmp(key, "CURSOR_COLOR") == 0) {
            cursorColor = draw.getColor(value);
            return true;
        }

        return false;
    }
};
