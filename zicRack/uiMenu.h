#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>

#include "draw/utils/Icon.h"
#include "draw/utils/inRect.h"
#include "zicRack/studio.h"

#define PROJECT_FOLDER std::string("../data/workspaces/rack")

namespace UiMenu {

bool needsRedraw = true;

enum MenuView {
    VIEW_PROJECTS = 0,
    VIEW_KEYBOARD
};

MenuView currentView = VIEW_PROJECTS;

Rect projectTabRect;

Rect listRect = { { -1, -1 }, { -1, -1 } };
std::vector<Rect> fileRects;

Rect loadBtnRect;
Rect saveBtnRect;
Rect saveNewBtnRect;
Rect backspaceRect;
Rect cancelRect;
Rect createProjectRect;

std::vector<std::string> projectFiles;

int selectedFile = -1;
std::string currentLoadedFile = "";

std::string newProjectName = "";

const int KEYS_COUNT = 64;

Rect keyboardRects[KEYS_COUNT];

const char* keyboardKeys[KEYS_COUNT] = {
    // clang-format off
    "A", "B", "C", "D", "E", "F", "G", "H",
    "I", "J", "K", "L", "M", "N", "O", "P",
    "Q", "R", "S", "T", "U", "V", "W", "X",
    "Y", "Z", "a", "b", "c", "d", "e", "f",
    "g", "h", "i", "j", "k", "l", "m", "n",
    "o", "p", "q", "r", "s", "t", "u", "v",
    "w", "x", "y", "z", "0", "1", "2", "3",
    "4", "5", "6", "7", "8", "9", ".", "_",
    // clang-format on
};

std::string shortenFilename(const std::string& name, int maxLen = 12)
{
    if ((int)name.size() <= maxLen) return name;
    return name.substr(0, maxLen - 3) + "...";
}

void refreshProjects()
{
    projectFiles.clear();

    try {
        currentLoadedFile = "";

        std::ifstream currentFile(PROJECT_FOLDER + "/.current");

        if (currentFile.good()) {
            std::getline(currentFile, currentLoadedFile);
        }

        for (const auto& entry : std::filesystem::directory_iterator(PROJECT_FOLDER)) {

            std::string filename = entry.path().filename().string();

            if (filename == ".current") continue;

            projectFiles.push_back(filename);
        }

        std::sort(projectFiles.begin(), projectFiles.end());

        selectedFile = -1;

        for (size_t i = 0; i < projectFiles.size(); i++) {
            if (projectFiles[i] == currentLoadedFile) {
                selectedFile = (int)i;
                break;
            }
        }

        if (selectedFile == -1 && !projectFiles.empty()) {
            selectedFile = 0;
        }
    } catch (const std::exception& e) {
        std::cout << "could not open project folder: " << e.what() << std::endl;
    }
}

void drawProjects(Draw& d, Rect rect)
{
    listRect = rect;

    d.filledRect(rect.position, rect.size, { .color = { 25, 25, 30 } });

    fileRects.clear();

    int itemH = 28;

    for (size_t i = 0; i < projectFiles.size(); i++) {
        Rect r = {
            { rect.position.x + 4, rect.position.y + 4 + (int)i * (itemH + 2) },
            { rect.size.w - 8, itemH }
        };

        fileRects.push_back(r);

        bool isSelected = selectedFile == (int)i;
        bool isCurrent = projectFiles[i] == currentLoadedFile;

        Color bg = { 45, 45, 55 };

        if (isSelected) {
            bg = { 70, 70, 90 };
        }

        d.filledRect(r.position, r.size, { .color = bg });
        d.text({ r.position.x + 6, r.position.y + 8 }, projectFiles[i], 12, { .color = Color { 255, 255, 255 }, .font = &PoppinsLight_12 });

        if (isCurrent) {
            d.textRight({ r.position.x + r.size.w - 6, r.position.y + 8 }, "CURRENT", 8, { .color = Color { 160, 160, 170 }, .font = &PoppinsLight_8 });
        }
    }

    int btnY = rect.position.y + rect.size.h - 40;

    int btnGap = 4;
    int btnW = (rect.size.w - 8 - btnGap * 2) / 3;

    std::string shortName = "";
    if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
        shortName = shortenFilename(projectFiles[selectedFile], 16);
    }

    loadBtnRect = { { rect.position.x + 4, btnY }, { btnW, 34 } };
    saveBtnRect = { { rect.position.x + 4 + btnW + btnGap, btnY }, { btnW, 34 } };
    saveNewBtnRect = { { rect.position.x + 4 + (btnW + btnGap) * 2, btnY }, { btnW, 34 } };

