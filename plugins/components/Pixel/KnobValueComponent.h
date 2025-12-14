/** Description:
This C++ header defines the **KnobValueComponent**, which serves as a highly customizable graphical dial or knob interface element within a digital application, typically a music or audio production environment.

### 1. Purpose and Role

The component’s primary function is to monitor, display, and allow control over a specific setting (parameter) belonging to an associated audio plugin. It acts as the visual bridge between the complex internal workings of an audio effect and the user interface.

### 2. Basic Idea of Operation

1.  **Configuration:** When initialized, the component reads configuration details (like its size, colors, and which specific audio plugin parameter it should control, e.g., "Reverb Decay").
2.  **Display:** It draws itself on the screen, calculating positions for the center of the knob, the text value, and the label. It uses specialized drawing functions to create the visual effect of a physical dial.
3.  **Visualization:** It displays the parameter’s current status in three ways:
    *   **Label:** The name of the parameter (e.g., "Volume").
    *   **Numerical Value:** The exact setting (e.g., "4.5 dB").
    *   **Bar/Arc:** A colored circular segment that visually fills up proportionally to the value (e.g., half-filled means 50% of the maximum setting).
4.  **Interaction:** The component is designed to receive input from an external physical device, usually a rotary encoder (a digital dial). When the user turns this external dial, the component registers the movement and immediately updates the numerical value of the connected audio plugin parameter, ensuring the visual display reflects the change instantly.

The entire structure allows for extensive customization of colors, font sizes, and behavior (such as displaying two-sided values for controls like Pan).

sha: c63a4ac93c29d7b478efaf3289d8a9378e9e0502b2b32fb33bb475ed74329d85
*/
#pragma once

#include "log.h"
#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

#include <math.h>
#include <string>

/*md
## KnobValue

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/KnobValue.png" />

KnobValue is used to display current value of a parameter for a given audio plugin.
*/

class KnobValueComponent : public Component {
protected:
    const char* name = NULL;
    std::string label;

    int radius = 20;
    int insideRadius = 15;

    int fontValueSize = 12;
    int fontUnitSize = 8;
    void* fontUnit = NULL;
    int fontLabelSize = 12;
    int twoSideMargin = 2;

    Point knobCenter = { 0, 0 };
    Point valuePosition = { 0, 0 };

    bool showValue = true;
    bool showUnit = true;
    bool stringValueReplaceTitle = false;
    bool showInnerCircle = true;

    int8_t encoderId = -1;
    uint8_t valueFloatPrecision = 0;

    ValueInterface* value = NULL;

    void renderLabel()
    {
        if (stringValueReplaceTitle && value->hasType(VALUE_STRING)) {
            draw.textCentered({ knobCenter.x, relativePosition.y + size.h - fontLabelSize - 4 }, value->string(), fontLabelSize, { titleColor, NULL, size.w - 4 });
        } else {
            draw.textCentered({ knobCenter.x, relativePosition.y + size.h - fontLabelSize - 4 }, label.empty() ? value->label() : label, fontLabelSize, { titleColor });
        }
    }

