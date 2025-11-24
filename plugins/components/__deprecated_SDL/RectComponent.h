/** Description:
This code defines a fundamental building block for a graphical user interface (GUI) framework, known as the `RectComponent`.

**Purpose and Functionality**

The primary role of the `RectComponent` is to draw a simple, customizable rectangle on the screen. It acts as a visual element, often used to create backgrounds, frames, or to logically group other components together in the UI.

The component manages its own appearance by tracking specific colors and settings:

1.  **Background Color:** The color that fills the entire shape.
2.  **Border Color:** The color used for the outline of the shape.
3.  **Rounded Corners:** A setting (radius) allows the rectangle to be drawn with soft, modern, rounded corners instead of sharp 90-degree edges.

**How It Works**

When the application needs to draw this component (`render`), the internal logic checks which features are active (e.g., whether the user wants a background or a border). It then instructs the underlying drawing system (like SDL) to paint the filled rectangle and/or the outline using the specified position, size, and colors.

**Configuration**

The component is highly configurable using simple text commands. Users can define its look by providing key-value pairs:

*   Setting a `BACKGROUND` color activates background drawing.
*   Setting a `BORDER` color activates border drawing.
*   Setting the `RADIUS` controls how round the corners are.

By reading these configurations, the component dynamically adapts its appearance without needing to change the core programming code.

sha: 9fde52794cb0865aa5226af4eb09b8b94d065b7b5c73fce3cc6f8473dfa3846c 
*/
#ifndef _UI_COMPONENT_RECT_H_
#define _UI_COMPONENT_RECT_H_

#include "../component.h"
#include <string>

/*md
## SDL RectComponent

Draw a rectangle on the view. Can be use to group elements together.
*/
class RectComponent : public Component {
protected:
    struct Colors {
        Color background;
        Color border;
    } colors;

    bool drawBorder = false;
    bool drawBackground = false;
    int radius = 0;

public:
    RectComponent(ComponentInterface::Props props)
        : Component(props)
    {
        colors.background = props.view->draw.styles.colors.background;
        colors.border = colors.background;
    }

    void render()
    {
        if (radius) {
            if (drawBackground) {
                draw.filledRect(position, size, radius, { colors.background });
            }
            if (drawBorder) {
                draw.rect(position, size, radius, { colors.border });
            }
        } else {
            if (drawBackground) {
                draw.filledRect(position, size, { colors.background });
            }
            if (drawBorder) {
                draw.rect(position, size, { colors.border });
            }
        }
    }

    bool config(char* key, char* value)
    {
        /*md - `BACKGROUND: #333333` set the background color and activate background drawing. */
        if (strcmp(key, "BACKGROUND") == 0) {
            colors.background = draw.getColor(value);
            drawBackground = true;
            return true;
        }

        /*md - `BORDER: #333333` set the border color and activate border drawing. */
        if (strcmp(key, "BORDER") == 0) {
            colors.border = draw.getColor(value);
            drawBorder = true;
            return true;
        }

        /*md - `RADIUS: 15` set the radius of the rounded corners. */
        if (strcmp(key, "RADIUS") == 0) {
            radius = atoi(value);
            return true;
        }

        return false;
    }
};

#endif
