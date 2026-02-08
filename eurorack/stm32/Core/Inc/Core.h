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
    // View state management
    enum class View { LIST,
        STEP_EDITOR };
    View currentView = View::LIST;

    DrawPrimitives& display;
    DrumKick2 kick;
    Clock clock;
    Sequencer sequencer;

    int selectedParam = 0;
    bool editMode = false;
    const int VISIBLE_ROWS = 4;
    // 12 Engine params + 4 Global params (Volume, BPM, Length, Edit)
    const int TOTAL_PARAMS = 16;
    int scrollOffset = 0;

    // Global Params
    float volume = 0.8f;
    float bpm = 140.0f;
    int stepCountIdx = 2; // Index for [4, 8, 16, 32, 64]
    const int stepCountValues[6] = { 4, 8, 16, 32, 64, 128 };

    // Step Editor Logic
    int selectedStep = 0;
    int stepEditSubParam = 0; // 0: Enable, 1: Note, 2: Velo, 3: Prob
    uint32_t currentPlayheadStep = 0;

    bool needsRedraw = true;
    uint32_t lastBtnTime = 0;
    int encoderAccumulator = 0;

    bool isTriggered = false;
    uint32_t triggerVisualCounter = 0;

    // Helper for global values
    void handleGlobalParamChange(int idx, int dir)
    {
        if (idx == 12) volume = CLAMP(volume + (dir * 0.05f), 0.0f, 1.0f);
        else if (idx == 13) {
            bpm = CLAMP(bpm + dir, 40.0f, 240.0f);
            clock.setBpm(bpm);
        } else if (idx == 14) {
            stepCountIdx = CLAMP(stepCountIdx + dir, 0, 5);
            sequencer.setStepCount(stepCountValues[stepCountIdx]);
        }
    }

public:
    Core(DrawPrimitives& display)
        : display(display)
        , kick(SAMPLE_RATE)
        , clock(SAMPLE_RATE)
    {
        clock.setBpm(bpm);
        sequencer.setStepCount(stepCountValues[stepCountIdx]);
        for (int i = 0; i < 128; i++) {
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
        if (currentView == View::STEP_EDITOR) {
            // Toggle focus between selecting step and editing its values
            editMode = !editMode;
        } else {
            if (selectedParam == 15) { // Step Edit entry
                currentView = View::STEP_EDITOR;
                editMode = false; // Start with step selection
            } else {
                editMode = !editMode;
            }
        }
        needsRedraw = true;
    }

    void onEncoder(int dir)
    {
        if (currentView == View::STEP_EDITOR) {
            if (!editMode) {
                // Navigate 128 steps
                selectedStep = CLAMP(selectedStep + dir, 0, 127);
            } else {
                // Edit parameters of the selected step
                Sequencer::Step& s = sequencer.getStep(selectedStep);
                // For now, let's just toggle enabled or change velocity as a test
                s.enabled = (dir > 0);
            }
            needsRedraw = true;
            return;
        }

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
            if (selectedParam < 12) {
                Param& p = kick.params[selectedParam];
                p.value += dir * p.step;
                p.value = CLAMP(p.value, p.min, p.max);
            } else {
                handleGlobalParamChange(selectedParam, dir);
            }
            needsRedraw = true;
        }
    }

    float sample()
    {
        uint32_t tick = clock.getClock();
        if (tick > 0 && tick % 6 == 0) {
            sequencer.onStep([&](const Sequencer::Event& ev) {
                if (ev.type == Sequencer::EventType::NoteOn) {
                    currentPlayheadStep = (tick / 6) % sequencer.getStepCount();
                    kick.noteOn(ev.stepRef->notes[0], ev.stepRef->velocity * volume);
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

        if (currentView == View::STEP_EDITOR) {
            // --- TOP HALF: 128 STEP GRID ---
            int stepW = 4;
            int stepH = 6;
            int pad = 1;
            int maxSteps = sequencer.getStepCount();

            for (int i = 0; i < 128; i++) {
                int col = i % 32;
                int row = i / 32;
                int x = 2 + (col * (stepW + pad));
                int y = 2 + (row * (stepH + pad));

                bool isAvailable = i < maxSteps;
                Sequencer::Step& s = sequencer.getStep(i);

                Color c;
                if (!isAvailable) c = { 20, 20, 20 }; // Grayed out
                else if (s.enabled) c = (i == (int)currentPlayheadStep ? Color({ 0, 255, 0 }) : Color({ 0x74, 0xa6, 0xd9 })); // Active
                else c = { 60, 60, 60 }; // Inactive but available

                display.filledRect({ x, y }, { stepW - 1, stepH - 1 }, { c });

                // Cursor
                if (i == selectedStep) {
                    display.rect({ x, y }, { stepW - 1, stepH - 1 }, { 255, 255, 255 });
                }
            }

            Sequencer::Step& s = sequencer.getStep(selectedStep);

            char info[32];
            snprintf(info, sizeof(info), "STEP %d %s", selectedStep + 1, editMode ? "[EDIT]" : "");
            display.text({ 5, 36 }, info, 12, { 255, 255, 255 });

            snprintf(info, sizeof(info), "Velo: %d%%  Prob: %d%%", (int)(s.velocity * 100), (int)(s.condition * 100));
            display.text({ 5, 52 }, info, 12, { 180, 180, 180 });

            display.text({ 5, 66 }, s.enabled ? "STATUS: ENABLED" : "STATUS: MUTED", 12, { s.enabled ? Color { 0, 255, 0 } : Color { 255, 0, 0 } });

        } else {
            // --- STANDARD LIST VIEW (Original Code) ---
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

                if (idx == selectedParam) {
                    Color bg = editMode ? Color { 200, 0, 0 } : Color { 0, 100, 200 };
                    display.filledRect({ 2, yPos - 1 }, { 153, 14 }, { bg });
                }

                char valBuffer[24] = "";
                if (idx < 12) {
                    Param& p = kick.params[idx];
                    display.text({ 5, yPos }, p.label, 12, { { 255, 255, 255 } });
                    if (p.precision <= 0) snprintf(valBuffer, sizeof(valBuffer), "%d%s", (int)p.value, p.unit ? p.unit : "");
                    else snprintf(valBuffer, sizeof(valBuffer), "%.*f%s", (int)p.precision, (double)p.value, p.unit ? p.unit : "");
                } else {
                    const char* labels[] = { "Volume", "BPM", "Steps", "Step Edit" };
                    display.text({ 5, yPos }, labels[idx - 12], 12, { { 255, 255, 255 } });
                    if (idx == 12) snprintf(valBuffer, sizeof(valBuffer), "%d%%", (int)(volume * 100));
                    else if (idx == 13) snprintf(valBuffer, sizeof(valBuffer), "%d", (int)bpm);
                    else if (idx == 14) snprintf(valBuffer, sizeof(valBuffer), "%d", stepCountValues[stepCountIdx]);
                    else if (idx == 15) snprintf(valBuffer, sizeof(valBuffer), "->");
                }
                display.textRight({ 150, yPos }, valBuffer, 12, { { 255, 255, 255 } });
            }
            if (isTriggered) display.filledCircle({ 150, 7 }, 4, { { 0, 255, 0 } });
        }

        needsRedraw = false;
        return true;
    }
};