#pragma once

#include "draw/draw.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include <memory>
#include <vector>

class View {
protected:
    std::string title;

public:
    View(const std::string& name) : title(name) {}
    virtual ~View() = default;

    const std::string& getTitle() const { return title; }

    virtual void onActivate() {}
    virtual void onDeactivate() {}
    virtual void updatePadLeds() {}
    virtual void updateEncoderLabels() {}

    virtual void render(Draw& d, int x, int y, int w, int h) = 0;
    virtual void handleDynamicPadPress(int col, int row, bool pressed) = 0;
    virtual void handleEncoder(int encoderId, int delta) = 0;
};

namespace ViewManager {
inline static std::vector<std::shared_ptr<View>> views;
inline static int activeViewIdx = VIEW_STEP_SEQ;

inline void registerView(int viewId, std::shared_ptr<View> view)
{
    if (viewId >= (int)views.size()) {
        views.resize(viewId + 1);
    }
    views[viewId] = view;
}

inline int getActiveViewIdx()
{
    return activeViewIdx;
}

inline std::shared_ptr<View> getActiveView()
{
    if (activeViewIdx >= 0 && activeViewIdx < (int)views.size()) {
        return views[activeViewIdx];
    }
    return nullptr;
}

inline void setActiveView(int viewId)
{
    if (viewId < 0 || viewId >= (int)views.size() || !views[viewId]) return;

    if (views[activeViewIdx]) {
        views[activeViewIdx]->onDeactivate();
    }

    activeViewIdx = viewId;
    gridState.utility.activeView = viewId;
    studio.currentView = viewId;

    if (views[activeViewIdx]) {
        views[activeViewIdx]->onActivate();
        views[activeViewIdx]->updatePadLeds();
        views[activeViewIdx]->updateEncoderLabels();
    }
}

inline void handleGlobalUtilityPad(int col, int row, bool pressed)
{
    if (!pressed) return;

    int utilCol = col - DYNAMIC_PAD_COLS; // 0..3

    // Row 0: View Select
    if (row == 0) {
        if (utilCol < VIEW_COUNT) {
            setActiveView(utilCol);
        }
    }
    // Row 1: Track 1-4 Select / Mute
    else if (row == 1) {
        int trk = utilCol; // 0..3
        if (gridState.utility.shiftActive) {
            studio.tracks[trk]->isMuted = !studio.tracks[trk]->isMuted;
            gridState.utility.mutes[trk] = studio.tracks[trk]->isMuted;
        } else {
            studio.selTrack = trk;
            gridState.utility.activeTrack = trk;
            if (auto v = getActiveView()) {
                v->updatePadLeds();
                v->updateEncoderLabels();
            }
        }
    }
    // Row 2: Track 5-8 Select / Mute
    else if (row == 2) {
        int trk = utilCol + 4; // 4..7
        if (gridState.utility.shiftActive) {
            studio.tracks[trk]->isMuted = !studio.tracks[trk]->isMuted;
            gridState.utility.mutes[trk] = studio.tracks[trk]->isMuted;
        } else {
            studio.selTrack = trk;
            gridState.utility.activeTrack = trk;
            if (auto v = getActiveView()) {
                v->updatePadLeds();
                v->updateEncoderLabels();
            }
        }
    }
    // Row 3: Transport / Modifiers
    else if (row == 3) {
        if (utilCol == 0) { // Play / Stop
            studio.isPlaying = !studio.isPlaying;
            gridState.utility.playActive = studio.isPlaying;
        } else if (utilCol == 1) { // Record
            gridState.utility.recActive = !gridState.utility.recActive;
        } else if (utilCol == 2) { // Octave -
            if (gridState.utility.currentOctave > 0) gridState.utility.currentOctave--;
        } else if (utilCol == 3) { // Octave +
            if (gridState.utility.currentOctave < 7) gridState.utility.currentOctave++;
        }
    }
}

inline void handlePadPress(int col, int row, bool pressed)
{
    if (col >= DYNAMIC_PAD_COLS) {
        // Global Utility Zone pad press (cols 8..11)
        handleGlobalUtilityPad(col, row, pressed);
        return;
    }

    // Dynamic Zone pad press (cols 0..7)
    auto view = getActiveView();
    if (view && col >= 0 && col < DYNAMIC_PAD_COLS && row >= 0 && row < PAD_ROWS) {
        view->handleDynamicPadPress(col, row, pressed);
    }
}

inline void handleEncoder(int encoderId, int delta)
{
    auto view = getActiveView();
    if (view) {
        view->handleEncoder(encoderId, delta);
        view->updateEncoderLabels();
    }
}

inline void renderActiveView(Draw& d, int x, int y, int w, int h)
{
    auto view = getActiveView();
    if (view) {
        view->updatePadLeds();
        view->updateEncoderLabels();
        view->render(d, x, y, w, h);
    }
}
} // namespace ViewManager
