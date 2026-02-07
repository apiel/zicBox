#pragma once

#include "audio/engines/DrumKick2.h"
#include "draw/drawPrimitives.h"
#include "stm32/platform.h"
#include "helpers/clamp.h"
#include <cstdint>
#include <iomanip>
#include <sstream>

#ifdef IS_STM32
#include "main.h"
#endif

#define BPM 140.0f
#define SAMPLE_RATE 44104.0f

const uint32_t SAMPLES_PER_BEAT = (uint32_t)(SAMPLE_RATE * 60.0f / BPM);
const uint32_t SAMPLES_HALF_BEAT = SAMPLES_PER_BEAT / 2;

DrumKick2 kick(SAMPLE_RATE);

class Core {
protected:
    DrawPrimitives& display;

    int selectedParam = 0;
    bool editMode = false;
    const int VISIBLE_ROWS = 4;
    const int TOTAL_PARAMS = 12;
    int scrollOffset = 0;

    // Redraw optimization
    bool needsRedraw = true;
    bool lastWasBlinking = false;

public:
    Core(DrawPrimitives& display)
        : display(display)
    {
    }

    void encBtn()
    {
        editMode = !editMode;
        needsRedraw = true;
    }

    void onEncoder(int dir)
    {
        if (!editMode) {
            selectedParam = CLAMP((selectedParam + (-dir)), 0, TOTAL_PARAMS - 1);
            int targetOffset = selectedParam - (VISIBLE_ROWS / 2);
            scrollOffset = CLAMP(targetOffset, 0, TOTAL_PARAMS - VISIBLE_ROWS);
        } else {
            Param& p = kick.params[selectedParam];
            p.value += dir * p.step;
            if (p.value < p.min) p.value = p.min;
            if (p.value > p.max) p.value = p.max;
        }
        needsRedraw = true;
    }

    bool render()
    {
        bool isBlinking = (sampleCounter < SAMPLES_HALF_BEAT);
        if (isBlinking != lastWasBlinking) {
            needsRedraw = true;
            lastWasBlinking = isBlinking;
        }

        if (!needsRedraw) return false;

        display.filledRect({ 0, 0 }, { 160, 80 }, { { 0, 0, 0 } });

        // Header
        display.filledRect({ 0, 0 }, { 160, 15 }, { { 40, 40, 40 } });
        display.text({ 5, 2 }, kick.name, 12, { { 255, 255, 255 } });

        // --- Scroll Bar ---
        int trackHeight = 60;
        int barHeight = (VISIBLE_ROWS * trackHeight) / TOTAL_PARAMS;
        int barY = 18 + (scrollOffset * trackHeight) / TOTAL_PARAMS;
        
        display.filledRect({ 157, 18 }, { 2, trackHeight }, { { 30, 30, 30 } }); 
        display.filledRect({ 157, barY }, { 2, barHeight }, { { 150, 150, 150 } });

        // Parameter List
        for (int i = 0; i < VISIBLE_ROWS; i++) {
            int idx = i + scrollOffset;
            if (idx >= TOTAL_PARAMS) break;

            int yPos = 18 + (i * 15);
            Param& p = kick.params[idx];

            if (idx == selectedParam) {
                Color bg = editMode ? Color { 200, 0, 0 } : Color { 0, 100, 200 };
                display.filledRect({ 2, yPos - 1 }, { 153, 14 }, { bg });
            }

            display.text({ 5, yPos }, p.label, 12, { { 255, 255, 255 } });

            // --- Floating Point Formatting ---
            std::string valStr;
            if (p.precision <= 0) {
                valStr = std::to_string((int)p.value);
            } else {
                // Formatting to the specific precision stored in the Param
                std::stringstream ss;
                ss << std::fixed << std::setprecision(p.precision) << p.value;
                valStr = ss.str();
            }

            if (p.unit) valStr += p.unit;
            display.textRight({ 150, yPos }, valStr, 12, { { 255, 255, 255 } });
        }

        if (isBlinking) {
            display.filledCircle({ 150, 7 }, 4, { { 0, 255, 0 } });
        }

        needsRedraw = false;
        return true;
    }

protected:
    uint32_t sampleCounter = 0;

public:
    float sample()
    {
        if (sampleCounter >= SAMPLES_PER_BEAT) {
            kick.noteOn(60, 1.0f);
            sampleCounter = 0;
#ifdef IS_STM32
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
#endif
        } else if (sampleCounter == SAMPLES_HALF_BEAT) {
#ifdef IS_STM32
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
#endif
        }
        sampleCounter++;
        return kick.sample();
    }
};