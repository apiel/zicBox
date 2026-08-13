#pragma once

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "draw/draw.h"
#include "draw/utils/Icon.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/audioWorker.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/uiComponents.h"
#include "zicGridV2/uiMessage.h"

class MenuView : public View {
public:
    enum MenuMode {
        MODE_MAIN_MENU = 0,
        MODE_AUDIO_SELECT
    };

private:
    MenuMode currentMode = MODE_MAIN_MENU;
    int selectedOption = 0;
    static constexpr int OPTION_COUNT = 2;

    std::vector<AudioDeviceInfo> audioDevices;
    int selectedDeviceIdx = 0;
    int scrollOffset = 0;

    bool confirmShutdown = false;
    bool isShuttingDown = false;
    bool renderedGoodbye = false;

    void refreshAudioDevices()
    {
        audioDevices = getAudioOutputDevices();
        if (audioDevices.empty()) {
            audioDevices.push_back({ "default", "Default Audio Device" });
        }
        selectedDeviceIdx = 0;
        for (size_t i = 0; i < audioDevices.size(); ++i) {
            if (audioDevices[i].name == currentAudioDeviceName) {
                selectedDeviceIdx = (int)i;
                break;
            }
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

    void turnOffAllPads()
    {
        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < PAD_COLS; ++c) {
                auto& pad = gridState.pads[c][r];
                pad.selected = false;
                pad.active = false;
                pad.label = "";
                pad.color = { 0, 0, 0, 0 };
            }
        }
    }

    void executeHalt()
    {
        std::cout << "Shutting down..." << std::endl;
        turnOffAllPads();
        keep_running = false;
#if defined(IS_RPI)
        std::cout << "Shutting down RPi..." << std::endl;
        int exitCode = std::system("sync && (halt || /sbin/halt || /bin/halt)");
        std::cout << "[System] halt exit code: " << exitCode << std::endl;
#else
        std::cout << "[System] IS_RPI not defined, skipping halt command." << std::endl;
#endif
    }

public:
    MenuView()
        : View("SYSTEM MENU")
    {
    }

    void onActivate() override
    {
        currentMode = MODE_MAIN_MENU;
        confirmShutdown = false;
        isShuttingDown = false;
        renderedGoodbye = false;
        selectedOption = 0;
        scrollOffset = 0;
        refreshAudioDevices();
        updatePadLeds();
        updateEncoderLabels();
    }

    void onDeactivate() override
    {
        currentMode = MODE_MAIN_MENU;
        confirmShutdown = false;
        isShuttingDown = false;
        renderedGoodbye = false;
        restoreDefaultUtilityPads();

        for (int i = 0; i < TOTAL_ENCODERS; ++i) {
            gridState.setEncoderBg(i, { 0, 0, 0, 0 });
        }
    }

