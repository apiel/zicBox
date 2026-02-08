#pragma once

#include "audio/Clock.h"
#include "audio/Sequencer.h"
#include "audio/engines/DrumKick2.h"
#include "draw/drawPrimitives.h"
#include "helpers/clamp.h"
#include "helpers/midiNote.h"
#include "stm32/platform.h"
#include <cstdint>
#include <cstdio>
#include <vector>

#ifdef IS_STM32
#include "main.h"
#endif

#define SAMPLE_RATE 44104.0f

class Core {
protected:
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
    const int TOTAL_PARAMS = 15;
    int scrollOffset = 0;

    float volume = 0.8f;
    float bpm = 140.0f;
    int stepCountIdx = 2;
    const int stepCountValues[6] = { 4, 8, 16, 32, 64, 128 };

    // Editor logic: -2: Exit, -1: Step Count, 0-127: Grid
    int editorIndex = 0;
    int selectedStep = 0;
    uint32_t currentPlayheadStep = 0;

    // 0: Selection, 1: Enabled, 2: Note, 3: Velo, 4: Prob
    int stepEditState = 0;

    bool needsRedraw = true;
    uint32_t lastBtnTime = 0;
    int encoderAccumulator = 0;

    bool isTriggered = false;
    uint32_t triggerVisualCounter = 0;