    Color btnColor = { 110, 110, 120 };
    d.filledRect(saveNewBtnRect.position, saveNewBtnRect.size, { .color = btnColor });
    d.textCentered(
        { saveNewBtnRect.position.x + saveNewBtnRect.size.w / 2, saveNewBtnRect.position.y + 9 },
        "SAVE NEW", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    if (!shortName.empty()) {
        d.filledRect(loadBtnRect.position, loadBtnRect.size, { .color = btnColor });
        d.filledRect(saveBtnRect.position, saveBtnRect.size, { .color = btnColor });

        d.textCentered({ loadBtnRect.position.x + loadBtnRect.size.w / 2, loadBtnRect.position.y + 9 },
            "LOAD", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

        d.textCentered({ saveBtnRect.position.x + saveBtnRect.size.w / 2, saveBtnRect.position.y + 9 },
            "SAVE AS", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

        d.textCentered({ loadBtnRect.position.x + loadBtnRect.size.w / 2, loadBtnRect.position.y + 23 },
            shortName, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        d.textCentered({ saveBtnRect.position.x + saveBtnRect.size.w / 2, saveBtnRect.position.y + 23 },
            shortName, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    }
}

void drawKeyboard(Draw& d, Rect rect)
{
    int cols = 8;
    int keyW = (rect.size.w - 16) / cols;
    int keyH = 38;

    Icon icon(d);

    d.filledRect(rect.position, rect.size, { .color = { 25, 25, 30 } });

    int x = d.text({ rect.position.x + 8, rect.position.y + 12 }, "NEW PROJECT:", 12, { .color = { 180, 180, 190 }, .font = &PoppinsLight_12 });
    d.text({ x + 8, rect.position.y + 12 }, newProjectName, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12, .fontSpacing = 2 });

    backspaceRect = { { rect.position.x + rect.size.w - keyW - 8, rect.position.y + 4 }, { keyW - 4, 32 } };
    d.filledRect(backspaceRect.position, backspaceRect.size, { .color = { 45, 45, 55 } });
    icon.backspace({ backspaceRect.position.x + backspaceRect.size.w / 2 - 13, backspaceRect.position.y + 4 }, { 26, 26 }, { 200, 200, 210 });

    int startY = rect.position.y + 40;

    for (int i = 0; i < KEYS_COUNT; i++) {
        int row = i / cols;
        int col = i % cols;

        int x = rect.position.x + 8 + col * keyW;
        int y = startY + row * (keyH + 4);

        keyboardRects[i] = {
            { x, y },
            { keyW - 4, keyH }
        };

        d.filledRect(
            keyboardRects[i].position,
            keyboardRects[i].size,
            { .color = { 45, 45, 55 } });

        d.textCentered(
            { keyboardRects[i].position.x + keyboardRects[i].size.w / 2,
                keyboardRects[i].position.y + 10 },
            keyboardKeys[i],
            12,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
    }

    createProjectRect = { { rect.position.x + 8, rect.position.y + rect.size.h - 40 }, { rect.size.w / 2 - 16, 30 } };

    d.filledRect(createProjectRect.position, createProjectRect.size, { .color = { 70, 120, 70 } });

    d.textCentered({ createProjectRect.position.x + createProjectRect.size.w / 2, createProjectRect.position.y + 9 },
        "CREATE PROJECT", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    cancelRect = {
        { rect.position.x + rect.size.w / 2 + 4, rect.position.y + rect.size.h - 40 },
        { rect.size.w / 2 - 16, 30 }
    };

    d.filledRect(cancelRect.position, cancelRect.size, { .color = { 120, 120, 120 } });

    d.textCentered({ cancelRect.position.x + cancelRect.size.w / 2, cancelRect.position.y + 9 },
        "CANCEL", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
}

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    if (selectedFile == -1) refreshProjects();

    int margin = 8;

    int leftW = 120;

    Rect leftRect = {
        { margin, currentY + margin },
        { leftW, winH - currentY - margin * 2 }
    };

    Rect rightRect = {
        { leftRect.position.x + leftRect.size.w + margin, currentY + margin },
        { winW - leftW - margin * 3, winH - currentY - margin * 2 }
    };

    d.filledRect(leftRect.position, leftRect.size, { .color = { 20, 20, 25 } });

    projectTabRect = {
        { leftRect.position.x + 4, leftRect.position.y + 4 },
        { leftRect.size.w - 8, 32 }
    };

    d.filledRect(projectTabRect.position, projectTabRect.size, { .color = { 70, 70, 90 } });

    d.text({ projectTabRect.position.x + 8, projectTabRect.position.y + 10 },
        "PROJECT", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    if (currentView == VIEW_PROJECTS) {
        drawProjects(d, rightRect);
    } else {
        drawKeyboard(d, rightRect);
    }

    return true;
}

void mouseButtonPressed(Point position)
{
    if (studio.currentView != ViewMenu) return;

    if (currentView == VIEW_PROJECTS) {

        for (size_t i = 0; i < fileRects.size(); i++) {
            if (inRect(fileRects[i], position)) {
                selectedFile = (int)i;
                std::cout << "select: " << projectFiles[selectedFile] << std::endl;
                needsRedraw = true;
                return;
            }
        }

        if (inRect(loadBtnRect, position)) {
            if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
                std::cout << "load: " << projectFiles[selectedFile] << std::endl;
            }
            return;
        }

        if (inRect(saveBtnRect, position)) {
            if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
                std::cout << "save as: " << projectFiles[selectedFile] << std::endl;
            }
            return;
        }

        if (inRect(saveNewBtnRect, position)) {
            newProjectName = "";
            currentView = VIEW_KEYBOARD;
            needsRedraw = true;
            return;
        }
    }

    else if (currentView == VIEW_KEYBOARD) {

        for (int i = 0; i < KEYS_COUNT; i++) {
            if (!inRect(keyboardRects[i], position)) continue;
            std::string key = keyboardKeys[i];
            newProjectName += key;
            needsRedraw = true;
            return;
        }

        if (inRect(backspaceRect, position)) {
            if (!newProjectName.empty()) {
                newProjectName.pop_back();
            }
            needsRedraw = true;
            return;
        }

        if (inRect(createProjectRect, position)) {
            std::cout << "save as new: " << newProjectName << std::endl;

            currentView = VIEW_PROJECTS;

            refreshProjects();

            needsRedraw = true;
            return;
        }

        if (inRect(cancelRect, position)) {
            currentView = VIEW_PROJECTS;
            needsRedraw = true;
            return;
        }
    }
}

}