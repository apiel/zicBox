#pragma once

#include "draw/draw.h"
#include <chrono>
#include <string>

namespace UiMessage {

inline std::string statusMessage = "";
inline std::chrono::steady_clock::time_point statusUntil;
inline const int STATUS_DURATION_MS = 1500;

inline void show(const std::string& msg, bool& needsRedraw, int durationMs = STATUS_DURATION_MS)
{
    statusMessage = msg;
    statusUntil = std::chrono::steady_clock::now() + std::chrono::milliseconds(durationMs);
    needsRedraw = true;
}

inline void clear(bool& needsRedraw)
{
    if (!statusMessage.empty()) {
        statusMessage = "";
        needsRedraw = true;
    }
}

inline bool draw(Draw& d, int winW, int winH, bool& needsRedraw)
{
    if (statusMessage.empty())
        return false;

    auto now = std::chrono::steady_clock::now();

    if (now > statusUntil) {
        statusMessage = "";
        needsRedraw = true;
        return false;
    }

    int w = 320;
    int h = 30;

    Rect r = { { (winW - w) / 2, (winH - h) / 2 }, { w, h } };

    d.filledRect(r.position, r.size, { .color = { 20, 26, 38, 235 } });
    d.rect(r.position, r.size, { .color = { 100, 130, 180, 255 } });
    d.textCentered({ r.position.x + r.size.w / 2, r.position.y + 9 }, statusMessage, 8, { .color = { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

    return true;
}

}
