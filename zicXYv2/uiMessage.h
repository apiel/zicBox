#pragma once

#include "zicXYv2/studio.h"

namespace UiMessage {

std::string statusMessage = "";
std::chrono::steady_clock::time_point statusUntil;
const int STATUS_DURATION_MS = 1500;

void show(const std::string& msg, bool& needsRedraw)
{
    statusMessage = msg;
    statusUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(STATUS_DURATION_MS);
    needsRedraw = true;
}

bool draw(Draw& d, int winW, int winH, bool& needsRedraw)
{
    if (statusMessage.empty())
        return false;

    auto now = std::chrono::steady_clock::now();

    if (now > statusUntil) {
        statusMessage = "";
        needsRedraw = true;
    }

    int w = 300;
    int h = 28;

    Rect r = { { (winW - w) / 2, (winH - h) / 2 }, { w, h } };

    d.filledRect(r.position, r.size, { .color = { 45, 45, 55, 200 } });
    d.rect(r.position, r.size, { .color = { 120, 120, 130 } });
    d.textCentered({ r.position.x + r.size.w / 2, r.position.y + 8 }, statusMessage, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    return true;
}

}