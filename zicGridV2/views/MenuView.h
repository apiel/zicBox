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
private:
    int selectedOption = 0;
    static constexpr int OPTION_COUNT = 1;
    bool confirmShutdown = false;
    bool isShuttingDown = false;
    bool renderedGoodbye = false;

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

    void executeHalt()
    {
#if defined(IS_RPI)
        int exitCode = std::system("sudo halt || systemctl poweroff || halt");
        (void)exitCode;
#endif
        keep_running = false;
    }

public:
    MenuView()
        : View("SYSTEM MENU")
    {
    }

    void onActivate() override
    {
        confirmShutdown = false;
        isShuttingDown = false;
        renderedGoodbye = false;
        selectedOption = 0;
        updatePadLeds();
        updateEncoderLabels();
    }

    void onDeactivate() override
    {
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
        // Dynamic Pads matrix (rows 0..3, cols 0..7) stay completely empty
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
        // Z = Up icon
        gridState.pads[8][3].label = "&icon::arrowUp::filled";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };

        // X = Down icon
        gridState.pads[9][3].label = "&icon::arrowDown::filled";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };

        // C = OK
        gridState.pads[10][3].label = "OK";
        gridState.pads[10][3].color = confirmShutdown ? Color { 40, 220, 140, 255 } : Color { 200, 200, 200, 255 };

        // V = Cancel (visible ONLY when confirmShutdown is true)
        gridState.pads[11][3].label = confirmShutdown ? "Cancel" : "";
        gridState.pads[11][3].color = confirmShutdown ? Color { 220, 60, 60, 255 } : Color { 25, 30, 40, 255 };
    }

    void updateEncoderLabels() override
    {
        const char* optionStr = "1. Shutdown RPi";
        gridState.setEncoder(0, "MENU ACTION", (float)selectedOption, 0.0f, (float)(OPTION_COUNT - 1), 1.0f, optionStr, { 255, 100, 100, 255 });

        for (int i = 1; i < TOTAL_ENCODERS; ++i) {
            gridState.setEncoder(i, "", 0.0f, 0.0f, 1.0f, 1.0f, nullptr, { 0, 0, 0, 0 });
        }
    }

    void handleEncoder(int encoderId, int delta) override
    {
        if (isShuttingDown) return;
        if (encoderId == 0) {
            selectedOption = std::clamp(selectedOption + delta, 0, OPTION_COUNT - 1);
            updateEncoderLabels();
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        // DynamicPad stays empty for now
    }

    void handleUtilityPadPress(int utilCol, bool pressed) override
    {
        if (!pressed || isShuttingDown) return;

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
            if (!confirmShutdown) {
                confirmShutdown = true;
                updatePadLeds();
                updateEncoderLabels();
            } else {
                isShuttingDown = true;
            }
        } else if (utilCol == 3) { // Pad V = Cancel (only active during confirmShutdown)
            if (confirmShutdown) {
                confirmShutdown = false;
                updatePadLeds();
                updateEncoderLabels();
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

        // Header Title
        d.filledRect({ x + 1, y + 1 }, { w - 2, 22 }, { .color = Color { 24, 30, 42, 255 } });
        d.line({ x, y + 23 }, { x + w, y + 23 }, { .color = Color { 45, 55, 75, 200 } });

        icon.render("&icon::menu", { x + 8, y + 5 }, 12, Color { 255, 160, 40, 255 });
        d.text({ x + 26, y + 6 }, "SYSTEM MENU", 8, { .color = Color { 240, 245, 255, 255 }, .font = &PoppinsLight_8 });

        // Content Area
        int cardMargin = 12;
        int cardX = x + cardMargin;
        int cardY = y + 32;
        int cardW = w - cardMargin * 2;
        int cardH = 54;

        // Option 1 Card: Shutdown RPi
        Color cardBg = (selectedOption == 0) ? Color { 40, 22, 25, 240 } : Color { 22, 28, 38, 220 };
        Color cardBorder = confirmShutdown ? Color { 255, 60, 60, 255 } : ((selectedOption == 0) ? Color { 220, 70, 70, 255 } : Color { 45, 55, 75, 200 });

        d.filledRect({ cardX, cardY }, { cardW, cardH }, { .color = cardBg });
        d.rect({ cardX, cardY }, { cardW, cardH }, { .color = cardBorder });

        icon.render("&icon::shutdown", { cardX + 10, cardY + 14 }, 24, Color { 255, 80, 80, 255 });

        d.text({ cardX + 44, cardY + 12 }, "Power Off / Shutdown RPi", 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });
        d.text({ cardX + 44, cardY + 30 }, "Safely powers down Raspberry Pi system", 8, { .color = Color { 160, 175, 195, 255 }, .font = &PoppinsLight_8 });

        // Confirmation Modal / Banner if active
        if (confirmShutdown) {
            int modalW = cardW - 20;
            int modalH = 44;
            int modalX = cardX + 10;
            int modalY = cardY + cardH + 12;

            d.filledRect({ modalX, modalY }, { modalW, modalH }, { .color = Color { 50, 15, 18, 250 } });
            d.rect({ modalX, modalY }, { modalW, modalH }, { .color = Color { 255, 60, 60, 255 } });

            d.textCentered({ modalX + modalW / 2, modalY + 8 }, "ARE YOU SURE YOU WANT TO SHUTDOWN?", 8, { .color = Color { 255, 220, 220, 255 }, .font = &PoppinsLight_8 });
            d.textCentered({ modalX + modalW / 2, modalY + 24 }, "Press OK to confirm or Cancel", 8, { .color = Color { 255, 160, 160, 255 }, .font = &PoppinsLight_8 });
        }
    }
};
