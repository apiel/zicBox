#pragma once

#include "zicXYv2/uiMasterFx.h"
#include "zicXYv2/uiTopBar.h"
#include "zicXYv2/uiTrack.h"
#include "zicXYv2/uiSeq.h"
#include "zicXYv2/uiClips.h"
#include "zicXYv2/uiMenu.h"
#include "zicXYv2/uiChain.h"

bool drawUI(Draw& d, const int winW, const int winH, bool& needFullRedraw)
{
    if (needFullRedraw) d.clear();

    bool rendered = false;
    int currentY = 0;
    rendered |= TopBar::draw(d, winW, needFullRedraw, currentY); // Hardcoded SCREEN_W for beginning

    if (studio.currentView == ViewTrack) {
        rendered |= UiTrack::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewSeq) {
        rendered |= UiSeq::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewClips) {
        rendered |= UiClips::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewChain) {
        rendered |= UiChain::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewMaster) {
        rendered |= MasterFx::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewProject) {
        rendered |= UiMenu::draw(d, winW, winH, needFullRedraw, currentY);
    } else if (studio.currentView == ViewClipName) {
        rendered |= UiViewClipName::draw(d, winW, winH, needFullRedraw, currentY);
    }

    needFullRedraw = false;

    rendered |= UiMessage::draw(d, winW, winH, needFullRedraw);

    return rendered;
}
