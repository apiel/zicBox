#pragma once

#include "draw/utils/inRect.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/button.h"

namespace UiKeyboard {

const int KEYBOARD_COLS = 8;
const int KEYBOARD_ROWS = 8;
int keyboardSelectedRow = 0;
int keyboardSelectedCol = 0;

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

int getKeyboardSelectedCol()
{
    return keyboardSelectedCol;
}

std::vector<Button> getKeyboardCurrentRowLabels()
{
    std::vector<Button> labels;
    labels.reserve(KEYBOARD_COLS);

    int row = std::clamp(keyboardSelectedRow, 0, KEYBOARD_ROWS - 1);
    int start = row * KEYBOARD_COLS;
    for (int c = 0; c < KEYBOARD_COLS; c++) {
        if (c == keyboardSelectedCol) {
            labels.push_back(Button(keyboardKeys[start + c], Color { 40, 40, 40 }, Color { 150, 150, 150 }));
        } else {
            labels.push_back(keyboardKeys[start + c]);
        }
    }
    return labels;
}

bool draw(Draw& d, const int winW, const int winH, int currentY, std::string title, std::string label, std::string &value)
{
    int margin = 4;
    Rect rect = { { margin, currentY + margin }, { winW - margin * 2, winH - currentY - margin * 2 } };

    d.filledRect(rect.position, rect.size, { .color = { 18, 18, 24 } });
    d.text({ rect.position.x + 6, rect.position.y + 6 }, title, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    Rect kbRect = { { rect.position.x + 4, rect.position.y + 24 }, { rect.size.w - 8, rect.size.h - 36 } };

    keyboardSelectedRow = std::clamp(keyboardSelectedRow, 0, KEYBOARD_ROWS - 1);
    keyboardSelectedCol = std::clamp(keyboardSelectedCol, 0, KEYBOARD_COLS - 1);

    d.filledRect(kbRect.position, kbRect.size, { .color = { 25, 25, 30 } });

    int x = d.text({ kbRect.position.x + 6, kbRect.position.y + 6 }, label, 8, { .color = { 180, 180, 190 }, .font = &PoppinsLight_8 });
    d.text({ x + 6, kbRect.position.y + 6 }, value, 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8, .fontSpacing = 1 });

    std::string rowLabel = "ROW " + std::to_string(keyboardSelectedRow + 1) + "/" + std::to_string(KEYBOARD_ROWS);
    d.text({ kbRect.position.x + kbRect.size.w - 58, kbRect.position.y + 6 }, rowLabel, 8, { .color = { 190, 190, 200 }, .font = &PoppinsLight_8 });

    int sidePad = 4;
    int topPad = 18;
    int bottomPad = 12;
    int gap = 1;
    int keyW = std::max(12, (kbRect.size.w - sidePad * 2 - gap * (KEYBOARD_COLS - 1)) / KEYBOARD_COLS);
    int keyH = std::max(14, (kbRect.size.h - topPad - bottomPad - gap * (KEYBOARD_ROWS - 1)) / KEYBOARD_ROWS);

    for (int i = 0; i < KEYS_COUNT; i++) {
        int row = i / KEYBOARD_COLS;
        int col = i % KEYBOARD_COLS;

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
            keyboardKeys[i],
            8,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    }

    return true;
}

void mouseButtonPressed(Point position, bool& needFullRedraw, std::string &value)
{
    for (int i = 0; i < KEYS_COUNT; i++) {
        if (!inRect(keyboardRects[i], position)) continue;
        keyboardSelectedRow = i / KEYBOARD_COLS;
        keyboardSelectedCol = i % KEYBOARD_COLS;
        value += keyboardKeys[i];
        needFullRedraw = true;
        return;
    }
}


void keyPressed(int key, bool& needFullRedraw, std::string &value)
{
    if (key == KEY_F1) { // backspace
        if (!value.empty()) {
            value.pop_back();
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
        if (keyboardSelectedRow < KEYBOARD_ROWS - 1) {
            keyboardSelectedRow++;
            needFullRedraw = true;
        }
        return;
    }

    if (key >= KEY_1 && key <= KEY_8) { // add selected row char by column
        keyboardSelectedCol = key - KEY_1;
        int selectedGlobalIdx = (keyboardSelectedRow * KEYBOARD_COLS) + keyboardSelectedCol;
        value += keyboardKeys[selectedGlobalIdx];
        needFullRedraw = true;
        return;
    }
}

}