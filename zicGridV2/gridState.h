#pragma once

#include "draw/utils/color.h"
#include "audio/engines/EngineParam.h"
#include <array>
#include <string>
#include <cstdint>
#include <cstdio>

static constexpr int DYNAMIC_PAD_COLS = 8;
static constexpr int GLOBAL_PAD_COLS = 4;
static constexpr int PAD_COLS = 12;
static constexpr int PAD_ROWS = 4;
static constexpr int TOTAL_PADS = PAD_COLS * PAD_ROWS; // 48

static constexpr int ENCODER_COLS = 4;
static constexpr int ENCODER_ROWS = 3;
static constexpr int TOTAL_ENCODERS = ENCODER_COLS * ENCODER_ROWS; // 12

enum GridViewMode {
    VIEW_INSTRUMENT = 0,
    VIEW_STEP_SEQ = 1,
    VIEW_MASTER = 2,
    VIEW_COUNT = 3
};

struct PadState {
    Color color = { 40, 40, 50, 255 };
    bool pressed = false;
    std::string label;
    bool active = false;
    uint8_t note = 0;
    bool selected = false;
};

struct GlobalUtilityState {
    bool playActive = false;
    bool recActive = false;
    bool shiftActive = false;
    int currentOctave = 3; // 0..7
    int activeView = VIEW_INSTRUMENT;
    int activeTrack = 0;   // 0..7
    bool mutes[8] = { false };
    bool solos[8] = { false };
};

struct GridHardwareState {
    PadState pads[PAD_COLS][PAD_ROWS];
    Param encoders[TOTAL_ENCODERS];
    char encoderLabels[TOTAL_ENCODERS][32];
    char encoderStrings[TOTAL_ENCODERS][32];
    Color encoderColors[TOTAL_ENCODERS];
    Color encoderBgColors[TOTAL_ENCODERS];
    GlobalUtilityState utility;

    GridHardwareState() {
        initDefaultColors();
        for (int i = 0; i < TOTAL_ENCODERS; ++i) {
            encoderLabels[i][0] = '\0';
            encoderStrings[i][0] = '\0';
            encoders[i].label = encoderLabels[i];
            encoders[i].string = nullptr;
            encoderColors[i] = { 0, 180, 255, 255 };
            encoderBgColors[i] = { 0, 0, 0, 0 };
        }
    }

    void setEncoderBg(int idx, Color bgCol) {
        if (idx >= 0 && idx < TOTAL_ENCODERS) {
            encoderBgColors[idx] = bgCol;
        }
    }

    void setEncoder(int idx, const char* label, float val, float minV, float maxV, float stepV = 1.0f, const char* strVal = nullptr, Color col = { 0, 180, 255, 255 }, const char* unitVal = nullptr) {
        if (idx < 0 || idx >= TOTAL_ENCODERS) return;
        if (!label || label[0] == '\0') {
            encoders[idx].label = nullptr;
            encoderLabels[idx][0] = '\0';
        } else {
            snprintf(encoderLabels[idx], sizeof(encoderLabels[idx]), "%s", label);
            encoders[idx].label = encoderLabels[idx];
        }
        encoders[idx].value = val;
        encoders[idx].min = minV;
        encoders[idx].max = maxV;
        encoders[idx].step = stepV;
        encoders[idx].unit = unitVal;
        encoders[idx].precision = calculatePrecision(stepV);
        encoderColors[idx] = col;
        if (strVal) {
            snprintf(encoderStrings[idx], sizeof(encoderStrings[idx]), "%s", strVal);
            encoders[idx].string = encoderStrings[idx];
            encoders[idx].type = VALUE_STRING;
        } else {
            encoders[idx].string = nullptr;
            encoders[idx].type = VALUE_BASIC;
        }
    }

    void setEncoderParam(int idx, const Param& p, Color col = { 0, 180, 255, 255 }) {
        if (idx < 0 || idx >= TOTAL_ENCODERS) return;
        encoders[idx] = p;
        encoderColors[idx] = col;
        if (p.label) {
            snprintf(encoderLabels[idx], sizeof(encoderLabels[idx]), "%s", p.label);
            encoders[idx].label = encoderLabels[idx];
        } else {
            encoderLabels[idx][0] = '\0';
            encoders[idx].label = nullptr;
        }
        if (p.string) {
            snprintf(encoderStrings[idx], sizeof(encoderStrings[idx]), "%s", p.string);
            encoders[idx].string = encoderStrings[idx];
        } else {
            encoderStrings[idx][0] = '\0';
            encoders[idx].string = nullptr;
        }
    }

    void initDefaultColors() {
        Color trackColors[8] = {
            { 255, 80, 80, 255 },   // T1 Red
            { 255, 160, 40, 255 },  // T2 Orange
            { 240, 220, 40, 255 },  // T3 Yellow
            { 60, 220, 100, 255 },  // T4 Green
            { 40, 200, 255, 255 },  // T5 Cyan
            { 100, 120, 255, 255 }, // T6 Blue
            { 200, 80, 255, 255 },  // T7 Purple
            { 255, 100, 180, 255 }  // T8 Pink
        };

        for (int c = 0; c < 4; ++c) {
            pads[8 + c][0].color = trackColors[c];
            pads[8 + c][0].label = "T" + std::to_string(c + 1);
        }

        for (int c = 0; c < 4; ++c) {
            pads[8 + c][1].color = trackColors[c + 4];
            pads[8 + c][1].label = "T" + std::to_string(c + 5);
        }

        pads[8][2].color  = { 200, 200, 200, 255 };   pads[8][2].label = "Instr.";
        pads[9][2].color  = { 200, 200, 200, 255 };   pads[9][2].label = "Seq";
        pads[10][2].color = { 200, 200, 200, 255 };  pads[10][2].label = "Master";
        pads[11][2].color = { 200, 200, 200, 255 };  pads[11][2].label = "Shift";

        pads[8][3].color  = { 255, 160, 40, 255 };    pads[8][3].label = "&icon::arrowLeft::filled";
        pads[9][3].color  = { 255, 160, 40, 255 };   pads[9][3].label = "&icon::arrowRight::filled";
        pads[10][3].color = { 100, 120, 255, 255 };  pads[10][3].label = "Oct-";
        pads[11][3].color = { 100, 120, 255, 255 };  pads[11][3].label = "Oct+";
    }
};

inline GridHardwareState gridState;
