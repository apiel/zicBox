#ifndef _UI_COMPONENT2_ENCODER_H_
#define _UI_COMPONENT2_ENCODER_H_

#include "component.h"
#include <math.h>
#include <string>

/*md
## Encoder2

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/Encoder2.png" />

Encoder2 is used to display current audio plugin value for a given parameter.
*/
class Encoder2Component : public Component {
protected:
    const char* name = NULL;
    std::string label;
    char labelBuffer[32];
    uint8_t type = 0;
    int radius = 20;
    int insideRadius = 15;

    int fontValueSize = 10;
    int fontUnitSize = 9;
    int twoSideMargin = 2;
    int knobMargin = 2;

    struct DrawArea {
        int x;
        int xCenter;
        int y;
        int yCenter;
        int w;
        int h;
    } area;

    Point valuePosition;

    bool showKnob = true;
    bool showValue = true;
    bool showGroup = true;
    bool showUnit = true;

    const int marginTop = 3;

    bool encoderActive = true;
    int8_t encoderId = -1;
    uint8_t valueFloatPrecision = 0;

    ValueInterface* value = NULL;

    void renderLabel()
    {
        draw.textCentered({ area.xCenter, area.yCenter + insideRadius }, label, colors.title, 12);
    }

    void renderActiveGroup()
    {
        if (showGroup && encoderActive) {
            draw.filledRect({ position.x + margin, position.y + margin }, { 12, 12 }, colors.id);
            // draw.filledEllipse({ position.x + margin + 6, position.y + margin + 6 }, 6, 6, colors.id);
            draw.textCentered({ position.x + margin + 6, position.y + margin }, std::to_string(encoderId + 1).c_str(), colors.background, 8);
        }
    }

    void renderKnob()
    {
        if (showKnob) {
            int knobRadius = insideRadius - knobMargin;
            draw.filledEllipse({ area.xCenter, area.yCenter - marginTop }, knobRadius, knobRadius, colors.knob);

            // draw dot at value position
            int cx = area.xCenter;
            int cy = area.yCenter - marginTop;
            int r = knobRadius - 3;
            float angleDegrees = 280 * value->pct();
            float angleRadians = angleDegrees * M_PI / 180.0 - 180;
            int x = cx + r * cos(angleRadians);
            int y = cy + r * sin(angleRadians);

            draw.filledEllipse({ x, y }, 2, 2, colors.knobDot);
        }
    }

    void renderBar()
    {
        int val = 280 * value->pct();

        if (val < 280) {
            draw.filledPie({ area.xCenter, area.yCenter - marginTop }, radius, 130, 50, colors.barBackground);
        }
        if (val > 0) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }
            draw.filledPie({ area.xCenter, area.yCenter - marginTop }, radius, 130, endAngle, colors.bar);
        }
        draw.filledEllipse({ area.xCenter, area.yCenter - marginTop }, insideRadius, insideRadius, colors.background);
    }

    void renderCenteredBar()
    {
        int val = 280 * value->pct();

        draw.filledPie({ area.xCenter, area.yCenter - marginTop }, radius, 130, 50, colors.barBackground);
        if (val > 140) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }

            draw.filledPie({ area.xCenter, area.yCenter - marginTop }, radius, 270, endAngle, colors.bar);
        } else if (val < 140) {
            draw.filledPie({ area.xCenter, area.yCenter - marginTop }, radius, 270 - (140 - val), 270, colors.bar);
        }
        draw.filledEllipse({ area.xCenter, area.yCenter - marginTop }, insideRadius, insideRadius, colors.background);
    }

    void renderUnit()
    {
        if (showUnit && value->props().unit != NULL) {
            draw.textCentered({ valuePosition.x, valuePosition.y + fontValueSize - 5 }, value->props().unit, colors.unit, fontUnitSize);
        }
    }

    void renderValue()
    {
        std::string valStr = std::to_string(value->get());
        valStr = valStr.substr(0, valStr.find(".") + valueFloatPrecision + (valueFloatPrecision > 0 ? 1 : 0));

        int x = draw.textCentered({ valuePosition.x, valuePosition.y - 5 }, valStr.c_str(), colors.value, fontValueSize);
    }

    void renderTwoSidedValue()
    {
        int val = value->get();
        // FIXME use floating point...
        draw.textRight({ valuePosition.x - twoSideMargin, valuePosition.y - 5 }, std::to_string((int)value->props().max - val).c_str(),
            colors.value, fontValueSize - 3);
        draw.text({ valuePosition.x + twoSideMargin, valuePosition.y - 5 }, std::to_string(val).c_str(),
            colors.value, fontValueSize - 3);

        draw.line({ valuePosition.x, valuePosition.y - 10 }, { valuePosition.x, valuePosition.y + 10 }, colors.barTwoSide);
        draw.line({ valuePosition.x - 1, valuePosition.y - 10 }, { valuePosition.x - 1, valuePosition.y + 10 }, colors.barTwoSide);
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

        renderKnob();

        if (showValue) {
            renderUnit();
            if (value->props().type == VALUE_CENTERED && type == 1) {
                renderTwoSidedValue();
            } else {
                renderValue();
            }
        }
    }

    struct Colors {
        Color background;
        Color id;
        Color title;
        Color value;
        Color unit;
        Color bar;
        Color barBackground;
        Color barTwoSide;
        Color knob;
        Color knobDot;
    } colors;

    const int margin;

