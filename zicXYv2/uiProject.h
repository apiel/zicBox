#pragma once

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

#include "draw/utils/Icon.h"
#include "draw/utils/inRect.h"
#include "zicXYv2/project.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/uiMessage.h"

namespace UiProject {

bool needsRedraw = true;

Rect listRect;
std::vector<Rect> fileRects;

std::vector<std::string> projectFiles;
int selectedFile = -1;
int scrollOffset = 0;
std::string currentLoadedFile = "";
bool confirmSave = false;
std::string pendingSaveFilename = "";

const int ITEM_H = 20;
const int ITEM_GAP = 2;

std::string shortenFilename(const std::string& name, int maxLen = 26)
{
    if ((int)name.size() <= maxLen) return name;
    return name.substr(0, maxLen - 3) + "...";
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

    int margin = 4;
    Rect rect = { { margin, currentY + margin }, { winW - margin * 2, winH - currentY - margin * 2 } };

    d.filledRect(rect.position, rect.size, { .color = { 18, 18, 24 } });
    d.text({ rect.position.x + 6, rect.position.y + 6 }, "PROJECT", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });

    Rect listRect = { { rect.position.x + 4, rect.position.y + 24 }, { rect.size.w - 8, rect.size.h - 36 } };
    drawList(d, listRect);

    if (confirmSave) {
        Rect overlay = { { (winW - 340) / 2, (winH - 120) / 2 }, { 340, 120 } };
        d.filledRect(overlay.position, overlay.size, { .color = { 30, 30, 40, 230 } });
        d.rect(overlay.position, overlay.size, { .color = { 140, 140, 150 } });

        d.textCentered({ overlay.position.x + overlay.size.w / 2, overlay.position.y + 18 },
            "Confirm Save", 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
        d.textCentered({ overlay.position.x + overlay.size.w / 2, overlay.position.y + 42 },
            "Overwrite selected project?", 8, { .color = { 220, 220, 230 }, .font = &PoppinsLight_8 });
        d.textCentered({ overlay.position.x + overlay.size.w / 2, overlay.position.y + 56 },
            "Save to \"" + shortenFilename(pendingSaveFilename, 24) + "\"?", 8, { .color = { 220, 220, 230 }, .font = &PoppinsLight_8 });
        d.textCentered({ overlay.position.x + overlay.size.w / 2, overlay.position.y + 86 },
            "Press confirm", 8, { .color = { 200, 200, 220 }, .font = &PoppinsLight_8 });
    }

    return true;
}

void mouseButtonPressed(Point position, bool& needFullRedraw)
{
    if (studio.currentView != ViewProject) return;

    if (confirmSave) {
        // keyboard-only confirmation: ignore mouse clicks while waiting for KEY_8
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

    if (key == KEY_7) {
        if (confirmSave) { // Cancel
            confirmSave = false;
            pendingSaveFilename.clear();
            needFullRedraw = true;
            return;
        }
    }

    if (key == KEY_8) { // Confirm save when overlay shown
        if (confirmSave && !pendingSaveFilename.empty()) {
            std::string filepath = PROJECT_FOLDER + "/" + pendingSaveFilename;
            saveProject(filepath);
            setCurrentLoadedProject(pendingSaveFilename);
            refreshProjects();
            UiMessage::show("Saved " + shortenFilename(pendingSaveFilename), needsRedraw);
            needFullRedraw = true;
            confirmSave = false;
            pendingSaveFilename.clear();
        }
        return;
    }

    if (key == KEY_F2) {
        if (selectedFile < (int)projectFiles.size() - 1) {
            selectedFile++;
            if (selectedFile >= scrollOffset + 8) scrollOffset = selectedFile - 7;
            needsRedraw = true;
            // needFullRedraw = true;
        }
        return;
    }

    if (key == KEY_F3) {
        if (selectedFile > 0) {
            selectedFile--;
            if (selectedFile < scrollOffset) scrollOffset = selectedFile;
            needsRedraw = true;
            // needFullRedraw = true;
        }
        return;
    }
}

bool mouseWheelScrolled(int delta)
{
    if (studio.currentView != ViewProject) return false;
    scrollOffset -= (delta > 0 ? 1 : -1);
    needsRedraw = true;
    return true;
}

}
