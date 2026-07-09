#pragma once

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "draw/utils/Icon.h"
#include "draw/utils/inRect.h"
#include "zicXYv2/project.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/uiKeyboard.h"
#include "zicXYv2/uiMessage.h"

namespace UiMenu {

bool needsRedraw = true;

Rect listRect;
std::vector<Rect> fileRects;

// keyboard / new project view
enum ProjectView { VIEW_LIST = 0,
    VIEW_KEYBOARD_NEW };
ProjectView currentView = VIEW_LIST;

std::string newProjectName = "";

std::vector<std::string> projectFiles;
int selectedFile = -1;
int scrollOffset = 0;
std::string currentLoadedFile = "";
bool confirmSave = false;
std::string pendingSaveFilename = "";
bool confirmDelete = false;
std::string pendingDeleteFilename = "";
bool confirmShutdown = false;

const int ITEM_H = 20;
const int ITEM_GAP = 2;

std::string shortenFilename(const std::string& name, int maxLen = 26)
{
    if ((int)name.size() <= maxLen) return name;
    return name.substr(0, maxLen - 3) + "...";
}

bool isKeyboardMode()
{
    return currentView == VIEW_KEYBOARD_NEW;
}

void refreshProjects()
{
    projectFiles.clear();

    try {
        currentLoadedFile = getCurrentLoadedProject();
        for (const auto& entry : std::filesystem::directory_iterator(PROJECT_FOLDER)) {
            std::string filename = entry.path().filename().string();
            if (filename == CURRENT_FILE) continue;
            projectFiles.push_back(filename);
        }

        std::sort(projectFiles.begin(), projectFiles.end());

        if (projectFiles.empty()) {
            selectedFile = -1;
        } else {
            selectedFile = 0;
            for (size_t i = 0; i < projectFiles.size(); i++) {
                if (projectFiles[i] == currentLoadedFile) {
                    selectedFile = (int)i;
                    break;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cout << "could not open project folder: " << e.what() << std::endl;
    }
}

void drawList(Draw& d, Rect rect)
{
    listRect = rect;
    d.filledRect(rect.position, rect.size, { .color = { 24, 24, 30 } });

    fileRects.clear();

    int visibleHeight = rect.size.h;
    int maxVisibleItems = std::max(1, visibleHeight / (ITEM_H + ITEM_GAP));
    int maxScroll = std::max(0, (int)projectFiles.size() - maxVisibleItems);
    scrollOffset = std::clamp(scrollOffset, 0, maxScroll);

    if (projectFiles.empty()) {
        d.textCentered({ rect.position.x + rect.size.w / 2, rect.position.y + 20 }, "No projects found", 8, { .color = { 200, 200, 210 }, .font = &PoppinsLight_8 });
        return;
    }

    for (int visibleIdx = 0; visibleIdx < maxVisibleItems; visibleIdx++) {
        int fileIdx = visibleIdx + scrollOffset;
        if (fileIdx >= (int)projectFiles.size()) break;

        Rect r = { { rect.position.x + 4, rect.position.y + 4 + visibleIdx * (ITEM_H + ITEM_GAP) }, { rect.size.w - 8, ITEM_H } };
        fileRects.push_back(r);

        bool isSelected = selectedFile == fileIdx;
        bool isCurrent = projectFiles[fileIdx] == currentLoadedFile;
        Color bg = isSelected ? Color { 70, 70, 90 } : Color { 45, 45, 55 };
        d.filledRect(r.position, r.size, { .color = bg });
        d.text({ r.position.x + 6, r.position.y + 4 }, shortenFilename(projectFiles[fileIdx]), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        if (isCurrent) {
            Rect tag = { { r.position.x + r.size.w - 52, r.position.y + 3 }, { 48, 12 } };
            d.filledRect(tag.position, tag.size, { .color = { 70, 140, 70 } });
            d.textCentered({ tag.position.x + tag.size.w / 2, tag.position.y + 2 }, "LOADED", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        }
    }

    if ((int)projectFiles.size() > maxVisibleItems) {
        float pct = (float)scrollOffset / std::max(1.0f, (float)maxScroll);
        int barH = std::max(18, visibleHeight * maxVisibleItems / (int)projectFiles.size());
        int barY = rect.position.y + (visibleHeight - barH) * pct;
        d.filledRect({ rect.position.x + rect.size.w - 4, barY }, { 3, barH }, { .color = { 90, 90, 100 } });
    }
}

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    if (selectedFile == -1) refreshProjects();

    if (currentView == VIEW_KEYBOARD_NEW) {
        std::string value = shortenFilename(newProjectName, 30);
        UiKeyboard::draw(d, winW, winH, currentY, "Projects", "NEW:", value);
        return true;
    }

    int margin = 4;
    Rect rect = { { margin, currentY + margin }, { winW - margin * 2, winH - currentY - margin * 2 } };

    d.filledRect(rect.position, rect.size, { .color = { 18, 18, 24 } });
    d.text({ rect.position.x + 6, rect.position.y + 6 }, "Projects", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    Rect listRect = { { rect.position.x + 4, rect.position.y + 24 }, { rect.size.w - 8, rect.size.h - 36 } };
    drawList(d, listRect);

    if (confirmSave || confirmDelete || confirmShutdown) {
        int overlayW = std::min(340, winW - 20);
        int overlayH = 120;
        Rect overlay = { { (winW - overlayW) / 2, (winH - overlayH) / 2 }, { overlayW, overlayH } };
        d.filledRect(overlay.position, overlay.size, { .color = { 30, 30, 40, 230 } });
        d.rect(overlay.position, overlay.size, { .color = { 140, 140, 150 } });

        std::string title;
        std::string line1;
        std::string line2;
        if (confirmShutdown) {
            title = "Confirm Shutdown";
            line1 = "Power off Raspberry Pi?";
            line2 = "Unsaved changes may be lost.";
        } else {
            title = confirmDelete ? "Confirm Delete" : "Confirm Save";
            line1 = confirmDelete ? "Delete selected project?" : "Overwrite selected project?";
            std::string fileName = confirmDelete ? pendingDeleteFilename : pendingSaveFilename;
            line2 = (confirmDelete ? "Delete \"" : "Save to \"") + shortenFilename(fileName, 24) + "\"?";
        }

        d.textCentered({ overlay.position.x + overlay.size.w / 2, overlay.position.y + 18 },
            title, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
        d.textCentered({ overlay.position.x + overlay.size.w / 2, overlay.position.y + 42 },
            line1, 8, { .color = { 220, 220, 230 }, .font = &PoppinsLight_8 });
        d.textCentered({ overlay.position.x + overlay.size.w / 2, overlay.position.y + 56 },
            line2, 8, { .color = { 220, 220, 230 }, .font = &PoppinsLight_8 });
        d.textCentered({ overlay.position.x + overlay.size.w / 2, overlay.position.y + 86 },
            confirmSave ? "Press confirm or save" : "Press confirm", 8, { .color = { 200, 200, 220 }, .font = &PoppinsLight_8 });
    }

    return true;
}

void mouseButtonPressed(Point position, bool& needFullRedraw)
{
    if (studio.currentView != ViewProject) return;

    if (currentView == VIEW_LIST && (confirmSave || confirmDelete || confirmShutdown)) {
        // keyboard-only confirmation: ignore mouse clicks while waiting for KEY_8
        return;
    }

    if (currentView == VIEW_KEYBOARD_NEW) {
        UiKeyboard::mouseButtonPressed(position, needFullRedraw, newProjectName);
        return;
    }

    for (size_t i = 0; i < fileRects.size(); i++) {
        if (inRect(fileRects[i], position)) {
            selectedFile = (int)i + scrollOffset;
            needsRedraw = true;
            needFullRedraw = true;
            return;
        }
    }
}

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentView != ViewProject) return;

    if (currentView == VIEW_KEYBOARD_NEW) {
        UiKeyboard::keyPressed(key, needFullRedraw, newProjectName);

        if (key == KEY_F4) {
            if (newProjectName.empty()) {
                UiMessage::show("Name is empty", needsRedraw);
                needFullRedraw = true;
                return;
            }

            std::string filepath = PROJECT_FOLDER + "/" + newProjectName;
            saveProject(filepath);
            setCurrentLoadedProject(newProjectName);
            currentView = VIEW_LIST;
            refreshProjects();
            UiMessage::show("Saved " + shortenFilename(newProjectName), needsRedraw);
            needFullRedraw = true;
            return;
        }

        if (key == KEY_F5) {
            currentView = VIEW_LIST;
            needFullRedraw = true;
            return;
        }

        return;
    }

    if ((key == KEY_8 && studio.currentCombinationKey == KeyNone) || (key == KEY_2 && confirmSave)) { // Confirm action when overlay shown
        if (confirmShutdown) {
#ifdef IS_RPI
            int exitCode = std::system("halt");
            if (exitCode != 0) {
                UiMessage::show("Shutdown failed", needsRedraw);
                needFullRedraw = true;
            } else {
                UiMessage::show("Goodbye...", needsRedraw);
                keep_running = false;
            }
#else
            UiMessage::show("Shutdown only on RPi", needsRedraw);
            needFullRedraw = true;
#endif
            confirmShutdown = false;
            return;
        }

        if (confirmSave && !pendingSaveFilename.empty()) {
            std::string filepath = PROJECT_FOLDER + "/" + pendingSaveFilename;
            saveProject(filepath);
            setCurrentLoadedProject(pendingSaveFilename);
            refreshProjects();
            UiMessage::show("Saved " + shortenFilename(pendingSaveFilename), needsRedraw);
            needFullRedraw = true;
            confirmSave = false;
            pendingSaveFilename.clear();
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
                    UiMessage::show("Deleted " + shortenFilename(pendingDeleteFilename), needsRedraw);
                } else {
                    UiMessage::show("Delete failed", needsRedraw);
                }
            } catch (...) {
                UiMessage::show("Delete failed", needsRedraw);
            }

            confirmDelete = false;
            pendingDeleteFilename.clear();
            refreshProjects();
            needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_1) {
        if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
            std::string filepath = PROJECT_FOLDER + "/" + projectFiles[selectedFile];
            loadProject(filepath);
            setCurrentLoadedProject(projectFiles[selectedFile]);
            refreshProjects();
            UiMessage::show("Loaded " + shortenFilename(projectFiles[selectedFile]), needsRedraw);
            needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_2) { // Save - require confirmation if saving to different loaded project
        if (selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
            std::string target = projectFiles[selectedFile];
            if (!currentLoadedFile.empty() && target != currentLoadedFile) {
                // show confirmation overlay and wait for KEY_8
                confirmSave = true;
                pendingSaveFilename = target;
                needFullRedraw = true;
                return;
            }
            std::string filepath = PROJECT_FOLDER + "/" + target;
            saveProject(filepath);
            setCurrentLoadedProject(target);
            refreshProjects();
            UiMessage::show("Saved " + shortenFilename(target), needsRedraw);
            needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_3) { // New
        if (!confirmSave && !confirmDelete && !confirmShutdown) {
            newProjectName.clear();
            UiKeyboard::keyboardSelectedRow = 0;
            UiKeyboard::keyboardSelectedCol = 0;
            currentView = VIEW_KEYBOARD_NEW;
            needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_6) { // Delete selected project with confirmation
        if (!confirmSave && !confirmDelete && !confirmShutdown && selectedFile >= 0 && selectedFile < (int)projectFiles.size()) {
            confirmDelete = true;
            pendingDeleteFilename = projectFiles[selectedFile];
            needsRedraw = true;
            needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_7) {
        if (confirmSave || confirmDelete || confirmShutdown) { // Cancel
            confirmSave = false;
            pendingSaveFilename.clear();
            confirmDelete = false;
            pendingDeleteFilename.clear();
            confirmShutdown = false;
            needFullRedraw = true;
            return;
        }
    }

    if (key == KEY_F2) {
        if (selectedFile > 0) {
            selectedFile--;
            if (selectedFile < scrollOffset) scrollOffset = selectedFile;
            needsRedraw = true;
            // needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_F3) {
        if (selectedFile < (int)projectFiles.size() - 1) {
            selectedFile++;
            if (selectedFile >= scrollOffset + 8) scrollOffset = selectedFile - 7;
            needsRedraw = true;
            // needFullRedraw = true;
        }
        return;
    }
}

void onEncoder(int encoderId, int8_t direction)
{
    (void)encoderId;
    if (studio.currentView != ViewProject) return;
    if (currentView != VIEW_LIST) return;
    if (direction == 0) return;

    selectedFile -= (direction > 0 ? 1 : -1);
    if (selectedFile < 0) selectedFile = 0;
    if (selectedFile >= (int)projectFiles.size()) selectedFile = (int)projectFiles.size() - 1;
    if (selectedFile < scrollOffset) scrollOffset = selectedFile;
    if (selectedFile >= scrollOffset + 8) scrollOffset = selectedFile - 7;
    needsRedraw = true;
}

bool mouseWheelScrolled(int delta)
{
    if (studio.currentView != ViewProject) return false;
    if (currentView != VIEW_LIST) return false;
    onEncoder(1, (int8_t)delta);
    return true;
}

}
