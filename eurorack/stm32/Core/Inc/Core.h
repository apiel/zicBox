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

class IView {
public:
    virtual ~IView() { }
    virtual void onEncoder(int dir) = 0;
    virtual void onButton() = 0;
    virtual void render(DrawPrimitives& display) = 0;
};

class MainListView : public IView {
    DrumKick2& kick;
    Clock& clock;
    float& volume;
    float& bpm;
    bool& isMuted;
    bool& needsRedraw;

    int selectedParam = 0;
    bool editMode = false;
    int scrollOffset = 0;
    const int VISIBLE_ROWS = 4;
    const int TOTAL_PARAMS = 16;
    int encoderAccumulator = 0;

    std::function<void()> onOpenEditor;

public:
    MainListView(DrumKick2& k, Clock& clk, float& v, float& b, bool& m, bool& redraw, std::function<void()> openEdit)
        : kick(k)
        , clock(clk)
        , volume(v)
        , bpm(b)
        , isMuted(m)
        , needsRedraw(redraw)
        , onOpenEditor(openEdit)
    {
    }

    void onButton() override
    {
        if (selectedParam == 15) {
            onOpenEditor();
        } else if (selectedParam == 0) {
            isMuted = !isMuted;
            needsRedraw = true;
        } else {
            editMode = !editMode;
        }
    }

    void onEncoder(int dir) override
    {
        if (!editMode) {
            encoderAccumulator += dir;
#ifdef IS_STM32
            if (abs(encoderAccumulator) >= 3) {
#else
            if (abs(encoderAccumulator) >= 1) {
#endif
                int move = (encoderAccumulator > 0) ? -1 : 1;
                selectedParam = CLAMP((selectedParam + move), 0, TOTAL_PARAMS - 1);
                scrollOffset = CLAMP(selectedParam - (VISIBLE_ROWS / 2), 0, TOTAL_PARAMS - VISIBLE_ROWS);
                encoderAccumulator = 0;
                needsRedraw = true;
            }
        } else {
#ifdef IS_STM32
            dir = -dir;
#endif
            if (selectedParam == 0) {
                isMuted = (dir > 0);
            } else if (selectedParam > 0 && selectedParam < 13) {
                Param& p = kick.params[selectedParam - 1];
                p.set(p.value + (dir * p.step));
            } else if (selectedParam == 13) {
                volume = CLAMP(volume + (dir * 0.05f), 0.0f, 1.0f);
            } else if (selectedParam == 14) {
                bpm = CLAMP(bpm + dir, 40.0f, 240.0f);
                clock.setBpm(bpm);
            }
            needsRedraw = true;
        }
    }

    void render(DrawPrimitives& display) override
    {
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

            if (idx == 0) {
                display.text({ 5, yPos }, "Mute", 12, { { 255, 255, 255 } });
                snprintf(valBuffer, sizeof(valBuffer), isMuted ? "Muted" : "playing");
            } else if (idx > 0 && idx < 13) {
                Param& p = kick.params[idx - 1];
                display.text({ 5, yPos }, p.label, 12, { { 255, 255, 255 } });
                if (p.precision <= 0) snprintf(valBuffer, sizeof(valBuffer), "%d%s", (int)p.value, p.unit ? p.unit : "");
                else snprintf(valBuffer, sizeof(valBuffer), "%.*f%s", (int)p.precision, (double)p.value, p.unit ? p.unit : "");
            } else {
                const char* labels[] = { "Volume", "BPM", "Step Edit" };
                display.text({ 5, yPos }, labels[idx - 13], 12, { { 255, 255, 255 } });
                if (idx == 13) snprintf(valBuffer, sizeof(valBuffer), "%d%%", (int)(volume * 100));
                else if (idx == 14) snprintf(valBuffer, sizeof(valBuffer), "%d", (int)bpm);
                else snprintf(valBuffer, sizeof(valBuffer), "->");
            }
            display.textRight({ 150, yPos }, valBuffer, 12, { { 255, 255, 255 } });
        }
    }
};

// --- STEP EDITOR VIEW ---
class StepEditView : public IView {
    Sequencer& sequencer;
    bool& needsRedraw;
    uint32_t& playhead;
    int& stepCountIdx;
    const int* stepCountValues;

    int editorIndex = -2;
    int selectedStep = 0;
    int stepEditState = 0;
    bool editMode = false;
    int encoderAccumulator = 0;

    std::function<void()> onExit;

public:
    StepEditView(Sequencer& seq, bool& redraw, uint32_t& ph, int& scIdx, const int* scVals, std::function<void()> exitCb)
        : sequencer(seq)
        , needsRedraw(redraw)
        , playhead(ph)
        , stepCountIdx(scIdx)
        , stepCountValues(scVals)
        , onExit(exitCb)
    {
    }

    void reset()
    {
        editorIndex = -2;
        editMode = false;
        stepEditState = 0;
    }

    void onButton() override
    {
        if (editorIndex == -2) {
            onExit();
        } else if (editorIndex == -1) {
            editMode = !editMode;
        } else {
            stepEditState = (stepEditState + 1) % 5;
            editMode = (stepEditState > 0);
        }
    }

