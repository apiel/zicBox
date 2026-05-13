#pragma once

#include "zicRack/studio.h"

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

bool shouldDrawMessage()
{
    if (statusMsg.active) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - statusMsg.time).count();
        return elapsed < 3000;
    }
    return false;
}

void drawMessage(Draw& d, sf::Vector2u size)
{
    if (shouldDrawMessage()) {
        const int winW = (int)size.x;
        int msgW = winW / 2, msgH = 30;
        int msgX = (winW - msgW) / 2;
        int msgY = 5;
        d.filledRect({ msgX, msgY }, { msgW, msgH }, { .color = { 30, 30, 40 } });
        d.rect({ msgX, msgY }, { msgW, msgH }, { .color = { 100, 100, 120 } });
        d.textCentered({ msgX + msgW / 2, msgY + 8 }, statusMsg.text, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
    }
}
