#pragma once

#include "log.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## Value

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/value.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/value2.png" />

Value component is used to display an audio plugin parameter value.
*/

class ValueComponent : public Component {
protected:
    Color bgColor;
    Color valueColor;
    Color barColor;
    Color unitColor;
    Color labelColor;

    int valueFontSize = 12;
    int labelFontSize = 8;
    int unitFontSize = 12;
    int maxFontSize = 8;
    void* fontValue = NULL;
    void* fontLabel = NULL;
    int fontHeightValue = 0;
    int valueH = 12;
    int barH = 2;
    int barBgH = 1;

    ValueInterface* val = NULL;
    int8_t encoderId = -1;

    bool showValue = true;
    bool showUnit = true;
    bool showLabel = true;
    int showLabelOverValue = -1;
    int labelOverValueX = -1;
    bool useNumberValue = false;
    bool verticalAlignCenter = false;
    bool alignLeft = false;
    std::string label;

    std::string getLabel()
    {
        if (!label.length()) {
            return val->props().label;
        }
        return label;
    }

    void setMaxFontSize()
    {
        valueH = fontHeightValue == 0 ? valueFontSize : fontHeightValue;
        if (showValue) {
            maxFontSize = std::max({ valueH, labelFontSize, unitFontSize });
        } else {
            maxFontSize = std::max({ labelFontSize, unitFontSize });
        }
    }

    std::string getValStr()
    {
        if (!useNumberValue && val->hasType(VALUE_STRING)) {
            return val->string();
        }
        std::string valStr = std::to_string(val->get());
        float floatPrecision = val->props().floatingPoint;
        valStr = valStr.substr(0, valStr.find(".") + floatPrecision + (floatPrecision > 0 ? 1 : 0));
        return valStr;
    }

public:
    ValueComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , valueColor(styles.colors.text)
        , barColor(lighten(styles.colors.background, 0.5))
        , unitColor(darken(styles.colors.text, 0.5))
        , labelColor(darken(styles.colors.text, 0.5))
    {
        fontLabel = draw.getFont("PoppinsLight_8");

        /*md md_config:KnobValue */
        nlohmann::json& config = props.config;

        /// The audio plugin to get control on.
        AudioPlugin* audioPlugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        /// The audio plugin key parameter to get control on.
        std::string param = getConfig(config, "param"); //eg: "parameter_name"

        val = watch(audioPlugin->getValue(param));

        /// The encoder id that will interract with this component.
        encoderId = config.value("encoderId", encoderId); //eg: 0

        /// Override the label of the parameter.
        if (config.contains("label")) {
            label = config["label"].get<std::string>(); //eg: "custom_label"
        }

        /// Use the number value instead of string value (default: false).
        useNumberValue = config.value("useNumberValue", useNumberValue); //eg: true

        /// Set the bar height (default: 2).
        barH = config.value("barHeight", barH); //eg: 2

        /// Set the bar background height (default: 1).
        barBgH = config.value("barBgHeight", barBgH); //eg: 1

        /// Set the text vertical alignment to center (default: false).
        verticalAlignCenter = config.value("verticalAlignCenter", verticalAlignCenter); //eg: true

        /// Set the text horizontal alignment to left (default: false).
        alignLeft = config.value("alignLeft", alignLeft); //eg: true

        /// Hide the label (default: false).
        showLabel = !config.value("hideLabel", !showLabel); //eg: true

        /// Hide the unit (default: false).
        showUnit = !config.value("hideUnit", !showUnit); //eg: true

        /// Hide the value (default: false).
        showValue = !config.value("hideValue", !showValue); //eg: true

        /// Shows the label over the value, 4 px from the top of the component (default: -1) `
        showLabelOverValue = config.value("showLabelOverValue", showLabelOverValue); //eg: 4

        /// Set the distance from the left of the component where the label will be placed. If -1 it is centered (default: -1)
        labelOverValueX = config.value("labelOverValueX", labelOverValueX); //eg: 0

        /// The font of the value. Default is null.
        if (config.contains("fontValue")) {
            fontValue = draw.getFont(config["fontValue"].get<std::string>().c_str()); //eg: "PoppinsLight_8"
            int fontSize = draw.getDefaultFontSize(fontValue);
            if (fontSize > 0) {
                valueFontSize = fontSize;
            }
        }

        /// The font of the label. Default is null.
        if (config.contains("fontLabel")) {
            fontLabel = draw.getFont(config["fontLabel"].get<std::string>().c_str()); //eg: "PoppinsLight_8"
            int fontSize = draw.getDefaultFontSize(fontLabel);
            if (fontSize > 0) {
                labelFontSize = fontSize;
            }
        }

        /// Set the font size of the value (default: 8).
        valueFontSize = config.value("valueSize", valueFontSize); //eg: 8

        /// Set the font size of the label (default: 6).
        labelFontSize = config.value("labelSize", labelFontSize); //eg: 6

        /// Set the font size of the unit (default: 6).
        unitFontSize = config.value("unitSize", unitFontSize); //eg: 6

        /// The font height of the text. Default is 0.
        fontHeightValue = config.value("valueHeight", fontHeightValue); //eg: 16
        setMaxFontSize();

        /// Set the background color of the component.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"

        /// Set the color of the label.
        valueColor = draw.getColor(config["valueColor"], valueColor); //eg: "#FF0000"

        /// Set the color of the bar.
        barColor = draw.getColor(config["barColor"], barColor); //eg: "#FF0000"

        /// Set the color of the unit.
        unitColor = draw.getColor(config["unitColor"], unitColor); //eg: "#FF0000"

        /// Set the color of the label.
        labelColor = draw.getColor(config["labelColor"], labelColor); //eg: "#FF0000"

        /*md md_config_end */
    }

