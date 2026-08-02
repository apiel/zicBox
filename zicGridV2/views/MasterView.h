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

    int activeClipPadHeld = -1; // -1 if none, 0..15 if clip pad held

    enum Row3Mode {
        ROW3_MODE_TRIG,
        ROW3_MODE_SCATTER
    };
    Row3Mode row3Mode = ROW3_MODE_TRIG;

public:
    MasterView() : View("MASTER CONTROL") {}

    void onActivate() override
    {
        if (studio.tracks[0]->chain.empty()) {
            studio.tracks[0]->chain = { 0, 0, 1, -1, 2 };
        }
        if (studio.tracks[1]->chain.empty()) {
            studio.tracks[1]->chain = { 0, 1, 1, 2, 2 };
        }
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

        // Row 2: Track 1-8 Chain Start / Stop Toggles
        for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
            auto& pad = gridState.pads[c][2];
            auto& trk = studio.tracks[c];

            pad.active = trk->chainPlaying;
            pad.label = trk->chainPlaying ? ("PLAY" + std::to_string(c + 1)) : ("CHN" + std::to_string(c + 1));
            pad.color = trk->chainPlaying ? Color { 100, 240, 120, 255 } : Color { (uint8_t)(trk->themeColor.r / 3), (uint8_t)(trk->themeColor.g / 3), (uint8_t)(trk->themeColor.b / 3), 255 };
        }

        // Row 3: Track 1-8 Triggers or Scatter FX
        for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
            auto& pad = gridState.pads[c][3];
            auto& trk = studio.tracks[c];

            if (row3Mode == ROW3_MODE_TRIG) {
                pad.label = "TRIG" + std::to_string(c + 1);
                pad.color = pad.pressed ? Color { 255, 255, 255, 255 } : trk->themeColor;
            } else {
                pad.label = "SCAT" + std::to_string(c + 1);
                pad.color = pad.pressed ? Color { 255, 255, 255, 255 } : Color { 255, 160, 40, 255 };
            }
        }

        // Row 3 Global Utility Pads (Cols 8..11 - Z, X, C, V)
        if (activeClipPadHeld >= 0) {
            gridState.pads[8][3].label = "NEXT";
            gridState.pads[9][3].label = "NOW";
            gridState.pads[10][3].label = "+CHAIN";
            gridState.pads[11][3].label = "";
        } else {
            gridState.pads[8][3].label = "POP";
            gridState.pads[9][3].label = "REST";
            gridState.pads[10][3].label = (selTrack->chainLoopMode == 0) ? "LOOP" : "HOLD";
            gridState.pads[11][3].label = (row3Mode == ROW3_MODE_TRIG) ? "TRIG" : "SCAT";
        }
    }

    void updateEncoderLabels() override
    {
        Color grayColor = { 160, 160, 160, 255 };
        auto& selTrack = studio.tracks[studio.selTrack];
        bool isShift = gridState.utility.shiftActive;

        gridState.setEncoder(0, "BPM", studio.bpm, 20.0f, 300.0f, 1.0f, std::to_string((int)studio.bpm).c_str(), grayColor);
        gridState.setEncoder(1, "Master", studio.masterFx.volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, grayColor, "%");
        gridState.setEncoder(2, "Track", (float)(studio.selTrack + 1), 1.0f, 8.0f, 1.0f, ("T" + std::to_string(studio.selTrack + 1)).c_str(), grayColor);

        if (isShift) {
            std::string modeStr = (selTrack->chainLoopMode == 0) ? "Loop" : "Hold";
            gridState.setEncoder(3, "Chain Mode", (float)selTrack->chainLoopMode, 0.0f, 1.0f, 1.0f, modeStr.c_str(), selTrack->themeColor);
        } else {
            std::string chainStatus = selTrack->chainPlaying ? "PLAYING" : (selTrack->chain.empty() ? "EMPTY" : "STOPPED");
            gridState.setEncoder(3, "Chain Play", (float)(selTrack->chainPlaying ? 1 : 0), 0.0f, 1.0f, 1.0f, chainStatus.c_str(), selTrack->themeColor);
        }

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
        auto& selTrack = studio.tracks[studio.selTrack];
        Color selTrackColor = selTrack->themeColor;

        d.text({ x + 4, y + 2 }, "VIEW: MASTER CONTROL (16 CLIPS, 8 CHAINS, TRIG/SCATTER)", 8, { .color = selTrackColor, .font = &PoppinsLight_8 });

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
                smoothVu[ch] += (tgt - smoothVu[ch]) * 0.40f;
            } else {
                smoothVu[ch] += (tgt - smoothVu[ch]) * 0.08f;
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

        // Render 9 Channel Strips (fH = 95px)
        int totalStrips = 9;
        int stripW = (w - 8) / totalStrips;
        int startX = x + 4;
        int startY = y + 16;
        int fH = 95; // Fader height for clear, prominent sliders and VU meters

        Color goldCol = Color { 255, 215, 0, 255 };

        for (int ch = 0; ch < 9; ++ch) {
            int colX = startX + ch * stripW;
            Color themeCol = (ch < 8) ? studio.tracks[ch]->themeColor : goldCol;
            float lvl = (ch < 8) ? studio.tracks[ch]->volume : studio.masterFx.volume;

            // Channel Label
            std::string labelStr = (ch < 8) ? ("T" + std::to_string(ch + 1)) : "MST";
            d.text({ colX + 4, startY + 2 }, labelStr.c_str(), 8, { .color = themeCol, .font = &PoppinsLight_8 });

            // Fader slot track for Volume
            int fX = colX + 4;
            int fY = startY + 14;
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
            int vuY = startY + 14;
            int vuW = 10;
            int vuH = fH;

            // Faded VU Track Container
            d.filledRect({ vuX, vuY }, { vuW, vuH }, { .color = Color { 12, 16, 24, 150 } });
            d.rect({ vuX, vuY }, { vuW, vuH }, { .color = Color { 35, 45, 65, 150 } });

            float sigVal = std::clamp(smoothVu[ch], 0.0f, 1.0f);
            int actVuH = (int)((vuH - 2) * sigVal);

            if (actVuH > 0) {
                for (int py = 0; py < actVuH; py++) {
                    float normY = (float)py / (float)(vuH - 2);
                    uint8_t alpha = (uint8_t)(90 + normY * 90.0f);

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
            }

            // Peak Hold Cap Line
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
            d.text({ colX + 4, fY + fH + 2 }, ssL.str(), 8, { .color = Color { 170, 185, 205, 220 }, .font = &PoppinsLight_8 });
        }

        // Master Output Waveform Box (waveH = 60px)
        int waveY = startY + 14 + fH + 14;
        int waveH = 60;
        int waveX = x + 4;
        int waveW = w - 8;

        if (waveH > 20) {
            d.filledRect({ waveX, waveY }, { waveW, waveH }, { .color = Color { 12, 16, 24, 220 } });
            d.rect({ waveX, waveY }, { waveW, waveH }, { .color = Color { 35, 45, 65, 220 } });

            d.text({ waveX + 6, waveY + 3 }, "MASTER AUDIO OUTPUT", 8, { .color = goldCol, .font = &PoppinsLight_8 });

            std::vector<float> masterHist;
            {
                std::lock_guard<std::mutex> hl(studio.masterHistoryMtx);
                masterHist.assign(studio.masterHistory.begin(), studio.masterHistory.end());
            }
            int histSize = (int)masterHist.size();

            int oscCenterY = waveY + waveH / 2 + 1;
            int oscAmp = (waveH - 14) / 2;

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
                ptY = std::clamp(ptY, waveY + 12, waveY + waveH - 3);
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

        // Multi-Track 8-Track Chain Overview Box (Tight, compact height)
        int chainBoxY = waveY + waveH + 4;
        int chainBoxH = h - (chainBoxY - y) - 4;

        if (chainBoxH >= 40) {
            d.filledRect({ waveX, chainBoxY }, { waveW, chainBoxH }, { .color = Color { 12, 16, 24, 230 } });
            d.rect({ waveX, chainBoxY }, { waveW, chainBoxH }, { .color = Color { 35, 45, 65, 230 } });

            d.text({ waveX + 6, chainBoxY + 3 }, "8-TRACK CLIP CHAINS OVERVIEW", 8, { .color = Color { 140, 200, 255, 255 }, .font = &PoppinsLight_8 });

            int rowH = 18; // Compact 18px per track row to eliminate wasted vertical space

            struct VisualItem {
                int startIdx;
                int endIdx;
                int clipIdx;
                int count;
            };

            for (int t = 0; t < MAX_TRACKS; ++t) {
                auto& trk = studio.tracks[t];
                int ry = chainBoxY + 16 + t * rowH;

                // Highlight selected track row
                if (t == studio.selTrack) {
                    d.filledRect({ waveX + 2, ry }, { waveW - 4, rowH - 1 }, { .color = Color { (uint8_t)(trk->themeColor.r / 6), (uint8_t)(trk->themeColor.g / 6), (uint8_t)(trk->themeColor.b / 6), 160 } });
                    d.filledRect({ waveX + 2, ry }, { 2, rowH - 1 }, { .color = trk->themeColor });
                }

                // Track Badge Label
                std::string tLabel = "T" + std::to_string(t + 1);
                d.text({ waveX + 7, ry + 4 }, tLabel.c_str(), 8, { .color = trk->themeColor, .font = &PoppinsLight_8 });

                // Play status icon badge
                if (trk->chainPlaying) {
                    d.text({ waveX + 22, ry + 4 }, "P", 8, { .color = Color { 100, 240, 120, 255 }, .font = &PoppinsLight_8 });
                } else {
                    d.text({ waveX + 22, ry + 4 }, "-", 8, { .color = Color { 100, 110, 125, 200 }, .font = &PoppinsLight_8 });
                }

                // Loop/Hold mode badge
                std::string modeChar = (trk->chainLoopMode == 0) ? "L" : "H";
                d.text({ waveX + 32, ry + 4 }, modeChar.c_str(), 8, { .color = Color { 140, 200, 255, 200 }, .font = &PoppinsLight_8 });

                // Render Chain Sequence Blocks
                int curX = waveX + 46;
                std::vector<VisualItem> items;
                if (!trk->chain.empty()) {
                    int curClip = trk->chain[0];
                    int count = 1;
                    int sIdx = 0;
                    for (size_t i = 1; i < trk->chain.size(); i++) {
                        if (trk->chain[i] == curClip) {
                            count++;
                        } else {
                            items.push_back({ sIdx, (int)i - 1, curClip, count });
                            curClip = trk->chain[i];
                            count = 1;
                            sIdx = (int)i;
                        }
                    }
                    items.push_back({ sIdx, (int)trk->chain.size() - 1, curClip, count });
                }

                if (items.empty()) {
                    d.text({ curX, ry + 2 }, "-- empty chain --", 8, { .color = Color { 80, 90, 105, 160 }, .font = &PoppinsLight_8 });
                } else {
                    for (size_t i = 0; i < items.size(); i++) {
                        const auto& item = items[i];
                        int itemW = (item.clipIdx == -1) ? (item.count > 1 ? 26 : 14) : (item.count > 1 ? 32 : 20);

                        if (curX + itemW > waveX + waveW - 4) break;

                        Color blockBg = (item.clipIdx == -1) ? Color { 65, 70, 80, 220 } : trk->themeColor;
                        bool isActiveStep = (trk->chainPlaying && trk->chainActiveIdx >= item.startIdx && trk->chainActiveIdx <= item.endIdx);

                        d.filledRect({ curX, ry + 1 }, { itemW, rowH - 3 }, { .color = blockBg });

                        if (isActiveStep) {
                            d.rect({ curX, ry + 1 }, { itemW, rowH - 3 }, { .color = Color { 255, 255, 255, 255 } });
                        } else {
                            d.rect({ curX, ry + 1 }, { itemW, rowH - 3 }, { .color = Color { 40, 50, 65, 180 } });
                        }

                        std::string lbl = (item.clipIdx == -1) ? "R" : ("C" + std::to_string(item.clipIdx + 1));
                        if (item.count > 1) lbl += "x" + std::to_string(item.count);

                        d.textCentered({ curX + itemW / 2, ry + 4 }, lbl.c_str(), 8, { .color = Color { 255, 255, 255, 255 }, .font = &PoppinsLight_8 });

                        curX += itemW + 2;
                    }
                }
            }
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        auto& selTrack = studio.tracks[studio.selTrack];

        if (row < 2) { // Rows 0 & 1: 16 Clips of Selected Track
            int clipIdx = row * DYNAMIC_PAD_COLS + col;
            if (pressed) {
                activeClipPadHeld = clipIdx;
                selTrack->pendingClipIdx = clipIdx;
                selTrack->activeClipIdx = clipIdx;
            } else {
                if (activeClipPadHeld == clipIdx) {
                    activeClipPadHeld = -1;
                }
            }
        } else if (row == 2) { // Row 2: Track 1-8 Chain Start / Stop Toggles
            if (!pressed) return;
            int trkIdx = col;
            if (trkIdx >= 0 && trkIdx < MAX_TRACKS) {
                auto& trk = studio.tracks[trkIdx];
                trk->chainPlaying = !trk->chainPlaying;
                if (trk->chainPlaying) {
                    trk->chainActiveIdx = 0;
                    trk->chainMuted = false;
                    if (!trk->chain.empty() && trk->chain[0] == -1) {
                        trk->chainMuted = true;
                    }
                } else {
                    trk->chainMuted = false;
                }
            }
        } else if (row == 3) { // Row 3: Trigger or Scatter for Track 1..8
            int trkIdx = col;
            if (trkIdx >= 0 && trkIdx < MAX_TRACKS) {
                auto& trk = studio.tracks[trkIdx];
                gridState.pads[col][3].pressed = pressed;
                if (row3Mode == ROW3_MODE_TRIG) {
                    if (pressed) {
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        trk->engine->noteOn(60, 0.9f);
                    } else {
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        trk->engine->noteOff(60);
                    }
                } else { // SCATTER Mode
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.masterFx.scatter.setModeActive(col % 8, pressed);
                }
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

    void handleUtilityPadPress(int utilCol, bool pressed) override
    {
        if (!pressed) return;
        auto& selTrack = studio.tracks[studio.selTrack];

        if (activeClipPadHeld >= 0) {
            // When a clip pad is pressed / held
            if (utilCol == 0) { // Pad Z: Load clip next
                selTrack->pendingClipIdx = activeClipPadHeld;
            } else if (utilCol == 1) { // Pad X: Load clip now
                selTrack->activeClipIdx = activeClipPadHeld;
                selTrack->pendingClipIdx = -1;
            } else if (utilCol == 2) { // Pad C: Add clip to chain
                selTrack->chain.push_back(activeClipPadHeld);
            }
        } else {
            // Default state (no clip pad held)
            if (utilCol == 0) { // Pad Z: Chain pop
                if (!selTrack->chain.empty()) {
                    selTrack->chain.pop_back();
                    if (selTrack->chain.empty()) {
                        selTrack->chainPlaying = false;
                        selTrack->chainMuted = false;
                    }
                }
            } else if (utilCol == 1) { // Pad X: Chain add rest
                selTrack->chain.push_back(-1);
            } else if (utilCol == 2) { // Pad C: Chain mode (loop / hold)
                selTrack->chainLoopMode = (selTrack->chainLoopMode == 0) ? 1 : 0;
            } else if (utilCol == 3) { // Pad V: Row 3 Mode (scatter / trigger)
                row3Mode = (row3Mode == ROW3_MODE_TRIG) ? ROW3_MODE_SCATTER : ROW3_MODE_TRIG;
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        auto& selTrack = studio.tracks[studio.selTrack];
        bool isShift = gridState.utility.shiftActive;

        if (encoderId == 0) {
            studio.updateBpm(studio.bpm + delta);
        } else if (encoderId == 1) {
            studio.masterFx.volume = std::clamp(studio.masterFx.volume + delta * 0.05f, 0.0f, 1.0f);
        } else if (encoderId == 2) {
            studio.selTrack = std::clamp(studio.selTrack + delta, 0, MAX_TRACKS - 1);
            gridState.utility.activeTrack = studio.selTrack;
        } else if (encoderId == 3) { // Chain Play / Mode encoder
            if (isShift) {
                if (delta != 0) {
                    selTrack->chainLoopMode = (selTrack->chainLoopMode == 0) ? 1 : 0;
                }
            } else {
                if (delta != 0) {
                    if (selTrack->chainPlaying) {
                        selTrack->chainPlaying = false;
                        selTrack->chainMuted = false;
                    } else if (!selTrack->chain.empty()) {
                        selTrack->chainPlaying = true;
                        selTrack->chainActiveIdx = 0;
                        selTrack->chainMuted = false;
                        int firstItem = selTrack->chain[0];
                        if (firstItem == -1) {
                            selTrack->chainMuted = true;
                        } else {
                            selTrack->activeClipIdx = firstItem;
                        }
                    }
                }
            }
        } else if (encoderId >= 4 && encoderId <= 11) {
            int trk = encoderId - 4;
            auto& t = studio.tracks[trk];
            t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
        }

        updatePadLeds();
        updateEncoderLabels();
    }
};

