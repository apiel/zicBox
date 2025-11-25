/** Description:
## Overview of the View Interface Blueprint

This code defines the `ViewInterface`, which acts as a foundational blueprint or contract for creating any specific screen, panel, or menu within a larger application's user interface (UI). It sets mandatory requirements and provides necessary tools that every screen must use to function correctly.

### Structure and Functionality

The interface primarily serves as a centralized hub, bundling three essential services required for a view to operate:

1.  **The Drawing System:** It holds a direct connection (`draw`) to the dedicated rendering engine responsible for putting images and text onto the screen. This is how the view communicates what needs to be displayed.
2.  **View Navigation:** It includes a specialized mechanism (`setView`) that allows the current screen to easily request navigation—telling the system to switch to a different screen, identified by a text name.
3.  **Shared Context:** It provides access to application-wide data (`contextVar`), allowing the screen to read or update specific global settings or states relevant to the entire program.

Additionally, it contains small status variables (`track`, `saveForPrevious`) used to manage internal state, such as which element is currently selected or whether the screen’s state should be remembered when switching away.

### Mandatory Rule

The most critical part of this blueprint is the definition of the `pushToRenderingQueue` function. By requiring this function, the interface ensures that any class built using this template **must** define a standardized process for gathering all its visual components (like buttons, text, and graphics) and lining them up to be processed and drawn by the rendering engine. This structure guarantees that every screen adheres to consistent rendering rules.

sha: f64e4de1845f0d5ceba682a5b5cc9ea0966b58e4bcaa7970fab35832c1935741 
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
