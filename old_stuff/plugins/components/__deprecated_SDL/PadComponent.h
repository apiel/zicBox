/** Description:
This header file defines a specialized visual element known as the **Pad Component**, commonly used to manage two separate system or audio parameters simultaneously, such as controlling effects in a music application. It functions like an XY pad.

### Core Functionality

The component provides a single, interactive surface (the pad) that visually maps its horizontal position (X-axis) and vertical position (Y-axis) to two distinct external values. When a user drags a pointer across this pad, the component calculates the position and instantly updates the linked external parameters.

### How It Works

1.  **Configuration:** The component must first be configured to link its X-axis and Y-axis to specific parameters found within other software plugins.
2.  **Interaction:** It captures user input, such as mouse movement or touch, and translates the location within the pad area into corresponding percentage values for Value X and Value Y.
3.  **Visualization:** The component renders the pad's background, displays a small pointer indicating the current control position, and provides numerical readouts and labels for the two parameters being adjusted.
4.  **Release Behavior:** A customizable feature allows the component to define whether the values "hold" at their current position after interaction stops, or if they should automatically "release" and snap back to predefined default settings.

In essence, the Pad Component acts as a dual-control interface, simplifying the simultaneous adjustment of two related variables through intuitive visual feedback.

sha: 27deba250f2afb079e7caf0b391e2f4f85d299f75c12980e743e40406d420259 
*/
#ifndef _UI_COMPONENT_PAD_H_
#define _UI_COMPONENT_PAD_H_

#include "../utils/color.h"
#include "../component.h"
#include <string>

/*md
## SDL Pad

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/Pad.png" />

XY pad component, allows to control two audio plugin value parameters.
*/
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
        x = draw.text({ x, position.y }, c, 12, { colors.title });
        int val = value->get();
        void* fontBold = draw.getFont(styles.font.bold);
        if (value->props().type == VALUE_CENTERED) {
            x = draw.text({ x + 3, position.y }, std::to_string(100 - val).c_str(), 12, { colors.value, fontBold });
        }
        x = draw.text({ x + 3, position.y }, value->label(), 12, { colors.title });
        x = draw.text({ x + 3, position.y }, std::to_string(val).c_str(), 12, { colors.value, fontBold });
        if (value->props().unit.length() > 0) {
            x = draw.text({ x + 2, position.y }, value->props().unit, 10, { colors.title });
        }
        return x;
    }

    void setValueX(const char* pluginName, const char* key)
    {
        valueX = watch(getPlugin(pluginName, track).getValue(key));
    }

    void setValueY(const char* pluginName, const char* key)
    {
        valueY = watch(getPlugin(pluginName, track).getValue(key));
    }
    struct Colors {
        Color background;
        Color value;
        Color title;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({ darken(color, 0.75),
            color,
            darken(color, 0.2) });
    }

    const int margin;

public:
    PadComponent(ComponentInterface::Props props)
        : Component(props)
        , colors(getColorsFromColor(styles.colors.primary))
        , margin(styles.margin)
    {
    }

    void render()
    {
        draw.filledRect(
            { position.x + margin, position.y + margin },
            { size.w - 2 * margin, size.h - 2 * margin },
            { colors.background });

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
                { colors.value });
        }
    }

    bool config(char* key, char* value)
    {
        /*md `VALUE_X: pluginName key` set X value to update on audio plugin parameter */
        if (strcmp(key, "VALUE_X") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            setValueX(pluginName, keyValue);
            return true;
        }

        /*md `VALUE_Y: pluginName key` set Y value to update on audio plugin parameter */
        if (strcmp(key, "VALUE_Y") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            setValueY(pluginName, keyValue);
            return true;
        }

        /*md `HOLD_VALUE: true|false` set if the value should be held or not */
        if (strcmp(key, "HOLD_VALUE") == 0) {
            holdValue = strcmp(value, "true") == 0;
            return true;
        }

        /*md `RELEASE_X: value` X value set when pad is released */
        if (strcmp(key, "RELEASE_X") == 0) {
            releaseX = atof(value);
            return true;
        }

        /*md `RELEASE_Y: value` Y value set when pad is released */
        if (strcmp(key, "RELEASE_Y") == 0) {
            releaseY = atof(value);
            return true;
        }

        /*md `COLOR: color` set the color of the pad */
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
