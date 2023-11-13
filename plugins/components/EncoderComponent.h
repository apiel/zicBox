#ifndef _UI_COMPONENT_ENCODER_H_
#define _UI_COMPONENT_ENCODER_H_

#include "component.h"
#include <string>

// TODO use function pointer on encoder initialisation to assign draw function base on type

class EncoderComponent : public Component {
protected:
    const char* name = NULL;
    const char* label = NULL;
    char labelBuffer[32];
    uint8_t type = 0;

    struct DrawArea {
        int x;
        int xCenter;
        int y;
        int w;
        int h;
    } area;

    const int valueMarginTop = 15;

    bool encoderActive = false;
    int8_t encoderId = -1;

    ValueInterface* value = NULL;

    void drawLabel()
    {
        draw.text({ area.x, area.y }, label, colors.title, 12);
    }

    void drawBar()
    {
        draw.line({ area.x, area.y + size.h - 10 },
            { position.x + size.w - 10, area.y + size.h - 10 },
            colors.title);

        int x = area.x;
        int y = area.y + size.h - 10;
        int x2 = x + ((size.w - 20) * value->pct());
        draw.line({ x, y }, { x2, y }, colors.value);
        draw.line({ x, y - 1 }, { x2, y - 1 }, colors.value);
    }

    void drawValue()
    {
        // fixme use floating point...
        int x = draw.textCentered({ area.xCenter, area.y + valueMarginTop }, std::to_string((int)value->get()).c_str(),
            colors.value, 20, { styles.font.bold });

        if (value->props().unit != NULL) {
            draw.text({ x + 2, area.y + valueMarginTop + 8 }, value->props().unit, colors.title, 10);
        }
    }

    void drawEncoder()
    {
        drawLabel();
        drawValue();
        drawBar();
    }

    void drawCenteredBar()
    {
        int x = area.x + (area.w * 0.5);
        int y = area.y + area.h - 10;
        int x2 = area.x + (area.w * value->pct());
        draw.line({ x, y }, { x2, y }, colors.value);
        draw.line({ x, y - 1 }, { x2, y - 1 }, colors.value);
    }

    void drawCenteredEncoder()
    {
        if (type == 1) {
            draw.textCentered({ area.xCenter, area.y }, label, colors.title, 12);

            int val = value->get();
            // fixme use floating point...
            draw.textRight({ area.x + area.w, area.y + valueMarginTop }, std::to_string(val).c_str(),
                colors.value, 20, { styles.font.bold });
            draw.text({ area.x, area.y + valueMarginTop }, std::to_string((int)value->props().max - val).c_str(),
                colors.value, 20, { styles.font.bold });
        } else {
            drawLabel();
            drawValue();
        }

        drawCenteredBar();
    }

    void drawStringEncoder()
    {
        if (type == 1) {
            draw.text({ area.x, area.y + 5 }, value->string().c_str(), colors.value, 12, { .maxWidth = area.w });
            char valueStr[20];
            sprintf(valueStr, "%d / %d", (int)(value->get()), (int)value->props().max);
            draw.textRight({ area.x + area.w, area.y + 25 }, valueStr, colors.title, 10);
        } else {
            drawLabel();
            draw.text({ area.x, area.y + 18 }, value->string().c_str(), colors.value, 12, { .maxWidth = area.w });
        }
        drawBar();
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
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.6),
            draw.darken(color, 0.3),
            draw.darken(color, 0.2),
            color });
    }

    const int margin;

public:
    // margin left 15
    // margin right 10
    EncoderComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
        , area({ position.x + 15, 0, position.y, size.w - (15 + 10), size.h })
    {
        area.xCenter = (int)(area.x + (area.w * 0.5));
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        if (encoderActive) {
            draw.filledRect(
                { position.x + margin, position.y + margin },
                { 12, 12 },
                colors.id);
            draw.textCentered({ position.x + margin + 6, position.y + margin }, std::to_string(encoderId + 1).c_str(), colors.background, 8);
        }

        if (value != NULL) {
            if (value->props().type == VALUE_CENTERED) {
                drawCenteredEncoder();
            } else if (value->props().type == VALUE_STRING) {
                drawStringEncoder();
            } else {
                drawEncoder();
            }
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
            colors = getColorsFromColor(draw.getColor(value));
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
