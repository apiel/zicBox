#ifndef _UI_COMPONENT_ENCODER_BAR_H_
#define _UI_COMPONENT_ENCODER_BAR_H_

#include "component.h"
#include <math.h>
#include <string>

/*md
## EncoderBar

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/EncoderBar.png" />

EncoderBar is used to display current audio plugin value for a given parameter.
*/
class EncoderBarComponent : public Component {
protected:
    const char* name = NULL;
    std::string label;
    char labelBuffer[32];
    uint8_t type = 0;
    int radius = 20;
    int insideRadius = 15;

    int fontValueSize = 11;
    int fontUnitSize = 9;
    int twoSideMargin = 2;
    int knobMargin = 2;

    Point knobCenter;
    Point valuePosition;

    bool showKnob = true;
    bool showValue = true;
    bool showUnit = true;
    bool stringValueReplaceTitle = false;

    const int marginTop = 3;

    int8_t encoderId = -1;
    uint8_t valueFloatPrecision = 0;

    ValueInterface* value = NULL;

    int xCenter = 0;
    int yValue = 0;
    int yUnit = 0;

    int titleSize = 9;

    void renderLabel()
    {
        // if (stringValueReplaceTitle && value->props().type == VALUE_STRING) {
        //     draw.textCentered({ knobCenter.x, knobCenter.y + insideRadius }, value->string(), colors.title, 12, { .maxWidth = size.w - 4 });
        // } else {
        draw.textCentered({ xCenter, position.y + 1 }, label, colors.title, titleSize);
        // }
    }

    void renderValue()
    {
        if (!stringValueReplaceTitle && value->props().type == VALUE_STRING) {
            draw.textCentered({ xCenter, yValue }, value->string(), colors.value, fontValueSize);
        } else {
            std::string valStr = std::to_string(value->get());
            valStr = valStr.substr(0, valStr.find(".") + valueFloatPrecision + (valueFloatPrecision > 0 ? 1 : 0));

            int xEnd = draw.textCentered({ xCenter, yValue }, valStr.c_str(), colors.value, fontValueSize);

            if (showUnit && value->props().unit != NULL) {
                draw.text({ xEnd + 2, yUnit }, value->props().unit, colors.unit, fontUnitSize);
            }
        }
    }

    void renderTwoSidedValue()
    {
        int val = value->get();
        // FIXME use floating point...
        draw.textRight({ xCenter - twoSideMargin, yValue }, std::to_string((int)value->props().max - val).c_str(),
            colors.value, fontValueSize);
        draw.text({ xCenter + twoSideMargin, yValue }, std::to_string(val).c_str(),
            colors.value, fontValueSize);

        // draw.line({ xCenter, valuePosition.y - 10 }, { xCenter, valuePosition.y + 10 }, colors.barTwoSide);
        // draw.line({ xCenter - 1, valuePosition.y - 10 }, { xCenter - 1, valuePosition.y + 10 }, colors.barTwoSide);
    }

    void renderEncoder()
    {
        if (value->props().type == VALUE_CENTERED) {
            // renderCenteredBar();
        } else {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { (int)((size.w - 2 * margin) * value->pct()), size.h - 2 * margin },
            colors.backgroundProgress);
        }

        renderLabel();

        if (showValue) {
            if (value->props().type == VALUE_CENTERED && type == 1) {
                renderTwoSidedValue();
            } else {
                renderValue();
            }
        }
    }

    struct Colors {
        Color background;
        Color backgroundProgress;
        Color title;
        Color value;
        Color unit;
        // Color bar;
        // Color barBackground;
        // Color barTwoSide;
        // Color knob;
        // Color knobDot;
    } colors;

    const int margin;

public:
    // margin left 15
    // margin right 10
    EncoderBarComponent(ComponentInterface::Props props)
        : Component(props)
        , margin(styles.margin)
    {
        colors = {
            styles.colors.background,
            draw.lighten(styles.colors.background, 0.2),
            draw.darken(styles.colors.grey, 0.3),
            draw.alpha(styles.colors.white, 0.4),
            draw.alpha(styles.colors.white, 0.4),
            // draw.alpha(styles.colors.white, 0.2),
            // styles.colors.blue,
            // draw.alpha(styles.colors.blue, 0.5),
            // draw.alpha(styles.colors.blue, 0.2),
            // draw.getColor((char*)"#35373b"),
            // draw.alpha(styles.colors.white, 0.6),
        };

        knobCenter = { (int)(position.x + (size.w * 0.5)), (int)(position.y + ((size.h - 12) * 0.5)) };
        valuePosition = { knobCenter.x, knobCenter.y - marginTop - 2 };

        xCenter = position.x + (size.w * 0.5);
        yValue = position.y + (size.h - fontValueSize) - 4;
        yUnit = position.y + (size.h - fontUnitSize) - 4;
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        if (value != NULL) {
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
            // colors.bar = draw.getColor(params);
            // colors.barBackground = draw.alpha(colors.bar, 0.5);
            // colors.barTwoSide = draw.alpha(colors.bar, 0.2);
            return true;
        }

        /*md - `BACKGROUND: #000000` set the background color */
        if (strcmp(key, "BACKGROUND") == 0) {
            colors.background = draw.getColor(params);
            colors.backgroundProgress = draw.lighten(styles.colors.background, 0.5);
            return true;
        }

        /*md - `TEXT_COLOR: #ffffff` set the text color */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            colors.title = draw.alpha(draw.getColor(params), 0.4);
            colors.value = draw.alpha(draw.getColor(params), 0.4);
            colors.unit = draw.alpha(draw.getColor(params), 0.2);
            return true;
        }

        /*md - `FLOAT_PRECISION: 2` set how many digits after the decimal point (by default none) */
        if (strcmp(key, "FLOAT_PRECISION") == 0) {
            valueFloatPrecision = atoi(params);
            return true;
        }

        /*md - `SHOW_KNOB: FALSE` show the knob (middle circle) (default TRUE) */
        if (strcmp(key, "SHOW_KNOB") == 0) {
            showKnob = (strcmp(params, "TRUE") == 0);
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

        /*md - `STRING_VALUE_REPLACE_TITLE: true` instead to show string value in knob, show under the knob. Can be useful for long string value. */
        if (strcmp(key, "STRING_VALUE_REPLACE_TITLE") == 0) {
            stringValueReplaceTitle = (strcmp(params, "TRUE") == 0);
            return true;
        }

        return false;
    }

    void onEncoder(int id, int8_t direction)
    {
        if (id == encoderId) {
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
