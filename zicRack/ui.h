#pragma once

#include "zicRack/uiMasterFx.h"
#include "zicRack/uiTopBar.h"
#include "zicRack/uiTrack.h"

bool drawUI(Draw& d, const int winW, const int winH, bool& needFullRedraw)
{
    if (needFullRedraw) d.clear();

    bool rendered = false;
    int currentY = 0;
    rendered |= TopBar::draw(d, winW, needFullRedraw);

    currentY += TopBar::height;

    if (studio.currentView == ViewTrack) {
        rendered |= UiTrack::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewMaster) {
        rendered |= MasterFx::draw(d, winW, needFullRedraw, currentY);
    }

    needFullRedraw = false;

    return rendered;
}
