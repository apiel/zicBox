#pragma once

#include "zicRack/uiMasterFx.h"
#include "zicRack/uiTopBar.h"

void drawStaticUI(Draw& d, const int winW, bool& needFullRedraw)
{
    if (needFullRedraw) d.clear();

    int currentY = 0;
    currentY += TopBar::draw(d, winW, needFullRedraw);
    currentY += 10;

    MasterFx::draw(d, winW, needFullRedraw, currentY);

    needFullRedraw = false;
}
