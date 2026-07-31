#pragma once

#include "draw/utils/color.h"
#include <array>
#include <string>
#include <cstdint>

static constexpr int DYNAMIC_PAD_COLS = 8;
static constexpr int GLOBAL_PAD_COLS = 4;
static constexpr int PAD_COLS = 12;
static constexpr int PAD_ROWS = 4;
static constexpr int TOTAL_PADS = PAD_COLS * PAD_ROWS; // 48

static constexpr int ENCODER_COLS = 4;
static constexpr int ENCODER_ROWS = 3;
static constexpr int TOTAL_ENCODERS = ENCODER_COLS * ENCODER_ROWS; // 12

enum GridViewMode {
    VIEW_STEP_SEQ = 0,
    VIEW_INSTRUMENT = 1,
    VIEW_KEYBOARD = 2,
    VIEW_CLIP_LAUNCH = 3,
    VIEW_COUNT = 4
};

struct PadState {
    Color color = { 40, 40, 50, 255 };
    bool pressed = false;
    std::string label;
    bool active = false;
    uint8_t note = 0;
};

struct EncoderState {
    std::string label = "Param";
    float value = 0.0f;
    float minVal = 0.0f;
    float maxVal = 1.0f;
    float step = 0.01f;
    std::string displayVal = "0.00";
    Color color = { 100, 200, 255, 255 };
    bool active = true;
};

struct GlobalUtilityState {
    // Utility functions mapped to the 4x4 right grid (cols 8..11, rows 0..3)
    // Row 0: View selection (Seq, Inst, Key, Clip)
    // Row 1: Quick Track Select / Mute 1..4
    // Row 2: Quick Track Select / Mute 5..8
    // Row 3: Play/Stop, Record, Octave-, Octave+ / Shift
    bool playActive = false;
    bool recActive = false;
    bool shiftActive = false;
    int currentOctave = 3; // 0..7
    int activeView = VIEW_STEP_SEQ;
    int activeTrack = 0;   // 0..7
    bool mutes[8] = { false };
    bool solos[8] = { false };
};

struct GridHardwareState {
    PadState pads[PAD_COLS][PAD_ROWS];
    EncoderState encoders[TOTAL_ENCODERS];
    GlobalUtilityState utility;

    GridHardwareState() {
        initDefaultColors();
    }

    void initDefaultColors() {
        // Initialize Global Utility Zone RGB LEDs (cols 8..11)
        // Row 0: Views
        pads[8][0].color  = { 255, 100, 0, 255 };   pads[8][0].label = "SEQ";
        pads[9][0].color  = { 0, 200, 255, 255 };   pads[9][0].label = "INST";
        pads[10][0].color = { 200, 0, 255, 255 };  pads[10][0].label = "KEY";
        pads[11][0].color = { 0, 255, 120, 255 };  pads[11][0].label = "CLIP";

        // Row 1: Tracks 1-4
        for (int c = 0; c < 4; ++c) {
            pads[8 + c][1].color = { 60, 140, 220, 255 };
            pads[8 + c][1].label = "T" + std::to_string(c + 1);
        }

        // Row 2: Tracks 5-8
        for (int c = 0; c < 4; ++c) {
            pads[8 + c][2].color = { 220, 140, 60, 255 };
            pads[8 + c][2].label = "T" + std::to_string(c + 5);
        }

        // Row 3: Transport & Modifiers
        pads[8][3].color  = { 0, 255, 80, 255 };    pads[8][3].label = "PLAY";
        pads[9][3].color  = { 255, 40, 40, 255 };   pads[9][3].label = "REC";
        pads[10][3].color = { 180, 180, 0, 255 };  pads[10][3].label = "OCT-";
        pads[11][3].color = { 220, 220, 0, 255 };  pads[11][3].label = "OCT+";
    }
};

inline GridHardwareState gridState;
