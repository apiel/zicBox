#pragma once

#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class MasterView : public View {
private:
    float smoothVu[9] = { 0.0f };
    float peakHoldVal[9] = { 0.0f };
    float peakHoldDecay[9] = { 0.0f };
    float animTime = 0.0f;

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
                pad.selected = (selTrack->activeClipIdx == clipIdx);

                bool isPlaying = (selTrack->activeClipIdx == clipIdx && studio.isPlaying);
                bool isPending = (selTrack->pendingClipIdx == clipIdx);

                pad.active = isPlaying || (selTrack->activeClipIdx == clipIdx);
                pad.label = "C" + std::to_string(clipIdx + 1);

                if (isPlaying || selTrack->activeClipIdx == clipIdx) {
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
        Color grayColor = { 160, 160, 160, 255 };

        // Params not related to tracks are gray
        gridState.setEncoder(0, "BPM", studio.bpm, 20.0f, 300.0f, 1.0f, std::to_string((int)studio.bpm).c_str(), grayColor);
        gridState.setEncoder(1, "Master", studio.masterFx.volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, grayColor, "%");
        gridState.setEncoder(2, "Track", (float)(studio.selTrack + 1), 1.0f, 8.0f, 1.0f, ("T" + std::to_string(studio.selTrack + 1)).c_str(), grayColor);
        gridState.setEncoder(3, "Scene", 1.0f, 1.0f, 4.0f, 1.0f, "Scene 1", grayColor);

        // Params related to tracks keep track theme colors
        for (int i = 0; i < 8; ++i) {
            auto& t = studio.tracks[i];
            std::string label = "Vol T" + std::to_string(i + 1);
            gridState.setEncoder(4 + i, label.c_str(), t->volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, t->themeColor, "%");
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        animTime += 0.03f;
        Color selTrackColor = studio.tracks[studio.selTrack]->themeColor;

        d.text({ x + 4, y + 2 }, "VIEW: MASTER CONTROL (16 CLIPS, 8 MUTES, 8 TRIGGERS)", 8, { .color = selTrackColor, .font = &PoppinsLight_8 });

        // Calculate Master Peak
        float masterPeak = 0.0f;
        for (int i = 0; i < MAX_TRACKS; ++i) {
            float trkPeak = studio.tracks[i]->vumeter.load() * (studio.tracks[i]->isMuted ? 0.0f : studio.tracks[i]->volume);
            masterPeak = std::max(masterPeak, trkPeak);
        }
        masterPeak *= studio.masterFx.volume;

        // Low-pass smoothing & peak hold decay update for 9 channels (T1..T8 + Master)
        for (int ch = 0; ch < 9; ++ch) {
            float rawTgt = (ch < 8) ? (studio.tracks[ch]->isMuted ? 0.0f : studio.tracks[ch]->vumeter.load()) : masterPeak;
            float tgt = std::clamp(rawTgt, 0.0f, 1.0f);

            if (tgt > smoothVu[ch]) {
                smoothVu[ch] += (tgt - smoothVu[ch]) * 0.40f; // Responsive fast attack
            } else {
                smoothVu[ch] += (tgt - smoothVu[ch]) * 0.08f; // Smooth exponential decay
            }

            if (smoothVu[ch] >= peakHoldVal[ch]) {
                peakHoldVal[ch] = smoothVu[ch];
                peakHoldDecay[ch] = 0.5f;
            } else {
                if (peakHoldDecay[ch] > 0.0f) {
                    peakHoldDecay[ch] -= 0.03f;
                } else {
                    peakHoldVal[ch] = std::max(smoothVu[ch], peakHoldVal[ch] - 0.015f);
                }
            }
        }

        // Render 9 Channel Strips with Faded VU Meters (Compact height)
        int totalStrips = 9;
        int stripW = (w - 8) / totalStrips;
        int startX = x + 4;
        int startY = y + 16;
        int fH = 115; // Compact fader height to allow clear spacing for labels

        Color goldCol = Color { 255, 215, 0, 255 };

        for (int ch = 0; ch < 9; ++ch) {
            int colX = startX + ch * stripW;
            Color themeCol = (ch < 8) ? studio.tracks[ch]->themeColor : goldCol;
            float lvl = (ch < 8) ? studio.tracks[ch]->volume : studio.masterFx.volume;

            // Channel Label
            std::string labelStr = (ch < 8) ? ("T" + std::to_string(ch + 1)) : "MST";
            d.text({ colX + 4, startY + 4 }, labelStr.c_str(), 8, { .color = themeCol, .font = &PoppinsLight_8 });

            // Fader slot track for Volume
            int fX = colX + 4;
            int fY = startY + 18;
            int fW = 8;

            d.filledRect({ fX, fY }, { fW, fH }, { .color = Color { 16, 20, 28, 140 } });
            d.rect({ fX, fY }, { fW, fH }, { .color = Color { 40, 50, 70, 140 } });

            int fillH = (int)(fH * std::clamp(lvl, 0.0f, 1.0f));
            if (fillH > 0) {
                d.filledRect({ fX + 1, fY + fH - fillH }, { fW - 2, fillH }, { .color = Color { themeCol.r, themeCol.g, themeCol.b, 120 } });
            }

            // Fader handle / cap
            int capY = fY + fH - fillH - 1;
            capY = std::clamp(capY, fY, fY + fH - 2);
            d.filledRect({ fX - 1, capY }, { fW + 2, 3 }, { .color = Color { 240, 245, 255, 220 } });

            // Smooth Live VU Meter next to volume fader
            int vuX = colX + 16;
            int vuY = startY + 18;
            int vuW = 10;
            int vuH = fH;

            // Faded VU Track Container
            d.filledRect({ vuX, vuY }, { vuW, vuH }, { .color = Color { 12, 16, 24, 150 } });
            d.rect({ vuX, vuY }, { vuW, vuH }, { .color = Color { 35, 45, 65, 150 } });

            float sigVal = std::clamp(smoothVu[ch], 0.0f, 1.0f);
            int actVuH = (int)((vuH - 2) * sigVal);

            if (actVuH > 0) {
                // Continuous smooth faded gradient bar using theme color
                for (int py = 0; py < actVuH; py++) {
                    float normY = (float)py / (float)(vuH - 2);
                    uint8_t alpha = (uint8_t)(90 + normY * 90.0f); // Faded alpha look

                    Color segCol = {
                        (uint8_t)std::min(255, (int)(themeCol.r * (0.80f + normY * 0.35f))),
                        (uint8_t)std::min(255, (int)(themeCol.g * (0.80f + normY * 0.35f))),
                        (uint8_t)std::min(255, (int)(themeCol.b * (0.80f + normY * 0.35f))),
                        alpha
                    };

                    if (sigVal > 0.90f && py >= actVuH - 3) {
                        segCol = Color { 255, 80, 60, 230 };
                    }

                    d.line({ vuX + 1, vuY + vuH - 2 - py }, { vuX + vuW - 2, vuY + vuH - 2 - py }, { .color = segCol });
                }

                // Faint horizontal LED grid divisions (every 4px)
                for (int gy = vuY + vuH - 5; gy > vuY + 1; gy -= 4) {
                    d.line({ vuX + 1, gy }, { vuX + vuW - 2, gy }, { .color = Color { 10, 14, 20, 140 } });
                }
            }

            // Smooth Peak Hold Cap Indicator Line
            float pkVal = std::clamp(peakHoldVal[ch], 0.0f, 1.0f);
            if (pkVal > 0.02f) {
                int pkY = vuY + vuH - 2 - (int)((vuH - 3) * pkVal);
                pkY = std::clamp(pkY, vuY + 1, vuY + vuH - 2);
                Color pkCol = (pkVal > 0.90f) ? Color { 255, 80, 60, 230 } : Color { 245, 250, 255, 220 };
                d.line({ vuX + 1, pkY }, { vuX + vuW - 2, pkY }, { .color = pkCol });
            }

            // Numeric volume readout below
            std::stringstream ssL;
            ssL << (int)(lvl * 100) << "%";
            d.text({ colX + 4, fY + fH + 3 }, ssL.str(), 8, { .color = Color { 170, 185, 205, 220 }, .font = &PoppinsLight_8 });
        }

        // Real Master Output Waveform Display Box (Positioned below percentage labels with proper margin)
        int waveY = startY + 18 + fH + 18;
        int waveH = h - (waveY - y) - 4;
        if (waveH > 30) {
            int waveX = x + 4;
            int waveW = w - 8;

            d.filledRect({ waveX, waveY }, { waveW, waveH }, { .color = Color { 12, 16, 24, 220 } });
            d.rect({ waveX, waveY }, { waveW, waveH }, { .color = Color { 35, 45, 65, 220 } });

            d.text({ waveX + 6, waveY + 4 }, "MASTER AUDIO OUTPUT", 8, { .color = goldCol, .font = &PoppinsLight_8 });

            std::vector<float> masterHist;
            {
                std::lock_guard<std::mutex> hl(studio.masterHistoryMtx);
                masterHist.assign(studio.masterHistory.begin(), studio.masterHistory.end());
            }
            int histSize = (int)masterHist.size();

            int oscCenterY = waveY + waveH / 2 + 2;
            int oscAmp = (waveH - 20) / 2;

            // Draw center zero line
            d.line({ waveX + 2, oscCenterY }, { waveX + waveW - 2, oscCenterY }, { .color = Color { 30, 42, 60, 180 } });

            int prevX = waveX + 2;
            int prevY = oscCenterY;

            for (int px = 0; px < waveW - 4; px += 2) {
                float sampleVal = 0.0f;
                if (histSize > 0) {
                    int histIdx = std::clamp((int)(px * ((float)histSize / (float)waveW)), 0, histSize - 1);
                    sampleVal = masterHist[histIdx];
                }

                int ptY = oscCenterY - (int)(sampleVal * (float)oscAmp);
                ptY = std::clamp(ptY, waveY + 16, waveY + waveH - 4);
                int drawX = waveX + 2 + px;

                Color fillCol = Color { 255, 215, 0, 30 };
                if (ptY >= oscCenterY) {
                    d.filledRect({ drawX, oscCenterY }, { 2, ptY - oscCenterY + 1 }, { .color = fillCol });
                } else {
                    d.filledRect({ drawX, ptY }, { 2, oscCenterY - ptY + 1 }, { .color = fillCol });
                }

                d.line({ prevX, prevY }, { drawX, ptY }, { .color = Color { 255, 215, 0, 230 } });
                prevX = drawX;
                prevY = ptY;
            }
        }
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
        if (encoderId == 0) {
            studio.updateBpm(studio.bpm + delta);
        } else if (encoderId == 1) {
            studio.masterFx.volume = std::clamp(studio.masterFx.volume + delta * 0.05f, 0.0f, 1.0f);
        } else if (encoderId == 2) {
            studio.selTrack = std::clamp(studio.selTrack + delta, 0, MAX_TRACKS - 1);
            gridState.utility.activeTrack = studio.selTrack;
        } else if (encoderId >= 4 && encoderId <= 11) {
            int trk = encoderId - 4;
            auto& t = studio.tracks[trk];
            t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
        }

        updatePadLeds();
        updateEncoderLabels();
    }
};

