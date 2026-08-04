#pragma once

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "draw/draw.h"
#include "draw/utils/Icon.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/project.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class ProjectView : public View {
public:
    enum ViewMode {
        VIEW_LIST = 0,
        VIEW_KEYBOARD_NEW,
        VIEW_KEYBOARD_RENAME
    };

private:
    ViewMode currentMode = VIEW_LIST;

    std::vector<std::string> projectFiles;
    int selectedFile = -1;
    int scrollOffset = 0;
    std::string currentLoadedFile = "";

    // Text entry for NEW / RENAME modes
    std::string inputProjectName = "";
    std::string renameProjectOldName = "";

    // Confirmation states
    bool confirmSave = false;
    std::string pendingSaveFilename = "";
    bool confirmDelete = false;
    std::string pendingDeleteFilename = "";

    // Virtual keyboard selection
    int kbSelectedRow = 0;
    int kbSelectedCol = 0;

    static const int KB_COLS = 8;
    static const int KB_ROWS = 8;
    static const int KB_KEYS_COUNT = 64;

    const char* kbKeys[64] = {
        "A", "B", "C", "D", "E", "F", "G", "H",
        "I", "J", "K", "L", "M", "N", "O", "P",
        "Q", "R", "S", "T", "U", "V", "W", "X",
        "Y", "Z", "a", "b", "c", "d", "e", "f",
        "g", "h", "i", "j", "k", "l", "m", "n",
        "o", "p", "q", "r", "s", "t", "u", "v",
        "w", "x", "y", "z", "0", "1", "2", "3",
        "4", "5", "6", "7", "8", "9", ".", "_"
    };

    std::string shorten(const std::string& name, int maxLen = 22) const
    {
        if ((int)name.size() <= maxLen) return name;
        return name.substr(0, maxLen - 3) + "...";
    }

