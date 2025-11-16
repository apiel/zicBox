#ifndef _UI_COMPONENT_TOGGLE_H_
#define _UI_COMPONENT_TOGGLE_H_

#include "../utils/color.h"
#include "../component.h"
#include <math.h>
#include <string>

/*md
## SDL Toggle

<img src="https://raw.githubusercontent.com/apiel/zicBox/main/plugins/components/SDL/Toggle.png" />

Toggle an audio plugin value parameter between his minimum and maximum value.
*/
class ToggleComponent : public Component {
protected:
    std::string label;
    char labelBuffer[32];

    const char* offLabel = NULL;
    char offLabelBuffer[32];

    int radius = 20;
    bool showGroup = false;

    Size toggleSize;
    Point togglePosition;
    Point labelPosition;

    bool encoderActive = true;
    int8_t encoderId = -1;

    ValueInterface* value = NULL;

    void renderLabel()
    {
        draw.textCentered(labelPosition, value->get() <= 0 && offLabel ? offLabel : label, 12, { colors.title });
    }

    void renderActiveGroup()
    {
        if (showGroup && encoderActive) {
            draw.filledRect({ position.x + margin, position.y + margin }, { 12, 12 }, { colors.id });
            // draw.filledCircle({ position.x + margin + 6, position.y + margin + 6 }, 6, colors.id);
            draw.textCentered({ position.x + margin + 6, position.y + margin }, std::to_string(encoderId + 1).c_str(), 8, { colors.background });
        }
    }

    void set(const char* pluginName, const char* key)
    {
        value = watch(getPlugin(pluginName, track).getValue(key));
        if (value != NULL && label.empty()) {
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
            darken({ 0x80, 0x80, 0x80, 255 }, 0.3),
            alpha(styles.colors.white, 0.4),
            alpha(styles.colors.white, 0.6),
            // draw.getColor((char*)"#35373b"),
            { 0x35, 0x37, 0x3b },
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
            { colors.background });

        if (value != NULL) {
            renderActiveGroup();
            renderLabel();

            draw.filledCircle({ togglePosition.x - radius, togglePosition.y }, radius, { colors.toggleBackground });
            draw.filledCircle({ togglePosition.x + radius, togglePosition.y }, radius, { colors.toggleBackground });
            draw.filledRect({ togglePosition.x - radius, togglePosition.y - radius }, { radius * 2, radius * 2 }, { colors.toggleBackground });

            if (value->get() > 0) {
                draw.filledCircle({ togglePosition.x + radius, togglePosition.y }, radius - 3, { colors.toggle });
            } else {
                draw.filledCircle({ togglePosition.x - radius, togglePosition.y }, radius - 3, { colors.toggle });
            }
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `VALUE: pluginName keyName` is used to set the value to control */
        if (strcmp(key, "VALUE") == 0) {
            char* pluginName = strtok(value, " ");
            char* keyValue = strtok(NULL, " ");
            set(pluginName, keyValue);
            return true;
        }

        /*md - `ENCODER_ID: 0` is used to set the encoder id that will interract with this component. Rotating left will turn of the toggle, rotating right will turn it on. */
        if (strcmp(key, "ENCODER_ID") == 0) {
            encoderId = atoi(value);
            return true;
        }

        /*md - `LABEL: custom_label` overwrite the value label */
        if (strcmp(key, "LABEL") == 0) {
            strcpy(labelBuffer, value);
            label = labelBuffer;
            return true;
        }

        /*md - `OFF_LABEL: custom_off_label` overwrite the label when status is off */
        if (strcmp(key, "OFF_LABEL") == 0) {
            strcpy(offLabelBuffer, value);
            offLabel = offLabelBuffer;
            return true;
        }

        /*md - `BACKGROUND_COLOR: #000000` set the background color */
        if (strcmp(key, "BACKGROUND_COLOR") == 0) {
            colors.background = draw.getColor(value);
            return true;
        }

        /*md - `TEXT_COLOR: #ffffff` set the text color */
        if (strcmp(key, "TEXT_COLOR") == 0) {
            colors.title = alpha(draw.getColor(value), 0.4);
            return true;
        }

        /*md - `TOGGLE_COLOR: #888888` set the toggle button color */
        if (strcmp(key, "TOGGLE_COLOR") == 0) {
            colors.toggle = draw.getColor(value);
            return true;
        }

        /*md - `SHOW_GROUP: TRUE` show group if the component is part of the current active group (default FALSE) */
        if (strcmp(key, "SHOW_GROUP") == 0) {
            showGroup = (strcmp(value, "TRUE") == 0);
            return true;
        }

        return false;
    }

    void onEncoder(int8_t id, int8_t direction)
    {
        if (encoderActive && id == encoderId) {
            value->increment(direction);
        }
    }

    /*md **Data**: */
    void* data(int id, void* userdata = NULL) override
    {
        if (id == 1) {
            if (value->get() == value->props().min) {
                value->set(value->props().max);
            } else {
                value->set(value->props().min);
            }
            return NULL;
        }
        return NULL;
    }

    void onMotion(MotionInterface& motion)
    {
        data(1);
    }
};

#endif
