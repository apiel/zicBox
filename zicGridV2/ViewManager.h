#pragma once

#include "draw/draw.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiMessage.h"
#include "zicGridV2/project.h"
#include <memory>
#include <vector>

class View {
protected:
    std::string title;

public:
    View(const std::string& name)
        : title(name)
    {
    }
    virtual ~View() = default;

    const std::string& getTitle() const { return title; }

    virtual void onActivate() { }
    virtual void onDeactivate() { }
    virtual void updatePadLeds() { }
    virtual void updateEncoderLabels() { }
    virtual void onTrackSelect(int trk, bool isSameTrack) { }
    virtual void changePage(int delta) { }
    virtual std::pair<int, int> getViewPageInfo() const { return { 1, 1 }; }

    virtual void render(Draw& d, int x, int y, int w, int h) = 0;
    virtual void handleDynamicPadPress(int col, int row, bool pressed) = 0;
    virtual void handleEncoder(int encoderId, int delta) = 0;
    virtual void handleUtilityPadPress(int utilCol, bool pressed) { }
};

namespace ViewManager {

inline static std::vector<std::shared_ptr<View>> views;
inline static int activeViewIdx = VIEW_INSTRUMENT;
inline static bool pendingShiftSaveConfirm = false;
inline static bool pendingShiftReloadConfirm = false;

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

