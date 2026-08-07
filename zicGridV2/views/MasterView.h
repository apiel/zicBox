#pragma once

#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/project.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class MasterView : public View {
private:
    float smoothVu[10] = { 0.0f };
    float peakHoldVal[10] = { 0.0f };
    float peakHoldDecay[10] = { 0.0f };
    float animTime = 0.0f;

    int activeClipPadHeld = -1; // -1 if none, 0..15 if clip pad held
    bool chainPadHeld = false;
    bool clipBtnHeld = false;

    Clip clipCopyBuffer;
    bool clipCopyValid = false;

    enum Row3Mode {
        ROW3_MODE_TRIG,
        ROW3_MODE_SCATTER
    };
    Row3Mode row3Mode = ROW3_MODE_TRIG;

public:
    MasterView()
        : View("MASTER CONTROL")
    {
    }

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

    void onDeactivate() override
    {
        activeClipPadHeld = -1;
        chainPadHeld = false;
        clipBtnHeld = false;
        gridState.utility.activeClipPadHeld = -1;
        gridState.pads[8][3].label = "&icon::arrowLeft::filled";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };
        gridState.pads[9][3].label = "&icon::arrowRight::filled";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };
        gridState.pads[10][3].label = "Oct-";
        gridState.pads[10][3].color = { 100, 120, 255, 255 };
        gridState.pads[11][3].label = "Oct+";
        gridState.pads[11][3].color = { 100, 120, 255, 255 };
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

                bool isLoaded = (selTrack->activeClipIdx == clipIdx);
                bool isPending = (selTrack->pendingClipIdx == clipIdx);

                pad.selected = isLoaded;
                pad.active = isLoaded;
                pad.label = "C" + std::to_string(clipIdx + 1);

                auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
                bool blink = (nowMs / 250) % 2 == 0;

                if (isLoaded) {
                    pad.color = selTrack->themeColor;
                } else if (isPending) {
                    pad.color = blink ? Color { 255, 255, 180, 255 } : (selTrack->clips[clipIdx].saved ? selTrack->themeColor : Color { 35, 45, 60, 255 });
                } else if (selTrack->clips[clipIdx].saved) {
                    pad.color = selTrack->themeColor;
                } else {
                    pad.color = { 35, 45, 60, 255 };
                }
            }
        }

        // Row 2: Track 1-8 Chain Start / Stop Toggles
        for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
            auto& pad = gridState.pads[c][2];
            auto& trk = studio.tracks[c];

            pad.selected = false;
            pad.active = trk->chainPlaying;
            pad.label = "&icon::play::filled";
            pad.color = trk->themeColor;
        }

        // Row 3: Track 1-8 Triggers or Scatter FX
        for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
            auto& pad = gridState.pads[c][3];
            auto& trk = studio.tracks[c];

            pad.selected = false;
            pad.active = pad.pressed;
            if (row3Mode == ROW3_MODE_TRIG) {
                pad.label = "TRIG" + std::to_string(c + 1);
                pad.color = pad.pressed ? Color { 255, 255, 255, 255 } : trk->themeColor;
            } else {
                pad.label = "SCAT" + std::to_string(c + 1);
                pad.color = pad.pressed ? Color { 255, 255, 255, 255 } : Color { 255, 160, 40, 255 };
            }
        }

        // Row 3 Global Utility Pads (Cols 8..11 - Z, X, C, V)
        Color orangeCol = { 255, 160, 40, 255 };
        Color highlightCol = { 255, 200, 50, 255 };
        Color blueCol = { 100, 120, 255, 255 };
        Color brightBlueCol = { 150, 180, 255, 255 };

        if (chainPadHeld) {
            gridState.pads[8][3].label = "POP";
            gridState.pads[8][3].color = blueCol;
            gridState.pads[8][3].active = false;

            gridState.pads[9][3].label = "REST";
            gridState.pads[9][3].color = blueCol;
            gridState.pads[9][3].active = false;

            gridState.pads[10][3].label = (selTrack->chainLoopMode == 0) ? "LOOP" : "HOLD";
            gridState.pads[10][3].color = blueCol;
            gridState.pads[10][3].active = false;

            gridState.pads[11][3].label = "CHAIN";
            gridState.pads[11][3].color = brightBlueCol;
            gridState.pads[11][3].active = true;
        } else if (clipBtnHeld) {
            gridState.pads[8][3].label = "CLIP";
            gridState.pads[8][3].color = highlightCol;
            gridState.pads[8][3].active = true;

            gridState.pads[9][3].label = "DELETE";
            gridState.pads[9][3].color = { 255, 80, 80, 255 };
            gridState.pads[9][3].active = false;

            gridState.pads[10][3].label = "COPY";
            gridState.pads[10][3].color = orangeCol;
            gridState.pads[10][3].active = false;

            gridState.pads[11][3].label = "PASTE";
            gridState.pads[11][3].color = orangeCol;
            gridState.pads[11][3].active = false;
        } else {
            gridState.pads[8][3].label = "CLIP";
            gridState.pads[8][3].color = orangeCol;
            gridState.pads[8][3].active = false;

            gridState.pads[9][3].label = "";
            gridState.pads[9][3].color = { 35, 45, 60, 255 };
            gridState.pads[9][3].active = false;

            gridState.pads[10][3].label = "ADD";
            gridState.pads[10][3].color = blueCol;
            gridState.pads[10][3].active = false;

            gridState.pads[11][3].label = "CHAIN";
            gridState.pads[11][3].color = blueCol;
            gridState.pads[11][3].active = false;
        }
    }

    void updateEncoderLabels() override
    {
        Color grayColor = { 160, 160, 160, 255 };

        // Encoder 0: Master Volume
        gridState.setEncoder(0, "Master", studio.masterFx.volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, grayColor, "%");

        // Encoder 1: Compressor Threshold
        float thresh = studio.masterFx.compressor.threshold;
        std::string threshStr = std::to_string((int)thresh) + " dB";
        gridState.setEncoder(1, "Compressor", thresh, -60.0f, 0.0f, 1.0f, threshStr.c_str(), grayColor);

        // Encoder 2: Comp. Ratio
        float ratio = studio.masterFx.compressor.ratio;
        char ratioBuf[16];
        snprintf(ratioBuf, sizeof(ratioBuf), "%.1f:1", ratio);
        gridState.setEncoder(2, "Comp. Ratio", ratio, 1.0f, 20.0f, 0.5f, ratioBuf, grayColor);

        // Encoder 3: Empty slot (reserved for future use)
        gridState.setEncoder(3, "", 0.0f, 0.0f, 0.0f, 1.0f, "", grayColor);

        // Params 4..11: Track volumes T1..T8
        for (int i = 0; i < 8; ++i) {
            auto& t = studio.tracks[i];
            std::string label = "Vol T" + std::to_string(i + 1);
            gridState.setEncoder(4 + i, label.c_str(), t->volume * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, t->themeColor, "%");
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        animTime += 0.03f;
        updatePadLeds();
        auto& selTrack = studio.tracks[studio.selTrack];
        Color selTrackColor = selTrack->themeColor;

        // Calculate Master Peak & Compressor Gain Reduction
        float masterPeak = 0.0f;
        for (int i = 0; i < MAX_TRACKS; ++i) {
            float trkPeak = studio.tracks[i]->vumeter.load() * (studio.tracks[i]->isMuted ? 0.0f : studio.tracks[i]->volume);
            masterPeak = std::max(masterPeak, trkPeak);
        }
        masterPeak *= studio.masterFx.volume;

        float grDb = studio.masterFx.compressor.getGainReductionDb();
        float grPct = std::clamp(-grDb / 20.0f, 0.0f, 1.0f);

        // Low-pass smoothing & peak hold decay update for 10 channels (T1..T8 + Master + CMP)
        for (int ch = 0; ch < 10; ++ch) {
            float rawTgt = 0.0f;
            if (ch < 8) {
                rawTgt = studio.tracks[ch]->isMuted ? 0.0f : studio.tracks[ch]->vumeter.load();
            } else if (ch == 8) {
                rawTgt = masterPeak;
            } else {
                rawTgt = grPct;
            }
            float tgt = std::clamp(rawTgt, 0.0f, 1.0f);

            if (ch < 9) {
                if (tgt > smoothVu[ch]) {
                    smoothVu[ch] += (tgt - smoothVu[ch]) * 0.40f;
                } else {
                    smoothVu[ch] += (tgt - smoothVu[ch]) * 0.08f;
                }
            } else {
                if (tgt > smoothVu[9]) {
                    smoothVu[9] += (tgt - smoothVu[9]) * 0.50f;
                } else {
                    smoothVu[9] += (tgt - smoothVu[9]) * 0.10f;
                }
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

        // Render 10 Channel Strips (fH = 95px)
        int totalStrips = 10;
        int stripW = (w - 8) / totalStrips;
        int startX = x + 4;
        int startY = y + 4;
        int fH = 95; // Fader height for clear, prominent sliders and VU meters

        Color goldCol = Color { 255, 215, 0, 255 };
        Color orangeCol = Color { 255, 140, 0, 255 };

        for (int ch = 0; ch < 10; ++ch) {
            int colX = startX + ch * stripW;

            if (ch < 9) {
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
            } else {
                // Compressor Gain Reduction Meter Bar (Channel 9)
                d.text({ colX + 4, startY + 2 }, "CMP", 8, { .color = orangeCol, .font = &PoppinsLight_8 });

                int cmpX = colX + 8;
                int cmpY = startY + 14;
                int cmpW = 14;
                int cmpH = fH;

                // Faded Container Track
                d.filledRect({ cmpX, cmpY }, { cmpW, cmpH }, { .color = Color { 18, 14, 12, 180 } });
                d.rect({ cmpX, cmpY }, { cmpW, cmpH }, { .color = Color { 65, 45, 30, 180 } });

                float grVal = std::clamp(smoothVu[9], 0.0f, 1.0f);
                int grBarH = (int)((cmpH - 2) * grVal);

                if (grBarH > 0) {
                    for (int py = 0; py < grBarH; py++) {
                        float normY = (float)py / (float)(cmpH - 2);
                        uint8_t alpha = (uint8_t)(140 + normY * 115.0f);
                        Color grSegCol = Color { 255, (uint8_t)(140 - normY * 40.0f), 0, alpha };
                        d.line({ cmpX + 1, cmpY + cmpH - 2 - py }, { cmpX + cmpW - 2, cmpY + cmpH - 2 - py }, { .color = grSegCol });
                    }
                }

                // Peak Hold Cap Line for Gain Reduction (bottom to top)
                float pkVal = std::clamp(peakHoldVal[9], 0.0f, 1.0f);
                if (pkVal > 0.02f) {
                    int pkY = cmpY + cmpH - 2 - (int)((cmpH - 3) * pkVal);
                    pkY = std::clamp(pkY, cmpY + 1, cmpY + cmpH - 2);
                    d.line({ cmpX + 1, pkY }, { cmpX + cmpW - 2, pkY }, { .color = Color { 255, 230, 120, 255 } });
                }

                // Gain Reduction numeric readout (dB)
                std::stringstream ssGR;
                if (grDb < -0.1f) {
                    ssGR << "-" << (int)std::round(std::abs(grDb)) << "dB";
                } else {
                    ssGR << "0dB";
                }
                d.text({ colX + 2, cmpY + cmpH + 2 }, ssGR.str(), 8, { .color = Color { 255, 170, 100, 220 }, .font = &PoppinsLight_8 });
            }
        }

        // Master Output Waveform Box (waveH = 60px)
        int waveY = startY + 14 + fH + 14;
        int waveH = 80;
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
                    Icon icon(d);
                    icon.render("&icon::play::filled", { waveX + 20, ry + 5 }, { 6, 6 }, trk->themeColor);
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
                gridState.utility.activeClipPadHeld = clipIdx;

                // if (selTrack->chainPlaying) {
                //     // Case 1: Chain is PLAYING -> add clip to chain
                //     selTrack->chain.push_back(clipIdx);
                // } else 
                if (!studio.isPlaying) {
                    // Case 2: Sequencer is OFF -> load current clip immediately
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    loadClip(*selTrack, clipIdx);
                    selTrack->pendingClipIdx = -1;
                } else {
                    // Case 3: Sequencer is PLAYING & Chain is NOT PLAYING
                    if (selTrack->pendingClipIdx == clipIdx) {
                        // Pressing it again will play the clip right away!
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        loadClip(*selTrack, clipIdx);
                        selTrack->pendingClipIdx = -1;
                    } else {
                        // Will start to play the clip at the next loop
                        selTrack->pendingClipIdx = clipIdx;
                    }
                }
            } else {
                if (activeClipPadHeld == clipIdx) {
                    activeClipPadHeld = -1;
                    gridState.utility.activeClipPadHeld = -1;
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
        auto& selTrack = studio.tracks[studio.selTrack];

        if (chainPadHeld || (utilCol == 3 && pressed)) {
            // CHAIN mode active (or CHAIN button pressed/released)
            if (utilCol == 3) {
                chainPadHeld = pressed;
            }
            if (pressed && chainPadHeld) {
                if (utilCol == 0) { // POP
                    if (!selTrack->chain.empty()) {
                        selTrack->chain.pop_back();
                        if (selTrack->chain.empty()) {
                            selTrack->chainPlaying = false;
                            selTrack->chainMuted = false;
                        }
                    }
                } else if (utilCol == 1) { // REST
                    selTrack->chain.push_back(-1);
                } else if (utilCol == 2) { // LOOP / HOLD toggle
                    selTrack->chainLoopMode = (selTrack->chainLoopMode == 0) ? 1 : 0;
                }
            }
        } else if (clipBtnHeld || (utilCol == 0 && pressed)) {
            // CLIP mode active (or CLIP button pressed/released)
            if (utilCol == 0) {
                clipBtnHeld = pressed;
            }
            if (pressed && clipBtnHeld) {
                int cIdx = selTrack->activeClipIdx;
                if (utilCol == 1) { // DELETE
                    if (cIdx >= 0 && cIdx < MAX_CLIP_COUNT) {
                        selTrack->clips[cIdx].saved = false;
                        selTrack->clips[cIdx].sequence.clear();
                        selTrack->clips[cIdx].paramValues.clear();
                        if (selTrack->pendingClipIdx == cIdx) {
                            selTrack->pendingClipIdx = -1;
                        }
                        bool dummy = true;
                        UiMessage::show("CLIP DELETED", dummy, 1500);
                    }
                } else if (utilCol == 2) { // COPY
                    if (cIdx >= 0 && cIdx < MAX_CLIP_COUNT) {
                        saveClip(*selTrack, cIdx);
                        clipCopyBuffer = selTrack->clips[cIdx];
                        clipCopyValid = true;
                        bool dummy = true;
                        UiMessage::show("CLIP COPIED", dummy, 1500);
                    }
                } else if (utilCol == 3) { // PASTE
                    if (cIdx >= 0 && cIdx < MAX_CLIP_COUNT && clipCopyValid) {
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        selTrack->clips[cIdx] = clipCopyBuffer;
                        selTrack->clips[cIdx].saved = true;
                        loadClip(*selTrack, cIdx);
                        bool dummy = true;
                        UiMessage::show("CLIP PASTED", dummy, 1500);
                    }
                }
            }
        } else {
            // Default State
            if (pressed) {
                if (utilCol == 0) { // CLIP button press
                    clipBtnHeld = true;
                } else if (utilCol == 1) { // Removed LOAD pad (empty)
                    // Pad left empty
                } else if (utilCol == 2) { // ADD
                    selTrack->chain.push_back(selTrack->activeClipIdx);
                } else if (utilCol == 3) { // CHAIN button press
                    chainPadHeld = true;
                }
            } else {
                if (utilCol == 0) clipBtnHeld = false;
                if (utilCol == 3) chainPadHeld = false;
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        if (encoderId == 0) {
            studio.masterFx.volume = std::clamp(studio.masterFx.volume + delta * 0.05f, 0.0f, 1.0f);
        } else if (encoderId == 1) {
            studio.masterFx.compressor.threshold = std::clamp(studio.masterFx.compressor.threshold + delta * 1.0f, -60.0f, 0.0f);
        } else if (encoderId == 2) {
            studio.masterFx.compressor.ratio = std::clamp(studio.masterFx.compressor.ratio + delta * 0.5f, 1.0f, 20.0f);
        } else if (encoderId == 3) {
            // Slot left empty for future use
        } else if (encoderId >= 4 && encoderId <= 11) {
            int trk = encoderId - 4;
            auto& t = studio.tracks[trk];
            t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
        }

        updatePadLeds();
        updateEncoderLabels();
    }
};
