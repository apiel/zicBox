#pragma once

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class ClipLaunchView : public View {
public:
    ClipLaunchView() : View("CLIP LAUNCH MATRIX") {}

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void updatePadLeds() override
    {
        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int trkIdx = c;
                int clipIdx = r;
                auto& track = studio.tracks[trkIdx];
                auto& pad = gridState.pads[c][r];

                bool isPlaying = (track->activeClipIdx == clipIdx && studio.isPlaying);
                bool isPending = (track->pendingClipIdx == clipIdx);

                pad.active = isPlaying;
                pad.label = "T" + std::to_string(trkIdx + 1) + "-C" + std::to_string(clipIdx + 1);

                if (isPlaying) {
                    pad.color = track->themeColor;
                } else if (isPending) {
                    pad.color = { 255, 255, 200, 255 };
                } else {
                    pad.color = { (uint8_t)(track->themeColor.r / 4), (uint8_t)(track->themeColor.g / 4), (uint8_t)(track->themeColor.b / 4), 255 };
                }
            }
        }
    }

    void updateEncoderLabels() override
    {
        Color selTrackColor = studio.tracks[studio.selTrack]->themeColor;
        gridState.setEncoder(0, "BPM", studio.bpm, 20.0f, 300.0f, 1.0f, std::to_string((int)studio.bpm).c_str(), selTrackColor);
        gridState.setEncoder(1, "Scene", 1.0f, 1.0f, 4.0f, 1.0f, "Scene 1", selTrackColor);
        gridState.setEncoder(2, "Quantize", 1.0f, 1.0f, 4.0f, 1.0f, "1 Bar", selTrackColor);
        gridState.setEncoder(3, "Master", studio.masterFx.volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, selTrackColor, "%");

        for (int i = 0; i < 8; ++i) {
            auto& t = studio.tracks[i];
            std::string label = "Vol T" + std::to_string(i + 1);
            gridState.setEncoder(4 + i, label.c_str(), t->volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, t->themeColor, "%");
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        d.text({ x + 4, y + 2 }, "VIEW: MATRIX CLIP LAUNCHER (8 TRACKS x 4 SCENES)", 8, { .color = { 0, 255, 120, 255 }, .font = &PoppinsLight_8 });
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        if (!pressed) return;

        int trk = col;
        int clip = row;

        if (trk >= 0 && trk < MAX_TRACKS) {
            auto& t = studio.tracks[trk];
            t->pendingClipIdx = clip;
            t->activeClipIdx = clip;
        }

        updatePadLeds();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        if (encoderId == 1) {
            studio.updateBpm(studio.bpm + delta);
        } else if (encoderId == 4) {
            studio.masterFx.volume = std::clamp(studio.masterFx.volume + delta * 0.05f, 0.0f, 1.0f);
        } else if (encoderId >= 5 && encoderId <= 12) {
            int trk = encoderId - 5;
            auto& t = studio.tracks[trk];
            t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
        }

        updatePadLeds();
        updateEncoderLabels();
    }
};
