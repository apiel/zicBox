#pragma once

#include "helpers/clamp.h"
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
        float incRatio;
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
    Color titleColor;
    Color cursorColor;

    std::string title = "";

    int encoders[4] = { 0, 1, 2, 3 };

    int fontSize = 12;
    int margin = 6;

    void updateGraphHeight()
    {
        envelopHeight = size.h - ((fontSize + margin) * 2);
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
            draw.text({ x + 2, relativePosition.y + size.h - (fontSize + margin - 4) }, std::to_string((int)(macro2 * 100)) + "%", fontSize, { textColor });
            draw.textRight({ x + size.w - 2, relativePosition.y + size.h - (fontSize + margin - 4) }, std::to_string((int)(macro3 * 100)) + "%", fontSize, { textColor });
        } else {
            draw.textRight({ x + size.w - 2, relativePosition.y }, std::to_string((int)macro1 + 1) + "/" + std::to_string(envData->size()), fontSize, { textColor });
            draw.text({ x + 2, relativePosition.y + size.h - (fontSize + margin - 4) }, std::to_string((int)(macro2 * 100)) + "%", fontSize, { textColor });
            draw.textRight({ x + size.w - 2, relativePosition.y + size.h - (fontSize + margin - 4) }, std::to_string((int)macro3) + "ms", fontSize, { textColor });
        }

        if (title != "") {
            draw.textCentered({ x + size.w / 2, relativePosition.y + size.h - (fontSize + margin - 4) }, title, fontSize, { titleColor });
        }
    }

public:
    MacroEnvelopComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , textColor(styles.colors.text)
        , titleColor(alpha(styles.colors.text, 0.4))
        , cursorColor(styles.colors.text)
        , fillColor(styles.colors.primary)
        , outlineColor(lighten(styles.colors.primary, 0.5))
    {
        updateGraphHeight();

        /*md md_config:MacroEnvelop */
        nlohmann::json& config = props.config;

        /// The audio plugin to get control on.
        plugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        /// Envelop data id to get/set data in the audio plugin.
        uint8_t id = plugin->getDataId(getConfig(config, "envelopDataId")); //eg: "ENV_AMP"
        envData = (std::vector<Data>*)plugin->data(id);
        modeDataId = id + 1; // A bit cluncky, need to find better solution..
        isMacroDataId = id + 2;
        macro1DataId = id + 3;
        macro2DataId = id + 4;
        macro3DataId = id + 5;

        isMacro = (bool*)plugin->data(isMacroDataId);

        modePtr = (std::string*)plugin->data(modeDataId);
        macro1 = *(float*)plugin->data(macro1DataId);
        macro2 = *(float*)plugin->data(macro2DataId);
        macro3 = *(float*)plugin->data(macro3DataId);

        /// Set if the envelop should be outlined or not. (default: true)
        outline = config.value("outline", outline); //eg: false

        /// Set if the envelop should be filled or not. (default: true)
        filled = config.value("filled", filled); //eg: false

        /// Set the color of the graph.
        fillColor = draw.getColor(config["fillColor"], fillColor); //eg: "#ffffff"

        /// Set the color of the outline.
        outlineColor = draw.getColor(config["outlineColor"], outlineColor); //eg: "#000000"

        /// Set the color of the cursor.
        cursorColor = draw.getColor(config["cursorColor"], cursorColor); //eg: "#000000"

        /// Set the color of the text.
        textColor = draw.getColor(config["textColor"], textColor); //eg: "#000000"

        /// Set the color of the title.
        titleColor = draw.getColor(config["titleColor"], titleColor); //eg: "#000000"

        /// Set the id of the encoder to change the envelop prarameters.
        if (config.contains("encoders") && config["encoders"].is_array()) { //eg: [0, 1, 2, 3]
            encoders[0] = config["encoders"][0].get<int>();
            encoders[1] = config["encoders"][1].get<int>();
            encoders[2] = config["encoders"][2].get<int>();
            encoders[3] = config["encoders"][3].get<int>();
        }

        /// Set title to be displayed on bttom of the envelop.
        title = config.value("title", title); //eg: "Macro Envelop"

        /*md md_config_end */
    }

    void render() override
    {
        envPosition = { relativePosition.x, relativePosition.y + fontSize + 5 };
        draw.filledRect(relativePosition, size, { bgColor });

        if (envData && envData->size() > 0) {
            renderEnvelop();
            renderTitles();

            if (!(*isMacro)) {
                renderEditStep();
            }
        }
    }

    void onEncoder(int id, int8_t direction) override
    {
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
};
