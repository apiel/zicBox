#pragma once

#include "../component.h"
#include "../utils/color.h"
#include <math.h>
#include <string>

/*md
## KnobValue

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/KnobValue.png" />

KnobValue is used to display current audio plugin value for a given parameter.
*/
class KnobValueComponent : public Component {
protected:
    const char* name = NULL;
    std::string label;
    char labelBuffer[32];

    int radius = 20;
    int insideRadius = 15;

    int fontValueSize = 8;
    int fontUnitSize = 8;
    int fontLabelSize = 6;
    int twoSideMargin = 2;

    Point knobCenter = { 0, 0 };
    Point valuePosition = { 0, 0 };

    bool showValue = true;
    bool showUnit = true;
    bool stringValueReplaceTitle = false;

    // FIXME should remove marginTop
    const int marginTop = 2;

    int8_t encoderId = -1;
    uint8_t valueFloatPrecision = 0;

    ValueInterface* value = NULL;

    void renderLabel()
    {
        if (stringValueReplaceTitle && value->hasType(VALUE_STRING)) {
            draw.textCentered({ knobCenter.x, relativePosition.y + size.h - fontLabelSize }, value->string(), fontLabelSize, { titleColor, NULL, size.w - 4 });
        } else {
            draw.textCentered({ knobCenter.x, relativePosition.y + size.h - fontLabelSize }, label.empty() ? value->label() : label, fontLabelSize, { titleColor });
        }
    }

