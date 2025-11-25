/** Description:
This C++ header file serves as a simple utility tool for basic spatial geometry, likely used within a graphics or drawing application.

**Core Purpose**

The primary function of this file is to accurately determine if a specific coordinate or location falls within the boundaries of a defined rectangular area.

**How the Code Works**

The file contains one key operation called `inRect`. This function takes two inputs:
1. A **Rectangle** (or a "Box") which has a position (top-left corner) and a size (width and height).
2. A single **Point** (an exact location defined by X and Y coordinates).

The function performs a calculation: it checks if the Point’s X-coordinate is greater than the Box’s starting X-coordinate but less than the Box’s ending X-coordinate (start X plus width). It simultaneously checks the Y-coordinate against the Box’s height.

If the Point satisfies all these boundary conditions, the function returns a "True" result, meaning the location is successfully contained within the Box. This type of check is fundamental for tasks like detecting if a user has clicked on a button or if two graphical objects are colliding.

The file also includes definitions for the basic geometric shapes (like Rectangle and Point) from an external drawing interface library.

sha: 7f120258a32c09b58c3fa54963f6a8aea633f65517cd52f3ce6d7af9be642de3 
*/
#pragma once

#include "plugins/components/drawInterface.h"

bool inRect(Rect rect, Point point)
{
    return (point.x >= rect.position.x &&
            point.x <= rect.position.x + rect.size.w &&
            point.y >= rect.position.y &&
            point.y <= rect.position.y + rect.size.h);
}
