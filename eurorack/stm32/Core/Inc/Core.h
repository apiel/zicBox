#pragma once

#include "audio/Clock.h"
#include "audio/Sequencer.h"
#include "audio/engines/DrumKick2.h"
#include "draw/drawPrimitives.h"
#include "helpers/clamp.h"
#include "stm32/platform.h"
#include <cstdint>
#include <cstdio>

#ifdef IS_STM32
#include "main.h"
#endif

#define SAMPLE_RATE 44104.0f

class Core {
protected:
    DrawPrimitives& display;
    DrumKick2 kick;
    Clock clock;
    Sequencer sequencer;

    int selectedParam = 0;
    bool editMode = false;
    const int VISIBLE_ROWS = 4;
    const int TOTAL_PARAMS = 12;
    int scrollOffset = 0;

    bool needsRedraw = true;
    uint32_t lastBtnTime = 0;
    int encoderAccumulator = 0;

    bool isTriggered = false;
    uint32_t triggerVisualCounter = 0;

public:
    Core(DrawPrimitives& display)
        : display(display)
        , kick(SAMPLE_RATE)
        , clock(SAMPLE_RATE)
    {
        clock.setBpm(140.0f);
        sequencer.setStepCount(16);
        for (int i = 0; i < 16; i++) {
            if (i % 4 == 0) {
                sequencer.getStep(i).enabled = true;
                sequencer.getStep(i).len = 1;
            }
        }
    }

    void encBtn()
    {
#ifdef IS_STM32
        // Need to debounce only on stm32
        uint32_t currentTime = HAL_GetTick();
        if (currentTime - lastBtnTime < 200) return;
        lastBtnTime = currentTime;
        encoderAccumulator = 0;
#endif
        editMode = !editMode;
        needsRedraw = true;
    }

    void onEncoder(int dir)
    {
        if (!editMode) {
            encoderAccumulator += dir;
#ifdef IS_STM32
            // stm32 encoder is so sensitive, we slow it down but changing value only every 3 pulse
            if (abs(encoderAccumulator) >= 3) {
#else
            if (abs(encoderAccumulator) >= 1) {
#endif
                int move = (encoderAccumulator > 0) ? -1 : 1;
                selectedParam = CLAMP((selectedParam + move), 0, TOTAL_PARAMS - 1);
                int targetOffset = selectedParam - (VISIBLE_ROWS / 2);
                scrollOffset = CLAMP(targetOffset, 0, TOTAL_PARAMS - VISIBLE_ROWS);
                encoderAccumulator = 0;
                needsRedraw = true;
            }
        } else {
            Param& p = kick.params[selectedParam];
            p.value += dir * p.step;
            p.value = CLAMP(p.value, p.min, p.max);
            needsRedraw = true;
        }
    }

    float sample()
    {
        uint32_t tick = clock.getClock();
        if (tick > 0 && tick % 6 == 0) {
            sequencer.onStep([&](const Sequencer::Event& ev) {
                if (ev.type == Sequencer::EventType::NoteOn) {
                    kick.noteOn(ev.stepRef->notes[0], ev.stepRef->velocity);
                    isTriggered = true;
                    triggerVisualCounter = 2000;
                    needsRedraw = true;
#ifdef IS_STM32
                    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
#endif
                } else {
#ifdef IS_STM32
                    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
#endif
                }
            });
        }

        if (triggerVisualCounter > 0 && --triggerVisualCounter == 0) {
            isTriggered = false;
            needsRedraw = true;
        }
        return kick.sample();
    }

    bool render()
    {
        if (!needsRedraw) return false;

        display.filledRect({ 0, 0 }, { 160, 80 }, { { 0, 0, 0 } });
        display.filledRect({ 0, 0 }, { 160, 15 }, { { 40, 40, 40 } });
        display.text({ 5, 2 }, kick.name, 12, { { 255, 255, 255 } });

        int trackHeight = 60;
        int barHeight = (VISIBLE_ROWS * trackHeight) / TOTAL_PARAMS;
        int barY = 18 + (scrollOffset * trackHeight) / TOTAL_PARAMS;
        display.filledRect({ 157, 18 }, { 2, trackHeight }, { { 30, 30, 30 } });
        display.filledRect({ 157, barY }, { 2, barHeight }, { { 150, 150, 150 } });

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

            char valBuffer[24];
            if (p.precision <= 0) snprintf(valBuffer, sizeof(valBuffer), "%d%s", (int)p.value, p.unit ? p.unit : "");
            else snprintf(valBuffer, sizeof(valBuffer), "%.*f%s", (int)p.precision, (double)p.value, p.unit ? p.unit : "");
            display.textRight({ 150, yPos }, valBuffer, 12, { { 255, 255, 255 } });
        }

        if (isTriggered) display.filledCircle({ 150, 7 }, 4, { { 0, 255, 0 } });

        needsRedraw = false;
        return true;
    }
};