#pragma once

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class KeyboardView : public View {
private:
    int baseNote = 48;
    int scaleType = 0;

public:
    KeyboardView() : View("PAD KEYBOARD") {}

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

        gridState.setEncoder(0, "Octave", (float)gridState.utility.currentOctave, 0.0f, 7.0f, 1.0f, ("C" + std::to_string(gridState.utility.currentOctave)).c_str(), c);
        gridState.setEncoder(1, "Track", (float)(trk + 1), 1.0f, 8.0f, 1.0f, ("T" + std::to_string(trk + 1)).c_str(), c);
        gridState.setEncoder(2, "Scale", (float)scaleType, 0.0f, 3.0f, 1.0f, getScaleName(scaleType).c_str(), c);
        gridState.setEncoder(3, "Vol", t->volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");

        if (t->engine) {
            size_t paramCount = t->engine->getParamCount();
            auto* params = t->engine->getParams();
            for (int i = 4; i < TOTAL_ENCODERS; ++i) {
                int pIdx = i - 4;
                if ((size_t)pIdx < paramCount && params) {
                    gridState.setEncoderParam(i, params[pIdx], c);
                } else {
                    gridState.setEncoder(i, "---", 0.0f, 0.0f, 1.0f, 0.1f, "N/A", c);
                }
            }
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        std::string titleStr = "VIEW: KEYBOARD - OCTAVE C" + std::to_string(gridState.utility.currentOctave);
        d.text({ x + 4, y + 2 }, titleStr, 8, { .color = { 255, 200, 50, 255 }, .font = &PoppinsLight_8 });
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
        if (encoderId == 1) {
            gridState.utility.currentOctave = std::clamp(gridState.utility.currentOctave + delta, 0, 7);
        } else if (encoderId == 2) {
            studio.selTrack = std::clamp(studio.selTrack + delta, 0, MAX_TRACKS - 1);
            gridState.utility.activeTrack = studio.selTrack;
        } else if (encoderId == 3) {
            scaleType = std::clamp(scaleType + delta, 0, 3);
        } else if (encoderId == 4) {
            auto& t = studio.tracks[studio.selTrack];
            t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
        } else {
            int trk = studio.selTrack;
            auto& t = studio.tracks[trk];
            if (t->engine) {
                int pIdx = encoderId - 5;
                if (pIdx >= 0 && (size_t)pIdx < t->engine->getParamCount()) {
                    auto& p = t->engine->getParams()[pIdx];
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

    std::string getScaleName(int type)
    {
        switch (type) {
        case 0: return "CHROM";
        case 1: return "MAJOR";
        case 2: return "MINOR";
        case 3: return "PENTA";
        default: return "CHROM";
        }
    }
};
