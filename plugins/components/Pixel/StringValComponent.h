#pragma once

#include "log.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

class StringValComponent : public Component {
protected:
    Color bgColor;
    Color labelColor;

    int labelFontSize = 16;
    void* fontLabel = NULL;

    ValueInterface* val = NULL;

    bool showUnit = false;

    std::string getValStr()
    {
        if (showUnit) {
            return val->props().unit;
        }

        if (val->hasType(VALUE_STRING)) {
            return val->string();
        }
        std::string valStr = std::to_string(val->get());
        float floatPrecision = val->props().floatingPoint;
        valStr = valStr.substr(0, valStr.find(".") + floatPrecision + (floatPrecision > 0 ? 1 : 0));
        return valStr;
    }

public:
    StringValComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , labelColor(darken(styles.colors.text, 0.5))
    {
        fontLabel = draw.getFont("PoppinsLight_16");

        nlohmann::json& config = props.config;

        AudioPlugin* audioPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        std::string param = getConfig(config, "param"); //eg: "parameter_name"

        val = watch(audioPlugin->getValue(param));

        if (config.contains("fontLabel")) {
            fontLabel = draw.getFont(config["fontLabel"].get<std::string>().c_str()); //eg: "PoppinsLight_8"
            int fontSize = draw.getDefaultFontSize(fontLabel);
            if (fontSize > 0) {
                labelFontSize = fontSize;
            }
        }

        labelFontSize = config.value("labelSize", labelFontSize); //eg: 6

        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        labelColor = draw.getColor(config["labelColor"], labelColor); //eg: "#FF0000"

        showUnit = config.value("unit", showUnit);
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (val != NULL) {
            draw.text({ relativePosition.x, relativePosition.y }, getValStr(), labelFontSize, { labelColor, .font = fontLabel });
        }
    }
};
