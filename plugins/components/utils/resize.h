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
    if (resizeType & RESIZE_W) relativeSize.w = sizeOrigin.w * xFactor;
    else relativeSize.w = sizeOrigin.w;

    if (resizeType & RESIZE_H) relativeSize.h = sizeOrigin.h * yFactor;
    else relativeSize.h = sizeOrigin.h;

    if (resizeType & RESIZE_X) relativePosition.x = positionOrigin.x * xFactor;
    else relativePosition.x = positionOrigin.x;

    if (resizeType & RESIZE_Y) relativePosition.y = positionOrigin.y * yFactor;
    else relativePosition.y = positionOrigin.y;
}