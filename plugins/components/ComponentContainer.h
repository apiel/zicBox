/** Description:
This header file establishes the fundamental blueprint, or structure, for organizing visual elements within a software application. It defines a standardized "wrapper," called the `ComponentContainer`, which is used to manage and position anything that needs to be displayed on a screen, such as a window, a button, or a graph.

The goal of this structure is to separate the physical location of a component from its actual visual behavior.

### Key Features of the Component Container

The container holds four essential pieces of information about a visual element:

1.  **Identity:** A text `name` to label the component (e.g., "Main Menu Button").
2.  **Position:** Coordinates that define where the component starts on the screen.
3.  **Size:** Dimensions defining the width and height of the component.
4.  **Visual Logic Link:** A critical reference to a `ViewInterface`. This link is the component's instruction manual; it tells the container *how* to draw itself, what colors to use, and how to react to user interactions like mouse clicks.

### Basic Idea of Operation

This blueprint allows the system to manage complex layouts efficiently. The main program creates these containers, assigns a specific set of drawing rules (the View Interface) to them, and sets their physical location and dimensions. The container acts as the stable framework, holding the location and name, while relying on the linked interface to handle all the dynamic, visual aspects. This separation ensures that the software structure remains tidy, even as the screen content changes.

sha: 0bd8ac75aff7ec30e54c73e06071b4111ae37727cbedbb6b6e0d72278dd44c86 
*/
#pragma once

#include "plugins/components/ViewInterface.h"
#include "plugins/components/drawInterface.h"

class ComponentContainer {
public:
    std::string name;
    Point position = { 0, 0 };
    Size size = { 0, 0 };
    ViewInterface* view = NULL;

    ComponentContainer() { }

    ComponentContainer(ViewInterface* view, std::string name, Point position, Size size)
        : view(view)
        , name(name)
        , position(position)
        , size(size)
    {
    }
};
