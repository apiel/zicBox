/** Description:
This code defines a specialized building block for software interfaces, known as a **Rectangle Component** (`RectComponent`).

### Purpose and Function

The primary role of this definition is to create a visual element—a simple, colored rectangle—that can be easily integrated into a larger graphical system or plugin structure.

It is built upon a more generalized foundation called a `Component`. This inheritance means the rectangle automatically gains standard behaviors, such as knowing its position and size on the screen, without having to define them explicitly.

### How It Works

1.  **Initialization:** When a new `RectComponent` is created, it retrieves its default color from system styles (like a predefined theme setting) and stores it internally.
2.  **Core Task (Rendering):** The most critical function is `render`. When the application needs to draw this element, the `render` function is called.
3.  **Drawing:** This function instructs the system's drawing tools to paint a solid, filled rectangle. It uses the stored background color and applies it precisely to the area defined by the component's calculated location and dimensions.

In summary, this blueprint provides a simple, reusable structure for displaying a basic colored box on the screen, adhering to the standard conventions of the underlying component architecture.

sha: 8d905d8306162de4753a0deae094d54263a859835f851c0d1a25653104f4c09d 
*/
#pragma once

#include "plugins/components/component.h"

#include <string>

class RectComponent : public Component {
protected:
    Color background;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
        , background(styles.colors.background)
    {
    }
    void render()
    {
        draw.filledRect(relativePosition, size, { background });
    }
};
