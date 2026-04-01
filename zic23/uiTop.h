#pragma once

#include "zic23/studio.h"

static sf::IntRect helpBtnRect, saveAllBtnRect, reloadAllBtnRect, menuBtnRect;
static sf::IntRect menuSaveRect, menuSaveAsRect, menuOpenRect;
static bool showProjectMenu = false;

void drawTopBarUI(Draw& d, sf::Vector2u size)
{
    const int winW = (int)size.x;
    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    // Menu Toggle Button "..."
    menuBtnRect = { MARGIN, 4, 30, 17 };
    d.filledRect({ menuBtnRect.left, 4 }, { 30, 17 }, { .color = showProjectMenu ? Color{100, 100, 120} : Color{60, 60, 75} });
    d.textCentered({ menuBtnRect.left + 15, 7 }, "...", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    int currentX = MARGIN + 35;

    studio.transportRect = { currentX, 4, 60, 17 };
    d.filledRect({ currentX, 4 }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.textCentered({ currentX + 30, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    currentX += 70;
    helpBtnRect = { currentX, 4, 60, 17 };
    d.filledRect({ helpBtnRect.left, helpBtnRect.top }, { 60, 17 }, { .color = { 60, 60, 75 } });
    d.textCentered({ helpBtnRect.left + 30, 7 }, "HELP", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    currentX += 70;
    reloadAllBtnRect = { currentX, 4, 80, 17 };
    d.filledRect({ reloadAllBtnRect.left, 4 }, { 80, 17 }, { .color = { 100, 100, 110 } });
    d.textCentered({ reloadAllBtnRect.left + 40, 7 }, "Reload Clips", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // studio.projectPath
    currentX += 90;
    d.text({ currentX, 7 }, studio.projectPath, 8, { .color = { 150, 150, 150 }, .font = &PoppinsLight_8 });

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - MARGIN, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // Project Dropdown Menu
    if (showProjectMenu) {
        int menuW = 220, menuH = 80;
        d.filledRect({ MARGIN, 25 }, { menuW, menuH }, { .color = { 35, 35, 45 } });
        d.rect({ MARGIN, 25 }, { menuW, menuH }, { .color = { 100, 100, 110 } });

        menuSaveAsRect = { MARGIN, 25, menuW, 22 };
        menuSaveRect = { MARGIN, 50, menuW, 22 };
        menuOpenRect = { MARGIN, 75, menuW, 22 };

        auto drawMenuItem = [&](sf::IntRect r, std::string label, std::string shortcut) {
            d.text({ r.left + 10, r.top + 5 }, label, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
            d.textRight({ r.left + r.width - 10, r.top + 7 }, shortcut, 8, { .color = { 150, 150, 160 }, .font = &PoppinsLight_8 });
        };

        drawMenuItem(menuSaveAsRect, "Save Project As", "Ctrl+Shift+S");
        drawMenuItem(menuSaveRect, "Save Project", "Ctrl+S");
        drawMenuItem(menuOpenRect, "Open Project", "Ctrl+O");
    }
}