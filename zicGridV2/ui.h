#pragma once

#include "draw/draw.h"
#include "ui/uiParams.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

#include "zicGridV2/views/InstrumentView.h"
#include "zicGridV2/views/MasterView.h"
#include "zicGridV2/views/ProjectView.h"
#include "zicGridV2/views/StepSeqView.h"

inline void initViews()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    if (studio.tracks.size() >= 2) {
        studio.tracks[0]->chain = { 0, 0, 1, -1, 2 };
        studio.tracks[1]->chain = { 0, 1, 1, 2, 2 };
    }

    ViewManager::registerView(VIEW_STEP_SEQ, std::make_shared<StepSeqView>());
    ViewManager::registerView(VIEW_INSTRUMENT, std::make_shared<InstrumentView>());
    ViewManager::registerView(VIEW_MASTER, std::make_shared<MasterView>());
    ViewManager::registerView(VIEW_PROJECT, std::make_shared<ProjectView>());

    ViewManager::setActiveView(VIEW_INSTRUMENT);
}

inline bool drawUI(Draw& d, int w, int h, bool& needFullRedraw)
{
    initViews();

    // Clear background using zicXYv2 dark theme background color
    d.filledRect({ 0, 0 }, { w, h }, { .color = d.styles.colors.background });

    int margin = 4;
    int usableW = w - margin * 2;

    // 1. PARAMS / ENCODERS GRID ON TOP (3 rows x 4 cols = 12 encoders)
    int encoderH = 3 * UiDraw::ROW_H; // 108px
    renderEncoderGrid(d, margin, margin, usableW, encoderH);

    // 2. BOTTOM: DYNAMIC 8x4 PAD MATRIX & GLOBAL 4x4 UTILITY ZONE SIDE-BY-SIDE (12x4 Pad Grid)
    int footerH = 16;
    int padGridH = 118;
    int padGridY = h - padGridH - footerH - margin;

    int padMatrixW = (usableW * DYNAMIC_PAD_COLS) / PAD_COLS;
    int globalUtilityW = usableW - padMatrixW;

    // 3. ACTIVE VIEW REGION (Middle area between encoder grid and bottom pad grid)
    int currentY = margin + encoderH + 4;
    int availableMiddleH = padGridY - currentY - 4;
    ViewManager::renderActiveView(d, margin, currentY, usableW, availableMiddleH);

    renderDynamicPadMatrix(d, margin, padGridY, padMatrixW - 2, padGridH);
    renderGlobalUtilityZone(d, margin + padMatrixW, padGridY, globalUtilityW, padGridH);
    renderFooterBar(d, margin, padGridY + padGridH, usableW, footerH, padMatrixW, globalUtilityW);

    UiMessage::draw(d, w, h, needFullRedraw);

    return true;
}
