#ifndef _UI_COMPONENT_ADSR_H_
#define _UI_COMPONENT_ADSR_H_

#include "component.h"
#include <string>

class AdsrComponent : public Component {
protected:
    bool encoderActive = false;

    Rect graphArea;

    std::string label = "Adsr";

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
        Color filling;
        Color label;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({
            styles.colors.background,
            color,
            draw.alpha(color, 0.2),
            draw.alpha(styles.colors.white, 0.4),
        });
    }

public:
    AdsrComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
    {
        graphArea = {
            { position.x + 4, position.y + 4 },
            { size.w - 20, size.h - 20 }
        };
    }

    void render()
    {
        draw.filledRect(position, size, colors.background);

        if (adsr[0].value == NULL || adsr[1].value == NULL || adsr[2].value == NULL || adsr[3].value == NULL) {
            return;
        }

        int w = graphArea.size.w;
        int h = graphArea.size.h;
        int a = w * adsr[0].value->pct() * 0.25;
        int d = w * adsr[1].value->pct() * 0.25;
        int r = w * adsr[3].value->pct() * 0.25;
        int s = w - a - d - r;

        int sustainY = graphArea.position.y + h * adsr[2].value->pct();

        draw.filledPolygon({
                               { graphArea.position.x, graphArea.position.y + h },
                               { graphArea.position.x + a, graphArea.position.y },
                               { graphArea.position.x + a + d, sustainY },
                               { graphArea.position.x + a + d + s, sustainY },
                               { graphArea.position.x + w, graphArea.position.y + h },
                           },
            colors.filling);

        draw.aalines({
                         { graphArea.position.x, graphArea.position.y + h },
                         { graphArea.position.x + a, graphArea.position.y },
                         { graphArea.position.x + a + d, sustainY },
                         { graphArea.position.x + a + d + s, sustainY },
                         { graphArea.position.x + w, graphArea.position.y + h },
                     },
            colors.line);

        draw.filledRect({ graphArea.position.x - 2, graphArea.position.y + h - 2 }, { 4, 4 }, colors.line);
        draw.filledRect({ graphArea.position.x + a - 2, graphArea.position.y - 2 }, { 4, 4 }, colors.line);
        draw.filledRect({ graphArea.position.x + a + d - 2, sustainY - 2 }, { 4, 4 }, colors.line);
        draw.filledRect({ graphArea.position.x + a + d + s - 2, sustainY - 2 }, { 4, 4 }, colors.line);
        draw.filledRect({ graphArea.position.x + w - 2, graphArea.position.y + h - 2 }, { 4, 4 }, colors.line);

        int textY = graphArea.position.y + h + 2;
        // draw.textCentered({ (int)(graphArea.position.x + a * 0.5), textY }, "A", colors.line, 10);
        // draw.textCentered({ (int)(graphArea.position.x + a + d * 0.5), textY }, "D", colors.line, 10);
        // draw.textCentered({ (int)(graphArea.position.x + a + d + s * 0.5), textY }, "S", colors.line, 10);
        // draw.textCentered({ (int)(graphArea.position.x + w * 0.5), textY }, "R", colors.line, 10);

        draw.textCentered({ (int)(graphArea.position.x + w * 0.5), textY }, label.c_str(), colors.label, 10);
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

        if (strcmp(key, "LABEL_COLOR") == 0) {
            colors.label = draw.getColor(value);
            return true;
        }

        if (strcmp(key, "LABEL") == 0) {
            label = value;
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
