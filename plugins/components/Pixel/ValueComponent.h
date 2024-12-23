#ifndef _UI_PIXEL_COMPONENT_VALUE_H_
#define _UI_PIXEL_COMPONENT_VALUE_H_

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"
#include "utils/GroupColorComponent.h"

/*md
## Value

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/value.png" />

Value component is used to display an audio plugin value.
*/

class ValueComponent : public GroupColorComponent {
protected:
    Color bgColor;
    ToggleColor valueColor;
    ToggleColor barColor;
    ToggleColor unitColor;
    ToggleColor labelColor;

    int valueFontSize = 8;
    int labelFontSize = 6;
    int unitFontSize = 6;
    int maxFontSize = 8;
    void* font = NULL;

    ValueInterface* val = NULL;
    int8_t encoderId = -1;
    uint8_t floatPrecision = 0;

    bool showBar = true;
    bool showValue = true;
    bool showUnit = true;
    bool showLabel = true;
    bool useStringValue = false;

    void setMaxFontSize()
    {
        maxFontSize = std::max({ valueFontSize, labelFontSize, unitFontSize });
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
        : GroupColorComponent(props, { { "VALUE_COLOR", &valueColor }, { "BAR_COLOR", &barColor }, { "UNIT_COLOR", &unitColor }, { "LABEL_COLOR", &labelColor } })
        , bgColor(styles.colors.background)
        , valueColor(styles.colors.text, inactiveColorRatio)
        , barColor(lighten(styles.colors.background, 0.5), inactiveColorRatio)
        , unitColor(darken(styles.colors.text, 0.5), inactiveColorRatio)
        , labelColor(darken(styles.colors.text, 0.5), inactiveColorRatio)
    {
        updateColors();
    }

    void render() override
    {
        if (updatePosition()) {
            draw.filledRect(relativePosition, size, { bgColor });
            if (val != NULL) {
                if (showBar) {
                    if (val->hasType(VALUE_CENTERED)) {
                        float valPct = val->pct();
                        if (valPct < 0.5) {
                            int w = size.w * (0.5 - valPct);
                            draw.filledRect({ relativePosition.x + (int)(size.w * 0.5) - w, relativePosition.y }, { w, size.h }, { barColor.color });
                        } else {
                            draw.filledRect({ relativePosition.x + (int)(size.w * 0.5), relativePosition.y }, { (int)(size.w * (valPct - 0.5)), size.h }, { barColor.color });
                        }
                    } else {
                        draw.filledRect({ relativePosition.x, relativePosition.y }, { (int)(size.w * val->pct()), size.h }, { barColor.color });
                    }
                }

                int x = relativePosition.x + (size.w) * 0.5;

                if (showLabel && showValue) {
                    x -= labelFontSize * val->props().label.length() + 2;
                }

                int textY = (size.h - maxFontSize) * 0.5 + relativePosition.y;
                // Put all text in the same line
                int labelY = textY + maxFontSize - labelFontSize;
                int valueY = textY + maxFontSize - valueFontSize;
                int unitY = textY + maxFontSize - unitFontSize;

                if (showLabel) {
                    x = draw.text({ x, labelY }, val->props().label, labelFontSize, { labelColor.color, .font = font }) + 2;
                }

                if (showValue) {
                    x = showLabel ? draw.text({ x, valueY }, getValStr(), valueFontSize, { valueColor.color, .font = font })
                                   : draw.textCentered({ x, valueY }, getValStr(), valueFontSize, { valueColor.color, .font = font });
                    if (showUnit && val->props().unit != NULL) {
                        draw.text({ x, unitY }, val->props().unit, unitFontSize, { unitColor.color, .font = font });
                    }
                }
            }
        }
    }

    void onEncoder(int id, int8_t direction)
    {
        if (val && isActive && id == encoderId) {
            val->increment(direction);
        }
    }

    /*md **Config**: */
    bool config(char* key, char* params)
    {
        /*md - `VALUE: pluginName keyName` is used to set the value to control */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(params, " ");
            char* keyValue = strtok(NULL, " ");
            val = watch(getPlugin(pluginName, track).getValue(keyValue));
            if (val != NULL) {
                floatPrecision = val->props().floatingPoint;
            }
            useStringValue = val->hasType(VALUE_STRING);
            return true;
        }

        /*md - `ENCODER_ID: 0` is used to set the encoder id that will interract with this component */
        if (strcmp(key, "ENCODER_ID") == 0) {
            encoderId = atoi(params);
            return true;
        }

        /*md - `FLOAT_PRECISION: 2` set how many digits after the decimal point (by default none) */
        if (strcmp(key, "FLOAT_PRECISION") == 0) {
            floatPrecision = atoi(params);
            return true;
        }

        /*md - `USE_STRING_VALUE: true` use the string value instead of the floating point one (default: false) */
        if (strcmp(key, "USE_STRING_VALUE") == 0) {
            useStringValue = strcmp(params, "true") == 0;
            return true;
        }

        /*md - `SHOW_BAR: true` shows the bar (default: true) */
        if (strcmp(key, "SHOW_BAR") == 0) {
            showBar = atoi(params);
            return true;
        }

        /*md - `SHOW_VALUE: true` shows the value (default: true) */
        if (strcmp(key, "SHOW_VALUE") == 0) {
            showValue = atoi(params);
            return true;
        }

        /*md - `SHOW_UNIT: true` shows the unit (default: true) */
        if (strcmp(key, "SHOW_UNIT") == 0) {
            showUnit = atoi(params);
            return true;
        }

        /*md - `SHOW_LABEL: true` shows the label (default: true) */
        if (strcmp(key, "SHOW_LABEL") == 0) {
            showLabel = atoi(params);
            return true;
        }

        /*md - `VALUE_FONT_SIZE: size` is the font size of the component. */
        if (strcmp(key, "VALUE_FONT_SIZE") == 0) {
            valueFontSize = atoi(params);
            setMaxFontSize();
            return true;
        }

        /*md - `LABEL_FONT_SIZE: size` is the font size of the component. */
        if (strcmp(key, "LABEL_FONT_SIZE") == 0) {
            labelFontSize = atoi(params);
            setMaxFontSize();
            return true;
        }

        /*md - `UNIT_FONT_SIZE: size` is the font size of the component. */
        if (strcmp(key, "UNIT_FONT_SIZE") == 0) {
            unitFontSize = atoi(params);
            setMaxFontSize();
            return true;
        }

        /*md - `FONT: font` is the font of the component. */
        if (strcmp(key, "FONT") == 0) {
            font = draw.getFont(params);
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(params);
            return true;
        }

        /*md - `VALUE_COLOR: color` is the color of the value. */
        /*md - `BAR_COLOR: color` is the color of the bar. */
        /*md - `UNIT_COLOR: color` is the color of the unit. */
        /*md - `LABEL_COLOR: color` is the color of the label. */
        return GroupColorComponent::config(key, params);
    }
};

#endif
