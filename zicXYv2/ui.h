#pragma once

#include "zicXYv2/uiTrackShift.h"
#include "zicXYv2/uiMasterFx.h"
#include "zicXYv2/uiTopBar.h"
#include "zicXYv2/uiTrack.h"
#include "zicXYv2/uiMenu.h"

bool drawUI(Draw& d, const int winW, const int winH, bool& needFullRedraw)
{
    if (needFullRedraw) { 
        d.clear();
        d.rect({ 0, 0 }, { SCREEN_W, SCREEN_H }, { .color = { 30, 30, 200 } }); // temp to know screen size
    }

    bool rendered = false;
    int currentY = 0;
    rendered |= TopBar::draw(d, winW, needFullRedraw, currentY); // Hardcoded SCREEN_W for beginning

    if (studio.currentView == ViewTrack) {
        rendered |= UiTrack::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewTrackShift) {
        // (Draw& d, const int winW, const int winH, int currentY)
        rendered |= UiTrackShift::draw(d, winW, winH, needFullRedraw, currentY);        
    } else if (studio.currentView == ViewMaster) {
        rendered |= MasterFx::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewMenu) {
        rendered |= UiMenu::draw(d, winW, winH, needFullRedraw, currentY);
    }

    needFullRedraw = false;

    rendered |= UiMessage::draw(d, winW, winH, needFullRedraw);

    return rendered;
}
