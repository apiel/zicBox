#ifndef _UI_COMPONENT_ADSR_H_
#define _UI_COMPONENT_ADSR_H_

#include "component.h"
#include <string>

class AdsrComponent : public Component {
protected:
    bool encoderActive = false;

    struct Phase {
        ValueInterface* value;
        int8_t encoderId;
    } adsr[4];

    void set(uint8_t index, int8_t encoderId, const char* pluginName, const char* key)
    {
        adsr[index].value = val(getPlugin(pluginName).getValue(key));
        adsr[index].encoderId = encoderId;

        printf("set: %s %s %d = label %s\n", pluginName, key, encoderId, adsr[index].value->label());
    }

    void set(uint8_t index, char* value)
    {
        char* pluginName = strtok(value, " ");
        char* keyValue = strtok(NULL, " ");
        int8_t encoderId = atoi(strtok(NULL, " "));
        set(index, encoderId, pluginName, keyValue);
    }

    struct Colors {
        Color background;
        Color line;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({
            draw.darken(color, 0.75),
            draw.darken(color, 0.3),
        });
    }

public:
    AdsrComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
    {
    }

    void render()
    {
        draw.filledRect(position, size, colors.background);

        if (adsr[0].value == NULL || adsr[1].value == NULL || adsr[2].value == NULL || adsr[3].value == NULL) {
            return;
        }

        int w = size.w - 1;
        int h = size.h - 1;
        int a = w * adsr[0].value->pct() * 0.25;
        int d = w * adsr[1].value->pct() * 0.25;
        int r = w * adsr[3].value->pct() * 0.25;
        int s = w - a - d - r;

        int sustainY = position.y + h * adsr[2].value->pct();

        draw.line({ position.x, position.y + h }, { position.x + a, position.y }, colors.line);
        draw.line({ position.x + a, position.y }, { position.x + a + d, sustainY }, colors.line);
        draw.line({ position.x + a + d, sustainY }, { position.x + a + d + s, sustainY }, colors.line);
        draw.line({ position.x + a + d + s, sustainY }, { position.x + w, position.y + h }, colors.line);
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "ATTACK") == 0) {
            set(0, value);
            return true;
        }

        if (strcmp(key, "DECAY") == 0) {
            set(1, value);
            return true;
        }

        if (strcmp(key, "SUSTAIN") == 0) {
            set(2, value);
            return true;
        }

        if (strcmp(key, "RELEASE") == 0) {
            set(3, value);
            return true;
        }

        return false;
    }

    void onEncoder(int id, int8_t direction)
    {
        if (encoderActive) {
            for (int i = 0; i < 4; i++) {
                if (adsr[i].encoderId == id) {
                    adsr[i].value->increment(direction);
                    return;
                }
            }
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
