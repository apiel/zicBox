#ifndef _UI_COMPONENT2_ENCODER_H_
#define _UI_COMPONENT2_ENCODER_H_

#include "../component.h"
#include "../utils/color.h"
#include <math.h>
#include <string>

#include "utils/GroupColorComponent.h"

/*md
## Encoder3

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Pixel/Encoder3.png" />

Encoder3 is used to display current audio plugin value for a given parameter.
*/
class Encoder3Component : public GroupColorComponent {
protected:
    const char* name = NULL;
    std::string label;
    char labelBuffer[32];
    uint8_t type = 0;
    int radius = 20;
    int insideRadius = 15;

    int fontValueSize = 8;
    int fontUnitSize = 8;
    int fontLabelSize = 6;
    int twoSideMargin = 2;

    Point pos = { 0, 0 };
    Point knobCenter = { 0, 0 };
    Point valuePosition = { 0, 0 };

    bool showValue = true;
    bool showGroup = false;
    bool showUnit = true;
    bool stringValueReplaceTitle = false;

    const int marginTop = 2;

    int8_t encoderId = -1;
    uint8_t valueFloatPrecision = 0;

    ValueInterface* value = NULL;

    void renderLabel()
    {
        if (stringValueReplaceTitle && value->props().type == VALUE_STRING) {
            draw.textCentered({ knobCenter.x, pos.y + size.h - fontLabelSize }, value->string(), fontLabelSize, { titleColor.color, NULL, size.w - 4 });
        } else {
            draw.textCentered({ knobCenter.x, pos.y + size.h - fontLabelSize }, label, fontLabelSize, { titleColor.color });
        }
    }

    void renderActiveGroup()
    {
        if (showGroup && isActive) {
            draw.filledRect({ pos.x + margin, pos.y + margin }, { 12, 12 }, { idColor.color });
            draw.textCentered({ pos.x + margin + 6, pos.y + margin }, std::to_string(encoderId + 1).c_str(), 6, { bgColor });
        }
    }

