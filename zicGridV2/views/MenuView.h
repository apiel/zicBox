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
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"
#include "zicGridV2/uiMessage.h"

class MenuView : public View {
public:
    enum MenuMode {
        MODE_MAIN_MENU = 0,
        MODE_AUDIO_SELECT,
        MODE_SCATTER_PAD_SELECT,
        MODE_SCATTER_PAD_EDIT
    };

private:
    MenuMode currentMode = MODE_MAIN_MENU;
    int selectedOption = 0;
    static constexpr int OPTION_COUNT = 3;

    std::vector<AudioDeviceInfo> audioDevices;
    int selectedDeviceIdx = 0;
    int scrollOffset = 0;

    int selectedScatPad = 0;  // 0..7 for SCAT1..SCAT8
    int editFieldIndex = 0;   // Active field inside pad edit mode
    int previewModeActive = -1; // Currently previewing Scatter mode (-1 if none)

    bool confirmShutdown = false;
    bool isShuttingDown = false;
    bool renderedGoodbye = false;

    const char* getScatterModeName(int mode) const
    {
        switch (mode) {
        case 0: return "Comb LFO";
        case 1: return "Gater FX";
        case 2: return "Decimate & Dist";
        case 3: return "Reverb & Delay";
        case 4: return "Bitcrush Crunch";
        case 5: return "Hard Clip Drive";
        case 6: return "Acid Sweep";
        default: return "Unknown";
        }
    }

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

    void syncPreviewState()
    {
        if (currentMode == MODE_SCATTER_PAD_EDIT) {
            auto& cfg = studio.masterFx.scatPads[selectedScatPad];
            if (cfg.type == SCAT_TYPE_SCATTER) {
                for (int m = 0; m < 8; ++m) {
                    studio.masterFx.scatter.setModeActive(m, false);
                }
                for (int p = 0; p < 4; ++p) {
                    studio.masterFx.scatter.params[cfg.mode][p] = cfg.paramValues[p];
                }
                studio.masterFx.scatter.setModeActive(cfg.mode, true);
                previewModeActive = cfg.mode;
                return;
            }
        }
        if (previewModeActive >= 0) {
            studio.masterFx.scatter.setModeActive(previewModeActive, false);
            previewModeActive = -1;
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

    int getMaxFieldsForEdit() const
    {
        auto& cfg = studio.masterFx.scatPads[selectedScatPad];
        if (cfg.type == SCAT_TYPE_SCATTER) return 7;
        return 3;
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
        selectedScatPad = 0;
        editFieldIndex = 0;
        refreshAudioDevices();
        syncPreviewState();
        updatePadLeds();
        updateEncoderLabels();
    }

    void onDeactivate() override
    {
        currentMode = MODE_MAIN_MENU;
        confirmShutdown = false;
        isShuttingDown = false;
        renderedGoodbye = false;
        syncPreviewState();
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

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                auto& pad = gridState.pads[c][r];
                pad.selected = false;
                pad.active = false;
                pad.label = "";
                pad.color = { 25, 30, 40, 255 };
            }
        }

        gridState.pads[8][3].label = "&icon::arrowUp::filled";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };

