#pragma once

#include "plugins/components/component.h"
#include "plugins/components/utils/color.h"

/*md
## Value

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/value.png" />
<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/value2.png" />

Value component is used to display an audio plugin value.
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

        /*md - `BAR_HEIGHT: 2` set the bar height (default: 2) */
        if (strcmp(key, "BAR_HEIGHT") == 0) {
            barH = atoi(params);
            return true;
        }

        /*md - `BAR_BG_HEIGHT: 1` set the bar background height (default: 1) */
        if (strcmp(key, "BAR_BG_HEIGHT") == 0) {
            barBgH = atoi(params);
            return true;
        }

        /*md - `VERTICAL_ALIGN_CENTER: true` set the text vertical alignment to center (default: false) */
        if (strcmp(key, "VERTICAL_ALIGN_CENTER") == 0) {
            verticalAlignCenter = strcmp(params, "true") == 0;
            return true;
        }

        /*md - `SHOW_VALUE: true` shows the value (default: true) */
        if (strcmp(key, "SHOW_VALUE") == 0) {
            showValue = strcmp(params, "true") == 0;
            setMaxFontSize();
            return true;
        }

        /*md - `SHOW_UNIT: true` shows the unit (default: true) */
        if (strcmp(key, "SHOW_UNIT") == 0) {
            showUnit = strcmp(params, "true") == 0;
            setMaxFontSize();
            return true;
        }

        /*md - `SHOW_LABEL: true` shows the label (default: true) */
        if (strcmp(key, "SHOW_LABEL") == 0) {
            showLabel = strcmp(params, "true") == 0;
            setMaxFontSize();
            return true;
        }

        /*md - `SHOW_LABEL_OVER_VALUE: 4` shows the label over the value, 4 px from the top of the component (default: -1) `*/
        if (strcmp(key, "SHOW_LABEL_OVER_VALUE") == 0) {
            showLabelOverValue = atoi(params);
            if (showLabelOverValue != -1) {
                showLabel = false;
            }
            setMaxFontSize();
            return true;
        }

        /*md - `LABEL_OVER_VALUE_X: 0` set the distance from the left of the component where the label will be placed. If -1 it is centered (default: -1) */
        if (strcmp(key, "LABEL_OVER_VALUE_X") == 0) {
            labelOverValueX = atoi(params);
            return true;
        }

        /*md - `LABEL: label` is the label of the component. */
        if (strcmp(key, "LABEL") == 0) {
            label = params;
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

        /*md - `VALUE_FONT_HEIGHT: 16` is the font height of the value. */
        if (strcmp(key, "VALUE_FONT_HEIGHT") == 0) {
            fontHeightValue = atoi(params);
            setMaxFontSize();
            return true;
        }

        /*md - `BACKGROUND_COLOR: color` is the background color of the component. */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(params);
            return true;
        }

        /*md - `VALUE_COLOR: color` is the color of the value. */
        if (strcmp(key, "VALUE_COLOR") == 0) {
            valueColor = draw.getColor(params);
            return true;
        }

        /*md - `BAR_COLOR: color` is the color of the bar. */
        if (strcmp(key, "BAR_COLOR") == 0) {
            barColor = draw.getColor(params);
            return true;
        }

        /*md - `UNIT_COLOR: color` is the color of the unit. */
        if (strcmp(key, "UNIT_COLOR") == 0) {
            unitColor = draw.getColor(params);
            return true;
        }

        /*md - `LABEL_COLOR: color` is the color of the label. */
        if (strcmp(key, "LABEL_COLOR") == 0) {
            labelColor = draw.getColor(params);
            return true;
        }
        
        return false;
    }
};

