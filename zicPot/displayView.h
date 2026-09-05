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
            case 1: return "MRPH";  // A6
            case 2: return "FMD";   // A5
            case 3: return "SNAP";  // A4
            case 4: return "SWPD";  // A11
            case 5: return "SWPS";  // A8
            case 6: return "DRV";   // A1
            case 7: return "FOLD";  // A0
            case 8: return "CRSH";  // A3
            case 9: return "RES";   // A2
            default: return "";
        }
    }

    const char* getShortItemName(int index)
    {
        switch (index) {
            case 0: return "Play";
            case 1: return "BPM";
            case 2: return "Vol";
            case 3: return "Freq";
            case 4: return "FmRat.";
            case 5: return "Click";
            case 6: return "CliDCY";
            case 7: return "Boost";
            case 8: return "Eq.Low";
            case 9: return "Eq.Mid";
            case 10: return "Eq.Hi";
            case 11: return "S.Vel";
            case 12: return "S.Ghst";
            case 13: return "S.Rmbl";
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

        // 1. Top Segmented Progress Bar for Encoder Menu Position (y = 0..2)
        if (totalMenuItems > 0) {
            int gap = 1;
            int segWidth = (32 - (totalMenuItems - 1) * gap) / totalMenuItems;
            if (segWidth < 1) segWidth = 1;

            int totalWidth = totalMenuItems * segWidth + (totalMenuItems - 1) * gap;
            int marginLeft = (32 - totalWidth) / 2;

            for (int i = 0; i < totalMenuItems; i++) {
                int x1 = marginLeft + i * (segWidth + gap);
                int x2 = x1 + segWidth - 1;

                if (i == currentMenuItem) {
                    // Active segment: solid 3px high white block
                    canvas.filledRect({ x1, 0 }, { segWidth, 3 }, true);
                } else {
                    // Inactive segment: 1px baseline dash
                    canvas.line({ x1, 2 }, { x2, 2 }, true);
                }
            }
        }

        if (potOverlayTimer > 0) {
            // Pot takeover screen overlay for 32x64 OLED
            std::string potTitle = getShortPotName(lastMovedPotIndex);

            canvas.text({ 0, 7 }, potTitle, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });
            canvas.text({ 0, 20 }, potFormattedVal, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });

            // Knob fill bar outline & fill (y = 34..37)
            canvas.rect({ 0, 34 }, { 32, 4 }, true);
            if (potValue > 0.0f) {
                canvas.filledRect({ 0, 34 }, { (int)(32.0f * potValue), 4 }, true);
            }
        } else {
            // Encoder Menu for 32x64 OLED
            std::string titleStr = getShortItemName(currentMenuItem);
            canvas.text({ 0, 7 }, titleStr, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });

            if (isEditing) {
                // Inverted white box with black text for edit mode
                canvas.filledRect({ 0, 19 }, { 32, 14 }, true);
                canvas.text({ 2, 21 }, menuItemVal, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = false });
            } else {
                canvas.text({ 0, 21 }, menuItemVal, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });
            }

            // Solid Filled Audio Waveform Representation (y = 33..42, 10px height)
            int y_center = 37;
            if (brain.isPlaying) {
                // Dynamic solid waveform contour animating across ALL 32 pixels!
                for (int x = 0; x < 32; x++) {
                    float progress = (float)x / 31.0f; // 0.0 to 1.0
                    float ampEnv = std::exp(-progress * 1.8f); // Kick envelope shape

                    float phase = (brain.currentStep * 0.7f) + (x * 0.4f);
                    float wave = std::sin(phase) * 0.7f + std::sin(phase * 2.3f) * 0.3f;
                    float mod = std::abs(wave);

                    int h = (int)(ampEnv * (0.2f + 0.8f * mod) * 4.0f);
                    h = std::clamp(h, 0, 4);

                    if (h > 0) {
                        // Draw solid vertical line from (y_center - h) to (y_center + h)
                        canvas.line({ x, y_center - h }, { x, y_center + h }, true);
                    } else {
                        // Center baseline dot
                        canvas.setPixel({ x, y_center }, true);
                    }
                }
            } else {
                // When not playing: just a clean horizontal line at y_center = 37
                canvas.line({ 0, y_center }, { 31, y_center }, true);
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
