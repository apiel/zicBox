#pragma once

#include "audio/engines/PotKick.h"
#include "draw/drawMono.h"
#include "sequenceBrain.h"
#include <cstdio>
#include <string>

// Shared 32x64 OLED Display View - 1:1 Single Source of Truth for Hardware & Emulator
class DisplayView {
public:
    DrawMono<32, 64> canvas;

    const char* getShortPotName(int potIndex)
    {
        switch (potIndex) {
            case 0: return "DUR";   // A10
            case 1: return "MRPH";  // A4
            case 2: return "FMD";   // A5
            case 3: return "SNAP";  // A11
            case 4: return "SWPD";  // A2
            case 5: return "SWPS";  // A8
            case 6: return "DRV";   // A6
            case 7: return "Boost";  // A1
            case 8: return "EQL";   // A3
            case 9: return "EQM";   // A0
            default: return "";
        }
    }

    const char* getShortItemName(int index)
    {
        switch (index) {
            case 0: return "PLAY";
            case 1: return "BPM";
            case 2: return "VOL";
            case 3: return "SUB";
            case 4: return "EQH";
            case 5: return "FMR";
            case 6: return "CLCK";
            case 7: return "CDCY";
            case 8: return "VEL";
            case 9: return "GHST";
            case 10: return "RMBL";
            default: return "";
        }
    }

    void render(SequenceBrain& brain,
                PotKick& kick,
                int currentMenuItem,
                bool isEditing,
                int totalMenuItems,
                int potOverlayTimer,
                int lastMovedPotIndex,
                float potValue,
                const std::string& potFormattedVal,
                const std::string& menuItemVal)
    {
        canvas.clear();

        // Top Header Line inside 32x64 portrait screen (y = 0..10)
        char headerBuf[16];
        snprintf(headerBuf, sizeof(headerBuf), "%d/%d", currentMenuItem + 1, totalMenuItems);
        canvas.text({ 0, 0 }, std::string(headerBuf), DrawMonoTextOptions{ .font = &PoppinsLight_6, .color = true });
        canvas.text({ 24, 0 }, brain.isPlaying ? ">" : "||", DrawMonoTextOptions{ .font = &PoppinsLight_6, .color = true });

        // 1px divider line at y = 10
        canvas.line({ 0, 10 }, { 31, 10 }, true);

        if (potOverlayTimer > 0) {
            // Pot takeover screen overlay for 32x64 OLED
            std::string potTitle = getShortPotName(lastMovedPotIndex);

            canvas.text({ 0, 14 }, potTitle, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });
            canvas.text({ 0, 26 }, potFormattedVal, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });

            // Knob fill bar outline & fill (y = 40..46)
            canvas.rect({ 0, 38 }, { 32, 3 }, true);
            if (potValue > 0.0f) {
                canvas.filledRect({ 0, 38 }, { (int)(32.0f * potValue), 3 }, true);
            }
        } else {
            // Encoder Menu for 32x64 OLED
            std::string titleStr = getShortItemName(currentMenuItem);
            canvas.text({ 0, 14 }, titleStr, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });

            if (isEditing) {
                // Inverted white box with black text for edit mode
                canvas.filledRect({ 0, 25 }, { 32, 14 }, true);
                canvas.text({ 2, 27 }, menuItemVal, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = false });
            } else {
                canvas.text({ 0, 27 }, menuItemVal, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });
            }
        }

        // 64-step bar across bottom of 32x64 screen (4 rows of 16 steps)
        for (int s = 0; s < 64; s++) {
            int r = s / 16;
            int c = s % 16;
            int x = c * 2;
            int y_base = 44 + r * 5;

            bool active = (s < (int)brain.kickSequence.size()) && brain.kickSequence[s].active;
            bool isCurrent = brain.isPlaying && (brain.currentStep == s);

            if (active) {
                canvas.line({ x, y_base + 1 }, { x, y_base + 3 }, true);
            } else {
                canvas.setPixel({ x, y_base + 3 }, true);
            }
            if (isCurrent) {
                canvas.setPixel({ x, y_base }, true); // Playhead cursor dot at top of step
            }
        }
    }
};
