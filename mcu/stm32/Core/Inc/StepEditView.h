#pragma once

#include "IView.h"
#include "audio/Clock.h"
#include "audio/Sequencer.h"
#include "helpers/midiNote.h"
#include "audio/engines/EngineBase.h"
#include "engines.h"

// --- 4. STEP EDITOR VIEW ---
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
        if (editorIndex == -2) onExit();
        else if (editorIndex == -1) editMode = !editMode;
        else {
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
        std::array<Point, 3> tri = { { { 75, 72 }, { 75 + (int)(20 * s.velocity), 72 }, { 75 + (int)(20 * s.velocity), 72 - (int)(10 * s.velocity) } } };
        display.filledPolygon(tri, { { 255, 255, 255 } });
        snprintf(buf, sizeof(buf), "%d%%", (int)(s.condition * 100));
        drawParam(4, 115, buf);
    }
};