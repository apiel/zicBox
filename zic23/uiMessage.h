#pragma once

#include "zic23/studio.h"

struct StatusMsg {
    std::string text;
    std::chrono::steady_clock::time_point time;
    bool active = false;
} statusMsg;

void showMessage(std::string txt)
{
    statusMsg.text = txt;
    statusMsg.time = std::chrono::steady_clock::now();
    statusMsg.active = true;
}

int64_t shouldDrawMessage()
{
    if (statusMsg.active) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - statusMsg.time).count();
        return elapsed < 3000 ? std::max((int64_t)1, elapsed) : 0;
    }
    return 0;
}

void drawMessage(Draw& d, sf::Vector2u size)
{
    int64_t elapsed = shouldDrawMessage();
    if (elapsed) {
        const int winW = (int)size.x;
        const int winH = (int)size.y;
        int alpha = elapsed > 2000 ? (int)(255 * (1.0f - (elapsed - 2000) / 1000.0f)) : 255;
        int msgW = 200, msgH = 30;
        int msgX = (winW - msgW) / 2, msgY = winH - 50;
        d.filledRect({ msgX, msgY }, { msgW, msgH }, { .color = { 30, 30, 40, (sf::Uint8)alpha } });
        d.rect({ msgX, msgY }, { msgW, msgH }, { .color = { 100, 100, 120, (sf::Uint8)alpha } });
        d.text({ msgX + 10, msgY + 8 }, statusMsg.text, 12, { .color = { 255, 255, 255, (sf::Uint8)alpha }, .font = &PoppinsLight_12 });
    }
}
