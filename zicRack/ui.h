#pragma once

#include "zicRack/uiMasterFx.h"
#include "zicRack/uiTopBar.h"

bool drawUI(Draw& d, const int winW, bool& needFullRedraw)
{
    if (needFullRedraw) d.clear();

    bool rendered = false;
    int currentY = 0;
    rendered |= TopBar::draw(d, winW, needFullRedraw);
    
    currentY += TopBar::height + 10;

    rendered |= MasterFx::draw(d, winW, needFullRedraw, currentY);

    needFullRedraw = false;

    return rendered;
}
