#pragma once

#include "draw/utils/color.h"
#include "audio/engines/EngineParam.h"
#include <array>
#include <string>
#include <cstdint>
#include <cstdio>

static constexpr int SCREEN_W = 480;
static constexpr int SCREEN_H = 640;

static constexpr int PAD_COLS = 12;
static constexpr int PAD_ROWS = 4;
static constexpr int TOTAL_PADS = PAD_COLS * PAD_ROWS; // 48

static constexpr int ENCODER_COLS = 4;
static constexpr int ENCODER_ROWS = 3;
static constexpr int TOTAL_ENCODERS = ENCODER_COLS * ENCODER_ROWS; // 12

enum GridImpactViewMode {
    VIEW_MASTER = 0,
    VIEW_SEQUENCER = 1,
    VIEW_KICK = 2,
    VIEW_SYNTH1 = 3,
    VIEW_SYNTH2 = 4,
    VIEW_COUNT = 5
};

struct PadState {
    Color color = { 40, 40, 50, 255 };
    bool pressed = false;
    std::string label;
    bool active = false;
    uint8_t note = 0;
    bool selected = false;
};

struct GridHardwareState {
    PadState pads[PAD_COLS][PAD_ROWS];
    Param encoders[TOTAL_ENCODERS];
    char encoderLabels[TOTAL_ENCODERS][32];
    char encoderStrings[TOTAL_ENCODERS][32];
    Color encoderColors[TOTAL_ENCODERS];
    Color encoderBgColors[TOTAL_ENCODERS];

    int activeView = VIEW_MASTER;
    int currentOctave = 3; // 0..7

    // Performance controls from PixelDrift
    bool isDKeyHeld = false;
    bool isLatchedA = false;
    bool isLatchedS = false;
    bool isLatchedZ = false;
    bool isLatchedX = false;
    bool isLatchedC = false;
    bool isLatchedV = false;

    bool isPressedA = false;
    bool isPressedS = false;
    bool isPressedZ = false;
    bool isPressedX = false;
    bool isPressedC = false;
    bool isPressedV = false;

    bool isSynth1Muted = false;
    bool isSynth2Muted = false;

    float kickPulseLevel = 0.0f;
    float synth1PulseLevel = 0.0f;
    float synth2PulseLevel = 0.0f;

    bool isShutdownModalOpen = false;
    int shutdownChoice = 0;
    bool isShuttingDown = false;
    bool renderedGoodbye = false;

    GridHardwareState() {
        for (int i = 0; i < TOTAL_ENCODERS; ++i) {
            encoderLabels[i][0] = '\0';
            encoderStrings[i][0] = '\0';
            encoders[i].label = encoderLabels[i];
            encoders[i].string = nullptr;
            encoderColors[i] = { 0, 180, 255, 255 };
            encoderBgColors[i] = { 0, 0, 0, 0 };
        }
    }

    void setEncoder(int idx, const char* label, float val, float minV, float maxV, float stepV = 1.0f, const char* strVal = nullptr, Color col = { 0, 180, 255, 255 }, const char* unitVal = nullptr) {
        if (idx < 0 || idx >= TOTAL_ENCODERS) return;
        encoderBgColors[idx] = { 0, 0, 0, 0 };
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
            encoderStrings[idx][0] = '\0';
            encoders[idx].string = nullptr;
            encoders[idx].type = VALUE_BASIC;
        }
    }

    void setEncoderParam(int idx, const Param& p, Color col = { 0, 180, 255, 255 }) {
        if (idx < 0 || idx >= TOTAL_ENCODERS) return;
        encoderBgColors[idx] = { 0, 0, 0, 0 };
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
};

inline GridHardwareState gridState;