    void handleGlobalParamChange(int idx, int dir)
    {
        if (idx == 12) volume = CLAMP(volume + (dir * 0.05f), 0.0f, 1.0f);
        else if (idx == 13) {
            bpm = CLAMP(bpm + dir, 40.0f, 240.0f);
            clock.setBpm(bpm);
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
        uint32_t currentTime = HAL_GetTick();
        if (currentTime - lastBtnTime < 200) return;
        lastBtnTime = currentTime;
        encoderAccumulator = 0;
#endif
        if (currentView == View::STEP_EDITOR) {
            if (editorIndex == -2) {
                currentView = View::LIST;
                editMode = false;
            } else if (editorIndex == -1) {
                editMode = !editMode;
            } else {
                stepEditState = (stepEditState + 1) % 5;
                editMode = (stepEditState > 0);
            }
        } else {
            if (selectedParam == 14) {
                currentView = View::STEP_EDITOR;
                editorIndex = -2;
                stepEditState = 0;
                editMode = false;
            } else {
                editMode = !editMode;
            }
        }
        needsRedraw = true;
    }

    void onEncoder(int dir)
    {
        if (currentView == View::STEP_EDITOR) {
            if (!editMode && stepEditState == 0) {
#ifdef IS_STM32
                encoderAccumulator -= dir;
                if (abs(encoderAccumulator) >= 3) {
#else
                encoderAccumulator += dir;
                if (abs(encoderAccumulator) >= 1) {
#endif
                    int move = (encoderAccumulator > 0) ? 1 : -1;
                    editorIndex = CLAMP(editorIndex + move, -2, sequencer.getStepCount() - 1);
                    if (editorIndex >= 0) selectedStep = editorIndex;
                    encoderAccumulator = 0;
                    needsRedraw = true;
                }
            } else if (editMode) {
                if (editorIndex == -1) {
                    stepCountIdx = CLAMP(stepCountIdx + dir, 0, 5);
                    sequencer.setStepCount(stepCountValues[stepCountIdx]);
                } else {
                    Sequencer::Step& s = sequencer.getStep(selectedStep);
                    if (stepEditState == 1) s.enabled = (dir > 0);
                    else if (stepEditState == 2) s.notes[0] = CLAMP(s.notes[0] + dir, 0, 127);
                    else if (stepEditState == 3) s.velocity = CLAMP(s.velocity + (dir * 0.05f), 0.0f, 1.0f);
                    else if (stepEditState == 4) s.condition = CLAMP(s.condition + (dir * 0.01f), 0.0f, 1.0f);
                }
                needsRedraw = true;
            }
            return;
        }

        if (!editMode) {
            encoderAccumulator += dir;
#ifdef IS_STM32
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
            // --- HEADER BUTTONS ---
            auto drawHeaderBtn = [&](int idx, int x, const char* label, bool active) {
                Color bg = (editorIndex == idx) ? (active ? Color { 200, 0, 0 } : Color { 0, 100, 200 }) : Color { 40, 40, 40 };
                display.filledRect({ x, 2 }, { 34, 12 }, { bg });
                display.text({ x + 4, 1 }, label, 12, { { 255, 255, 255 } });
            };

            drawHeaderBtn(-2, 2, "EXIT", false);

            Sequencer::Step& s = sequencer.getStep(selectedStep);
            char buf[32];
            snprintf(buf, sizeof(buf), "STEP %d /", selectedStep + 1);
            display.textRight({ 130, 1 }, buf, 12, { { 150, 150, 150 } });

            char stepCntBuf[16];
            snprintf(stepCntBuf, sizeof(stepCntBuf), "%d", sequencer.getStepCount());
            drawHeaderBtn(-1, 130, stepCntBuf, editMode && editorIndex == -1);

            // --- GRID ---
            int stepW = 4, stepH = 6, pad = 1, startY = 17;
            for (int i = 0; i < 128; i++) {
                int col = i % 32, row = i / 32;
                int x = 2 + (col * (stepW + pad)), y = startY + (row * (stepH + pad));
                bool isAvailable = i < sequencer.getStepCount();
                Sequencer::Step& s = sequencer.getStep(i);
                Color c;
                if (!isAvailable) c = { 15, 15, 15 };
                else if (s.enabled) c = (i == (int)currentPlayheadStep ? Color({ 0, 255, 0 }) : Color({ 0x74, 0xa6, 0xd9 }));
                else c = { 80, 80, 110 };
                display.filledRect({ x, y }, { stepW - 1, stepH - 1 }, { c });
                if (i == selectedStep && editorIndex >= 0) {
                    display.rect({ x, y }, { stepW - 1, stepH - 1 }, { 255, 255, 255 });
                }
            }

            // Visual layout for params
            auto drawParam = [&](int stateIdx, int x, const char* label, Color txtColor) {
                if (stepEditState == stateIdx) display.filledRect({ x - 2, 59 }, { 30, 14 }, { { 0, 100, 200 } });
                display.text({ x, 60 }, label, 12, { txtColor });
            };

            drawParam(1, 5, s.enabled ? "ON" : "OFF", { 255, 255, 255 });
            drawParam(2, 40, MIDI_NOTES_STR[s.notes[0]], { 255, 255, 255 });

            // Velocity Triangle
            int vX = 75, vY = 72, vW = 20, vH = 10;
            if (stepEditState == 3) display.filledRect({ vX - 2, 59 }, { vW + 4, 14 }, { { 0, 100, 200 } });
            std::vector<Point> tri = { { vX, vY }, { vX + (int)(vW * s.velocity), vY }, { vX + (int)(vW * s.velocity), vY - (int)(vH * s.velocity) } };
            display.filledPolygon(tri, { { 255, 255, 255 } });

            snprintf(buf, sizeof(buf), "%d%%", (int)(s.condition * 100));
            drawParam(4, 115, buf, { 255, 255, 255 });

        } else {
            // --- STANDARD LIST VIEW ---
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
                    const char* labels[] = { "Volume", "BPM", "Step Edit" };
                    display.text({ 5, yPos }, labels[idx - 12], 12, { { 255, 255, 255 } });
                    if (idx == 12) snprintf(valBuffer, sizeof(valBuffer), "%d%%", (int)(volume * 100));
                    else if (idx == 13) snprintf(valBuffer, sizeof(valBuffer), "%d", (int)bpm);
                    else if (idx == 14) snprintf(valBuffer, sizeof(valBuffer), "->");
                }
                display.textRight({ 150, yPos }, valBuffer, 12, { { 255, 255, 255 } });
            }
            if (isTriggered) display.filledCircle({ 150, 7 }, 4, { { 0, 255, 0 } });
        }
        needsRedraw = false;
        return true;
    }
};
