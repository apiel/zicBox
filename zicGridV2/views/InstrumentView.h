#pragma once

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class InstrumentView : public View {
private:
    int baseNote = 48;
    int currentPage = 0;

public:
    InstrumentView() : View("INSTRUMENT & SYNTH") {}

    int getTotalPages() const
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        if (t && t->engine) {
            size_t paramCount = t->engine->getParamCount();
            if (paramCount == 0) return 1;
            return (int)((paramCount + TOTAL_ENCODERS - 1) / TOTAL_ENCODERS);
        }
        return 1;
    }

    std::pair<int, int> getViewPageInfo() const override
    {
        int totalPages = getTotalPages();
        int pageIdx = std::min(currentPage + 1, totalPages);
        return { pageIdx, totalPages };
    }

    void onTrackSelect(int trk, bool isSameTrack) override
    {
        if (isSameTrack) {
            int totalPages = getTotalPages();
            if (totalPages > 1) {
                currentPage = (currentPage + 1) % totalPages;
            }
        } else {
            currentPage = 0;
        }
    }

    void changePage(int delta) override
    {
        int totalPages = getTotalPages();
        if (totalPages > 1) {
            currentPage = (currentPage + delta + totalPages) % totalPages;
        }
    }

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void updatePadLeds() override
    {
        int octave = gridState.utility.currentOctave;
        baseNote = 12 + octave * 12;

        int activeTrk = studio.selTrack;
        Color theme = studio.tracks[activeTrk]->themeColor;

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int noteOffset = (3 - r) * DYNAMIC_PAD_COLS + c;
                int note = baseNote + noteOffset;

                auto& pad = gridState.pads[c][r];
                pad.note = (uint8_t)note;
                pad.selected = false;

                bool isRoot = (note % 12 == 0);
                bool isAccidental = (note % 12 == 1 || note % 12 == 3 || note % 12 == 6 || note % 12 == 8 || note % 12 == 10);

                if (pad.pressed) {
                    pad.color = { 255, 255, 255, 255 };
                } else if (isRoot) {
                    pad.color = { 255, 200, 50, 255 };
                } else if (isAccidental) {
                    pad.color = { 60, 70, 100, 255 };
                } else {
                    pad.color = theme;
                }

                pad.label = getNoteName(note);
            }
        }
    }

    void updateEncoderLabels() override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        Color c = t->themeColor;

        if (gridState.utility.shiftActive) {
            gridState.setEncoder(0, "Synth", t->currentEngineIdx, 0, ENGINE_REGISTRY_COUNT - 1, 1, engineRegistry[t->currentEngineIdx].name, c);
            gridState.setEncoder(1, "Volume", (int)(t->volume * 100.0f), 0, 100, 1, nullptr, c, "%");

            for (int i = 2; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoderParam(i, Param{}, c);
            }
            return;
        }

        int totalPages = getTotalPages();
        if (currentPage >= totalPages) {
            currentPage = 0;
        }

        if (t && t->engine) {
            size_t paramCount = t->engine->getParamCount();
            auto* params = t->engine->getParams();
            for (int i = 0; i < TOTAL_ENCODERS; ++i) {
                int actualParamIdx = currentPage * TOTAL_ENCODERS + i;
                if ((size_t)actualParamIdx < paramCount && params) {
                    gridState.setEncoderParam(i, params[actualParamIdx], c);
                } else {
                    gridState.setEncoderParam(i, Param{}, c);
                }
            }
        } else {
            for (int i = 0; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoderParam(i, Param{}, c);
            }
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        Color themeColor = studio.tracks[studio.selTrack]->themeColor;
        std::string titleStr = "VIEW: INST & KEYBOARD - T" + std::to_string(studio.selTrack + 1) + " (" + engineRegistry[studio.tracks[studio.selTrack]->currentEngineIdx].name + ")";
        d.text({ x + 4, y + 2 }, titleStr, 8, { .color = themeColor, .font = &PoppinsLight_8 });

        // Draw Page Indicator Dots on the right side of the View Title Badge
        auto [pageIdx, totalPages] = getViewPageInfo();
        if (totalPages > 1) {
            int dotW = 5;
            int dotH = 3;
            int gap = 2;
            int totalDotsW = totalPages * dotW + (totalPages - 1) * gap;
            int dotsX = x + w - 6 - totalDotsW;
            int dotsY = y + (h - dotH) / 2;
            for (int p = 0; p < totalPages; p++) {
                Color dotCol = (p + 1 == pageIdx) ? themeColor : Color { 60, 72, 95, 255 };
                d.filledRect({ dotsX + p * (dotW + gap), dotsY }, { dotW, dotH }, { .color = dotCol });
            }
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        auto& pad = gridState.pads[col][row];
        pad.pressed = pressed;

        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (pressed) {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            t->engine->noteOn(pad.note, 0.9f);
        } else {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            t->engine->noteOff(pad.note);
        }

        updatePadLeds();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (gridState.utility.shiftActive) {
            if (encoderId == 1) { // Synth engine selection
                int nextEngine = std::clamp((int)t->currentEngineIdx + delta, 0, ENGINE_REGISTRY_COUNT - 1);
                if (nextEngine != t->currentEngineIdx) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    t->setEngine(nextEngine);
                    currentPage = 0;
                }
            } else if (encoderId == 2) { // Track volume
                t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
            }
        } else if (t && t->engine) {
            int pIdx = encoderId - 1;
            if (pIdx >= 0 && pIdx < TOTAL_ENCODERS) {
                int actualParamIdx = currentPage * TOTAL_ENCODERS + pIdx;
                if ((size_t)actualParamIdx < t->engine->getParamCount()) {
                    auto& p = t->engine->getParams()[actualParamIdx];
                    p.set(p.value + delta * p.step);
                }
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

private:
    std::string getNoteName(int note)
    {
        static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int octave = (note / 12) - 1;
        return std::string(names[note % 12]) + std::to_string(octave);
    }
};
