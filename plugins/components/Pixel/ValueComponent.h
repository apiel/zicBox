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

    int valueFontSize = 8;
    int labelFontSize = 6;
    int unitFontSize = 6;
    int maxFontSize = 8;
    void* font = NULL;
    int fontHeightValue = 0;
    int valueH = 8;
    int barH = 2;
    int barBgH = 1;

    ValueInterface* val = NULL;
    int8_t encoderId = -1;
    uint8_t floatPrecision = 0;

    bool showValue = true;
    bool showUnit = true;
    bool showLabel = true;
    int showLabelOverValue = -1;
    int labelOverValueX = -1;
    bool useStringValue = false;
    bool verticalAlignCenter = false;
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
        if (useStringValue) {
            return val->string();
        }
        std::string valStr = std::to_string(val->get());
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

        /*md md_config:KnobValue */
        nlohmann::json config = props.config;

        /*md   // The audio plugin to get control on. */
        /*md   audioPlugin="audio_plugin_name" */
        if (!config.contains("audioPlugin")) {
            logWarn("KnobValue component is missing audioPlugin parameter.");
            return;
        }
        std::string audioPlugin = config["audioPlugin"].get<std::string>();

        /*md   // The audio plugin key parameter to get control on. */
        /*md   param="parameter_name" */
        if (!config.contains("param")) {
            logWarn("KnobValue component is missing param parameter.");
            return;
        }
        std::string param = config["param"].get<std::string>();

        val = watch(getPlugin(audioPlugin.c_str(), track).getValue(param));
        if (val != NULL) {
            floatPrecision = val->props().floatingPoint;
        }
        useStringValue = val->hasType(VALUE_STRING);

        /*md   // The encoder id that will interract with this component. */
        /*md   encoderId={0} */
        encoderId = config.value("encoderId", encoderId);

        /*md   // Override the label of the parameter. */
        /*md   label="custom_label" */
        if (config.contains("label")) {
            label = config["label"].get<std::string>();
        }

        /*md   // Set how many digits after the decimal point (by default none. */
        /*md   floatPrecision={2} */
        floatPrecision = config.value("floatPrecision", floatPrecision);

        /*md   // Use the string value instead of the floating point one (default: false). */
        /*md   useStringValue */
        useStringValue = config.value("useStringValue", useStringValue);

        /*md   // Set the bar height (default: 2). */
        /*md   barHeight={2} */
        barH = config.value("barHeight", barH);

        /*md   // Set the bar background height (default: 1). */
        /*md   barBgHeight={1} */
        barBgH = config.value("barBgHeight", barBgH);

        /*md   // Set the text vertical alignment to center (default: false). */
        /*md   verticalAlignCenter */
        verticalAlignCenter = config.value("verticalAlignCenter", verticalAlignCenter);

        /*md   // Hide the label (default: false). */
        /*md   hideLabel */
        showLabel = !config.value("hideLabel", !showLabel);

        /*md   // Hide the unit (default: false). */
        /*md   hideUnit */
        showUnit = !config.value("hideUnit", !showUnit);

        /*md   // Hide the value (default: false). */
        /*md   hideValue */
        showValue = !config.value("hideValue", !showValue);

        /*md   // Shows the label over the value, 4 px from the top of the component (default: -1) `*/
        /*md   showLabelOverValue={4} */
        showLabelOverValue = config.value("showLabelOverValue", showLabelOverValue);

        /*md   // Set the distance from the left of the component where the label will be placed. If -1 it is centered (default: -1) */
        /*md   labelOverValueX={0} */
        labelOverValueX = config.value("labelOverValueX", labelOverValueX);

        /*md   // Set the font size of the value (default: 8). */
        /*md   valueSize={8} */
        valueFontSize = config.value("valueSize", valueFontSize);

        /*md   // Set the font size of the label (default: 6). */
        /*md   labelSize={6} */
        labelFontSize = config.value("labelSize", labelFontSize);

        /*md   // Set the font size of the unit (default: 6). */
        /*md   unitSize={6} */
        unitFontSize = config.value("unitSize", unitFontSize);

        /*md   // The font of the text. Default is null. */
        /*md   font="Sinclair_S" */
        if (config.contains("font")) {
            font = draw.getFont(config["font"].get<std::string>().c_str());
        }

        /*md   // The font height of the text. Default is 0. */
        /*md   valueHeight=16 */
        fontHeightValue = config.value("valueHeight", fontHeightValue);
        setMaxFontSize();

        /*md   // Set the background color of the component. */
        /*md   bgColor="#000000" */
        if (config.contains("bgColor")) {
            bgColor = draw.getColor(config["bgColor"].get<std::string>());
        }

        /*md   // Set the color of the label. */
        /*md   valueColor="#FF0000" */
        if (config.contains("valueColor")) {
            valueColor = draw.getColor(config["valueColor"].get<std::string>());
        }

        /*md   // Set the color of the bar. */
        /*md   barColor="#FF0000" */
        if (config.contains("barColor")) {
            barColor = draw.getColor(config["barColor"].get<std::string>());
        }

        /*md   // Set the color of the unit. */
        /*md   unitColor="#00FF00" */
        if (config.contains("unitColor")) {
            unitColor = draw.getColor(config["unitColor"].get<std::string>());
        }

        /*md   // Set the color of the label. */
        /*md   labelColor="#00FF00" */
        if (config.contains("labelColor")) {
            labelColor = draw.getColor(config["labelColor"].get<std::string>());
        }

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
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            if (val != NULL) {
                if (barH > 0) {
                    renderBar();
                }

                int x = relativePosition.x + (size.w) * 0.5;

                if (showLabel && showValue) {
                    x -= labelFontSize * val->props().label.length() + 2;
                }

                int textY = verticalAlignCenter ? (size.h - maxFontSize) * 0.5 + relativePosition.y : (size.h - maxFontSize) + relativePosition.y;
                // Put all text in the same line
                int labelY = textY + maxFontSize - labelFontSize;
                int valueY = textY + maxFontSize - valueH;
                int unitY = textY + maxFontSize - unitFontSize;

                if (showLabelOverValue != -1) {
                    draw.textCentered({ labelOverValueX == -1 ? x : relativePosition.x + labelOverValueX, relativePosition.y + showLabelOverValue }, getLabel(), labelFontSize, { labelColor, .font = font });
                } else if (showLabel) {
                    x = draw.text({ x, labelY }, getLabel(), labelFontSize, { labelColor, .font = font }) + 2;
                }

                if (showValue) {
                    x = showLabel ? draw.text({ x, valueY }, getValStr(), valueFontSize, { valueColor, .font = font, .fontHeight = fontHeightValue })
                                  : draw.textCentered({ x, valueY }, getValStr(), valueFontSize, { valueColor, .font = font, .maxWidth = size.w - 4, .fontHeight = fontHeightValue });
                    if (showUnit && val->props().unit.length() > 0) {
                        draw.text({ x + 2, unitY }, val->props().unit, unitFontSize, { unitColor, .font = font });
                    }
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

    void onEncoder(int id, int8_t direction)
    {
        // printf("[ValueComponent] onEncoder %d %d component %s\n", id, direction, this->id.c_str());
        // printf("val %d isActive %d encoderId %d\n", val, isActive, encoderId);
        if (val && isActive && id == encoderId) {
            val->increment(direction);
        }
    }
};
