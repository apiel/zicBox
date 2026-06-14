#pragma once

#include "draw/utils/inRect.h"
#include "zicXYv2/studio.h"

void drawPad(Draw& d, Rect rect, const char* name, Color color, float fx, float fy)
{
    int pointerSize = 6;
    int padW = rect.size.w, padH = rect.size.h;
    int padX = rect.position.x, padY = rect.position.y;

    d.filledRect({ padX, padY }, { padW, padH }, { .color = { 20, 20, 25 } });
    d.rect({ padX, padY }, { padW, padH }, { .color = { 60, 60, 70 } });
    d.line({ padX + padW / 2, padY }, { padX + padW / 2, padY + padH }, { .color = { 40, 40, 45 } });
    d.text({ padX + 5, padY + 5 }, name, 8, { .color = color, .font = &PoppinsLight_8 });

    d.filledRect({ padX + (int)(fx * (padW - pointerSize)), padY + (int)(fy * (padH - pointerSize)) }, { pointerSize, pointerSize }, { .color = color });
}
