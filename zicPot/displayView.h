#pragma once

#include "audio/engines/EngineBase.h"
#include "draw/drawMono.h"
#include "sequenceBrain.h"
#include <cstdio>
#include <cstring>
#include <string>

// Shared 32x64 OLED Display View - 1:1 Single Source of Truth for Hardware & Emulator
class DisplayView {
public:
    DrawMono<32, 64> canvas;

    const char* getShortPotName(int potIndex, int engineIdx = 0)
    {
        if (engineIdx == 1) { // PotWavKick (WavePunch Kick)
            switch (potIndex) {
                case 0: return "DUR";   // A10
                case 1: return "MRPH";  // A6
                case 2: return "SYM";   // A5
                case 3: return "BITE";  // A4
                case 4: return "SWPD";  // A11
                case 5: return "SWPS";  // A8
                case 6: return "DRV";   // A1
                case 7: return "FORM";  // A0
                case 8: return "SUBP";  // A3
                case 9: return "TONE";  // A2
                default: return "";
            }
        }
        // PotKick
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

    const char* getShortItemName(const char* name, int index)
    {
        if (index == 0) return "ENG";
        if (index == 1) return "BPM";
        if (index == 2) return "Vol";
        if (!name) return "";

        struct Map { const char* full; const char* shortName; };
        static const Map map[] = {
            { "Sub Freq", "Freq" }, { "Duration", "Dur" }, { "Click Amt", "Click" },
            { "Click Dec", "CliDCY" }, { "VCO Morph", "Mrph" }, { "Symmetry", "Sym" },
            { "Trans Bite", "Bite" }, { "Sweep Depth", "SwpD" }, { "Sweep Shp", "SwpS" },
            { "FM Depth", "FMD" }, { "FM Ratio", "FmRat" }, { "FM Snap", "Snap" },
            { "Drive", "Drv" }, { "Formant", "Form" }, { "Sub Punch", "SubP" },
            { "Tone", "Tone" }, { "Bass boost", "Boost" }, { "EQ Low", "EqLow" },
            { "EQ Mid", "EqMid" }, { "EQ High", "EqHi" }, { "Gen Velocity", "S.Vel" },
            { "Gen Ghosts", "S.Ghst" }, { "Gen Rumble", "S.Rmbl" }, { "Rpt Rate", "RptRate" },
            { "Transpose", "Trsp" }, { "PLAY / STOP", "Play" }
        };
        for (const auto& item : map) {
            if (strcmp(name, item.full) == 0) return item.shortName;
        }
        return name;
    }

    void render(SequenceBrain& brain,
                IEngine& kick,
                bool isBodyMuted,
                int currentMenuItem,
                bool isEditing,
                int totalMenuItems,
                int potOverlayTimer,
                int lastMovedPotIndex,
                float potValue,
                const char* potFormattedVal,
                const char* menuItemVal,
                const char* menuItemName,
                int engineIdx = 0)
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
            const char* potTitle = getShortPotName(lastMovedPotIndex, engineIdx);

            canvas.text({ 0, 7 }, potTitle, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });
            canvas.text({ 0, 20 }, potFormattedVal, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });

            // Knob fill bar outline & fill (y = 34..37)
            canvas.rect({ 0, 34 }, { 32, 4 }, true);
            if (potValue > 0.0f) {
                canvas.filledRect({ 0, 34 }, { (int)(32.0f * potValue), 4 }, true);
            }
        } else {
            // Encoder Menu for 32x64 OLED
            const char* itemShortName = getShortItemName(menuItemName, currentMenuItem);
            if (isBodyMuted) {
                char titleBuf[16];
                snprintf(titleBuf, sizeof(titleBuf), "%s M", itemShortName);
                canvas.text({ 0, 7 }, titleBuf, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });
            } else {
                canvas.text({ 0, 7 }, itemShortName, DrawMonoTextOptions{ .font = &PoppinsLight_8, .color = true });
            }

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

                    float phase = (brain.midiTickCounter * 0.15f) + (x * 0.4f);
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

            bool active = brain.kickSequence[s].active;
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
