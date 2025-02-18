#pragma once

#include "helpers/range.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include <stdexcept>

/*md
## Drum envelop

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/DrumEnvelop.png" />

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
    Point envPosition = { 0, 0 };

    bool filled = true;
    bool outline = true;

    Color bgColor;

    Color fillColor;
    Color outlineColor;
    Color textColor;
    Color cursorColor;

    int encoderPhase = -1;
    int encoderTime = -1;
    int encoderModulation = -1;

    bool renderValuesOnTop = true;

    int fontSize = 8;

    void updateGraphHeight()
    {
        if (renderValuesOnTop) {
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
            draw.filledPolygon(points, { fillColor });
        }
        if (outline) {
            draw.lines(points, { outlineColor });
        }
    }

    void renderEditStep()
    {
        int cursorY = envPosition.y + envelopHeight + 5;
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
        if (renderValuesOnTop) {
            int cellWidth = size.w / 3;
            int x = relativePosition.x + cellWidth * 0.5;
            draw.textCentered({ x, relativePosition.y }, std::to_string(currentstep + 1) + "/" + std::to_string(envData->size()), fontSize, { textColor });
            draw.textCentered({ x + cellWidth, relativePosition.y }, std::to_string(currentTimeMs) + "ms", fontSize, { textColor });
            draw.textCentered({ x + cellWidth * 2, relativePosition.y }, std::to_string((int)(currentMod * 100)) + "%", fontSize, { textColor });
        } else {
            int x = relativePosition.x;
            draw.text({ x + 2, relativePosition.y }, std::to_string(currentTimeMs) + "ms", fontSize, { textColor });
            draw.textRight({ x + size.w - 2, relativePosition.y }, std::to_string((int)(currentMod * 100)) + "%", fontSize, { textColor });
            draw.text({ x + 2, relativePosition.y + size.h - 8 }, std::to_string(currentstep + 1) + "/" + std::to_string(envData->size()), fontSize, { textColor });
        }
    }

public:
    DrumEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , cursorColor(styles.colors.text)
        , fillColor(styles.colors.primary)
        , outlineColor(lighten(styles.colors.primary, 0.5))
    {
        updateGraphHeight();

        /*md md_config:DrumEnvelop */
        nlohmann::json config = props.config;

        /*md   // The audio plugin to get control on. */
        /*md   audioPlugin="audio_plugin_name" */
        if (!config.contains("audioPlugin")) {
            logWarn("DrumEnvelop component is missing audioPlugin parameter.");
            return;
        }
        plugin = &getPlugin(config["audioPlugin"].get<std::string>().c_str(), track);

        /*md   // Envelop data id to get/set data in the audio plugin. */
        /*md   envelopDataId="ENV_AMP" */
        if (!config.contains("envelopDataId")) {
            logWarn("DrumEnvelop component is missing envelopDataId parameter.");
            return;
        }
        uint8_t id = plugin->getDataId(config["envelopDataId"].get<std::string>().c_str());
        envData = (std::vector<Data>*)plugin->data(id);
        currentStepDataId = id + 1; // This is a bit cluncky, should try to find something better...
        timeDataId = id + 2;
        modDataId = id + 3;

        /*md   // Set if the envelop should be outlined or not. (default: true) */
        /*md   outline={false} */
        outline = config.value("outline", outline);

        /*md   // Set if the envelop should be filled or not. (default: true) */
        /*md   filled={false} */
        filled = config.value("filled", filled);

        /*md   // Set if the title should be rendered on top or not. (default: true) */
        /*md   renderTitleOnTop={false} */
        renderValuesOnTop = config.value("renderTitleOnTop", renderValuesOnTop);

        /*md   // Set the color of the graph. */
        /*md   fillColor="#000000" */
        fillColor = draw.getColor(config["fillColor"], fillColor);

        /*md   // Set the color of the outline. */
        /*md   outlineColor="#000000" */
        outlineColor = draw.getColor(config["outlineColor"], outlineColor);

        //md   // Set the color of the cursor.
        cursorColor = draw.getColor(config["cursorColor"], cursorColor); //eg: "#ffffff"

        /// Set the color of the text.
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#ffffff"

        //md   // Set the id of the encoder to change time parameter.
        encoderTime = config.value("encoderTime", encoderTime); //eg: 0

        //md   // Set the id of the encoder to change phase parameter.
        encoderPhase = config.value("encoderPhase", encoderPhase); //eg: 0

        //md   // Set the id of the encoder to change modulation parameter.
        //md   encoderModulation={0}
        encoderModulation = config.value("encoderModulation", encoderModulation);

        /*md md_config_end */
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
};
