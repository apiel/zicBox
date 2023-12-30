#ifndef _UI_COMPONENT_TOGGLE_H_
#define _UI_COMPONENT_TOGGLE_H_

#include "component.h"
#include <math.h>
#include <string>

class ToggleComponent : public Component {
protected:
    const char* label = NULL;
    char labelBuffer[32];

    const char* offLabel = NULL;
    char offLabelBuffer[32];

    int radius = 20;
    bool showGroup = true;

    Size toggleSize;
    Point togglePosition;
    Point labelPosition;

    bool encoderActive = false;
    int8_t encoderId = -1;

    ValueInterface* value = NULL;

    void renderLabel()
    {
        draw.textCentered(labelPosition, value->get() <= 0 && offLabel ? offLabel : label, colors.title, 12);
    }

    void renderActiveGroup()
    {
        if (showGroup && encoderActive) {
            draw.filledRect({ position.x + margin, position.y + margin }, { 12, 12 }, colors.id);
            // draw.filledEllipse({ position.x + margin + 6, position.y + margin + 6 }, 6, 6, colors.id);
            draw.textCentered({ position.x + margin + 6, position.y + margin }, std::to_string(encoderId + 1).c_str(), colors.background, 8);
        }
    }

    void set(const char* pluginName, const char* key)
    {
        value = val(getPlugin(pluginName, track).getValue(key));
        if (value != NULL && label == NULL) {
            label = value->label();
        }
    }

    struct Colors {
        Color background;
        Color id;
        Color title;
        Color toggle;
        Color toggleBackground;
    } colors;

    const int margin;

public:
    ToggleComponent(ComponentInterface::Props props)
        : Component(props)
        , margin(styles.margin)
    {
        colors = {
            styles.colors.background,
            draw.darken(styles.colors.grey, 0.3),
            draw.alpha(styles.colors.white, 0.4),
            draw.alpha(styles.colors.white, 0.6),
            draw.getColor((char*)"#35373b"),
        };

        togglePosition.y = position.y + (size.h * 0.5);
        togglePosition.x = position.x + (size.w * 0.5);

        // labelPosition = { (int)(position.x + (size.w * 0.5)), position.y + size.h - 22 };
        labelPosition = { (int)(position.x + (size.w * 0.5)), (int)(togglePosition.y + size.h / 3.0 - 5) }; // for size.h / 3.0 - 5 see encoder2

        radius = size.h * 0.1;
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        if (value != NULL) {
            renderActiveGroup();
            renderLabel();

            draw.filledEllipse({ togglePosition.x - radius, togglePosition.y }, radius, radius, colors.toggleBackground);
            draw.filledEllipse({ togglePosition.x + radius, togglePosition.y }, radius, radius, colors.toggleBackground);
            draw.filledRect({ togglePosition.x - radius, togglePosition.y - radius }, { radius * 2, radius * 2 }, colors.toggleBackground);

            if (value->get() > 0) {
                draw.filledEllipse({ togglePosition.x + radius, togglePosition.y }, radius - 3, radius - 3, colors.toggle);
            } else {
                draw.filledEllipse({ togglePosition.x - radius, togglePosition.y }, radius - 3, radius - 3, colors.toggle);
            }
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

        if (strcmp(key, "LABEL") == 0) {
            strcpy(labelBuffer, value);
            label = labelBuffer;
            return true;
        }

        if (strcmp(key, "OFF_LABEL") == 0) {
            strcpy(offLabelBuffer, value);
            offLabel = offLabelBuffer;
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

        if (strcmp(key, "TOGGLE_COLOR") == 0) {
            colors.toggle = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "SHOW_GROUP") == 0) {
            showGroup = (strcmp(value, "TRUE") == 0);
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
};

#endif
