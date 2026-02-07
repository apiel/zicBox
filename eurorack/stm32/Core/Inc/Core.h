#pragma once

#include <cstdint>

#include "eurorack/stm32/Core/Inc/kick.hpp"
#include "draw/drawPrimitives.h"
#include "stm32/platform.h"

#ifdef IS_STM32
#include "main.h"
#endif

// BPM Configuration
#define BPM 140.0f
#define SAMPLE_RATE 44104.0f

const uint32_t SAMPLES_PER_BEAT = (uint32_t)(SAMPLE_RATE * 60.0f / BPM);

class Core {
protected:
    DrawPrimitives& display;

public:
    Core(DrawPrimitives& display)
        : display(display)
    {
    }

protected:
    int valueToEdit = 0;

public:
    void encBtn()
    {
        valueToEdit++;
    }

protected:
    int x = 80;
    int y = 20;
    int16_t lastX = -1;
    int16_t lastY = -1; // Add as global

public:
    bool render()
    {
        if (y == lastY && x == lastX) return false; // Skip if no change

        // Erase old rect
        display.filledRect({ lastX, lastY }, { 20, 20 }, { { 0, 0, 0 } });
        // Draw new rect
        display.rect({ x, y }, { 20, 20 }, { { 0, 255, 0 } });
        display.filledRect({ x + 5, y + 5 }, { 10, 10 }, { { 255, 255, 255, 200 } });
        // display.filledRect({ x + 5, y + 5 }, { 10, 10 }, { { 255, 0, 255, 125 } });

        display.filledRect({ 10, 10 }, { 80, 20 }, { { 0, 0, 0 } });
        display.text({ 10, 10 }, "x: " + std::to_string(x) + " y: " + std::to_string(y), 12, { { 255, 255, 255 } });

        lastX = x;
        lastY = y;

        return true;
    }

    void onEncoder(int dir)
    {
        if (valueToEdit % 2 == 0) x += dir;
        if (valueToEdit % 2 == 1) y += dir;
    }

protected:
    uint32_t sampleCounter = 0;
    bool kickActive = false;
    uint32_t kickSampleCounter = 0;

public:
    float sample()
    {
        // Check if it's time for a new kick
        if (sampleCounter >= SAMPLES_PER_BEAT) {
            kick.trigger();
            kickActive = true;
            kickSampleCounter = 0;
            sampleCounter = 0;

#ifdef IS_STM32
            // LED ON at kick start
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
#endif
        }

        float out = 0.0f;
        // Generate audio sample
        if (kickActive) {
            out = kick.process(1.0f);
            kickSampleCounter++;

            if (kick.pitchEnv < 0.0001f || kickSampleCounter > 8000) {
                kickActive = false;
#ifdef IS_STM32
                HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
#endif
            }
        }
        sampleCounter++;

        return out;
    }
};