        gridState.pads[9][3].label = "&icon::arrowDown::filled";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };

        gridState.pads[10][3].label = "OK";
        if (confirmShutdown) {
            gridState.pads[10][3].color = { 40, 220, 140, 255 };
        } else if (currentMode == MODE_AUDIO_SELECT || currentMode == MODE_SCATTER_PAD_SELECT) {
            gridState.pads[10][3].color = { 40, 220, 140, 255 };
        } else {
            gridState.pads[10][3].color = { 200, 200, 200, 255 };
        }

        if (confirmShutdown) {
            gridState.pads[11][3].label = "Cancel";
            gridState.pads[11][3].color = { 220, 60, 60, 255 };
        } else if (currentMode != MODE_MAIN_MENU) {
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
            const char* optionStr = "1. Audio Output";
            if (selectedOption == 1) optionStr = "2. Scatter Setup";
            else if (selectedOption == 2) optionStr = "3. Shutdown RPi";
            gridState.setEncoder(0, "MENU OPTION", (float)selectedOption, 0.0f, (float)(OPTION_COUNT - 1), 1.0f, optionStr, { 255, 160, 40, 255 });
        } else if (currentMode == MODE_AUDIO_SELECT) {
            const char* devDisp = (!audioDevices.empty() && selectedDeviceIdx >= 0 && selectedDeviceIdx < (int)audioDevices.size())
                ? audioDevices[selectedDeviceIdx].displayName.c_str()
                : "None";
            gridState.setEncoder(0, "AUDIO DEVICE", (float)selectedDeviceIdx, 0.0f, (float)(audioDevices.size() - 1), 1.0f, devDisp, { 40, 200, 255, 255 });
        } else if (currentMode == MODE_SCATTER_PAD_SELECT) {
            std::string padStr = "SCAT " + std::to_string(selectedScatPad + 1);
            gridState.setEncoder(0, "SELECT SCAT PAD", (float)selectedScatPad, 0.0f, 7.0f, 1.0f, padStr.c_str(), { 255, 180, 40, 255 });
        } else if (currentMode == MODE_SCATTER_PAD_EDIT) {
            auto& cfg = studio.masterFx.scatPads[selectedScatPad];
            std::string fieldLabel = "FIELD " + std::to_string(editFieldIndex + 1);
            gridState.setEncoder(0, fieldLabel.c_str(), (float)editFieldIndex, 0.0f, (float)(getMaxFieldsForEdit() - 1), 1.0f, nullptr, { 40, 220, 180, 255 });
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
            } else if (currentMode == MODE_SCATTER_PAD_SELECT) {
                selectedScatPad = std::clamp(selectedScatPad + delta, 0, 7);
                updateEncoderLabels();
                updatePadLeds();
            } else if (currentMode == MODE_SCATTER_PAD_EDIT) {
                auto& cfg = studio.masterFx.scatPads[selectedScatPad];
                if (editFieldIndex == 0) { // Type field
                    cfg.type = (cfg.type == SCAT_TYPE_SCATTER) ? SCAT_TYPE_NOTE_REPEAT : SCAT_TYPE_SCATTER;
                    editFieldIndex = 0;
                } else if (cfg.type == SCAT_TYPE_SCATTER) {
                    if (editFieldIndex == 1) { // Mode
                        cfg.mode = std::clamp(cfg.mode + delta, 0, 6);
                        studio.masterFx.scatter.resetParams(cfg.mode);
                        for (int p = 0; p < 4; ++p) {
                            cfg.paramValues[p] = studio.masterFx.scatter.params[cfg.mode][p];
                        }
                    } else if (editFieldIndex >= 2 && editFieldIndex <= 5) { // P1..P4
                        int pIdx = editFieldIndex - 2;
                        cfg.paramValues[pIdx] = std::clamp(cfg.paramValues[pIdx] + delta * 0.02f, 0.0f, 1.0f);
                        studio.masterFx.scatter.params[cfg.mode][pIdx] = cfg.paramValues[pIdx];
                    } else if (editFieldIndex == 6) { // Master Param Selection
                        cfg.masterParamIdx = std::clamp(cfg.masterParamIdx + delta, 0, 3);
                    }
                } else if (cfg.type == SCAT_TYPE_NOTE_REPEAT) {
                    if (editFieldIndex == 1) { // Target Track
                        cfg.trackIdx = std::clamp(cfg.trackIdx + delta, 0, MAX_TRACKS - 1);
                    } else if (editFieldIndex == 2) { // Repeat Rate
                        const int rates[] = { 1, 2, 4, 8 };
                        int rIdx = 0;
                        for (int i = 0; i < 4; ++i) {
                            if (rates[i] == cfg.repeatRate) { rIdx = i; break; }
                        }
                        rIdx = std::clamp(rIdx + delta, 0, 3);
                        cfg.repeatRate = rates[rIdx];
                    }
                }
                syncPreviewState();
                updateEncoderLabels();
                updatePadLeds();
            }
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
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
                } else if (selectedOption == 1) {
                    currentMode = MODE_SCATTER_PAD_SELECT;
                    selectedScatPad = 0;
                } else if (selectedOption == 2) {
                    if (!confirmShutdown) {
                        confirmShutdown = true;
                    } else {
                        isShuttingDown = true;
                    }
                }
                updateEncoderLabels();
                updatePadLeds();
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
                }
            } else if (utilCol == 1) { // Pad X = Down
                if (!audioDevices.empty()) {
                    selectedDeviceIdx = std::clamp(selectedDeviceIdx + 1, 0, (int)audioDevices.size() - 1);
                }
            } else if (utilCol == 2) { // Pad C = OK
                if (!audioDevices.empty() && selectedDeviceIdx >= 0 && selectedDeviceIdx < (int)audioDevices.size()) {
                    changeAudioDevice(audioDevices[selectedDeviceIdx].name);
                    bool needsRedraw = true;
                    UiMessage::show("Audio Output Set: " + audioDevices[selectedDeviceIdx].displayName, needsRedraw);
                }
            } else if (utilCol == 3) { // Pad V = Back
                currentMode = MODE_MAIN_MENU;
            }
            updateEncoderLabels();
            updatePadLeds();
        } else if (currentMode == MODE_SCATTER_PAD_SELECT) {
            if (utilCol == 0) { // Pad Z = Up
                selectedScatPad = std::clamp(selectedScatPad - 1, 0, 7);
            } else if (utilCol == 1) { // Pad X = Down
                selectedScatPad = std::clamp(selectedScatPad + 1, 0, 7);
            } else if (utilCol == 2) { // Pad C = OK (Edit pad)
                currentMode = MODE_SCATTER_PAD_EDIT;
                editFieldIndex = 0;
                syncPreviewState();
            } else if (utilCol == 3) { // Pad V = Back
                currentMode = MODE_MAIN_MENU;
                syncPreviewState();
            }
            updateEncoderLabels();
            updatePadLeds();
        } else if (currentMode == MODE_SCATTER_PAD_EDIT) {
            int maxFields = getMaxFieldsForEdit();
            if (utilCol == 0) { // Pad Z = Up
                editFieldIndex = std::clamp(editFieldIndex - 1, 0, maxFields - 1);
            } else if (utilCol == 1) { // Pad X = Down
                editFieldIndex = std::clamp(editFieldIndex + 1, 0, maxFields - 1);
            } else if (utilCol == 2) { // Pad C = Tweak / Toggle
                handleEncoder(0, 1);
            } else if (utilCol == 3) { // Pad V = Back to pad list
                currentMode = MODE_SCATTER_PAD_SELECT;
                syncPreviewState();
            }
            updateEncoderLabels();
            updatePadLeds();
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        Icon icon(d);

        if (isShuttingDown) {
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

        d.filledRect({ x, y }, { w, h }, { .color = Color { 16, 20, 28, 255 } });
        d.rect({ x, y }, { w, h }, { .color = Color { 45, 55, 75, 200 } });

        if (currentMode == MODE_MAIN_MENU) {
            d.filledRect({ x + 1, y + 1 }, { w - 2, 22 }, { .color = Color { 24, 30, 42, 255 } });
            d.line({ x, y + 23 }, { x + w, y + 23 }, { .color = Color { 45, 55, 75, 200 } });

            icon.render("&icon::menu", { x + 8, y + 5 }, 12, Color { 255, 160, 40, 255 });
            d.text({ x + 26, y + 6 }, "SYSTEM MENU", 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });

            int cardMargin = 12;
            int cardX = x + cardMargin;
            int cardY0 = y + 32;
            int cardW = w - cardMargin * 2;
            int cardH = 48;
            int cardSpacing = 8;

            // Card 0: Audio Output Device
            Color card0Bg = (selectedOption == 0) ? Color { 20, 38, 50, 240 } : Color { 22, 28, 38, 220 };
            Color card0Border = (selectedOption == 0) ? Color { 40, 200, 255, 255 } : Color { 45, 55, 75, 200 };
            d.filledRect({ cardX, cardY0 }, { cardW, cardH }, { .color = card0Bg });
            d.rect({ cardX, cardY0 }, { cardW, cardH }, { .color = card0Border });
            icon.render("&icon::audio", { cardX + 10, cardY0 + 12 }, 20, Color { 40, 200, 255, 255 });
            d.text({ cardX + 38, cardY0 + 8 }, "Audio Output Device", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            std::string activeDevStr = currentAudioDeviceName.empty() ? "default" : currentAudioDeviceName;
            d.text({ cardX + 38, cardY0 + 22 }, activeDevStr.c_str(), 8, { .color = Color { 40, 220, 255, 255 }, .font = &PoppinsLight_8 });

            // Card 1: Scatter Pads Setup
            int cardY1 = cardY0 + cardH + cardSpacing;
            Color card1Bg = (selectedOption == 1) ? Color { 40, 32, 20, 240 } : Color { 22, 28, 38, 220 };
            Color card1Border = (selectedOption == 1) ? Color { 255, 180, 40, 255 } : Color { 45, 55, 75, 200 };
            d.filledRect({ cardX, cardY1 }, { cardW, cardH }, { .color = card1Bg });
            d.rect({ cardX, cardY1 }, { cardW, cardH }, { .color = card1Border });
            icon.render("&icon::settings", { cardX + 10, cardY1 + 12 }, 20, Color { 255, 180, 40, 255 });
            d.text({ cardX + 38, cardY1 + 8 }, "Scatter & Note Repeat Setup", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            d.text({ cardX + 38, cardY1 + 22 }, "Configure SCAT1..SCAT8 modes & defaults", 8, { .color = Color { 255, 200, 140, 255 }, .font = &PoppinsLight_8 });

            // Card 2: Shutdown RPi
            int cardY2 = cardY1 + cardH + cardSpacing;
            Color card2Bg = (selectedOption == 2) ? Color { 40, 22, 25, 240 } : Color { 22, 28, 38, 220 };
            Color card2Border = confirmShutdown ? Color { 255, 60, 60, 255 } : ((selectedOption == 2) ? Color { 220, 70, 70, 255 } : Color { 45, 55, 75, 200 });
            d.filledRect({ cardX, cardY2 }, { cardW, cardH }, { .color = card2Bg });
            d.rect({ cardX, cardY2 }, { cardW, cardH }, { .color = card2Border });
            icon.render("&icon::shutdown", { cardX + 10, cardY2 + 12 }, 20, Color { 255, 80, 80, 255 });
            d.text({ cardX + 38, cardY2 + 8 }, "Power Off / Shutdown RPi", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            d.text({ cardX + 38, cardY2 + 22 }, "Safely powers down Raspberry Pi system", 8, { .color = Color { 160, 175, 195, 255 }, .font = &PoppinsLight_8 });

            if (confirmShutdown) {
                int modalW = cardW - 20;
                int modalH = 40;
                int modalX = cardX + 10;
                int modalY = cardY2 + cardH + 8;
                d.filledRect({ modalX, modalY }, { modalW, modalH }, { .color = Color { 50, 15, 18, 250 } });
                d.rect({ modalX, modalY }, { modalW, modalH }, { .color = Color { 255, 60, 60, 255 } });
                d.textCentered({ modalX + modalW / 2, modalY + 6 }, "ARE YOU SURE YOU WANT TO SHUTDOWN?", 8, { .color = Color { 255, 220, 220, 255 }, .font = &PoppinsLight_8 });
                d.textCentered({ modalX + modalW / 2, modalY + 22 }, "Press OK to confirm or Cancel", 8, { .color = Color { 255, 160, 160, 255 }, .font = &PoppinsLight_8 });
            }
        } else if (currentMode == MODE_AUDIO_SELECT) {
            d.filledRect({ x + 1, y + 1 }, { w - 2, 22 }, { .color = Color { 20, 32, 48, 255 } });
            d.line({ x, y + 23 }, { x + w, y + 23 }, { .color = Color { 40, 120, 180, 200 } });
            icon.render("&icon::audio", { x + 8, y + 5 }, 12, Color { 40, 200, 255, 255 });
            d.text({ x + 26, y + 6 }, "AUDIO OUTPUT SELECTION", 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });

            int headerH = 24;
            int hintH = 18;
            int listY = y + headerH + 4;
            int itemH = 34;
            int slotH = itemH + 4;
            int availableH = h - headerH - hintH - 8;
            int maxVisible = std::max(1, availableH / slotH);
            int totalCount = (int)audioDevices.size();
            bool showScrollBar = (totalCount > maxVisible);
            int listW = showScrollBar ? (w - 18) : (w - 8);
            int listX = x + 4;

            for (int i = 0; i < totalCount && i < maxVisible; ++i) {
                int curY = listY + i * slotH;
                bool isSelected = (i == selectedDeviceIdx);
                bool isActive = (audioDevices[i].name == currentAudioDeviceName);
                Color itemBg = isSelected ? Color { 20, 48, 68, 240 } : Color { 22, 28, 38, 220 };
                Color itemBorder = isActive ? Color { 40, 220, 140, 255 } : (isSelected ? Color { 40, 200, 255, 255 } : Color { 45, 55, 75, 200 });

                d.filledRect({ listX, curY }, { listW, itemH }, { .color = itemBg });
                d.rect({ listX, curY }, { listW, itemH }, { .color = itemBorder });
                d.text({ listX + 26, curY + 10 }, audioDevices[i].displayName.c_str(), 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
            }
        } else if (currentMode == MODE_SCATTER_PAD_SELECT) {
            d.filledRect({ x + 1, y + 1 }, { w - 2, 22 }, { .color = Color { 40, 30, 18, 255 } });
            d.line({ x, y + 23 }, { x + w, y + 23 }, { .color = Color { 255, 160, 40, 200 } });
            icon.render("&icon::settings", { x + 8, y + 5 }, 12, Color { 255, 180, 40, 255 });
            d.text({ x + 26, y + 6 }, "SCATTER & NOTE REPEAT PADS SETUP", 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });

            int listY = y + 30;
            int itemH = 34;
            int slotH = itemH + 4;
            int listW = w - 16;
            int listX = x + 8;

            for (int i = 0; i < 8; ++i) {
                int curY = listY + i * slotH;
                bool isSelected = (i == selectedScatPad);
                auto& cfg = studio.masterFx.scatPads[i];

                Color itemBg = isSelected ? Color { 45, 34, 18, 240 } : Color { 22, 28, 38, 220 };
                Color itemBorder = isSelected ? Color { 255, 180, 40, 255 } : Color { 45, 55, 75, 200 };

                d.filledRect({ listX, curY }, { listW, itemH }, { .color = itemBg });
                d.rect({ listX, curY }, { listW, itemH }, { .color = itemBorder });

                std::string padBadge = "SCAT " + std::to_string(i + 1);
                d.text({ listX + 8, curY + 10 }, padBadge.c_str(), 8, { .color = Color { 255, 200, 120, 255 }, .font = &PoppinsLight_8 });

                std::string typeStr = "Scatter";
                Color typeColor = Color { 40, 220, 255, 255 };
                std::string detailStr = "";

                if (cfg.type == SCAT_TYPE_SCATTER) {
                    typeStr = "Scatter";
                    typeColor = Color { 255, 180, 40, 255 };
                    detailStr = std::string(getScatterModeName(cfg.mode)) + " | Master: " + studio.masterFx.scatter.getParamName(cfg.mode, cfg.masterParamIdx);
                } else if (cfg.type == SCAT_TYPE_NOTE_REPEAT) {
                    typeStr = "Note Repeat";
                    typeColor = Color { 40, 220, 140, 255 };
                    detailStr = "Track " + std::to_string(cfg.trackIdx + 1) + " | Rate: 1/" + std::to_string(cfg.repeatRate);
                }

                d.text({ listX + 70, curY + 10 }, typeStr.c_str(), 8, { .color = typeColor, .font = &PoppinsLight_8 });
                d.text({ listX + 180, curY + 10 }, detailStr.c_str(), 8, { .color = Color { 220, 230, 245, 255 }, .font = &PoppinsLight_8 });
            }
        } else if (currentMode == MODE_SCATTER_PAD_EDIT) {
            d.filledRect({ x + 1, y + 1 }, { w - 2, 22 }, { .color = Color { 20, 40, 36, 255 } });
            d.line({ x, y + 23 }, { x + w, y + 23 }, { .color = Color { 40, 220, 180, 200 } });
            icon.render("&icon::settings", { x + 8, y + 5 }, 12, Color { 40, 220, 180, 255 });

            std::string titleStr = "EDIT SCAT " + std::to_string(selectedScatPad + 1) + " (LIVE PREVIEW ACTIVE)";
            d.text({ x + 26, y + 6 }, titleStr.c_str(), 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });

            auto& cfg = studio.masterFx.scatPads[selectedScatPad];
            int startY = y + 32;
            int fieldH = 34;
            int cardW = w - 16;
            int cardX = x + 8;
            int maxFields = getMaxFieldsForEdit();

            for (int f = 0; f < maxFields; ++f) {
                int curY = startY + f * (fieldH + 6);
                bool isSelected = (f == editFieldIndex);

                Color cardBg = isSelected ? Color { 18, 50, 44, 240 } : Color { 22, 28, 38, 220 };
                Color cardBorder = isSelected ? Color { 40, 220, 180, 255 } : Color { 45, 55, 75, 200 };

                d.filledRect({ cardX, curY }, { cardW, fieldH }, { .color = cardBg });
                d.rect({ cardX, curY }, { cardW, fieldH }, { .color = cardBorder });

                std::string fName = "";
                std::string fVal = "";

                if (f == 0) {
                    fName = "Pad Function Type";
                    if (cfg.type == SCAT_TYPE_SCATTER) fVal = "Scatter FX";
                    else if (cfg.type == SCAT_TYPE_NOTE_REPEAT) fVal = "Note Repeat";
                } else if (cfg.type == SCAT_TYPE_SCATTER) {
                    if (f == 1) {
                        fName = "Scatter DSP Mode";
                        fVal = getScatterModeName(cfg.mode);
                    } else if (f >= 2 && f <= 5) {
                        int pIdx = f - 2;
                        fName = std::string("Default ") + studio.masterFx.scatter.getParamName(cfg.mode, pIdx);
                        fVal = std::to_string((int)(cfg.paramValues[pIdx] * 100.0f)) + " %";
                    } else if (f == 6) {
                        fName = "Master Encoder Parameter";
                        fVal = studio.masterFx.scatter.getParamName(cfg.mode, cfg.masterParamIdx);
                    }
                } else if (cfg.type == SCAT_TYPE_NOTE_REPEAT) {
                    if (f == 1) {
                        fName = "Target Track";
                        fVal = "Track " + std::to_string(cfg.trackIdx + 1);
                    } else if (f == 2) {
                        fName = "Repeat Rate";
                        fVal = "1/" + std::to_string(cfg.repeatRate) + " step";
                    }
                }

                d.text({ cardX + 12, curY + 10 }, fName.c_str(), 8, { .color = Color { 200, 215, 235, 255 }, .font = &PoppinsLight_8 });
                d.text({ cardX + cardW - 140, curY + 10 }, fVal.c_str(), 8, { .color = Color { 40, 220, 180, 255 }, .font = &PoppinsLight_8 });
            }
        }
    }
};
