#ifndef _UI_COMPONENT_TRIGGER_NOTE_H_
#define _UI_COMPONENT_TRIGGER_NOTE_H_

#include "component.h"
#include <string>

class TriggerNoteComponent : public Component
{
protected:
    AudioPlugin &plugin;

    bool isPressed = false;
    float pressedValue = 0.0f;

    void render()
    {
        draw.filledRect(
            {position.x + margin, position.y + margin},
            {size.w - 2 * margin, size.h - 2 * margin},
            isPressed ? colors.pressedBackground : colors.background);

        // draw.textCentered(labelPosition, label, colors.title, fontSize);
    }

    // void set(const char *pluginName, const char *key)
    // {
    //     value = val(getPlugin(pluginName).getValue(key));
    //     if (value != NULL && label == NULL)
    //     {
    //         label = (char *)value->label();
    //     }
    // }

    struct Colors
    {
        Color background;
        Color pressedBackground;
        Color title;
    } colors;

    Colors getColorsFromColor(Color color)
    {
        return Colors({draw.darken(color, 0.6), draw.darken(color, 0.3), color});
    }

    const int margin;

public:
    TriggerNoteComponent(ComponentInterface::Props &props)
        : Component(props),
          colors(getColorsFromColor(styles.colors.blue)),
          margin(styles.margin),
          plugin(getPlugin("Kick23"))
        //   labelPosition({(int)(position.x + size.w * 0.5f), (int)(position.y + size.h * 0.5f - (fontSize * 0.5f))})
    {
    }

    bool config(char *key, char *value)
    {
        // if (strcmp(key, "VALUE") == 0)
        // {
        //     printf("value: %s\n", value);
        //     char *pluginName = strtok(value, " ");
        //     char *keyValue = strtok(NULL, " ");
        //     set(pluginName, keyValue);
        //     return true;
        // }

        // if (strcmp(key, "LABEL") == 0)
        // {
        //     // maybe we check that the current point is not pointing to value->label()
        //     // and free the label, but why should we assign multiple time a label...
        //     label = new char[strlen(value) + 1];
        //     strcpy(label, value);
        //     return true;
        // }

        // if (strcmp(key, "PRESSED_VALUE") == 0)
        // {
        //     pressedValue = atof(value);
        //     return true;
        // }

        // TODO PRESSED_INT_VALUE 

        if (strcmp(key, "COLOR") == 0)
        {
            colors = getColorsFromColor(draw.getColor(value));
            return true;
        }

        return false;
    }

    void onMotion(MotionInterface &motion)
    {
        isPressed = true;
        renderNext();
        plugin.noteOn(48, 127);
    }

    void onMotionRelease(MotionInterface &motion)
    {
        if (motion.originIn({position, size}))
        {
            isPressed = false;
            renderNext();
            // value->set(pressedValue);
            plugin.noteOff(48, 0);
        }
    }
};

#endif
