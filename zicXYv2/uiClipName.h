#pragma once

#include "draw/utils/inRect.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/project.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/uiMenu.h"
#include "zicXYv2/uiMessage.h"
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
int keyboardSelectedRow = 0;
int keyboardSelectedCol = 0;
Rect keyboardRects[UiMenu::KEYS_COUNT];

std::vector<std::string> getKeyboardCurrentRowLabels()
{
    std::vector<std::string> labels;
    labels.reserve(UiMenu::KEYBOARD_COLS);

    int row = std::clamp(keyboardSelectedRow, 0, UiMenu::KEYBOARD_ROWS - 1);
    int start = row * UiMenu::KEYBOARD_COLS;
    for (int c = 0; c < UiMenu::KEYBOARD_COLS; c++) {
        labels.push_back(UiMenu::keyboardKeys[start + c]);
    }
    return labels;
}

int getKeyboardSelectedCol()
{
    return keyboardSelectedCol;
}

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    int margin = 4;
    Rect rect = { { margin, currentY + margin }, { winW - margin * 2, winH - currentY - margin * 2 } };

    d.filledRect(rect.position, rect.size, { .color = { 18, 18, 24 } });
    d.text({ rect.position.x + 6, rect.position.y + 6 }, "Clip Name", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    Rect kbRect = { { rect.position.x + 4, rect.position.y + 24 }, { rect.size.w - 8, rect.size.h - 36 } };

    keyboardSelectedRow = std::clamp(keyboardSelectedRow, 0, UiMenu::KEYBOARD_ROWS - 1);
    keyboardSelectedCol = std::clamp(keyboardSelectedCol, 0, UiMenu::KEYBOARD_COLS - 1);

    d.filledRect(kbRect.position, kbRect.size, { .color = { 25, 25, 30 } });

    int x = d.text({ kbRect.position.x + 6, kbRect.position.y + 6 }, "NAME:", 8, { .color = { 180, 180, 190 }, .font = &PoppinsLight_8 });
    d.text({ x + 6, kbRect.position.y + 6 }, UiMenu::shortenFilename(newClipName, 30), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8, .fontSpacing = 1 });

    std::string rowLabel = "ROW " + std::to_string(keyboardSelectedRow + 1) + "/" + std::to_string(UiMenu::KEYBOARD_ROWS);
    d.text({ kbRect.position.x + kbRect.size.w - 58, kbRect.position.y + 6 }, rowLabel, 8, { .color = { 190, 190, 200 }, .font = &PoppinsLight_8 });

    int sidePad = 4;
    int topPad = 18;
    int bottomPad = 12;
    int gap = 1;
    int keyW = std::max(12, (kbRect.size.w - sidePad * 2 - gap * (UiMenu::KEYBOARD_COLS - 1)) / UiMenu::KEYBOARD_COLS);
    int keyH = std::max(14, (kbRect.size.h - topPad - bottomPad - gap * (UiMenu::KEYBOARD_ROWS - 1)) / UiMenu::KEYBOARD_ROWS);

    for (int i = 0; i < UiMenu::KEYS_COUNT; i++) {
        int row = i / UiMenu::KEYBOARD_COLS;
        int col = i % UiMenu::KEYBOARD_COLS;

        int keyX = kbRect.position.x + sidePad + col * (keyW + gap);
        int keyY = kbRect.position.y + topPad + row * (keyH + gap);

        keyboardRects[i] = {
            { keyX, keyY },
            { keyW, keyH }
        };

        bool selectedRow = row == keyboardSelectedRow;
        bool selectedKey = selectedRow && col == keyboardSelectedCol;

        Color bg = { 45, 45, 55 };
        if (selectedRow) bg = { 70, 70, 95 };
        if (selectedKey) bg = { 100, 100, 135 };

        d.filledRect({ keyX, keyY }, { keyW, keyH }, { .color = bg });

        d.textCentered(
            { keyX + keyW / 2, keyY + std::max(2, keyH / 2 - 4) },
            UiMenu::keyboardKeys[i],
            8,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    }

    return true;
}

void mouseButtonPressed(Point position, bool& needFullRedraw)
{
    if (studio.currentView != ViewClipName) return;

    for (int i = 0; i < UiMenu::KEYS_COUNT; i++) {
        if (!inRect(keyboardRects[i], position)) continue;
        keyboardSelectedRow = i / UiMenu::KEYBOARD_COLS;
        keyboardSelectedCol = i % UiMenu::KEYBOARD_COLS;
        newClipName += UiMenu::keyboardKeys[i];
        needsRedraw = true;
        needFullRedraw = true;
        return;
    }
}

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentView != ViewClipName) return;

    if (key == KEY_F1) { // backspace
        if (!newClipName.empty()) {
            newClipName.pop_back();
            needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_F2) { // up
        if (keyboardSelectedRow > 0) {
            keyboardSelectedRow--;
            needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_F3) { // down
        if (keyboardSelectedRow < UiMenu::KEYBOARD_ROWS - 1) {
            keyboardSelectedRow++;
            needFullRedraw = true;
        }
        return;
    }

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

    if (key >= KEY_1 && key <= KEY_8) { // add selected row char by column
        keyboardSelectedCol = key - KEY_1;
        int selectedGlobalIdx = (keyboardSelectedRow * UiMenu::KEYBOARD_COLS) + keyboardSelectedCol;
        newClipName += UiMenu::keyboardKeys[selectedGlobalIdx];
        needFullRedraw = true;
        return;
    }
}

}
