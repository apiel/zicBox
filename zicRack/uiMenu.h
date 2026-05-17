#pragma once

#include <filesystem>
#include <iostream>

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicRack/studio.h"

#define PROJECT_FOLDER std::string("../data/workspaces/rack")

namespace UiMenu {

bool needsRedraw = true;

enum MenuAction {
    MENU_SAVE = 0,
    MENU_SAVE_AS,
    MENU_LOAD,
    MENU_COUNT
};

const char* MENU_LABELS[MENU_COUNT] = {
    "SAVE",
    "SAVE AS",
    "LOAD"
};

int selectedAction = MENU_SAVE;

Rect actionRects[MENU_COUNT];

Rect keyboardRects[12];
const char* keyboardKeys[12] = {
    "1", "2", "3",
    "4", "5", "6",
    "7", "8", "9",
    "_", "0", "<"
};

std::string projectName = "untitled";

std::vector<std::string> projectFiles;
std::vector<Rect> loadRects;

void refreshProjects()
{
    projectFiles.clear();

    try {
        for (const auto& entry : std::filesystem::directory_iterator(PROJECT_FOLDER)) {
            if (!entry.is_regular_file()) continue;

            projectFiles.push_back(entry.path().filename().string());
        }
    } catch (...) {
        std::cout << "could not open project folder" << std::endl;
    }
}

void drawKeyboard(Draw& d, Rect rect)
{
    d.filledRect(rect.position, rect.size, { .color = { 25, 25, 30 } });

    d.text(
        { rect.position.x + 8, rect.position.y + 8 },
        "PROJECT NAME: " + projectName,
        12,
        { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    int gridY = rect.position.y + 40;

    int cols = 3;
    int rows = 4;

    int keyW = (rect.size.w - 16) / cols;
    int keyH = 36;

    int keyIdx = 0;

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (keyIdx >= 12) break;

            int kx = rect.position.x + 8 + x * keyW;
            int ky = gridY + y * (keyH + 6);

            keyboardRects[keyIdx] = {
                { kx, ky },
                { keyW - 6, keyH }
            };

            d.filledRect(
                keyboardRects[keyIdx].position,
                keyboardRects[keyIdx].size,
                { .color = { 45, 45, 55 } });

            d.textCentered(
                { keyboardRects[keyIdx].position.x + keyboardRects[keyIdx].size.w / 2,
                    keyboardRects[keyIdx].position.y + 12 },
                keyboardKeys[keyIdx],
                12,
                { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

            keyIdx++;
        }
    }

    Rect saveBtn = {
        { rect.position.x + 8, rect.position.y + rect.size.h - 40 },
        { rect.size.w - 16, 30 }
    };

    d.filledRect(saveBtn.position, saveBtn.size, { .color = { 70, 120, 70 } });

    d.textCentered(
        { saveBtn.position.x + saveBtn.size.w / 2, saveBtn.position.y + 9 },
        "SAVE PROJECT",
        12,
        { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
}

void drawLoadList(Draw& d, Rect rect)
{
    d.filledRect(rect.position, rect.size, { .color = { 25, 25, 30 } });

    int itemH = 28;

    loadRects.clear();

    for (size_t i = 0; i < projectFiles.size(); i++) {
        Rect r = {
            { rect.position.x + 4, rect.position.y + 4 + (int)i * (itemH + 2) },
            { rect.size.w - 8, itemH }
        };

        loadRects.push_back(r);

        d.filledRect(r.position, r.size, { .color = { 45, 45, 55 } });

        d.text(
            { r.position.x + 6, r.position.y + 8 },
            projectFiles[i],
            12,
            { .color = { 220, 220, 220 }, .font = &PoppinsLight_12 });
    }
}

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    int margin = 8;

    int leftW = 140;

    Rect leftRect = {
        { margin, currentY + margin },
        { leftW, winH - currentY - (margin * 2) }
    };

    Rect rightRect = {
        { leftRect.position.x + leftRect.size.w + margin, currentY + margin },
        { winW - leftW - (margin * 3), winH - currentY - (margin * 2) }
    };

    d.filledRect(leftRect.position, leftRect.size, { .color = { 20, 20, 25 } });
    d.filledRect(rightRect.position, rightRect.size, { .color = { 30, 30, 35 } });

    int btnY = leftRect.position.y + 4;
    int btnH = 32;

    for (int i = 0; i < MENU_COUNT; i++) {
        actionRects[i] = {
            { leftRect.position.x + 4, btnY },
            { leftRect.size.w - 8, btnH }
        };

        Color bg = { 45, 45, 50 };
        Color txt = { 180, 180, 190 };

        if (selectedAction == i) {
            bg = { 70, 70, 90 };
            txt = { 255, 255, 255 };
        }

        d.filledRect(actionRects[i].position, actionRects[i].size, { .color = bg });

        d.text(
            { actionRects[i].position.x + 8, actionRects[i].position.y + 10 },
            MENU_LABELS[i],
            12,
            { .color = txt, .font = &PoppinsLight_12 });

        btnY += btnH + 4;
    }

    if (selectedAction == MENU_SAVE) {
        d.text(
            { rightRect.position.x + 12, rightRect.position.y + 12 },
            "PRESS ENTER TO SAVE PROJECT",
            12,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
    }

    if (selectedAction == MENU_SAVE_AS) {
        drawKeyboard(d, rightRect);
    }

    if (selectedAction == MENU_LOAD) {
        drawLoadList(d, rightRect);
    }

    return true;
}

void mouseButtonPressed(Point position)
{
    if (studio.currentView != ViewMenu) return;

    for (int i = 0; i < MENU_COUNT; i++) {
        if (inRect(actionRects[i], position)) {
            selectedAction = i;

            if (selectedAction == MENU_LOAD) {
                refreshProjects();
            }

            needsRedraw = true;
            return;
        }
    }

    if (selectedAction == MENU_SAVE_AS) {
        for (int i = 0; i < 12; i++) {
            if (!inRect(keyboardRects[i], position)) continue;

            std::string key = keyboardKeys[i];

            if (key == "<") {
                if (!projectName.empty()) {
                    projectName.pop_back();
                }
            } else {
                projectName += key;
            }

            needsRedraw = true;
            return;
        }
    }

    if (selectedAction == MENU_LOAD) {
        for (size_t i = 0; i < loadRects.size(); i++) {
            if (!inRect(loadRects[i], position)) continue;

            std::cout << "load: " << projectFiles[i] << std::endl;

            needsRedraw = true;
            return;
        }
    }
}

void keyPressed(int key)
{
    if (studio.currentView != ViewMenu) return;

    if (selectedAction == MENU_SAVE) {
        if (key == '\n') {
            std::cout << "save project" << std::endl;
        }
    }

    if (selectedAction == MENU_SAVE_AS) {
        if (key == '\n') {
            std::cout << "save as: " << projectName << std::endl;
        }
    }
}

}