    for (int r = 0; r < PAD_ROWS; ++r) {
        for (int c = 0; c < PAD_COLS; ++c) {
            gridState.pads[c][r].selected = false;
            gridState.pads[c][r].pressed = false;
            gridState.pads[c][r].active = false;
        }
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
    int utilCol = col - DYNAMIC_PAD_COLS; // 0..3

    // Shift pad (Row 2, Col 3) is momentary (active on press, inactive on release)
    if (row == 2 && utilCol == 3) {
        gridState.utility.shiftActive = pressed;
        if (!pressed) {
            pendingShiftSaveConfirm = false;
            pendingShiftReloadConfirm = false;
        }
        if (auto v = getActiveView()) {
            v->updatePadLeds();
            v->updateEncoderLabels();
        }
        return;
    }

    if (!pressed) return;

    // Row 0: Track 1-4 Select / Mute
    if (row == 0) {
        int trk = utilCol; // 0..3
        if (gridState.utility.shiftActive) {
            studio.tracks[trk]->isMuted = !studio.tracks[trk]->isMuted;
            gridState.utility.mutes[trk] = studio.tracks[trk]->isMuted;
        } else {
            bool isSameTrack = (studio.selTrack == trk);
            studio.selTrack = trk;
            gridState.utility.activeTrack = trk;
            if (auto v = getActiveView()) {
                v->onTrackSelect(trk, isSameTrack);
                v->updatePadLeds();
                v->updateEncoderLabels();
            }
        }
    }
    // Row 1: Track 5-8 Select / Mute
    else if (row == 1) {
        int trk = utilCol + 4; // 4..7
        if (gridState.utility.shiftActive) {
            studio.tracks[trk]->isMuted = !studio.tracks[trk]->isMuted;
            gridState.utility.mutes[trk] = studio.tracks[trk]->isMuted;
        } else {
            bool isSameTrack = (studio.selTrack == trk);
            studio.selTrack = trk;
            gridState.utility.activeTrack = trk;
            if (auto v = getActiveView()) {
                v->onTrackSelect(trk, isSameTrack);
                v->updatePadLeds();
                v->updateEncoderLabels();
            }
        }
    }
    // Row 2: View Select (INST, SEQ, MASTER) or Shift Shortcuts (F+S Reload, F+D Save)
    else if (row == 2) {
        if (gridState.utility.shiftActive) {
            if (utilCol == 0) { // F + A: Empty
                // Empty for now
            } else if (utilCol == 1) { // F + S: Reload Project
                if (pendingShiftReloadConfirm) {
                    bool loaded = loadProject();
                    bool dummyRedraw = true;
                    if (loaded) {
                        UiMessage::show("Project reloaded", dummyRedraw, 2500);
                        if (auto v = getActiveView()) v->onActivate();
                    } else {
                        UiMessage::show("No project loaded", dummyRedraw, 2000);
                    }
                    pendingShiftReloadConfirm = false;
                } else {
                    pendingShiftReloadConfirm = true;
                    pendingShiftSaveConfirm = false;
                    bool dummyRedraw = true;
                    UiMessage::show("Press RELOAD again to confirm", dummyRedraw, 3000);
                }
            } else if (utilCol == 2) { // F + D: Save Project
                if (pendingShiftSaveConfirm) {
                    bool saved = saveProject();
                    bool dummyRedraw = true;
                    if (saved) {
                        UiMessage::show("Project saved", dummyRedraw, 2500);
                    } else {
                        UiMessage::show("No project loaded", dummyRedraw, 2000);
                    }
                    pendingShiftSaveConfirm = false;
                } else {
                    pendingShiftSaveConfirm = true;
                    pendingShiftReloadConfirm = false;
                    bool dummyRedraw = true;
                    UiMessage::show("Press SAVE again to confirm", dummyRedraw, 3000);
                }
            }
        } else {
            int targetView = -1;
            if (utilCol == 0) targetView = VIEW_INSTRUMENT;
            else if (utilCol == 1) targetView = VIEW_STEP_SEQ;
            else if (utilCol == 2) targetView = VIEW_MASTER;

            if (targetView != -1) {
                if (activeViewIdx == targetView) {
                    if (auto v = getActiveView()) {
                        v->changePage(1);
                        v->updatePadLeds();
                        v->updateEncoderLabels();
                    }
                } else {
                    setActiveView(targetView);
                }
            }
        }
    }
    // Row 3: Page Switch / Shift Actions / Octave Adjust
    else if (row == 3) {
        if (!pressed) return;

        if (gridState.utility.shiftActive) {
            if (utilCol == 0) { // Play / Stop
                studio.isPlaying = !studio.isPlaying;
                gridState.utility.playActive = studio.isPlaying;
            } else if (utilCol == 1) { // Record
                gridState.utility.recActive = !gridState.utility.recActive;
            } else if (utilCol == 2) { // Tape
                bool isArmed = studio.masterFx.tape.armed.load();
                if (isArmed) {
                    studio.masterFx.tape.armed.store(false);
                    studio.masterFx.tape.recording.store(false);
                    bool dummyRedraw = true;
                    UiMessage::show("Tape recording canceled", dummyRedraw, 2000);
                } else {
                    studio.masterFx.tape.armed.store(true);
                    studio.lastRecordedTapeFilename = studio.masterFx.tape.filename;
                    bool dummyRedraw = true;
                    UiMessage::show("Tape recording armed...", dummyRedraw, 2000);
                }
            } else if (utilCol == 3) { // Project
                if (activeViewIdx == VIEW_PROJECT) {
                    setActiveView(VIEW_INSTRUMENT);
                } else {
                    setActiveView(VIEW_PROJECT);
                }
            }
        } else {
            if (auto v = getActiveView()) {
                v->handleUtilityPadPress(utilCol, pressed);
                v->updatePadLeds();
                v->updateEncoderLabels();
            }
            if (activeViewIdx == VIEW_STEP_SEQ) {
                auto& t = studio.tracks[studio.selTrack];
                if (t && pressed) {
                    if (utilCol == 0) { // Pad Z: Stretch -
                        t->stretchSequence(true);
                    } else if (utilCol == 1) { // Pad X: Compress +
                        t->compressSequence(true);
                    } else if (utilCol == 3) { // Pad V: Gen
                        t->runGeneration();
                    }
                }
            } else if (activeViewIdx == VIEW_INSTRUMENT) {
                if (pressed) {
                    if (utilCol == 0) { // Page Left
                        if (auto v = getActiveView()) v->changePage(-1);
                    } else if (utilCol == 1) { // Page Right
                        if (auto v = getActiveView()) v->changePage(1);
                    } else if (utilCol == 2) { // Octave -
                        if (gridState.utility.currentOctave > 0) gridState.utility.currentOctave--;
                    } else if (utilCol == 3) { // Octave +
                        if (gridState.utility.currentOctave < 7) gridState.utility.currentOctave++;
                    }
                }
            }
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