    void renderBar()
    {
        int val = 280 * value->pct();

        if (val < 280) {
            draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, -230, 50, { barBackgroundColor, .thickness = 3 });
        }
        if (val > 0) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }
            Color color = useBar2Color != -1 && value->pct() > useBar2Color ? bar2Color : barColor;
            draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, 130, endAngle, { color, .thickness = 5 });
        }
    }

    void renderCenteredBar()
    {
        int val = 280 * value->pct();

        draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, -230, 50, { barBackgroundColor, .thickness = 3 });
        if (val > 140) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }

            draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, 270, endAngle, { barColor, .thickness = 5 });
        } else if (val < 140) {
            draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, -230 + val, 270, { barColor, .thickness = 5 });
        }
    }

    void renderUnit()
    {
        if (showUnit && value->props().unit.length() > 0) {
            draw.textCentered({ valuePosition.x, valuePosition.y + fontValueSize - 5 }, value->props().unit, fontUnitSize, { unitColor });
        }
    }

    void renderValue()
    {
        if (!stringValueReplaceTitle && ((value->hasType(VALUE_STRING) && type != 3) || type == ENCODER_TYPE::STRING)) {
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
        draw.textRight({ valuePosition.x - twoSideMargin, valuePosition.y - 2 }, std::to_string((int)value->props().max - val).c_str(),
            fontValueSize - 3, { valueColor });
        draw.text({ valuePosition.x + twoSideMargin, valuePosition.y - 2 }, std::to_string(val).c_str(),
            fontValueSize - 3, { valueColor });

        draw.line({ valuePosition.x, valuePosition.y - 10 }, { valuePosition.x, valuePosition.y + 10 }, { barTwoSideColor });
        draw.line({ valuePosition.x - 1, valuePosition.y - 10 }, { valuePosition.x - 1, valuePosition.y + 10 }, { barTwoSideColor });
    }

    void renderEncoder()
    {
        renderLabel();
        if (value->hasType(VALUE_CENTERED)) {
            renderCenteredBar();
        } else {
            renderBar();
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
    Color bar2Color;
    Color barBackgroundColor;
    Color barTwoSideColor;

    float useBar2Color = -1.0f;

    const int margin;

    void setRadius(int _radius)
    {
        radius = _radius;
        insideRadius = radius - 5;

        if (radius > 30) {
            // should then change the font to bigger one
            fontValueSize = 16;
            fontLabelSize = 8;
            twoSideMargin = 3;
        }
    }

public:
    KnobValueComponent(ComponentInterface::Props props)
        : Component(props)
        , margin(styles.margin)
        , bgColor(styles.colors.background)
        , idColor({ 0x60, 0x60, 0x60, 255 })
        , titleColor(alpha(styles.colors.text, 0.4)) // instead of alpha color should we use plain color?
        , valueColor(alpha(styles.colors.text, 0.4))
        , unitColor(alpha(styles.colors.text, 0.2))
        , barColor(styles.colors.primary)
        , bar2Color(styles.colors.secondary)
        , barBackgroundColor(alpha(styles.colors.primary, 0.7))
        , barTwoSideColor(alpha(styles.colors.primary, 0.2))
    {
        if (size.h < 50) {
            printf("Encoder component height too small: %dx%d. Min height is 50.\n", size.w, size.h);
            size.h = 50;
        }

        if (size.w < 50) {
            printf("Encoder component width too small: %dx%d. Min width is 50.\n", size.w, size.h);
            size.w = 50;
        }

        setRadius((size.h - 6) * 0.5);
    }

    void render()
    {
        if (value != NULL && updatePosition()) {
            knobCenter = { (int)(relativePosition.x + (size.w * 0.5)), (int)(relativePosition.y + (size.h * 0.5) + marginTop - 1) };
            valuePosition = { knobCenter.x, knobCenter.y - marginTop - 2 };
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

    bool config(char* key, char* params)
    {
        /*md - `VALUE: pluginName keyName` is used to set the value to control */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(params, " ");
            char* keyValue = strtok(NULL, " ");
            value = watch(getPlugin(pluginName, track).getValue(keyValue));
            if (value != NULL) {
                valueFloatPrecision = value->props().floatingPoint;
            }
            return true;
        }

        /*md - `ENCODER_ID: 0` is used to set the encoder id that will interract with this component */
        if (strcmp(key, "ENCODER_ID") == 0) {
            encoderId = atoi(params);
            return true;
        }

        /*md
- `TYPE: TWO_SIDED` is used to set the encoder type
    - `TYPE: TWO_SIDED` when there is a centered value like PAN and you want to show both side value: 20%|80%
        */
        if (strcmp(key, "TYPE") == 0) {
            if (strcmp(params, "TWO_SIDED") == 0) {
                type = ENCODER_TYPE::TWO_SIDED;
            } else if (strcmp(params, "STRING") == 0) {
                type = ENCODER_TYPE::STRING;
            } else if (strcmp(params, "NUMBER") == 0) {
                type = ENCODER_TYPE::NUMBER;
            } else if (strcmp(params, "TWO_VALUES") == 0) {
                type = ENCODER_TYPE::TWO_VALUES;
            } else {
                type = (ENCODER_TYPE)atoi(params);
            }
            return true;
        }

        /*md - `LABEL: custom_label` overwrite the value label */
        if (strcmp(key, "LABEL") == 0) {
            strcpy(labelBuffer, params);
            label = labelBuffer;
            return true;
        }

        /*md - `COLOR: #3791a1` set the ring color */
        if (strcmp(key, "COLOR") == 0) {
            barColor = draw.getColor(params);
            barBackgroundColor = alpha(barColor, 0.7);
            barTwoSideColor = alpha(barColor, 0.2);
            return true;
        }

        /*md - `BACKGROUND_COLOR: #000000` set the background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(params);
            return true;
        }

        /*md - `TEXT_COLOR: #ffffff` set the text color */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            titleColor = alpha(draw.getColor(params), 0.4);
            valueColor = alpha(draw.getColor(params), 0.4);
            unitColor = alpha(draw.getColor(params), 0.2);
            return true;
        }

        /*md - `FLOAT_PRECISION: 2` set how many digits after the decimal point (by default none) */
        if (strcmp(key, "FLOAT_PRECISION") == 0) {
            valueFloatPrecision = atoi(params);
            return true;
        }

        /*md - `USE_SECOND_COLOR: value` set the percentage value when `BAR2_COLOR` should be used instead of `BAR_COLOR`, e.g. 0.5 will switch at 50% */
        if (strcmp(key, "USE_SECOND_COLOR") == 0) {
            useBar2Color = atof(params);
            return true;
        }

        /*md - `SHOW_VALUE: true` show value (default true) */
        if (strcmp(key, "SHOW_VALUE") == 0) {
            showValue = (strcmp(params, "true") == 0);
            return true;
        }

        /*md - `SHOW_UNIT: true` show unit (default true) */
        if (strcmp(key, "SHOW_UNIT") == 0) {
            showUnit = (strcmp(params, "true") == 0);
            return true;
        }

        /*md - `FONT_UNIT_SIZE: 12` set the unit font size */
        if (strcmp(key, "FONT_UNIT_SIZE") == 0) {
            fontUnitSize = atoi(params);
            return true;
        }

        /*md - `FONT_VALUE_SIZE: 12` set the value font size */
        if (strcmp(key, "FONT_VALUE_SIZE") == 0) {
            fontValueSize = atoi(params);
            return true;
        }

        /*md - `FONT_TITLE_SIZE: 12` set the title font size */
        if (strcmp(key, "FONT_TITLE_SIZE") == 0) {
            fontLabelSize = atoi(params);
            return true;
        }

        /*md - `STRING_VALUE_REPLACE_TITLE: true` instead to show string value in knob, show under the knob. Can be useful for long string value. */
        if (strcmp(key, "STRING_VALUE_REPLACE_TITLE") == 0) {
            stringValueReplaceTitle = (strcmp(params, "true") == 0);
            return true;
        }

        return false;
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
        // if (isActive && id == encoderId) {
        //     printf("[track %d group %d][%s] KnobValueComponent onEncoder: %d %d\n", track, group, label.c_str(), id, direction);
        // }
        if (value && isActive && id == encoderId) {
            value->increment(direction);
        }
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