    void renderBar()
    {
        int val = 280 * value->pct();

        if (val < 280) {
            draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, -230, 50, { barBackgroundColor.color, .thickness = 4 });
        }
        if (val > 0) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }
            draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, 130, endAngle, { barColor.color, .thickness = 4 });
        }
    }

    void renderCenteredBar()
    {
        int val = 280 * value->pct();

        draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, -230, 50, { barBackgroundColor.color, .thickness = 4 });
        if (val > 140) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }

            draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, 270, endAngle, { barColor.color, .thickness = 4 });
        } else if (val < 140) {
            draw.arc({ knobCenter.x, knobCenter.y - marginTop }, radius, -230 + val, 270, { barColor.color, .thickness = 4 });
        }
    }

    void renderUnit()
    {
        if (showUnit && value->props().unit != NULL) {
            draw.textCentered({ valuePosition.x, valuePosition.y + fontValueSize - 5 }, value->props().unit, fontUnitSize, { unitColor.color });
        }
    }

    void renderValue()
    {
        if (!stringValueReplaceTitle && value->props().type == VALUE_STRING) {
            draw.textCentered({ valuePosition.x, valuePosition.y - 5 }, value->string(), fontValueSize, { valueColor.color });
        } else {
            std::string valStr = std::to_string(value->get());
            valStr = valStr.substr(0, valStr.find(".") + valueFloatPrecision + (valueFloatPrecision > 0 ? 1 : 0));

            draw.textCentered({ valuePosition.x, valuePosition.y - 5 }, valStr.c_str(), fontValueSize, { valueColor.color });
        }
    }

    void renderTwoSidedValue()
    {
        int val = value->get();
        // FIXME use floating point...
        draw.textRight({ valuePosition.x - twoSideMargin, valuePosition.y - 5 }, std::to_string((int)value->props().max - val).c_str(),
            fontValueSize - 3, { valueColor.color });
        draw.text({ valuePosition.x + twoSideMargin, valuePosition.y - 5 }, std::to_string(val).c_str(),
            fontValueSize - 3, { valueColor.color });

        draw.line({ valuePosition.x, valuePosition.y - 10 }, { valuePosition.x, valuePosition.y + 10 }, { barTwoSideColor.color });
        draw.line({ valuePosition.x - 1, valuePosition.y - 10 }, { valuePosition.x - 1, valuePosition.y + 10 }, { barTwoSideColor.color });
    }

    void renderEncoder()
    {
        renderActiveGroup();

        renderLabel();
        if (value->props().type == VALUE_CENTERED) {
            renderCenteredBar();
        } else {
            renderBar();
        }

        if (showValue) {
            renderUnit();
            if (value->props().type == VALUE_CENTERED && type == 1) {
                renderTwoSidedValue();
            } else {
                renderValue();
            }
        }
    }

    Color bgColor;

    ToggleColor idColor;
    ToggleColor titleColor;
    ToggleColor valueColor;
    ToggleColor unitColor;
    ToggleColor barColor;
    ToggleColor barBackgroundColor;
    ToggleColor barTwoSideColor;

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
    Encoder3Component(ComponentInterface::Props props)
        : GroupColorComponent(props, {
                                         { "ID_COLOR", &idColor },
                                         { "TITLE_COLOR", &titleColor },
                                         { "VALUE_COLOR", &valueColor },
                                         { "UNIT_COLOR", &unitColor },
                                         { "BAR_COLOR", &barColor },
                                         { "BAR_BACKGROUND_COLOR", &barBackgroundColor },
                                         { "BAR_TWOSIDE_COLOR", &barTwoSideColor },
                                     })
        , margin(styles.margin)
        , bgColor(styles.colors.background)
        , idColor({ 0x60, 0x60, 0x60, 255 }, inactiveColorRatio)
        , titleColor(alpha(styles.colors.text, 0.4), inactiveColorRatio) // instead of alpha color should we use plain color?
        , valueColor(alpha(styles.colors.text, 0.4), inactiveColorRatio)
        , unitColor(alpha(styles.colors.text, 0.2), inactiveColorRatio)
        , barColor(styles.colors.primary, inactiveColorRatio)
        , barBackgroundColor(alpha(styles.colors.primary, 0.5), inactiveColorRatio)
        , barTwoSideColor(alpha(styles.colors.primary, 0.2), inactiveColorRatio)
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
        if (value != NULL) {
            pos = getPosition();
            knobCenter = { (int)(pos.x + (size.w * 0.5)), (int)(pos.y + (size.h * 0.5) + marginTop - 1) };
            valuePosition = { knobCenter.x, knobCenter.y - marginTop - 2 };
            draw.filledRect(pos, size, { bgColor });
            renderEncoder();
        }
    }

    bool config(char* key, char* params)
    {
        /*md - `VALUE: pluginName keyName` is used to set the value to control */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(params, " ");
            char* keyValue = strtok(NULL, " ");
            value = watch(getPlugin(pluginName, track).getValue(keyValue));
            if (value != NULL) {
                valueFloatPrecision = value->props().floatingPoint;
                if (label.empty()) {
                    label = value->label();
                }
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
                type = 1;
            } else {
                type = atoi(params);
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
            barColor.setColor(draw.getColor(params), inactiveColorRatio);
            barBackgroundColor.setColor(alpha(barColor.color, 0.5), inactiveColorRatio);
            barTwoSideColor.setColor(alpha(barColor.color, 0.2), inactiveColorRatio);
            return true;
        }

        /*md - `BACKGROUND_COLOR: #000000` set the background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            bgColor = draw.getColor(params);
            return true;
        }

        /*md - `TEXT_COLOR: #ffffff` set the text color */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            titleColor.setColor(alpha(draw.getColor(params), 0.4), inactiveColorRatio);
            valueColor.setColor(alpha(draw.getColor(params), 0.4), inactiveColorRatio);
            unitColor.setColor(alpha(draw.getColor(params), 0.2), inactiveColorRatio);
            return true;
        }

        /*md - `FLOAT_PRECISION: 2` set how many digits after the decimal point (by default none) */
        if (strcmp(key, "FLOAT_PRECISION") == 0) {
            valueFloatPrecision = atoi(params);
            return true;
        }

        /*md - `SHOW_GROUP: TRUE` show group if the component is part of the current active group (default FALSE) */
        if (strcmp(key, "SHOW_GROUP") == 0) {
            showGroup = (strcmp(params, "TRUE") == 0);
            return true;
        }

        /*md - `SHOW_VALUE: TRUE` show value (default TRUE) */
        if (strcmp(key, "SHOW_VALUE") == 0) {
            showValue = (strcmp(params, "TRUE") == 0);
            return true;
        }

        /*md - `SHOW_UNIT: TRUE` show unit (default TRUE) */
        if (strcmp(key, "SHOW_UNIT") == 0) {
            showUnit = (strcmp(params, "TRUE") == 0);
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
            stringValueReplaceTitle = (strcmp(params, "TRUE") == 0);
            return true;
        }

        return GroupColorComponent::config(key, params);
    }

    void onEncoder(int id, int8_t direction)
    {
        // if (isActive) {
        //     printf("[%s] Encoder2Component onEncoder: %d %d\n", label.c_str(), id, direction);
        // }

        if (isActive && id == encoderId) {
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

#endif
