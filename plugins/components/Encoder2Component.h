#ifndef _UI_COMPONENT2_ENCODER_H_
#define _UI_COMPONENT2_ENCODER_H_

#include "component.h"
#include <string>

// TODO use function pointer on encoder initialisation to assign draw function base on type

class Encoder2Component : public Component {
protected:
    const char* name = NULL;
    const char* label = NULL;
    char labelBuffer[32];
    uint8_t type = 0;

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
        draw.textCentered({ area.xCenter, area.yCenter + 15 }, label, colors.title, 12);
    }

    void drawBar()
    {
        int val = 280 * value->pct();
        int endAngle = 130 + val;
        if (endAngle > 360) {
            endAngle = endAngle - 360;
        }
        // printf("val: %d, endAngle: %d\n", val, endAngle);

        if (endAngle != 50) {
            draw.filledPie({ area.xCenter, area.yCenter - valueMarginTop }, 20, 130, 50, colors.barBackground);
        }
        if (endAngle != 130) {
            draw.filledPie({ area.xCenter, area.yCenter - valueMarginTop }, 20, 130, endAngle, colors.bar);
        }
        draw.filledEllipse({ area.xCenter, area.yCenter - valueMarginTop }, 15, 15, colors.background);
    }

    void drawValue()
    {
        std::string valStr = std::to_string(value->get());
        valStr = valStr.substr(0, valStr.find(".") + valueFloatPrecision + (valueFloatPrecision > 0 ? 1 : 0));

        int x = draw.textCentered({ area.xCenter, area.yCenter - 5 - valueMarginTop }, valStr.c_str(), colors.value, 10);

        if (value->props().unit != NULL) {
            draw.textCentered({ area.xCenter, area.yCenter + 7 - valueMarginTop }, value->props().unit, colors.unit, 10);
        }
    }

    void drawEncoder()
    {
        drawLabel();
        drawBar();
        drawValue();
    }

    void set(const char* pluginName, const char* key)
    {
        value = val(getPlugin(pluginName).getValue(key));
        if (value != NULL && label == NULL) {
            label = value->label();
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
        Color barEdge;
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
    }

    void render()
    {
        colors = {
            styles.colors.background,
            draw.alpha(styles.colors.white, 0.6),
            draw.alpha(styles.colors.white, 0.4),
            draw.alpha(styles.colors.white, 0.4),
            draw.alpha(styles.colors.white, 0.2),
            styles.colors.blue,
            draw.alpha(styles.colors.blue, 0.5),
            styles.colors.white,
        };

        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        // if (encoderActive) {
        //     draw.filledRect(
        //         { position.x + margin, position.y + margin },
        //         { 12, 12 },
        //         colors.id);
        //     draw.textCentered({ position.x + margin + 6, position.y + margin }, std::to_string(encoderId + 1).c_str(), colors.background, 8);
        // }

        if (value != NULL) {
            // if (value->props().type == VALUE_CENTERED) {
            //     drawCenteredEncoder();
            // } else if (value->props().type == VALUE_STRING) {
            //     drawStringEncoder();
            // } else {
            drawEncoder();
            // }
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "VALUE") == 0) {
            // printf("value: %s\n", value);
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
            return true;
        }

        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
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
