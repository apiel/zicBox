#pragma once

#include "zic23/studio.h"

static sf::IntRect helpBtnRect, saveAllBtnRect, loadAllBtnRect;

void drawTopBarUI(Draw& d, sf::Vector2u size)
{
    const int winW = (int)size.x;
    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    studio.transportRect = { MARGIN, 4, 60, 17 };
    d.filledRect({ MARGIN, 4 }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.textCentered({ MARGIN + 30, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    helpBtnRect = { MARGIN + 70, 4, 60, 17 };
    d.filledRect({ helpBtnRect.left, helpBtnRect.top }, { 60, 17 }, { .color = { 60, 60, 75 } });
    d.textCentered({ helpBtnRect.left + 30, 7 }, "HELP", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // Save All / Reload All
    saveAllBtnRect = { MARGIN + 140, 4, 60, 17 };
    d.filledRect({ saveAllBtnRect.left, 4 }, { 60, 17 }, { .color = { 100, 100, 110 } });
    d.textCentered({ saveAllBtnRect.left + 30, 7 }, "Save Clips", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    loadAllBtnRect = { MARGIN + 210, 4, 60, 17 };
    d.filledRect({ loadAllBtnRect.left, 4 }, { 60, 17 }, { .color = { 100, 100, 110 } });
    d.textCentered({ loadAllBtnRect.left + 30, 7 }, "Load Clips", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - MARGIN, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
}
