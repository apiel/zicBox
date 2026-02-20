#pragma once

#include "IView.h"
#include "engines.h"
#include "audio/Clock.h"
#include "audio/engines/EngineBase.h"

class MainListView : public IView {
    EngineType& currentEngineType;
    Clock& clock;
    float& volume;
    float& bpm;
    bool& isMuted;
    bool& needsRedraw;

    int selectedParam = 0;
    bool editMode = false;
    int scrollOffset = 0;
    const int VISIBLE_ROWS = 4;
    int encoderAccumulator = 0;
    std::function<void()> onOpenEditor;

    // Helper to calculate total rows: Mute + Engine + Params + Vol + BPM + StepEdit
    int getTotalRows() {
        return 2 + engines[currentEngineType]->getParamCount() + 3;
    }

public:
    MainListView(EngineType& eng, Clock& clk, float& v, float& b, bool& m, bool& redraw, std::function<void()> openEdit)
        : currentEngineType(eng), clock(clk), volume(v), bpm(b), isMuted(m), needsRedraw(redraw), onOpenEditor(openEdit) {}

    void onButton() override {
        int totalRows = getTotalRows();
        if (selectedParam == totalRows - 1) {
            onOpenEditor();
        } else if (selectedParam == 0) {
            isMuted = !isMuted;
        } else if (selectedParam == 1) {
            currentEngineType = static_cast<EngineType>((currentEngineType + 1) % ENGINE_COUNT);
            selectedParam = 1; // Stay on engine row
        } else {
            editMode = !editMode;
        }
        needsRedraw = true;
    }

    void onEncoder(int dir) override {
        int totalRows = getTotalRows();
        if (!editMode) {
            encoderAccumulator += dir;
            if (abs(encoderAccumulator) >= 1) {
                int move = (encoderAccumulator > 0) ? -1 : 1;
                selectedParam = CLAMP((selectedParam + move), 0, totalRows - 1);
                scrollOffset = CLAMP(selectedParam - (VISIBLE_ROWS / 2), 0, CLAMP(totalRows - VISIBLE_ROWS, 0, totalRows));
                encoderAccumulator = 0;
                needsRedraw = true;
            }
        } else {
            IEngine* engine = engines[currentEngineType];
            int pCount = engine->getParamCount();

            if (selectedParam == 0) isMuted = (dir > 0);
            else if (selectedParam == 1) {
                currentEngineType = static_cast<EngineType>(CLAMP(currentEngineType + (dir > 0 ? 1 : -1), 0, ENGINE_COUNT - 1));
            }
            else if (selectedParam >= 2 && selectedParam < 2 + pCount) {
                Param* p = &engine->getParams()[selectedParam - 2];
                p->set(p->value + (dir * p->step));
            } else {
                int lastIdx = 2 + pCount;
                if (selectedParam == lastIdx) volume = CLAMP(volume + (dir * 0.05f), 0.0f, 1.0f);
                else if (selectedParam == lastIdx + 1) {
                    bpm = CLAMP(bpm + dir, 40.0f, 240.0f);
                    clock.setBpm(bpm);
                }
            }
            needsRedraw = true;
        }
    }

    void render(DrawPrimitives& display) override {
        IEngine* engine = engines[currentEngineType];
        int pCount = engine->getParamCount();
        int totalRows = getTotalRows();

        display.filledRect({ 0, 0 }, { 160, 15 }, { { 40, 40, 40 } });
        display.text({ 5, 2 }, engine->getName(), 12, { { 255, 255, 255 } });

        for (int i = 0; i < VISIBLE_ROWS; i++) {
            int idx = i + scrollOffset;
            if (idx >= totalRows) break;
            int yPos = 18 + (i * 15);

            if (idx == selectedParam) {
                Color bg = editMode ? Color { 200, 0, 0 } : Color { 0, 100, 200 };
                display.filledRect({ 2, yPos - 1 }, { 153, 14 }, { bg });
            }

            char valBuffer[24] = "";
            if (idx == 0) {
                display.text({ 5, yPos }, "Mute", 12, { 255, 255, 255 });
                snprintf(valBuffer, sizeof(valBuffer), isMuted ? "ON" : "OFF");
            } else if (idx == 1) {
                display.text({ 5, yPos }, "Engine", 12, { 255, 255, 255 });
                snprintf(valBuffer, sizeof(valBuffer), "%s", engine->getName());
            } else if (idx >= 2 && idx < 2 + pCount) {
                Param* p = &engine->getParams()[idx - 2];
                display.text({ 5, yPos }, p->label, 12, { 255, 255, 255 });
                if (p->precision <= 0) snprintf(valBuffer, sizeof(valBuffer), "%d%s", (int)p->value, p->unit ? p->unit : "");
                else snprintf(valBuffer, sizeof(valBuffer), "%.*f%s", (int)p->precision, (double)p->value, p->unit ? p->unit : "");
            } else {
                int pos = idx - (2 + pCount);
                const char* labels[] = { "Volume", "BPM", "Step Edit" };
                display.text({ 5, yPos }, labels[pos], 12, { 255, 255, 255 });
                if (pos == 0) snprintf(valBuffer, sizeof(valBuffer), "%d%%", (int)(volume * 100));
                else if (pos == 1) snprintf(valBuffer, sizeof(valBuffer), "%d", (int)bpm);
                else snprintf(valBuffer, sizeof(valBuffer), "->");
            }
            display.textRight({ 150, yPos }, valBuffer, 12, { 255, 255, 255 });
        }
    }
};