public:
    ProjectView()
        : View("PROJECT MANAGER")
    {
    }

    ViewMode getCurrentMode() const { return currentMode; }

    void refreshProjects()
    {
        projectFiles.clear();
        try {
            currentLoadedFile = getCurrentLoadedProject();
            if (std::filesystem::exists(PROJECT_FOLDER)) {
                for (const auto& entry : std::filesystem::directory_iterator(PROJECT_FOLDER)) {
                    std::string filename = entry.path().filename().string();
                    if (filename == CURRENT_FILE || filename.empty() || filename[0] == '.') continue;
                    projectFiles.push_back(filename);
                }
            }
            std::sort(projectFiles.begin(), projectFiles.end());

            if (projectFiles.empty()) {
                selectedFile = -1;
            } else {
                if (selectedFile < 0 || selectedFile >= (int)projectFiles.size()) {
                    selectedFile = 0;
                }
                for (size_t i = 0; i < projectFiles.size(); i++) {
                    if (projectFiles[i] == currentLoadedFile) {
                        selectedFile = (int)i;
                        break;
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cout << "[ProjectView] Error reading project folder: " << e.what() << std::endl;
        }
    }

    void restoreDefaultUtilityPads()
    {
        gridState.pads[8][3].label = "&icon::arrowLeft::filled";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };
        gridState.pads[9][3].label = "&icon::arrowRight::filled";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };
        gridState.pads[10][3].label = "Oct-";
        gridState.pads[10][3].color = { 100, 120, 255, 255 };
        gridState.pads[11][3].label = "Oct+";
        gridState.pads[11][3].color = { 100, 120, 255, 255 };
    }

    void onActivate() override
    {
        currentMode = VIEW_LIST;
        confirmSave = false;
        confirmDelete = false;
        pendingSaveFilename.clear();
        pendingDeleteFilename.clear();
        refreshProjects();
        updatePadLeds();
        updateEncoderLabels();
    }

    void onDeactivate() override
    {
        confirmSave = false;
        confirmDelete = false;
        restoreDefaultUtilityPads();

        for (int i = 0; i < TOTAL_ENCODERS; ++i) {
            gridState.setEncoderBg(i, { 0, 0, 0, 0 });
        }
    }

    void updatePadLeds() override
    {
        if (currentMode == VIEW_LIST) {
            // Rows 0..2 of Dynamic Pads: Clean background
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                    auto& pad = gridState.pads[c][r];
                    pad.selected = false;
                    pad.active = false;
                    pad.label = "";
                    pad.color = { 25, 30, 40, 255 };
                }
            }

            // Dynamic Pads Row 3 (cols 0..7):
            for (int c = 0; c < 7; ++c) {
                gridState.pads[c][3].label = "";
                gridState.pads[c][3].color = { 25, 30, 40, 255 };
            }

            bool overlayActive = confirmSave || confirmDelete;

            if (overlayActive) {
                gridState.pads[5][3].label = "Confirm"; gridState.pads[5][3].color = { 40, 220, 140, 255 };
                gridState.pads[6][3].label = "Cancel";  gridState.pads[6][3].color = { 220, 60, 60, 255 };
                gridState.pads[7][3].label = "";        gridState.pads[7][3].color = { 25, 30, 40, 255 };
            } else {
                // Pad 7 (next to Z): DELETE with trash icon
                gridState.pads[7][3].label = "&icon::trash";
                gridState.pads[7][3].color = { 220, 60, 60, 255 };
            }

            // Global Utility Zone Row 3 (Cols 8..11 = Keys Z, X, C, V):
            // Pad 8  (utilCol 0 / Key Z): RENAME
            // Pad 9  (utilCol 1 / Key X): NEW
            // Pad 10 (utilCol 2 / Key C): LOAD
            // Pad 11 (utilCol 3 / Key V): SAVE
            gridState.pads[8][3].label = " Rename"; gridState.pads[8][3].color = { 240, 130, 40, 255 };
            gridState.pads[9][3].label = "New";    gridState.pads[9][3].color = { 220, 180, 40, 255 };
            gridState.pads[10][3].label = "Load";  gridState.pads[10][3].color = { 40, 160, 220, 255 };
            gridState.pads[11][3].label = "Save";  gridState.pads[11][3].color = { 40, 200, 80, 255 };
        } else {
            // Keyboard Mode Pad Layout
            restoreDefaultUtilityPads();

            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                    int keyIdx = r * DYNAMIC_PAD_COLS + c;
                    auto& pad = gridState.pads[c][r];
                    pad.selected = (kbSelectedRow == r && kbSelectedCol == c);
                    pad.active = false;
                    pad.label = kbKeys[keyIdx];
                    pad.color = pad.selected ? Color { 120, 140, 220, 255 } : Color { 45, 55, 75, 255 };
                }
            }

            gridState.pads[0][3].label = "BKSP";   gridState.pads[0][3].color = { 220, 60, 60, 255 };
            gridState.pads[1][3].label = "SPACE";  gridState.pads[1][3].color = { 60, 80, 120, 255 };
            gridState.pads[2][3].label = "CLEAR";  gridState.pads[2][3].color = { 220, 160, 40, 255 };
            gridState.pads[3][3].label = "ADD";    gridState.pads[3][3].color = { 40, 180, 220, 255 };
            gridState.pads[4][3].label = "";       gridState.pads[4][3].color = { 25, 30, 40, 255 };
            gridState.pads[5][3].label = "OK";     gridState.pads[5][3].color = { 40, 200, 80, 255 };
            gridState.pads[6][3].label = "CANCEL"; gridState.pads[6][3].color = { 160, 160, 170, 255 };
            gridState.pads[7][3].label = "";       gridState.pads[7][3].color = { 25, 30, 40, 255 };
        }
    }

    void updateEncoderLabels() override
    {
        if (currentMode == VIEW_LIST) {
            std::string selName = (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) ? projectFiles[selectedFile] : "None";
            gridState.setEncoder(0, "PROJECT", (float)selectedFile, 0.0f, std::max(0.0f, (float)projectFiles.size() - 1), 1.0f, shorten(selName, 12).c_str(), { 0, 180, 255, 255 });

            for (int i = 1; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoder(i, "", 0.0f, 0.0f, 1.0f, 1.0f, nullptr, { 0, 0, 0, 0 });
            }
        } else {
            gridState.setEncoder(0, "ROW", (float)kbSelectedRow, 0.0f, (float)(KB_ROWS - 1), 1.0f, nullptr, { 0, 180, 255, 255 });
            gridState.setEncoder(1, "COL", (float)kbSelectedCol, 0.0f, (float)(KB_COLS - 1), 1.0f, nullptr, { 0, 180, 255, 255 });
            int selectedGlobalIdx = (kbSelectedRow * KB_COLS) + kbSelectedCol;
            gridState.setEncoder(2, "CHAR", (float)selectedGlobalIdx, 0.0f, (float)(KB_KEYS_COUNT - 1), 1.0f, kbKeys[selectedGlobalIdx], { 255, 220, 40, 255 });

            for (int i = 3; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoder(i, "", 0.0f, 0.0f, 1.0f, 1.0f, nullptr, { 0, 0, 0, 0 });
            }
        }
    }

    void handleEncoder(int encoderId, int delta) override
    {
        if (delta == 0) return;

        if (currentMode == VIEW_LIST) {
            if (encoderId == 1) { // Encoder 0 (1-indexed id = 1)
                if (!projectFiles.empty()) {
                    selectedFile += delta;
                    selectedFile = std::clamp(selectedFile, 0, (int)projectFiles.size() - 1);
                    if (selectedFile < scrollOffset) scrollOffset = selectedFile;
                    if (selectedFile >= scrollOffset + 7) scrollOffset = selectedFile - 6;
                }
            }
        } else {
            if (encoderId == 1) { // ROW
                kbSelectedRow = std::clamp(kbSelectedRow + delta, 0, KB_ROWS - 1);
            } else if (encoderId == 2) { // COL
                kbSelectedCol = std::clamp(kbSelectedCol + delta, 0, KB_COLS - 1);
            } else if (encoderId == 3) { // CHAR
                int idx = (kbSelectedRow * KB_COLS) + kbSelectedCol;
                idx = std::clamp(idx + delta, 0, KB_KEYS_COUNT - 1);
                kbSelectedRow = idx / KB_COLS;
                kbSelectedCol = idx % KB_COLS;
            }
        }
        updatePadLeds();
        updateEncoderLabels();
    }

    void executeLoad()
    {
        if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
            std::string target = projectFiles[selectedFile];
            std::string filepath = PROJECT_FOLDER + "/" + target;
            loadProject(filepath);
            setCurrentLoadedProject(target);
            refreshProjects();
            bool dummy = true;
            UiMessage::show("Loaded: " + shorten(target, 16), dummy, 2500);
        }
    }

    void executeSave()
    {
        if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
            std::string target = projectFiles[selectedFile];
            if (!currentLoadedFile.empty() && target != currentLoadedFile) {
                confirmSave = true;
                pendingSaveFilename = target;
                updatePadLeds();
                return;
            }
            std::string filepath = PROJECT_FOLDER + "/" + target;
            saveProject(filepath);
            setCurrentLoadedProject(target);
            refreshProjects();
            bool dummy = true;
            UiMessage::show("Saved: " + shorten(target, 16), dummy, 2500);
        } else {
            startNewProject();
        }
    }

    void startNewProject()
    {
        if (confirmSave || confirmDelete) return;
        inputProjectName.clear();
        kbSelectedRow = 0;
        kbSelectedCol = 0;
        currentMode = VIEW_KEYBOARD_NEW;
        updatePadLeds();
        updateEncoderLabels();
    }

    void startRenameProject()
    {
        if (confirmSave || confirmDelete) return;
        if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
            renameProjectOldName = projectFiles[selectedFile];
            inputProjectName = projectFiles[selectedFile];
            kbSelectedRow = 0;
            kbSelectedCol = 0;
            currentMode = VIEW_KEYBOARD_RENAME;
            updatePadLeds();
            updateEncoderLabels();
        }
    }

    void startDeleteProject()
    {
        if (confirmSave || confirmDelete) return;
        if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
            confirmDelete = true;
            pendingDeleteFilename = projectFiles[selectedFile];
            updatePadLeds();
        }
    }

    void executeConfirm()
    {
        if (confirmSave && !pendingSaveFilename.empty()) {
            std::string filepath = PROJECT_FOLDER + "/" + pendingSaveFilename;
            saveProject(filepath);
            setCurrentLoadedProject(pendingSaveFilename);
            refreshProjects();
            bool dummy = true;
            UiMessage::show("Saved: " + shorten(pendingSaveFilename, 16), dummy, 2500);
            confirmSave = false;
            pendingSaveFilename.clear();
            updatePadLeds();
            return;
        }

        if (confirmDelete && !pendingDeleteFilename.empty()) {
            try {
                std::string filepath = PROJECT_FOLDER + "/" + pendingDeleteFilename;
                bool removed = std::filesystem::remove(filepath);
                if (removed) {
                    if (pendingDeleteFilename == currentLoadedFile) {
                        setCurrentLoadedProject("");
                        currentLoadedFile.clear();
                    }
                    bool dummy = true;
                    UiMessage::show("Deleted: " + shorten(pendingDeleteFilename, 16), dummy, 2500);
                } else {
                    bool dummy = true;
                    UiMessage::show("Delete failed", dummy, 2500);
                }
            } catch (...) {
                bool dummy = true;
                UiMessage::show("Delete failed", dummy, 2500);
            }
            confirmDelete = false;
            pendingDeleteFilename.clear();
            refreshProjects();
            updatePadLeds();
        }
    }

    void executeCancel()
    {
        confirmSave = false;
        pendingSaveFilename.clear();
        confirmDelete = false;
        pendingDeleteFilename.clear();
        updatePadLeds();
    }

    void submitKeyboardInput()
    {
        if (inputProjectName.empty()) {
            bool dummy = true;
            UiMessage::show("Name is empty", dummy, 2000);
            return;
        }

        if (currentMode == VIEW_KEYBOARD_RENAME) {
            if (inputProjectName != renameProjectOldName) {
                try {
                    std::string oldPath = PROJECT_FOLDER + "/" + renameProjectOldName;
                    std::string newPath = PROJECT_FOLDER + "/" + inputProjectName;
                    if (std::filesystem::exists(newPath)) {
                        bool dummy = true;
                        UiMessage::show("Name already exists", dummy, 2000);
                        return;
                    }
                    std::filesystem::rename(oldPath, newPath);
                    if (currentLoadedFile == renameProjectOldName) {
                        setCurrentLoadedProject(inputProjectName);
                    }
                    bool dummy = true;
                    UiMessage::show("Renamed to " + shorten(inputProjectName, 14), dummy, 2500);
                } catch (...) {
                    bool dummy = true;
                    UiMessage::show("Rename failed", dummy, 2000);
                }
            }
        } else {
            std::string filepath = PROJECT_FOLDER + "/" + inputProjectName;
            saveProject(filepath);
            setCurrentLoadedProject(inputProjectName);
            bool dummy = true;
            UiMessage::show("Saved: " + shorten(inputProjectName, 16), dummy, 2500);
        }

        currentMode = VIEW_LIST;
        refreshProjects();
        updatePadLeds();
        updateEncoderLabels();
    }

    void appendChar(char c)
    {
        if (inputProjectName.length() < 30) {
            inputProjectName += c;
        }
    }

    void handleCharTyped(char c)
    {
        if (currentMode != VIEW_LIST) {
            if (c >= 32 && c <= 126) {
                appendChar(c);
                updatePadLeds();
            }
        }
    }

    void handleKeyInput(int key)
    {
        // 8 = Backspace, 13 = Enter, 27 = Esc
        if (currentMode != VIEW_LIST) {
            if (key == 8) { // Backspace
                if (!inputProjectName.empty()) inputProjectName.pop_back();
            } else if (key == 13) { // Enter
                submitKeyboardInput();
            } else if (key == 27) { // Esc
                currentMode = VIEW_LIST;
                refreshProjects();
            }
            updatePadLeds();
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        if (!pressed) return;

        if (currentMode == VIEW_LIST) {
            if (confirmSave || confirmDelete) {
                if (row == 3 && col == 5) executeConfirm();
                else if (row == 3 && col == 6) executeCancel();
                return;
            }

            if (row == 3) {
                if (col == 7) { // Pad next to Z (Col 7, Row 3) -> DELETE
                    startDeleteProject();
                } else if (col == 5 && (confirmSave || confirmDelete)) {
                    executeConfirm();
                } else if (col == 6 && (confirmSave || confirmDelete)) {
                    executeCancel();
                }
            }
        } else { // Virtual Keyboard Mode
            if (row == 3) {
                switch (col) {
                    case 0: // BKSP
                        if (!inputProjectName.empty()) inputProjectName.pop_back();
                        break;
                    case 1: // SPACE
                        appendChar(' ');
                        break;
                    case 2: // CLEAR
                        inputProjectName.clear();
                        break;
                    case 3: { // ADD
                        int keyIdx = (kbSelectedRow * KB_COLS) + kbSelectedCol;
                        if (keyIdx >= 0 && keyIdx < KB_KEYS_COUNT) {
                            appendChar(kbKeys[keyIdx][0]);
                        }
                        break;
                    }
                    case 5: // OK
                        submitKeyboardInput();
                        break;
                    case 6: // CANCEL
                        currentMode = VIEW_LIST;
                        refreshProjects();
                        break;
                    default: break;
                }
            } else if (row >= 0 && row < 3) {
                kbSelectedRow = row;
                kbSelectedCol = col;
                int keyIdx = (row * DYNAMIC_PAD_COLS) + col;
                if (keyIdx < KB_KEYS_COUNT) {
                    appendChar(kbKeys[keyIdx][0]);
                }
            }
            updatePadLeds();
            updateEncoderLabels();
        }
    }

    void handleUtilityPadPress(int utilCol, bool pressed) override
    {
        if (!pressed) return;

        if (currentMode == VIEW_LIST) {
            if (confirmSave || confirmDelete) {
                if (utilCol == 2) executeConfirm();
                else if (utilCol == 3) executeCancel();
                return;
            }

            if (utilCol == 0) { // Key Z (Col 8) -> RENAME
                startRenameProject();
            } else if (utilCol == 1) { // Key X (Col 9) -> NEW
                startNewProject();
            } else if (utilCol == 2) { // Key C (Col 10) -> LOAD
                executeLoad();
            } else if (utilCol == 3) { // Key V (Col 11) -> SAVE
                executeSave();
            }
        } else {
            if (utilCol == 0) { // BKSP
                if (!inputProjectName.empty()) inputProjectName.pop_back();
            } else if (utilCol == 1) { // SPACE
                appendChar(' ');
            } else if (utilCol == 2) { // OK
                submitKeyboardInput();
            } else if (utilCol == 3) { // CANCEL
                currentMode = VIEW_LIST;
                refreshProjects();
            }
            updatePadLeds();
            updateEncoderLabels();
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        // Container box
        d.filledRect({ x, y }, { w, h }, { .color = Color { 18, 22, 30, 255 } });
        d.rect({ x, y }, { w, h }, { .color = Color { 40, 50, 70, 255 } });

        // Header Bar
        int headerH = 24;
        d.filledRect({ x, y }, { w, headerH }, { .color = Color { 28, 35, 48, 255 } });
        d.line({ x, y + headerH }, { x + w, y + headerH }, { .color = Color { 50, 65, 90, 255 } });

        Icon icon(d);
        icon.render("&icon::project", { x + 6, y + 5 }, 14, Color { 0, 180, 255, 255 });

        std::string modeTitle = (currentMode == VIEW_LIST) ? "PROJECT MANAGER" : (currentMode == VIEW_KEYBOARD_NEW ? "CREATE NEW PROJECT" : "RENAME PROJECT");
        d.text({ x + 26, y + 5 }, modeTitle, 12, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_12 });

        // Loaded Project Badge in Header
        if (!currentLoadedFile.empty()) {
            std::string loadedTag = "ACTIVE: " + shorten(currentLoadedFile, 14);
            int tagW = (int)loadedTag.length() * 6 + 12;
            int tagX = x + w - tagW - 6;
            d.filledRect({ tagX, y + 4 }, { tagW, 16 }, { .color = Color { 20, 80, 40, 220 } });
            d.rect({ tagX, y + 4 }, { tagW, 16 }, { .color = Color { 40, 200, 80, 255 } });
            d.textCentered({ tagX + tagW / 2, y + 6 }, loadedTag, 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
        }

        int contentY = y + headerH + 4;
        int contentH = h - headerH - 8;

        if (currentMode == VIEW_LIST) {
            renderProjectList(d, x + 6, contentY, w - 12, contentH);
        } else {
            renderVirtualKeyboard(d, x + 6, contentY, w - 12, contentH);
        }

        // Confirmation Modal Overlay
        if (confirmSave || confirmDelete) {
            renderConfirmationModal(d, x, y, w, h);
        }
    }

private:
    void renderProjectList(Draw& d, int lx, int ly, int lw, int lh)
    {
        if (projectFiles.empty()) {
            d.textCentered({ lx + lw / 2, ly + lh / 2 - 10 }, "No projects found in workspace", 12, { .color = Color { 160, 170, 190, 255 }, .font = &PoppinsLight_12 });
            d.textCentered({ lx + lw / 2, ly + lh / 2 + 10 }, "Tap 'NEW' pad or turn encoder to create one", 8, { .color = Color { 110, 125, 145, 255 }, .font = &PoppinsLight_8 });
            return;
        }

        int itemH = 26;
        int itemGap = 3;
        int maxVisible = std::max(1, lh / (itemH + itemGap));
        int maxScroll = std::max(0, (int)projectFiles.size() - maxVisible);
        scrollOffset = std::clamp(scrollOffset, 0, maxScroll);

        for (int i = 0; i < maxVisible; ++i) {
            int fileIdx = i + scrollOffset;
            if (fileIdx >= (int)projectFiles.size()) break;

            int itemY = ly + i * (itemH + itemGap);
            bool isSelected = (selectedFile == fileIdx);
            bool isLoaded = (projectFiles[fileIdx] == currentLoadedFile);

            Color bg = isSelected ? Color { 45, 60, 90, 255 } : Color { 24, 30, 42, 255 };
            Color border = isSelected ? Color { 0, 180, 255, 255 } : Color { 38, 48, 66, 255 };

            d.filledRect({ lx, itemY }, { lw - 10, itemH }, { .color = bg });
            d.rect({ lx, itemY }, { lw - 10, itemH }, { .color = border });

            Icon icon(d);
            icon.render("&icon::project", { lx + 8, itemY + 6 }, 14, isSelected ? Color { 0, 180, 255, 255 } : Color { 120, 140, 170, 255 });

            d.text({ lx + 28, itemY + 6 }, shorten(projectFiles[fileIdx], 26), 12, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_12 });

            if (isLoaded) {
                int badgeW = 54;
                int badgeX = lx + lw - 10 - badgeW - 6;
                d.filledRect({ badgeX, itemY + 4 }, { badgeW, 18 }, { .color = Color { 30, 120, 50, 255 } });
                d.textCentered({ badgeX + badgeW / 2, itemY + 7 }, "LOADED", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            }
        }

        // Scroll bar
        if ((int)projectFiles.size() > maxVisible) {
            float pct = (float)scrollOffset / std::max(1.0f, (float)maxScroll);
            int barH = std::max(16, lh * maxVisible / (int)projectFiles.size());
            int barY = ly + (lh - barH) * pct;
            d.filledRect({ lx + lw - 5, ly }, { 4, lh }, { .color = Color { 25, 32, 45, 255 } });
            d.filledRect({ lx + lw - 5, barY }, { 4, barH }, { .color = Color { 0, 180, 255, 255 } });
        }
    }

    void renderVirtualKeyboard(Draw& d, int kx, int ky, int kw, int kh)
    {
        // Text Input Display Box
        int boxH = 32;
        d.filledRect({ kx, ky }, { kw, boxH }, { .color = Color { 12, 16, 24, 255 } });
        d.rect({ kx, ky }, { kw, boxH }, { .color = Color { 0, 180, 255, 255 } });

        int textX = d.text({ kx + 10, ky + 8 }, inputProjectName, 12, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_12, .fontSpacing = 1 });

        // Glowing animated blinking cursor bar
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
        bool blink = (nowMs / 400) % 2 == 0;
        if (blink) {
            d.filledRect({ textX + 2, ky + 7 }, { 3, 18 }, { .color = Color { 0, 220, 255, 255 } });
            d.rect({ textX + 2, ky + 7 }, { 3, 18 }, { .color = Color { 120, 240, 255, 255 } });
        }

        // Virtual Keyboard Keys Grid (8 cols x 8 rows)
        int gridY = ky + boxH + 8;
        int gridH = kh - boxH - 12;
        int keyW = std::max(12, (kw - (KB_COLS - 1) * 2) / KB_COLS);
        int keyH = std::max(12, (gridH - (KB_ROWS - 1) * 2) / KB_ROWS);

        for (int r = 0; r < KB_ROWS; ++r) {
            for (int c = 0; c < KB_COLS; ++c) {
                int keyIdx = r * KB_COLS + c;
                if (keyIdx >= KB_KEYS_COUNT) break;

                int px = kx + c * (keyW + 2);
                int py = gridY + r * (keyH + 2);

                bool isSelected = (kbSelectedRow == r && kbSelectedCol == c);
                Color bg = isSelected ? Color { 0, 150, 220, 255 } : Color { 32, 42, 58, 255 };
                Color textCol = isSelected ? Color { 255, 255, 255, 255 } : Color { 200, 215, 235, 255 };

                d.filledRect({ px, py }, { keyW, keyH }, { .color = bg });
                d.textCentered({ px + keyW / 2, py + keyH / 2 - 4 }, kbKeys[keyIdx], 8, { .color = textCol, .font = &PoppinsLight_8 });
            }
        }
    }

    void renderConfirmationModal(Draw& d, int vx, int vy, int vw, int vh)
    {
        int mw = std::min(360, vw - 20);
        int mh = 130;
        int mx = vx + (vw - mw) / 2;
        int my = vy + (vh - mh) / 2;

        d.filledRect({ mx, my }, { mw, mh }, { .color = Color { 22, 26, 36, 245 } });
        d.rect({ mx, my }, { mw, mh }, { .color = confirmDelete ? Color { 255, 60, 60, 255 } : Color { 255, 180, 40, 255 } });

        std::string title = confirmDelete ? "CONFIRM DELETE" : "CONFIRM OVERWRITE";
        std::string target = confirmDelete ? pendingDeleteFilename : pendingSaveFilename;
        std::string line1 = confirmDelete ? "Permanently delete project file?" : "Overwrite existing project file?";

        d.textCentered({ mx + mw / 2, my + 16 }, title, 12, { .color = confirmDelete ? Color { 255, 90, 90, 255 } : Color { 255, 200, 50, 255 }, .font = &PoppinsLight_12 });
        d.textCentered({ mx + mw / 2, my + 44 }, line1, 8, { .color = Color { 220, 225, 235, 255 }, .font = &PoppinsLight_8 });
        d.textCentered({ mx + mw / 2, my + 62 }, "\"" + shorten(target, 24) + "\"", 12, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_12 });

        d.textCentered({ mx + mw / 2, my + 96 }, "Press CONFIRM (Pad 5) or CANCEL (Pad 6)", 8, { .color = Color { 160, 175, 195, 255 }, .font = &PoppinsLight_8 });
    }
};
