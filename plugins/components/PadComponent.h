#ifndef _UI_COMPONENT_PAD_H_
#define _UI_COMPONENT_PAD_H_

#include "component.h"
#include <string>

class PadComponent : public Component {
protected:
    const int pointerSize = 4;
    ValueInterface* valueX = NULL;
    ValueInterface* valueY = NULL;
    bool holdValue = true;
    float releaseX = 0.0f;
    float releaseY = 0.0f;

    int drawValue(const char* c, ValueInterface* value, Point position)
    {
        int x = position.x;
        x = draw.text({ x, position.y }, c, colors.title, 12);
        int val = value->get();
        if (value->props().type == VALUE_CENTERED) {
            x = draw.text({ x + 3, position.y }, std::to_string(100 - val).c_str(), colors.value, 12, { styles.font.bold });
        }
        x = draw.text({ x + 3, position.y }, value->label(), colors.title, 12);
        x = draw.text({ x + 3, position.y }, std::to_string(val).c_str(), colors.value, 12, { styles.font.bold });
        if (value->props().unit != NULL) {
            x = draw.text({ x + 2, position.y }, value->props().unit, colors.title, 10);
        }
        return x;
    }

    void setValueX(const char* pluginName, const char* key)
    {
        valueX = val(getPlugin(pluginName, track).getValue(key));
    }

    void setValueY(const char* pluginName, const char* key)
    {
        valueY = val(getPlugin(pluginName, track).getValue(key));
    }
    struct Colors {
        Color background;
        Color value;
        Color title;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ draw.darken(color, 0.75),
            color,
            draw.darken(color, 0.2) });
    }

    const int margin;

public:
    PadComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.blue))
        , margin(styles.margin)
    {
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            colors.background);

        if (valueX != NULL && valueY != NULL) {
            int y = position.y + size.h - 20;
            drawValue("X:", valueX, { position.x + 10, y });
            drawValue("Y:", valueY, { (int)(position.x + size.w * 0.5), y });

            // NOTE if it is not a centered value, should draw 0 in the middle
            // and both side from the center being positive value to 1.0 ?

            draw.filledRect(
                { position.x + (margin * 2) + (int)((size.w - (pointerSize * 2)) * valueX->pct()),
                    position.y + (margin * 2) + (int)((size.h - (pointerSize * 2)) * (1.0 - valueY->pct())) },
                { pointerSize, pointerSize },
                colors.value);
        }
    }

    bool config(char* key, char* value)
    {
        if (strcmp(key, "VALUE_X") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            setValueX(pluginName, keyValue);
            return true;
        }

        if (strcmp(key, "VALUE_Y") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            setValueY(pluginName, keyValue);
            return true;
        }

        if (strcmp(key, "HOLD_VALUE") == 0) {
            holdValue = strcmp(value, "true") == 0;
            return true;
        }

        if (strcmp(key, "RELEASE_X") == 0) {
            releaseX = atof(value);
            return true;
        }

        if (strcmp(key, "RELEASE_Y") == 0) {
            releaseY = atof(value);
            return true;
        }

        if (strcmp(key, "COLOR") == 0) {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }

    void onMotion(MotionInterface& motion)
    {
        if (valueX == NULL || valueY == NULL) {
            return;
        }

        float x = (motion.position.x - position.x - margin) / (float)(size.w - 2 * margin);
        if (x - valueX->pct() > 0.01 || valueX->pct() - x > 0.01) {
            valueX->set(x * 100.0f);
        }
        float y = 1.0 - (motion.position.y - position.y - margin) / (float)(size.h - 2 * margin);
        if (y - valueY->pct() > 0.01 || valueY->pct() - y > 0.01) {
            valueY->set(y * 100.0f);
        }
    }

    void onMotionRelease(MotionInterface& motion)
    {
        Component::onMotionRelease(motion);
        if (holdValue == false && valueX != NULL && valueY != NULL && (valueX->get() != releaseX || valueY->get() != releaseY)) {
            valueX->set(releaseX);
            valueY->set(releaseY);
        }
    }
};

#endif
