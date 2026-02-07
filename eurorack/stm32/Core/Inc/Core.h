#pragma once

#include <cstdint>

#include "audio/engines/DrumKick2.h"
#include "draw/drawPrimitives.h"
#include "stm32/platform.h"

#ifdef IS_STM32
#include "main.h"
#endif

// BPM Configuration
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
    const int VISIBLE_ROWS = 4; // How many params to show at once
    int scrollOffset = 0;

public:
    Core(DrawPrimitives& display)
        : display(display)
    {
    }

    void encBtn()
    {
        editMode = !editMode;
    }

    bool render()
    {
        display.filledRect({ 0, 0 }, { 160, 80 }, { { 0, 0, 0 } });

        display.filledRect({ 0, 0 }, { 160, 15 }, { { 40, 40, 40 } });
        display.text({ 5, 2 }, "KICK 2 CONFIG", 12, { { 255, 255, 255 } });

        for (int i = 0; i < VISIBLE_ROWS; i++) {
            int idx = i + scrollOffset;
            if (idx >= 12) break;

            int yPos = 18 + (i * 15);
            Param& p = kick.params[idx];

            if (idx == selectedParam) {
                Color bg = editMode ? Color { 200, 0, 0 } : Color { 0, 100, 200 };
                display.filledRect({ 2, yPos - 1 }, { 156, 14 }, { bg });
            }

            display.text({ 5, yPos }, p.label, 12, { { 255, 255, 255 } });

            std::string valStr = std::to_string((int)p.value) + (p.unit ? p.unit : "");
            display.textRight({ 155, yPos }, valStr, 12, { { 255, 255, 255 } });
        }

        if (sampleCounter < SAMPLES_HALF_BEAT) {
            display.filledCircle({ 150, 7 }, 4, { { 0, 255, 0 } }); // Blink LED on UI
        }

        return true;
    }

    void onEncoder(int dir)
    {
        if (!editMode) {
            selectedParam = (selectedParam + (-dir) + 12) % 12;
            if (selectedParam < scrollOffset) scrollOffset = selectedParam;
            if (selectedParam >= scrollOffset + VISIBLE_ROWS) scrollOffset = selectedParam - VISIBLE_ROWS + 1;
        } else {
            Param& p = kick.params[selectedParam];
            p.value += dir * p.step;
            if (p.value < p.min) p.value = p.min;
            if (p.value > p.max) p.value = p.max;
        }
    }

protected:
    uint32_t sampleCounter = 0;

public:
    float sample()
    {
        // Check if it's time for a new kick
        if (sampleCounter >= SAMPLES_PER_BEAT) {
            kick.noteOn(60, 1.0f);
            sampleCounter = 0;

#ifdef IS_STM32
            // LED ON at kick start
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