    void renderBar()
    {
        int val = 280 * value->pct();

        if (val < 280) {
            draw.arc({ knobCenter.x, knobCenter.y }, radius, -230, 50, { barBackgroundColor, .thickness = 3 });
        }
        if (val > 0) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }
            draw.arc({ knobCenter.x, knobCenter.y }, radius, 130, endAngle, { barColor, .thickness = 5 });
        }
    }

    void renderCenteredBar()
    {
        int val = 280 * value->pct();

        draw.arc({ knobCenter.x, knobCenter.y }, radius, -230, 50, { barBackgroundColor, .thickness = 3 });
        if (val > 140) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }

            draw.arc({ knobCenter.x, knobCenter.y }, radius, 270, endAngle, { barColor, .thickness = 5 });
        } else if (val < 140) {
            draw.arc({ knobCenter.x, knobCenter.y }, radius, -230 + val, 270, { barColor, .thickness = 5 });
        }
    }

    void renderUnit()
    {
        if (showUnit && value->props().unit.length() > 0) {
            draw.textCentered({ valuePosition.x, valuePosition.y + fontValueSize - 5 }, value->props().unit, fontUnitSize, { unitColor, .font = fontUnit });
        }
    }

    void renderValue()
    {
        if (!stringValueReplaceTitle && ((value->hasType(VALUE_STRING) && type != ENCODER_TYPE::NUMBER) || type == ENCODER_TYPE::STRING)) {
            draw.textCentered({ valuePosition.x, valuePosition.y - 5 }, value->string(), fontValueSize, { valueColor });
        } else {
            float val = value->get();
            if (type == ENCODER_TYPE::TWO_VALUES) {
                if (value->pct() > 0.5) {
                    val = val - (value->props().max * 0.5);
                } else {
                    val = (value->props().max * 0.5) - val;
                }
            }

            std::string valStr = std::to_string(val);
            valStr = valStr.substr(0, valStr.find(".") + valueFloatPrecision + (valueFloatPrecision > 0 ? 1 : 0));

            draw.textCentered({ valuePosition.x, valuePosition.y - 5 }, valStr.c_str(), fontValueSize, { valueColor });
        }
    }

    void renderTwoSidedValue()
    {
        int val = value->get();
        // FIXME use floating point...
        draw.textRight({ valuePosition.x - twoSideMargin, valuePosition.y - 2 }, std::to_string((int)value->props().max - val).c_str(), fontValueSize, { valueColor });
        draw.text({ valuePosition.x + twoSideMargin, valuePosition.y - 2 }, std::to_string(val).c_str(), fontValueSize, { valueColor });

        draw.line({ valuePosition.x, valuePosition.y - 10 }, { valuePosition.x, valuePosition.y + 10 }, { barTwoSideColor });
        draw.line({ valuePosition.x - 1, valuePosition.y - 10 }, { valuePosition.x - 1, valuePosition.y + 10 }, { barTwoSideColor });
    }

    void renderInnerCircle()
    {
        draw.filledCircle({ knobCenter.x, knobCenter.y + 2 }, insideRadius, { shadowColor }); // shadow effect
        draw.filledCircle({ knobCenter.x, knobCenter.y }, insideRadius - 2, { centerColor });
    }

    void renderEncoder()
    {
        renderLabel();
        if (value->hasType(VALUE_CENTERED)) {
            renderCenteredBar();
        } else {
            renderBar();
        }

        if (showInnerCircle) {
            renderInnerCircle();
        }

        if (showValue) {
            renderUnit();
            if (value->hasType(VALUE_CENTERED) && type == ENCODER_TYPE::TWO_SIDED) {
                renderTwoSidedValue();
            } else {
                renderValue();
            }
        }
    }

    Color bgColor;

    Color idColor;
    Color titleColor;
    Color valueColor;
    Color unitColor;
    Color barColor;
    Color barBackgroundColor;
    Color barTwoSideColor;
    Color centerColor;
    Color shadowColor;