public:
    // margin left 15
    // margin right 10
    Encoder2Component(ComponentInterface::Props props)
        : Component(props)
        , margin(styles.margin)
        , area({ position.x, 0, position.y, 0, size.w, size.h })
    {
        colors = {
            styles.colors.background,
            draw.darken(styles.colors.grey, 0.3),
            draw.alpha(styles.colors.white, 0.4),
            draw.alpha(styles.colors.white, 0.4),
            draw.alpha(styles.colors.white, 0.2),
            styles.colors.blue,
            draw.alpha(styles.colors.blue, 0.5),
            draw.alpha(styles.colors.blue, 0.2),
            draw.getColor((char*)"#35373b"),
            draw.alpha(styles.colors.white, 0.6),
        };

        area.xCenter = (int)(area.x + (area.w * 0.5));
        area.yCenter = (int)(area.y + (area.h * 0.5));

        valuePosition = { area.xCenter, area.yCenter - marginTop - 2 };

        if (size.h < 60) {
            printf("Encoder component height too small: %dx%d. Min height is 60.\n", size.w, size.h);
            size.h = 60;
        }

        if (size.w < 60) {
            printf("Encoder component width too small: %dx%d. Min width is 60.\n", size.w, size.h);
            size.w = 60;
        }

        radius = size.h / 3.0;
        insideRadius = radius - 5;

        if (radius > 35) {
            fontValueSize = 15;
            fontUnitSize = 11;
            twoSideMargin = 5;
            knobMargin = 4;
        } else if (radius > 26) {
            fontValueSize = 14;
            twoSideMargin = 3;
            knobMargin = 3;
        } else if (radius > 24) {
            fontValueSize = 12;
            knobMargin = 3;
        }
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
            value = val(getPlugin(pluginName, track).getValue(keyValue));
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
- `TYPE: ...` is used to set the encoder type
    - `TYPE: BROWSE` TBD..
    - `TYPE: TWO_SIDED` is use for centered values like PAN
        */
        if (strcmp(key, "TYPE") == 0) {
            if (strcmp(params, "BROWSE") == 0) {
                type = 1;
            } else if (strcmp(params, "TWO_SIDED") == 0) {
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
            colors.bar = draw.getColor(params);
            colors.barBackground = draw.alpha(colors.bar, 0.5);
            colors.barTwoSide = draw.alpha(colors.bar, 0.2);
            return true;
        }

        /*md - `BACKGROUND_COLOR: #000000` set the background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(params);
            return true;
        }

        /*md - `TEXT_COLOR: #ffffff` set the text color */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            colors.title = draw.alpha(draw.getColor(params), 0.4);
            colors.value = draw.alpha(draw.getColor(params), 0.4);
            colors.unit = draw.alpha(draw.getColor(params), 0.2);
            return true;
        }

        /*md - `KNOB_COLOR: #888888` set the knob color (middle circle) */
        if (strcmp(key, "KNOB_COLOR") == 0) {
            colors.knob = draw.getColor(params);
            // colors.knobDot = draw.lighten(colors.knob, 0.7);
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

        return false;
    }

    void onEncoder(int id, int8_t direction)
    {
        if (encoderActive && id == encoderId) {
            value->increment(direction);
        }
    }

    void onGroupChanged(int8_t index) override
    {
        bool shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != encoderActive) {
            encoderActive = shouldActivate;
            renderNext();
        }
        // printf("current group: %d inccoming group: %d drawId: %d\n", group, index, drawId);
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