    void onEncoder(int dir) override
    {
        encoderAccumulator += dir;
#ifdef IS_STM32
        if (abs(encoderAccumulator) >= 3) {
#else
        if (abs(encoderAccumulator) >= 1) {
#endif
            if (!editMode && stepEditState == 0) {
#ifdef IS_STM32
                int move = (encoderAccumulator > 0) ? -1 : 1;
#else
                int move = (encoderAccumulator > 0) ? 1 : -1;
#endif
                editorIndex = CLAMP(editorIndex + move, -2, sequencer.getStepCount() - 1);
                if (editorIndex >= 0) selectedStep = editorIndex;
            } else if (editMode) {
#ifdef IS_STM32
                dir = -dir;
#endif
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
            }
            encoderAccumulator = 0;
            needsRedraw = true;
        }
    }

    void render(DrawPrimitives& display) override
    {
        auto drawHeaderBtn = [&](int idx, int x, const char* label, bool active) {
            Color bg = (editorIndex == idx) ? (active ? Color { 200, 0, 0 } : Color { 0, 100, 200 }) : Color { 40, 40, 40 };
            display.filledRect({ x, 2 }, { 34, 12 }, { bg });
            display.text({ x + 4, 1 }, label, 12, { { 255, 255, 255 } });
        };

        drawHeaderBtn(-2, 2, "EXIT", false);
        char buf[32];
        snprintf(buf, sizeof(buf), "STEP %d /", selectedStep + 1);
        display.textRight({ 130, 1 }, buf, 12, { { 150, 150, 150 } });
        snprintf(buf, sizeof(buf), "%d", sequencer.getStepCount());
        drawHeaderBtn(-1, 130, buf, editMode && editorIndex == -1);

        int stepW = 4, stepH = 6, pad = 1;
        for (int i = 0; i < 128; i++) {
            int col = i % 32, row = i / 32;
            int x = 2 + (col * (stepW + pad)), y = 17 + (row * (stepH + pad));
            bool isAvail = i < sequencer.getStepCount();
            Sequencer::Step& s = sequencer.getStep(i);
            Color c = !isAvail ? Color { 15, 15, 15 } : (s.enabled ? (i == (int)playhead ? Color { 0, 255, 0 } : Color { 0x74, 0xa6, 0xd9 }) : Color { 80, 80, 110 });
            display.filledRect({ x, y }, { stepW - 1, stepH - 1 }, { c });
            if (i == selectedStep && editorIndex >= 0) display.rect({ x, y }, { stepW - 1, stepH - 1 }, { 255, 255, 255 });
        }

        Sequencer::Step& s = sequencer.getStep(selectedStep);
        auto drawParam = [&](int stateIdx, int x, const char* label) {
            if (stepEditState == stateIdx) display.filledRect({ x - 2, 59 }, { 30, 14 }, { { 0, 100, 200 } });
            display.text({ x, 60 }, label, 12, { { 255, 255, 255 } });
        };
        drawParam(1, 5, s.enabled ? "ON" : "OFF");
        drawParam(2, 40, MIDI_NOTES_STR[s.notes[0]]);

        if (stepEditState == 3) display.filledRect({ 73, 59 }, { 24, 14 }, { { 0, 100, 200 } });
        std::vector<Point> tri = { { 75, 72 }, { 75 + (int)(20 * s.velocity), 72 }, { 75 + (int)(20 * s.velocity), 72 - (int)(10 * s.velocity) } };
        display.filledPolygon(tri, { { 255, 255, 255 } });

        snprintf(buf, sizeof(buf), "%d%%", (int)(s.condition * 100));
        drawParam(4, 115, buf);
    }
};

class Core {
    DrawPrimitives& display;
    DrumKick2 kick;
    Clock clock;
    Sequencer sequencer;

    float volume = 0.8f;
    float bpm = 140.0f;
    bool isMuted = false;
    int stepCountIdx = 2;
    const int stepCountValues[6] = { 4, 8, 16, 32, 64, 128 };
    uint32_t currentPlayheadStep = 0;
    bool needsRedraw = true;

    MainListView mainView;
    StepEditView stepEditView;
    IView* currentView;

    bool isTriggered = false;
    uint32_t triggerVisualCounter = 0;
    uint32_t lastBtnTime = 0;

public:
    Core(DrawPrimitives& display)
        : display(display)
        , kick(SAMPLE_RATE)
        , clock(SAMPLE_RATE)
        , mainView(kick, clock, volume, bpm, isMuted, needsRedraw, [this]() { stepEditView.reset(); currentView = &stepEditView; needsRedraw = true; })
        , stepEditView(sequencer, needsRedraw, currentPlayheadStep, stepCountIdx, stepCountValues, [this]() { currentView = &mainView; needsRedraw = true; })
    {
        currentView = &mainView;
        clock.setBpm(bpm);
        sequencer.setStepCount(stepCountValues[stepCountIdx]);
        for (int i = 0; i < 128; i++)
            if (i % 4 == 0) {
                sequencer.getStep(i).enabled = true;
                sequencer.getStep(i).len = 1;
            }
    }

    void encBtn()
    {
#ifdef IS_STM32
        uint32_t currentTime = HAL_GetTick();
        if (currentTime - lastBtnTime < 200) return;
        lastBtnTime = currentTime;
#endif
        currentView->onButton();
    }

    void onEncoder(int dir) { currentView->onEncoder(dir); }

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

        return isMuted ? 0.0f : kick.sample();
    }

    bool render()
    {
        if (!needsRedraw) return false;
        display.filledRect({ 0, 0 }, { 160, 80 }, { { 0, 0, 0 } });
        currentView->render(display);
        if (currentView == &mainView && isTriggered) display.filledCircle({ 150, 7 }, 4, { { 0, 255, 0 } });
        needsRedraw = false;
        return true;
    }
};