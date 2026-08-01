#pragma once

#include "draw/draw.h"
#include "ui/uiParams.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

#include "zicGridV2/views/InstrumentView.h"
#include "zicGridV2/views/MasterView.h"
#include "zicGridV2/views/StepSeqView.h"

inline void initViews()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;

    ViewManager::registerView(VIEW_STEP_SEQ, std::make_shared<StepSeqView>());
    ViewManager::registerView(VIEW_INSTRUMENT, std::make_shared<InstrumentView>());
    ViewManager::registerView(VIEW_MASTER, std::make_shared<MasterView>());

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

    // 2. ACTIVE VIEW HEADER
    int currentY = margin + encoderH + 4;
    int viewHeaderH = 16;
    ViewManager::renderActiveView(d, margin, currentY, usableW, viewHeaderH);

    // 3. BOTTOM: DYNAMIC 8x4 PAD MATRIX & GLOBAL 4x4 UTILITY ZONE SIDE-BY-SIDE (12x4 Pad Grid)
    // Middle region (between currentY + viewHeaderH and padGridY) is left empty for future use.
    int padGridH = 130;
    int padGridY = h - padGridH - margin;

    int padMatrixW = (usableW * DYNAMIC_PAD_COLS) / PAD_COLS;
    int globalUtilityW = usableW - padMatrixW;

    renderDynamicPadMatrix(d, margin, padGridY, padMatrixW - 2, padGridH);
    renderGlobalUtilityZone(d, margin + padMatrixW, padGridY, globalUtilityW, padGridH);

    return true;
}
