#pragma once

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class MasterView : public View {
public:
    MasterView() : View("MASTER CONTROL") {}

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void updatePadLeds() override
    {
        int selTrk = studio.selTrack;
        auto& selTrack = studio.tracks[selTrk];

        // Rows 0 & 1: 16 Clips of Selected Track
        for (int r = 0; r < 2; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int clipIdx = r * DYNAMIC_PAD_COLS + c; // 0..15
                auto& pad = gridState.pads[c][r];
                pad.selected = false;

                bool isPlaying = (selTrack->activeClipIdx == clipIdx && studio.isPlaying);
                bool isPending = (selTrack->pendingClipIdx == clipIdx);

                pad.active = isPlaying;
                pad.label = "C" + std::to_string(clipIdx + 1);

                if (isPlaying) {
                    pad.color = selTrack->themeColor;
                } else if (isPending) {
                    pad.color = { 255, 255, 180, 255 };
                } else {
                    pad.color = { (uint8_t)(selTrack->themeColor.r / 4), (uint8_t)(selTrack->themeColor.g / 4), (uint8_t)(selTrack->themeColor.b / 4), 255 };
                }
            }
        }

        // Row 2: Track 1-8 Mute Toggles
        for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
            auto& pad = gridState.pads[c][2];
            auto& trk = studio.tracks[c];

            pad.active = trk->isMuted;
            pad.label = trk->isMuted ? "MUTED" + std::to_string(c + 1) : "MUTE" + std::to_string(c + 1);
            pad.color = trk->isMuted ? Color{ 255, 60, 60, 255 } : Color{ (uint8_t)(trk->themeColor.r / 2), (uint8_t)(trk->themeColor.g / 2), (uint8_t)(trk->themeColor.b / 2), 255 };
        }

        // Row 3: Track 1-8 Triggers
        for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
            auto& pad = gridState.pads[c][3];
            auto& trk = studio.tracks[c];

            pad.label = "TRIG" + std::to_string(c + 1);
            pad.color = pad.pressed ? Color{ 255, 255, 255, 255 } : trk->themeColor;
        }
    }

    void updateEncoderLabels() override
    {
        Color selTrackColor = studio.tracks[studio.selTrack]->themeColor;

        gridState.setEncoder(0, "BPM", studio.bpm, 20.0f, 300.0f, 1.0f, std::to_string((int)studio.bpm).c_str(), selTrackColor);
        gridState.setEncoder(1, "Master", studio.masterFx.volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, selTrackColor, "%");
        gridState.setEncoder(2, "Track", (float)(studio.selTrack + 1), 1.0f, 8.0f, 1.0f, ("T" + std::to_string(studio.selTrack + 1)).c_str(), selTrackColor);
        gridState.setEncoder(3, "Scene", 1.0f, 1.0f, 4.0f, 1.0f, "Scene 1", selTrackColor);

        for (int i = 0; i < 8; ++i) {
            auto& t = studio.tracks[i];
            std::string label = "Vol T" + std::to_string(i + 1);
            gridState.setEncoder(4 + i, label.c_str(), t->volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, t->themeColor, "%");
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        d.text({ x + 4, y + 2 }, "VIEW: MASTER CONTROL (16 CLIPS, 8 MUTES, 8 TRIGGERS)", 8, { .color = studio.tracks[studio.selTrack]->themeColor, .font = &PoppinsLight_8 });
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        if (row < 2) { // Rows 0 & 1: 16 Clips of Selected Track
            if (!pressed) return;
            int clipIdx = row * DYNAMIC_PAD_COLS + col;
            auto& t = studio.tracks[studio.selTrack];
            t->pendingClipIdx = clipIdx;
            t->activeClipIdx = clipIdx;
        } else if (row == 2) { // Row 2: Mute Track 1..8
            if (!pressed) return;
            int trkIdx = col;
            if (trkIdx >= 0 && trkIdx < MAX_TRACKS) {
                studio.tracks[trkIdx]->isMuted = !studio.tracks[trkIdx]->isMuted;
            }
        } else if (row == 3) { // Row 3: Trigger Track 1..8
            int trkIdx = col;
            if (trkIdx >= 0 && trkIdx < MAX_TRACKS) {
                auto& t = studio.tracks[trkIdx];
                gridState.pads[col][3].pressed = pressed;
                if (pressed) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    t->engine->noteOn(60, 0.9f);
                } else {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    t->engine->noteOff(60);
                }
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        if (encoderId == 1) {
            studio.updateBpm(studio.bpm + delta);
        } else if (encoderId == 2) {
            studio.masterFx.volume = std::clamp(studio.masterFx.volume + delta * 0.05f, 0.0f, 1.0f);
        } else if (encoderId == 3) {
            studio.selTrack = std::clamp(studio.selTrack + delta, 0, MAX_TRACKS - 1);
            gridState.utility.activeTrack = studio.selTrack;
        } else if (encoderId >= 5 && encoderId <= 12) {
            int trk = encoderId - 5;
            auto& t = studio.tracks[trk];
            t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
        }

        updatePadLeds();
        updateEncoderLabels();
    }
};