    void updatePadLeds() override
    {
        if (isShuttingDown) {
            turnOffAllPads();
            return;
        }

        // Dynamic Pads matrix (rows 0..3, cols 0..7) stay empty
        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                auto& pad = gridState.pads[c][r];
                pad.selected = false;
                pad.active = false;
                pad.label = "";
                pad.color = { 25, 30, 40, 255 };
            }
        }

        // Global Utility Zone Row 3 (Pads Z, X, C, V):
        // Z = Up
        gridState.pads[8][3].label = "&icon::arrowUp::filled";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };

        // X = Down
        gridState.pads[9][3].label = "&icon::arrowDown::filled";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };

        // C = OK
        gridState.pads[10][3].label = "OK";
        if (confirmShutdown) {
            gridState.pads[10][3].color = { 40, 220, 140, 255 };
        } else if (currentMode == MODE_AUDIO_SELECT) {
            gridState.pads[10][3].color = { 40, 220, 140, 255 };
        } else {
            gridState.pads[10][3].color = { 200, 200, 200, 255 };
        }

        // V = Back / Cancel
        if (confirmShutdown) {
            gridState.pads[11][3].label = "Cancel";
            gridState.pads[11][3].color = { 220, 60, 60, 255 };
        } else if (currentMode == MODE_AUDIO_SELECT) {
            gridState.pads[11][3].label = "Back";
            gridState.pads[11][3].color = { 220, 100, 60, 255 };
        } else {
            gridState.pads[11][3].label = "";
            gridState.pads[11][3].color = { 25, 30, 40, 255 };
        }
    }

    void updateEncoderLabels() override
    {
        if (currentMode == MODE_MAIN_MENU) {
            const char* optionStr = (selectedOption == 0) ? "1. Audio Output" : "2. Shutdown RPi";
            gridState.setEncoder(0, "MENU OPTION", (float)selectedOption, 0.0f, (float)(OPTION_COUNT - 1), 1.0f, optionStr, { 255, 160, 40, 255 });
        } else {
            const char* devDisp = (!audioDevices.empty() && selectedDeviceIdx >= 0 && selectedDeviceIdx < (int)audioDevices.size())
                ? audioDevices[selectedDeviceIdx].displayName.c_str()
                : "None";
            gridState.setEncoder(0, "AUDIO DEVICE", (float)selectedDeviceIdx, 0.0f, (float)(audioDevices.size() - 1), 1.0f, devDisp, { 40, 200, 255, 255 });
        }

        for (int i = 1; i < TOTAL_ENCODERS; ++i) {
            gridState.setEncoder(i, "", 0.0f, 0.0f, 1.0f, 1.0f, nullptr, { 0, 0, 0, 0 });
        }
    }

    void handleEncoder(int encoderId, int delta) override
    {
        if (isShuttingDown) return;

        if (encoderId == 0) {
            if (currentMode == MODE_MAIN_MENU) {
                if (!confirmShutdown) {
                    selectedOption = std::clamp(selectedOption + delta, 0, OPTION_COUNT - 1);
                    updateEncoderLabels();
                    updatePadLeds();
                }
            } else if (currentMode == MODE_AUDIO_SELECT) {
                if (!audioDevices.empty()) {
                    selectedDeviceIdx = std::clamp(selectedDeviceIdx + delta, 0, (int)audioDevices.size() - 1);
                    updateEncoderLabels();
                    updatePadLeds();
                }
            }
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        // DynamicPad stays empty for now
    }

    void handleUtilityPadPress(int utilCol, bool pressed) override
    {
        if (!pressed || isShuttingDown) return;

        if (currentMode == MODE_MAIN_MENU) {
            if (utilCol == 0) { // Pad Z = Up
                if (!confirmShutdown) {
                    selectedOption = std::clamp(selectedOption - 1, 0, OPTION_COUNT - 1);
                    updateEncoderLabels();
                    updatePadLeds();
                }
            } else if (utilCol == 1) { // Pad X = Down
                if (!confirmShutdown) {
                    selectedOption = std::clamp(selectedOption + 1, 0, OPTION_COUNT - 1);
                    updateEncoderLabels();
                    updatePadLeds();
                }
            } else if (utilCol == 2) { // Pad C = OK
                if (selectedOption == 0) {
                    currentMode = MODE_AUDIO_SELECT;
                    refreshAudioDevices();
                    updateEncoderLabels();
                    updatePadLeds();
                } else if (selectedOption == 1) {
                    if (!confirmShutdown) {
                        confirmShutdown = true;
                        updatePadLeds();
                        updateEncoderLabels();
                    } else {
                        isShuttingDown = true;
                        updatePadLeds();
                    }
                }
            } else if (utilCol == 3) { // Pad V = Cancel
                if (confirmShutdown) {
                    confirmShutdown = false;
                    updatePadLeds();
                    updateEncoderLabels();
                }
            }
        } else if (currentMode == MODE_AUDIO_SELECT) {
            if (utilCol == 0) { // Pad Z = Up
                if (!audioDevices.empty()) {
                    selectedDeviceIdx = std::clamp(selectedDeviceIdx - 1, 0, (int)audioDevices.size() - 1);
                    updateEncoderLabels();
                    updatePadLeds();
                }
            } else if (utilCol == 1) { // Pad X = Down
                if (!audioDevices.empty()) {
                    selectedDeviceIdx = std::clamp(selectedDeviceIdx + 1, 0, (int)audioDevices.size() - 1);
                    updateEncoderLabels();
                    updatePadLeds();
                }
            } else if (utilCol == 2) { // Pad C = OK (Activate selected device)
                if (!audioDevices.empty() && selectedDeviceIdx >= 0 && selectedDeviceIdx < (int)audioDevices.size()) {
                    changeAudioDevice(audioDevices[selectedDeviceIdx].name);
                    bool needsRedraw = true;
                    UiMessage::show("Audio Output Set: " + audioDevices[selectedDeviceIdx].displayName, needsRedraw);
                }
            } else if (utilCol == 3) { // Pad V = Back to main menu
                currentMode = MODE_MAIN_MENU;
                updateEncoderLabels();
                updatePadLeds();
            }
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        Icon icon(d);

        if (isShuttingDown) {
            // Render Goodbye screen matching zicPixelDrift badge style
            d.filledRect({ x, y }, { w, h }, { .color = Color { 14, 18, 26, 255 } });

            int cx = x + w / 2;
            int cy = y + h / 2;
            int boxW = 200;
            int boxH = 64;
            int boxX = cx - boxW / 2;
            int boxY = cy - boxH / 2;

            d.filledRect({ boxX, boxY }, { boxW, boxH }, { .color = Color { 14, 18, 26, 240 } });
            d.rect({ boxX, boxY }, { boxW, boxH }, { .color = Color { 55, 70, 95, 255 } });

            d.textCentered({ cx, boxY + 10 }, "Zic Grid V2", 8, { .color = Color { 140, 160, 190, 255 }, .font = &PoppinsLight_8 });
            d.textCentered({ cx, boxY + 28 }, "GOODBYE", 12, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_12 });
            d.textCentered({ cx, boxY + 46 }, "SYSTEM HALTED", 8, { .color = Color { 100, 118, 145, 255 }, .font = &PoppinsLight_8 });

            if (!renderedGoodbye) {
                renderedGoodbye = true;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                executeHalt();
            }
            return;
        }

        // Card Container Background
        d.filledRect({ x, y }, { w, h }, { .color = Color { 16, 20, 28, 255 } });
        d.rect({ x, y }, { w, h }, { .color = Color { 45, 55, 75, 200 } });

        if (currentMode == MODE_MAIN_MENU) {
            // Header Title
            d.filledRect({ x + 1, y + 1 }, { w - 2, 22 }, { .color = Color { 24, 30, 42, 255 } });
            d.line({ x, y + 23 }, { x + w, y + 23 }, { .color = Color { 45, 55, 75, 200 } });

            icon.render("&icon::menu", { x + 8, y + 5 }, 12, Color { 255, 160, 40, 255 });
            d.text({ x + 26, y + 6 }, "SYSTEM MENU", 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });

            // Content Area
            int cardMargin = 12;
            int cardX = x + cardMargin;
            int cardY0 = y + 32;
            int cardW = w - cardMargin * 2;
            int cardH = 54;
            int cardSpacing = 10;

            // Card 0: Audio Output
            Color card0Bg = (selectedOption == 0) ? Color { 20, 38, 50, 240 } : Color { 22, 28, 38, 220 };
            Color card0Border = (selectedOption == 0) ? Color { 40, 200, 255, 255 } : Color { 45, 55, 75, 200 };

            d.filledRect({ cardX, cardY0 }, { cardW, cardH }, { .color = card0Bg });
            d.rect({ cardX, cardY0 }, { cardW, cardH }, { .color = card0Border });

            icon.render("&icon::audio", { cardX + 10, cardY0 + 14 }, 24, Color { 40, 200, 255, 255 });

            d.text({ cardX + 44, cardY0 + 10 }, "Audio Output Device", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

            std::string activeDevStr = currentAudioDeviceName.empty() ? "default" : currentAudioDeviceName;
            std::string activeDevDisp = activeDevStr;
            for (const auto& dev : audioDevices) {
                if (dev.name == activeDevStr) {
                    activeDevDisp = dev.displayName;
                    break;
                }
            }

            d.text({ cardX + 44, cardY0 + 25 }, activeDevDisp.c_str(), 8, { .color = Color { 40, 220, 255, 255 }, .font = &PoppinsLight_8 });
            d.text({ cardX + 44, cardY0 + 39 }, "Press OK to select audio output device", 8, { .color = Color { 160, 175, 195, 255 }, .font = &PoppinsLight_8 });

            // Card 1: Shutdown RPi
            int cardY1 = cardY0 + cardH + cardSpacing;

            Color card1Bg = (selectedOption == 1) ? Color { 40, 22, 25, 240 } : Color { 22, 28, 38, 220 };
            Color card1Border = confirmShutdown ? Color { 255, 60, 60, 255 } : ((selectedOption == 1) ? Color { 220, 70, 70, 255 } : Color { 45, 55, 75, 200 });

            d.filledRect({ cardX, cardY1 }, { cardW, cardH }, { .color = card1Bg });
            d.rect({ cardX, cardY1 }, { cardW, cardH }, { .color = card1Border });

            icon.render("&icon::shutdown", { cardX + 10, cardY1 + 14 }, 24, Color { 255, 80, 80, 255 });

            d.text({ cardX + 44, cardY1 + 12 }, "Power Off / Shutdown RPi", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            d.text({ cardX + 44, cardY1 + 30 }, "Safely powers down Raspberry Pi system", 8, { .color = Color { 160, 175, 195, 255 }, .font = &PoppinsLight_8 });

            // Confirmation Modal / Banner if active
            if (confirmShutdown) {
                int modalW = cardW - 20;
                int modalH = 44;
                int modalX = cardX + 10;
                int modalY = cardY1 + cardH + 12;

                d.filledRect({ modalX, modalY }, { modalW, modalH }, { .color = Color { 50, 15, 18, 250 } });
                d.rect({ modalX, modalY }, { modalW, modalH }, { .color = Color { 255, 60, 60, 255 } });

                d.textCentered({ modalX + modalW / 2, modalY + 8 }, "ARE YOU SURE YOU WANT TO SHUTDOWN?", 8, { .color = Color { 255, 220, 220, 255 }, .font = &PoppinsLight_8 });
                d.textCentered({ modalX + modalW / 2, modalY + 24 }, "Press OK to confirm or Cancel", 8, { .color = Color { 255, 160, 160, 255 }, .font = &PoppinsLight_8 });
            }
        } else if (currentMode == MODE_AUDIO_SELECT) {
            // Header Title
            d.filledRect({ x + 1, y + 1 }, { w - 2, 22 }, { .color = Color { 20, 32, 48, 255 } });
            d.line({ x, y + 23 }, { x + w, y + 23 }, { .color = Color { 40, 120, 180, 200 } });

            icon.render("&icon::audio", { x + 8, y + 5 }, 12, Color { 40, 200, 255, 255 });
            d.text({ x + 26, y + 6 }, "AUDIO OUTPUT SELECTION", 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });

            int headerH = 24;
            int hintH = 18;
            int listY = y + headerH + 4;
            int itemH = 34;
            int itemSpacing = 4;
            int slotH = itemH + itemSpacing;

            // Dynamically utilize the entire available view height h
            int availableH = h - headerH - hintH - 8;
            int maxVisible = std::max(1, availableH / slotH);

            int totalCount = (int)audioDevices.size();
            bool showScrollBar = (totalCount > maxVisible);

            int listMargin = 4;
            int scrollBarW = 6;
            int listW = showScrollBar ? (w - listMargin * 2 - scrollBarW - 4) : (w - listMargin * 2);
            int listX = x + listMargin;

            if (selectedDeviceIdx < scrollOffset) {
                scrollOffset = selectedDeviceIdx;
            } else if (selectedDeviceIdx >= scrollOffset + maxVisible) {
                scrollOffset = selectedDeviceIdx - maxVisible + 1;
            }

            int maxScroll = std::max(0, totalCount - maxVisible);
            scrollOffset = std::clamp(scrollOffset, 0, maxScroll);
            int endIdx = std::min(scrollOffset + maxVisible, totalCount);

            for (int i = scrollOffset; i < endIdx; ++i) {
                int curY = listY + (i - scrollOffset) * slotH;
                bool isSelected = (i == selectedDeviceIdx);
                bool isActive = (audioDevices[i].name == currentAudioDeviceName);

                Color itemBg = isSelected ? Color { 20, 48, 68, 240 } : Color { 22, 28, 38, 220 };
                Color itemBorder = isActive ? Color { 40, 220, 140, 255 } : (isSelected ? Color { 40, 200, 255, 255 } : Color { 45, 55, 75, 200 });

                d.filledRect({ listX, curY }, { listW, itemH }, { .color = itemBg });
                d.rect({ listX, curY }, { listW, itemH }, { .color = itemBorder });

                if (isActive) {
                    icon.render("&icon::valid", { listX + 6, curY + 9 }, 16, Color { 40, 220, 140, 255 });
                } else if (isSelected) {
                    icon.render("&icon::arrowRight::filled", { listX + 8, curY + 11 }, 12, Color { 40, 200, 255, 255 });
                } else {
                    icon.render("&icon::audio", { listX + 6, curY + 9 }, 16, Color { 120, 135, 160, 255 });
                }

                Color textColor = isSelected ? Color { 255, 255, 255, 255 } : Color { 180, 195, 215, 255 };
                d.text({ listX + 26, curY + 10 }, audioDevices[i].displayName.c_str(), 8, { .color = textColor, .font = &PoppinsLight_8 });

                if (isActive) {
                    d.filledRect({ listX + listW - 52, curY + 9 }, { 46, 16 }, { .color = Color { 30, 90, 60, 220 } });
                    d.rect({ listX + listW - 52, curY + 9 }, { 46, 16 }, { .color = Color { 40, 220, 140, 255 } });
                    d.textCentered({ listX + listW - 29, curY + 13 }, "ACTIVE", 8, { .color = Color { 180, 255, 210, 255 }, .font = &PoppinsLight_8 });
                }
            }

            // Scrollbar rendering
            if (showScrollBar) {
                int totalListH = (endIdx - scrollOffset) * slotH - itemSpacing;
                int sbX = listX + listW + 4;
                int sbY = listY;
                int sbH = totalListH;

                d.filledRect({ sbX, sbY }, { scrollBarW, sbH }, { .color = Color { 22, 28, 38, 220 } });
                d.rect({ sbX, sbY }, { scrollBarW, sbH }, { .color = Color { 45, 55, 75, 180 } });

                int thumbH = std::max(12, (int)((float)maxVisible / totalCount * sbH));
                int maxThumbY = sbH - thumbH;
                int thumbY = sbY + (maxScroll > 0 ? (int)((float)scrollOffset / maxScroll * maxThumbY) : 0);

                d.filledRect({ sbX + 1, thumbY }, { scrollBarW - 2, thumbH }, { .color = Color { 40, 200, 255, 255 } });
            }

            // Sub-footer instruction line at the bottom
            int hintY = y + h - hintH + 2;
            d.textCentered({ x + w / 2, hintY }, "Z/X: Navigate   OK: Activate   V: Back", 8, { .color = Color { 140, 160, 195, 255 }, .font = &PoppinsLight_8 });
        }
    }
};
