#pragma once

#include "draw/utils/inRect.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/project.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/uiMenu.h"
#include "zicXYv2/uiMessage.h"
#include "zicXYv2/uiKeyboard.h"
#include <algorithm>
#include <string>
#include <vector>

namespace UiClips {
    extern int selectedClipIdx;
    extern bool needsRedraw;
}

namespace UiViewClipName {

bool needsRedraw = true;
std::string newClipName = "";

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;
    return UiKeyboard::draw(d, winW, winH, currentY, "Clip Name", "NAME:", newClipName);
}

void mouseButtonPressed(Point position, bool& needFullRedraw)
{
    if (studio.currentView != ViewClipName) return;

    UiKeyboard::mouseButtonPressed(position, needFullRedraw, newClipName);
}

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentView != ViewClipName) return;
    
    UiKeyboard::keyPressed(key, needFullRedraw, newClipName);

    if (key == KEY_F4) { // Done
        if (newClipName.empty()) {
            UiMessage::show("Name is empty", needsRedraw);
            needFullRedraw = true;
            return;
        }

        Track& trk = *studio.tracks[studio.selTrack];
        trk.clips[UiClips::selectedClipIdx].name = newClipName;
        studio.currentView = ViewClips;
        UiClips::needsRedraw = true;
        needFullRedraw = true;
        return;
    }

    if (key == KEY_F5) { // Cancel
        studio.currentView = ViewClips;
        UiClips::needsRedraw = true;
        needFullRedraw = true;
        return;
    }
}

}