public:
    KnobValueComponent(ComponentInterface::Props props)
        : Component(props)
        , bgColor(styles.colors.background)
        , idColor({ 0x60, 0x60, 0x60, 255 })
        , titleColor(alpha(styles.colors.text, 0.4)) // instead of alpha color should we use plain color?
        , valueColor(alpha(styles.colors.text, 0.4))
        , unitColor(alpha(styles.colors.text, 0.2))
        , barColor(styles.colors.primary)
        , barBackgroundColor(alpha(styles.colors.primary, 0.7))
        , barTwoSideColor(alpha(styles.colors.primary, 0.2))
        , centerColor(lighten(styles.colors.background, 0.3))
        , shadowColor(darken(styles.colors.background, 0.1))
    {
        if (size.h < 50) {
            printf("Encoder component height too small: %dx%d. Min height is 50.\n", size.w, size.h);
            size.h = 50;
        }

        if (size.w < 50) {
            printf("Encoder component width too small: %dx%d. Min width is 50.\n", size.w, size.h);
            size.w = 50;
        }

        fontUnit = draw.getFont("PoppinsLight_8");

        /*md md_config:KnobValue */
        nlohmann::json& config = props.config;

        /// The audio plugin to get control on.
        AudioPlugin* plugin = getPluginPtr(config, "audioPlugin", track); //eg: "audio_plugin_name"

        /// The audio plugin key parameter to get control on.
        std::string param = getConfig(config, "param"); //eg: "parameter_name"

        value = watch(plugin->getValue(param));
        if (value != NULL) {
            valueFloatPrecision = value->props().floatingPoint;
        }

        /// The encoder id that will interract with this component.
        /*md   encoderId={0} */
        encoderId = config.value("encoderId", encoderId);

        // TODO see if this could be simplified
        /// The type of the encoder. By default the component will define the type own his own. */
        /*md   type="TWO_SIDED" // when there is a centered value like PAN and you want to show both side value: 20%|80% */
        /*md   type="TWO_VALUES" // when there is 2 possible values, for example one side is drive and the other side is compressor. */
        /*md   type="NUMBER" // when the value is of type string but we still want to enforce to see his numeric value */
        /*md   type="STRING" */
        if (config.contains("type")) {
            std::string typeStr = config["type"].get<std::string>();
            if (typeStr == "TWO_SIDED") {
                type = ENCODER_TYPE::TWO_SIDED;
            } else if (typeStr == "STRING") {
                type = ENCODER_TYPE::STRING;
            } else if (typeStr == "NUMBER") {
                type = ENCODER_TYPE::NUMBER;
            } else if (typeStr == "TWO_VALUES") {
                type = ENCODER_TYPE::TWO_VALUES;
            }
        }

        /// Override the label of the parameter.
        if (config.contains("label")) {
            label = config["label"].get<std::string>(); //eg: "custom_label"
        }

        /// Set the color of the knob.
        barColor = draw.getColor(config["color"], barColor); //eg: "#3791a1"
        barBackgroundColor = alpha(barColor, 0.7);
        barTwoSideColor = alpha(barColor, 0.2);

        /// Set the background color of the component.
        bgColor = draw.getColor(config["bgColor"], bgColor); //eg: "#000000"
        shadowColor = darken(bgColor, 0.1);

        /// Set the color of the text.
        Color textColor = draw.getColor(config["textColor"], styles.colors.text); //eg: "#ffffff"
        titleColor = alpha(textColor, 0.4);
        valueColor = alpha(textColor, 0.4);
        unitColor = alpha(textColor, 0.2);

        /// Set how many digits after the decimal point (by default none.
        valueFloatPrecision = config.value("floatPrecision", valueFloatPrecision); //eg: 2

        /// Hide the value of the parameter.
        showValue = !config.value("hideValue", false); //eg: true

        /// Hide the unit of the parameter.
        showUnit = !config.value("hideUnit", false); //eg: true

        /// Hide middle circle background.
        showInnerCircle = !config.value("hideCenterBackground", !showInnerCircle); //eg: true

        /// Set the font size of the unit.
        fontUnitSize = config.value("unitSize", fontUnitSize); //eg: 8

        /// Set the font size of the value.
        fontValueSize = config.value("valueSize", fontValueSize); //eg: 8

        /// Set the font size of the title.
        fontLabelSize = config.value("titleSize", fontLabelSize); //eg: 8

        /// Instead to show the value of the parameter in knob, show it under the knob. Can be useful for long string value.
        stringValueReplaceTitle = config.value("valueReplaceTitle", stringValueReplaceTitle); //eg: true

        /// Set inside radius of the knob.
        insideRadius = config.value("insideRadius", insideRadius); //eg: 15

        /*md md_config_end */
    }

    void resize() override
    {
        radius = (size.h - fontLabelSize - 2) * 0.5;
        insideRadius = radius - 5;
        knobCenter = { (int)(relativePosition.x + (size.w * 0.5)), (int)(relativePosition.y + radius + 2) }; // 2 is the thickness of the bar...
        valuePosition = { knobCenter.x, knobCenter.y - 2 };
    }

    void render() override
    {
        if (value != NULL) {
            draw.filledRect(relativePosition, size, { bgColor });
            renderEncoder();
        }
    }

    enum ENCODER_TYPE {
        NORMAL,
        TWO_SIDED,
        STRING,
        NUMBER,
        TWO_VALUES,
    } type
        = ENCODER_TYPE::NORMAL;

    void onEncoder(int8_t id, int8_t direction) override
    {
        // if (id == encoderId) {
        //     printf("[track %d group %d][%s] KnobValueComponent onEncoder: %d %d\n", track, group, label.c_str(), id, direction);
        // }
        if (value && id == encoderId) {
            logDebug("KnobValueComponent onEncoder: %d %d", id, direction);
            value->increment(direction);
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

    void* data(int id, void* userdata = NULL) override
    {
        if (id == 0) {
            int8_t* direction = (int8_t*)userdata;
            value->increment(*direction);
            return NULL;
        }
        return NULL;
    }
};
