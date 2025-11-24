/** Description:
This header file defines a specialized class named `ToggleColor`, designed to manage the visual state of an element by easily switching between two different colors. It acts as a smart switch for graphical components.

**Core Concept**

The primary purpose of this tool is to hold and manage two distinct color options—a primary color and a secondary color. Only one of these colors is active and visible at any time, determined by a simple internal on/off state.

**Key Features**

1.  **Dual Color Management:** The class stores two base colors (`color1` and `color2`). The publicly accessible `color` variable holds the currently selected color.
2.  **State Toggling:** The main function, `toggle`, allows external code to set the internal state (usually 1 or 0). This instantaneously updates the active `color` to match either `color1` (on) or `color2` (off).
3.  **Automatic Shading:** When setting up the object, you can provide a base color and a "darkening ratio." The class will automatically calculate the secondary color (`color2`) by applying that shade percentage to the base color, eliminating the need to manually define two hues.
4.  **Dynamic Updates:** Functions are provided to change the base colors or adjust the darkening ratio while the program is running. For instance, if you change the darkening ratio, the darker shade (`color2`) is immediately recalculated, ensuring the current active color is correct based on the new settings.

In essence, `ToggleColor` is a utility for managing visual feedback, ensuring an element instantly displays the correct color when its operational state changes.

sha: 02430e21b20ca608638a5076e10c5fd7ab7890b148a30faac1c5847d22d80508 
*/
#ifndef _UTILS_TOGGLE_COLOR_H_
#define _UTILS_TOGGLE_COLOR_H_

#include "../../drawInterface.h"
#include "../../utils/color.h"

class ToggleColor {
protected:
    uint8_t value = 1;

public:
    Color color;
    Color color1;
    Color color2;

    ToggleColor(Color color, Color color1, Color color2)
        : color(color)
        , color1(color1)
        , color2(color2)
    {
    }

    ToggleColor(Color color, float darkenRatio)
        : color(color)
        , color1(color)
        , color2(darken(color, darkenRatio))
    {
    }

    void toggle(uint8_t _value)
    {
        value = _value;
        color = value == 1 ? color1 : color2;
    }

    void darkness(float darkenRatio)
    {
        color2 = darken(color1, darkenRatio);
        color = value == 1 ? color1 : color2;
    }

    void setColors(Color _color1, Color _color2)
    {
        color1 = _color1;
        color2 = _color2;
        color = value == 1 ? color1 : color2;
    }

    void setColor(Color _color, float darkenRatio)
    {
        color1 = _color;
        color2 = darken(_color, darkenRatio);
        color = value == 1 ? color1 : color2;
    }
};

#endif