/** Description:
This code defines a specialized building block for a graphical interface, called the "String Value Component." Think of it as a customizable digital label often used in audio software or plugins to show a single, dynamic setting.

**Core Purpose:**
This component’s main job is to retrieve a fluctuating value (like a volume level, a frequency, or a status text) from the underlying application logic and format it for screen display.

**Data Handling and Formatting:**
The component is configured to "watch" a specific parameter within an external audio system or plugin. If the watched value is a number, the component automatically converts it into a clean string, ensuring it displays the correct number of decimal places based on the setting's precision. It can also be instructed to display only the unit (e.g., "Hz" or "dB") instead of the numerical figure.

**Customization:**
Every aspect of the component’s appearance is configurable upon creation, including the background color, the text color, the font size, and the exact font style used for the label.

**Display Logic:**
When the component draws itself on screen, it first paints its background using the assigned color. Then, it fetches the latest formatted value and draws that text on top, using the specified custom colors and font. This ensures the displayed information is always current and visually distinct.

sha: f991431b2d17bb54772c31398925b155ccc085d381c72ee5abb525a19ba948cb 
*/
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
