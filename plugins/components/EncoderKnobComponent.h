#ifndef _UI_COMPONENT_ENCODER_KNOB_H_
#define _UI_COMPONENT_ENCODER_KNOB_H_

#include "component.h"
#include <math.h>
#include <string>

class EncoderKnobComponent : public Component {
protected:
    const char* name = NULL;
    const char* label = NULL;
    char labelBuffer[32];
    uint8_t type = 0;
    int radius = 20;
    int insideRadius = 15;

    int fontValueSize = 10;
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

    const int valueMarginTop = 3;

    bool encoderActive = false;
    int8_t encoderId = -1;
    uint8_t valueFloatPrecision = 0;

    ValueInterface* value = NULL;

    void renderLabel()
    {
        draw.textCentered({ area.xCenter, area.yCenter + insideRadius }, label, colors.title, 12);
    }

    void renderActiveGroup()
    {
        if (encoderActive) {
            draw.filledRect({ position.x + margin, position.y + margin }, { 12, 12 }, colors.id);
            // draw.filledEllipse({ position.x + margin + 6, position.y + margin + 6 }, 6, 6, colors.id);
            draw.textCentered({ position.x + margin + 6, position.y + margin }, std::to_string(encoderId + 1).c_str(), colors.background, 8);
        }
    }

    void renderBar()
    {
        int val = 280 * value->pct();

        if (val < 280) {
            draw.filledPie({ area.xCenter, area.yCenter - valueMarginTop }, radius, 130, 50, colors.barBackground);
        }
        if (val > 0) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }
            draw.filledPie({ area.xCenter, area.yCenter - valueMarginTop }, radius, 130, endAngle, colors.bar);
        }
        draw.filledEllipse({ area.xCenter, area.yCenter - valueMarginTop }, insideRadius, insideRadius, colors.background);
    }

    void renderCenteredBar()
    {
        int val = 280 * value->pct();

        draw.filledPie({ area.xCenter, area.yCenter - valueMarginTop }, radius, 130, 50, colors.barBackground);
        if (val > 140) {
            int endAngle = 130 + val;
            if (endAngle > 360) {
                endAngle = endAngle - 360;
            }

            draw.filledPie({ area.xCenter, area.yCenter - valueMarginTop }, radius, 270, endAngle, colors.bar);
        } else if (val < 140) {
            draw.filledPie({ area.xCenter, area.yCenter - valueMarginTop }, radius, 270 - (140 - val), 270, colors.bar);
        }
        draw.filledEllipse({ area.xCenter, area.yCenter - valueMarginTop }, insideRadius, insideRadius, colors.background);
    }

    void renderKnob()
    {
        int knobRadius = insideRadius - knobMargin;
        draw.filledEllipse({ area.xCenter, area.yCenter - valueMarginTop }, knobRadius, knobRadius, colors.knob);

        // draw dot at value position
        int cx = area.xCenter;
        int cy = area.yCenter - valueMarginTop;
        int r = knobRadius - 3;
        float angleDegrees = 280 * value->pct();
        float angleRadians = angleDegrees * M_PI / 180.0 - 180;
        int x = cx + r * cos(angleRadians);
        int y = cy + r * sin(angleRadians);

        draw.filledEllipse({ x, y }, 2, 2, colors.knobDot);
    }

    void renderEncoder()
    {
        renderLabel();
        if (value->props().type == VALUE_CENTERED) {
            renderCenteredBar();
        } else {
            renderBar();
        }
        renderKnob();
        renderActiveGroup();
    }

    void set(const char* pluginName, const char* key)
    {
        value = val(getPlugin(pluginName).getValue(key));
        if (value != NULL && label == NULL) {
            valueFloatPrecision = value->props().floatingPoint;
            label = value->label();
        }
    }

    struct Colors {
        Color background;
        Color id;
        Color title;
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
    EncoderKnobComponent(ComponentInterface::Props props)
        : Component(props)
        , margin(styles.margin)
        , area({ position.x, 0, position.y, 0, size.w, size.h })
    {
        colors = {
            styles.colors.background,
            draw.darken(styles.colors.grey, 0.3),
            draw.alpha(styles.colors.white, 0.4),
            styles.colors.blue,
            draw.alpha(styles.colors.blue, 0.5),
            draw.alpha(styles.colors.blue, 0.2),
            draw.getColor((char*)"#35373b"),
            // draw.lighten(styles.colors.grey, 0.7),
            draw.alpha(styles.colors.white, 0.6),
        };

        area.xCenter = (int)(area.x + (area.w * 0.5));
        area.yCenter = (int)(area.y + (area.h * 0.5));

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
            twoSideMargin = 5;
            knobMargin = 3;
        } else if (radius > 26) {
            fontValueSize = 14;
            twoSideMargin = 3;
            knobMargin = 4;
        } else if (radius > 24) {
            fontValueSize = 12;
            knobMargin = 4;
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

    bool config(char* key, char* value)
    {
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            set(pluginName, keyValue);
            return true;
        }

        if (strcmp(key, "ENCODER_ID") == 0) {
            encoderId = atoi(value);
            return true;
        }

        if (strcmp(key, "TYPE") == 0) {
            if (strcmp(value, "BROWSE") == 0) {
                type = 1;
            } else if (strcmp(value, "TWO_SIDED") == 0) {
                type = 1;
            } else {
                type = atoi(value);
            }
            return true;
        }

        if (strcmp(key, "LABEL") == 0) {
            strcpy(labelBuffer, value);
            label = labelBuffer;
            return true;
        }

        if (strcmp(key, "COLOR") == 0) {
            colors.bar = draw.getColor(value);
            colors.barBackground = draw.alpha(colors.bar, 0.5);
            colors.barTwoSide = draw.alpha(colors.bar, 0.2);
            return true;
        }

        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "TEXT_COLOR") == 0) {
            colors.title = draw.alpha(draw.getColor(value), 0.4);
            return true;
        }

        if (strcmp(key, "KNOB_COLOR") == 0) {
            colors.knob = draw.getColor(value);
            // colors.knobDot = draw.lighten(colors.knob, 0.7);
            return true;
        }

        if (strcmp(key, "FLOAT_PRECISION") == 0) {
            valueFloatPrecision = atoi(value);
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
        int8_t shouldActivate = false;
        if (group == index || group == -1) {
            shouldActivate = true;
        }
        if (shouldActivate != encoderActive) {
            encoderActive = shouldActivate;
            renderNext();
        }
        // printf("current group: %d inccoming group: %d drawId: %d\n", group, index, drawId);
    }
};

#endif
