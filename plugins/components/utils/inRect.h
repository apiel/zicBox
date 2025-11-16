#pragma once

#include "plugins/components/drawInterface.h"

bool inRect(Rect rect, Point point)
{
    return (point.x >= rect.position.x &&
            point.x <= rect.position.x + rect.size.w &&
            point.y >= rect.position.y &&
            point.y <= rect.position.y + rect.size.h);
}
