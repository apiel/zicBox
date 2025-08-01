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

    bool useStringValue = false;
    uint8_t floatPrecision = 0;

    std::string getValStr()
    {
        if (useStringValue) {
            return val->string();
        }
        std::string valStr = std::to_string(val->get());
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
        if (val != NULL) {
            useStringValue = val->hasType(VALUE_STRING);
            floatPrecision = val->props().floatingPoint;
        }

        labelFontSize = config.value("labelSize", labelFontSize); //eg: 6

        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        labelColor = draw.getColor(config["labelColor"], labelColor); //eg: "#FF0000"
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (val != NULL) {
            draw.text({ relativePosition.x, relativePosition.y }, getValStr(), labelFontSize, { labelColor, .font = fontLabel });
        }
    }
};