    void renderBar()
    {
        draw.filledRect({ relativePosition.x, relativePosition.y }, { size.w, barBgH }, { darken(barColor, 0.4) });
        if (val->hasType(VALUE_CENTERED)) {
            float valPct = val->pct();
            if (valPct < 0.5) {
                int w = size.w * (0.5 - valPct);
                draw.filledRect({ relativePosition.x + (int)(size.w * 0.5) - w, relativePosition.y }, { w, barH }, { barColor });
            } else {
                draw.filledRect({ relativePosition.x + (int)(size.w * 0.5), relativePosition.y }, { (int)(size.w * (valPct - 0.5)), barH }, { barColor });
            }
        } else {
            draw.filledRect({ relativePosition.x, relativePosition.y }, { (int)(size.w * val->pct()), barH }, { barColor });
        }
    }

    void render() override
    {
        draw.filledRect(relativePosition, size, { bgColor });
        if (val != NULL) {
            if (barH > 0) {
                renderBar();
            }

            int x = relativePosition.x + (size.w) * 0.5;

            if (alignLeft) {
                x = relativePosition.x + 2;
            } else if (showLabel && showValue) {
                x -= labelFontSize * val->props().label.length() + 2;
            }

            int textY = verticalAlignCenter ? (size.h - maxFontSize) * 0.5 + relativePosition.y : ((size.h - maxFontSize) * 0.7) + relativePosition.y;
            // Put all text in the same line
            int labelY = textY + maxFontSize - labelFontSize;
            int valueY = textY + maxFontSize - valueH;
            int unitY = textY + maxFontSize - unitFontSize;

            if (showLabelOverValue != -1) {
                if (alignLeft) {
                    draw.text({ x, relativePosition.y + showLabelOverValue }, getLabel(), labelFontSize, { labelColor, .font = fontLabel });
                } else {
                    draw.textCentered({ labelOverValueX == -1 ? x : relativePosition.x + labelOverValueX, relativePosition.y + showLabelOverValue }, getLabel(), labelFontSize, { labelColor, .font = fontLabel });
                }
            } else if (showLabel) {
                x = draw.text({ x, labelY }, getLabel(), labelFontSize, { labelColor, .font = fontLabel }) + 2;
            }

            if (showValue) {
                x = showLabel ? draw.text({ x, valueY }, getValStr(), valueFontSize, { valueColor, .font = fontValue, .maxWidth = size.w - 4, .fontHeight = fontHeightValue })
                              : draw.textCentered({ x, valueY }, getValStr(), valueFontSize, { valueColor, .font = fontValue, .maxWidth = size.w - 4, .fontHeight = fontHeightValue });
                if (showUnit && val->props().unit.length() > 0) {
                    draw.text({ x + 2, unitY }, val->props().unit, unitFontSize, { unitColor });
                }
            }
        }
    }

    void onEncoder(int8_t id, int8_t direction) override
    {
        if (val && id == encoderId) {
            val->increment(direction);
        }
    }

    const std::vector<EventInterface::EncoderPosition> getEncoderPositions() override
    {
        if (encoderId < 0) {
            return {};
        }
        
        return {
            { encoderId, size, relativePosition },
        };
    }
};
