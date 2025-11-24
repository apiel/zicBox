/** Description:
This C++ header defines a foundational blueprint, called `ViewInterface`, which sets the mandatory rules and structure for any visual screen or component within an application.

In essence, this file defines the standard contract that all user interface screens must uphold, ensuring they can seamlessly interact with the core system services.

### How the View Blueprint Works

The `ViewInterface` acts as a central coordinator, linking the specific screen content (buttons, text) to the overall system framework:

1.  **Drawing Engine Connection:** It maintains a direct link (`draw`) to the system’s dedicated drawing mechanism. This ensures the view can tell the application *what* to display, while the drawing mechanism handles *how* to display it on the screen.
2.  **Navigation Control:** It holds a specialized function (`setView`) that allows the current screen to request a navigational change, such as switching to a different menu or loading a new panel.
3.  **Shared System Data:** It links to a shared data item (`contextVar`). This allows the screen to read or modify external system states, like volume levels or configuration settings, that persist across different views.
4.  **Managing Screen Elements:** The blueprint requires two critical, mandatory functions for any inheriting screen:
    *   A way to add new elements (like text boxes or images) to a list waiting to be drawn (`pushToRenderingQueue`).
    *   A way to retrieve the current list of elements that the view is managing (`getComponents`).

The remaining internal variables (like `activeGroup` and `track`) are used to maintain the screen's interactive state, managing which element is currently selected or highlighted by the user.

sha: b5ac9c888bf6c2c85c376cfbd1c13dd19c873186896a6e3bd7f9b1fc011dc993 
*/
#pragma once

#include <string>
#include <vector>
#include <functional>

#include "./drawInterface.h"

class ViewInterface {
public:
    DrawInterface& draw;
    std::function<void(std::string name)> setView;
    float *contextVar;
    int8_t activeGroup = 0;
    int8_t track = -1;
    bool saveForPrevious = true;

    ViewInterface(DrawInterface& draw, std::function<void(std::string name)> setView, float *contextVar)
        : draw(draw)
        , setView(setView)
        , contextVar(contextVar)
    {
    }

    virtual void pushToRenderingQueue(void* component) = 0;
};
