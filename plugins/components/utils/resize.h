/** Description:
**Component Resizing Logic Definition**

This header file serves as a crucial blueprint for managing how visual components adapt to changes in screen or window size within an application. It defines the core rules necessary for scaling objects dynamically.

**Key Definitions**

The file establishes a set of distinct numerical codes, known as flags, which are used to specify the exact type of resizing allowed for any component. These codes permit fine-grained control, allowing developers to choose combinations such as resizing only the width, or resizing both the horizontal position (X) and the height (H) simultaneously. A convenience flag, `RESIZE_ALL`, groups all possible dimension and position resizing options together.

**Core Functionality**

The primary purpose is encapsulated in a dedicated scaling function. This function takes an object’s original dimensions and position, along with specified scaling factors (e.g., indicating how much the overall screen or window size has grown horizontally and vertically).

**How It Works**

The function intelligently calculates the object’s new position and size. It applies the scaling factors (the growth percentages) only to the dimensions and coordinates that were explicitly enabled using the resizing codes. For example, if an object is configured only to resize its width, its calculated new position and height will remain unchanged, ensuring predictable layout behavior as the application window changes size.

sha: f4ec5b27efb9b9eba29cc1c9f7cd4c2cb584f0bbad40e9c0da0e1991388ec615 
*/
#pragma once

#include "plugins/components/baseInterface.h"

enum ResizeType {
    RESIZE_NONE = 1 << 0, // 1
    RESIZE_X = 1 << 1, // 2
    RESIZE_Y = 1 << 2, // 4
    RESIZE_W = 1 << 3, // 8
    RESIZE_H = 1 << 4, // 16
};

const uint8_t RESIZE_ALL = ResizeType::RESIZE_X | ResizeType::RESIZE_Y | ResizeType::RESIZE_W | ResizeType::RESIZE_H;

void resizeOriginToRelative(uint8_t resizeType, float xFactor, float yFactor,
    Point& positionOrigin, Size& sizeOrigin, Point& relativePosition, Size& relativeSize)
{
    relativeSize.w = (resizeType & RESIZE_W) ? sizeOrigin.w * xFactor : sizeOrigin.w;
    relativeSize.h = (resizeType & RESIZE_H) ? sizeOrigin.h * yFactor : sizeOrigin.h;

    relativePosition.x = (resizeType & RESIZE_X) ? positionOrigin.x * xFactor : positionOrigin.x;
    relativePosition.y = (resizeType & RESIZE_Y) ? positionOrigin.y * yFactor : positionOrigin.y;
}
