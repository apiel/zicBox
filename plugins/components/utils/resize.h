/** Description:
This C/C++ header file acts as a fundamental utility for managing the scaling and repositioning of software components, such as graphical elements, buttons, or window frames. It ensures that when a program’s size changes, internal objects resize correctly and predictably.

### Core Functionality

The main purpose of this code is to provide a "smart calculator" that selectively applies scaling factors to geometric data. Instead of forcing an object to scale uniformly, developers can specify exactly which dimensions—width, height, X-position, or Y-position—should be affected by a resizing event.

### Key Components

1.  **Resize Modes:** The file defines distinct, non-overlapping resizing options. These options act like permissions or "flags." For example, one flag allows resizing the Width (W), another allows moving the X position, and they can be combined to create a comprehensive scaling plan. A predefined constant is also included to easily select all available resizing and moving options simultaneously.

2.  **The Resizing Tool:** The core function takes the original size and position of an object, along with horizontal and vertical scaling percentages (factors), and the specific resizing plan (which dimensions to affect).

### How It Works

The function processes the original dimensions step-by-step. It checks the provided resizing plan:

*   If the plan includes permission to resize the width, it multiplies the original width by the horizontal scaling factor.
*   If the plan *does not* include width resizing, the original width is preserved.

This selective application of scaling ensures that the resulting position and size accurately reflect only the changes intended by the software designer, maintaining alignment and aspect ratios as needed.

sha: 3ee567f2f76224f7aba20c95d7c82e0b8f44083cedff90d3eadbae26a61bdab4
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
