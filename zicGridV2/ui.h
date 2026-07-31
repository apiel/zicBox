#pragma once

#include "draw/draw.h"
#include "zicGridV2/DriftVisualizer.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

#include "zicGridV2/views/ClipLaunchView.h"
#include "zicGridV2/views/InstrumentView.h"
#include "zicGridV2/views/KeyboardView.h"
#include "zicGridV2/views/StepSeqView.h"

static GlobalUtilityZoneComponent globalUtilityComp;
static EncoderGridComponent encoderGridComp;
static DynamicPadMatrixComponent padMatrixComp;
static VUVisualizerComponent vuVisComp;

inline void initViews()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    viewManager.registerView(VIEW_STEP_SEQ, std::make_shared<StepSeqView>());
    viewManager.registerView(VIEW_INSTRUMENT, std::make_shared<InstrumentView>());
    viewManager.registerView(VIEW_KEYBOARD, std::make_shared<KeyboardView>());
    viewManager.registerView(VIEW_CLIP_LAUNCH, std::make_shared<ClipLaunchView>());

    viewManager.setActiveView(VIEW_STEP_SEQ);
}

inline bool drawUI(Draw& d, int w, int h, bool& needFullRedraw)
{
    initViews();

    // 1. Animated background particle drift visualizer
    driftVisualizer.updateAndDraw(d, studio.bpm, w, h, 0, 0, w, h);

    // 2. Render Top Header Bar (24px high)
    int topBarH = 24;
    d.filledRect({ 0, 0 }, { w, topBarH }, { .color = { 12, 14, 20, 240 } });
    d.rect({ 0, 0 }, { w, topBarH }, { .color = { 40, 50, 70, 255 }, .thickness = 1 });

    d.text({ 8, 5 }, "ZIC GRID V2", 11, { .color = { 255, 200, 50, 255 } });

    std::string playStateStr = studio.isPlaying ? "[PLAYING]" : "[STOPPED]";
    Color playCol = studio.isPlaying ? Color{ 0, 255, 120, 255 } : Color{ 180, 180, 180, 255 };
    d.text({ 140, 5 }, playStateStr, 10, { .color = playCol });

    std::string bpmStr = "BPM: " + std::to_string((int)studio.bpm);
    d.text({ 240, 5 }, bpmStr, 10, { .color = { 100, 200, 255, 255 } });

    std::string trkStr = "T" + std::to_string(studio.selTrack + 1);
    d.text({ 340, 5 }, trkStr, 10, { .color = studio.tracks[studio.selTrack]->themeColor });

    // 3. Layout Regions for 480x640 resolution (Top-to-Bottom flow)
    int encoderH = 165;       // 1. Encoders/Params ON TOP
    int viewHeaderH = 18;     // Active View Header
    int padMatrixH = 120;     // Dynamic 8x4 Pad Matrix
    int vuH = 24;             // 8 Track VU Visualizer
    int globalUtilityH = 120; // Compact Global Utility Zone AT THE BOTTOM

    int currentY = topBarH + 2;

    // 1. PARAMS / ENCODERS GRID ON TOP
    encoderGridComp.render(d, 4, currentY, w - 8, encoderH);
    currentY += encoderH + 4;

    // 2. ACTIVE VIEW HEADER
    viewManager.renderActiveView(d, 4, currentY, w - 8, viewHeaderH);
    currentY += viewHeaderH + 2;

    // 3. DYNAMIC 8x4 PAD MATRIX
    padMatrixComp.render(d, 4, currentY, w - 8, padMatrixH);
    currentY += padMatrixH + 4;

    // 4. 8-TRACK VU VISUALIZER
    // vuVisComp.render(d, 4, currentY, w - 8, vuH);
    // currentY += vuH + 4;

    // 5. GLOBAL UTILITY ZONE AT THE BOTTOM
    globalUtilityComp.render(d, 4, currentY, w - 8, globalUtilityH);

    return true;
}
