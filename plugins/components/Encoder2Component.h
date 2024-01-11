#ifndef _UI_COMPONENT2_ENCODER_H_
#define _UI_COMPONENT2_ENCODER_H_

#include "component.h"
#include <math.h>
#include <string>

/*md
## Encoder2Component

![Encoder2](data:image/png;base64,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)

Encoders are used to control values.
*/
class Encoder2Component : public Component {
protected:
    const char* name = NULL;
    const char* label = NULL;
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

    /*md ### Config file */
    bool config(char* key, char* params)
    {
        /*md - `VALUE: pluginName keyName` is used to set the value to control */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(params, " ");
            char* keyValue = strtok(NULL, " ");
            value = val(getPlugin(pluginName, track).getValue(keyValue));
            if (value != NULL && label == NULL) {
                valueFloatPrecision = value->props().floatingPoint;
                label = value->label();
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
    - `TYPE: TWO_SIDED` is use for centered values like PAN ![TWO_SIDED](data:image/png;base64,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)
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
