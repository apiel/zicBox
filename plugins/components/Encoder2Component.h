#ifndef _UI_COMPONENT2_ENCODER_H_
#define _UI_COMPONENT2_ENCODER_H_

#include "component.h"
#include <string>

class Encoder2Component : public Component {
protected:
    const char* name = NULL;
    const char* label = NULL;
    char labelBuffer[32];
    uint8_t type = 0;
    int radius = 20;
    int insideRadius = 15;

    int fontValueSize = 10;
    int twoSideMargin = 2;

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

    void drawLabel()
    {
        draw.textCentered({ area.xCenter, area.yCenter + insideRadius }, label, colors.title, 12);
    }

    void drawBar()
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

    void drawUnit()
    {
        if (value->props().unit != NULL) {
            draw.textCentered({ area.xCenter, area.yCenter + fontValueSize - 2 - valueMarginTop }, value->props().unit, colors.unit, 10);
        }
    }

    void drawCenteredBar()
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

    void drawValue()
    {
        std::string valStr = std::to_string(value->get());
        printf("valStr: %s (valueFloatPrecision = %d)\n", valStr.c_str(), valueFloatPrecision);
        valStr = valStr.substr(0, valStr.find(".") + valueFloatPrecision + (valueFloatPrecision > 0 ? 1 : 0));

        int x = draw.textCentered({ area.xCenter, area.yCenter - 5 - valueMarginTop }, valStr.c_str(), colors.value, fontValueSize);
    }

    void drawTwoSidedValue()
    {
        int val = value->get();
        // FIXME use floating point...
        draw.textRight({ area.xCenter - twoSideMargin, area.yCenter - 5 - valueMarginTop }, std::to_string((int)value->props().max - val).c_str(),
            colors.value, fontValueSize - 3);
        draw.text({ area.xCenter + twoSideMargin, area.yCenter - 5 - valueMarginTop }, std::to_string(val).c_str(),
            colors.value, fontValueSize - 3);

        draw.line({ area.xCenter, area.yCenter - 10 - valueMarginTop }, { area.xCenter, area.yCenter + 10 - valueMarginTop },
            colors.barTwoSide);
        draw.line({ area.xCenter - 1, area.yCenter - 10 - valueMarginTop }, { area.xCenter - 1, area.yCenter + 10 - valueMarginTop },
            colors.barTwoSide);
    }

    void drawEncoder()
    {
        drawLabel();
        if (value->props().type == VALUE_CENTERED) {
            drawCenteredBar();
        } else {
            drawBar();
        }

        drawUnit();

        if (value->props().type == VALUE_CENTERED && type == 1) {
            drawTwoSidedValue();
        } else {
            drawValue();
        }
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
        Color title;
        Color value;
        Color unit;
        Color bar;
        Color barBackground;
        Color barTwoSide;
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
            draw.alpha(styles.colors.white, 0.4),
            draw.alpha(styles.colors.white, 0.4),
            draw.alpha(styles.colors.white, 0.2),
            styles.colors.blue,
            draw.alpha(styles.colors.blue, 0.5),
            draw.alpha(styles.colors.blue, 0.2),
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
        } else if (radius > 26) {
            fontValueSize = 14;
            twoSideMargin = 3;
        } else if (radius > 24) {
            fontValueSize = 12;
        }
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        if (value != NULL) {
            drawEncoder();
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
            colors.value = draw.alpha(draw.getColor(value), 0.4);
            colors.unit = draw.alpha(draw.getColor(value), 0.2);
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
