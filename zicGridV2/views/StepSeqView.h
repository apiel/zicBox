#pragma once

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class StepSeqView : public View {
private:
    int stepPage = 0;

public:
    StepSeqView() : View("STEP SEQUENCER") {}

    std::pair<int, int> getViewPageInfo() const override
    {
        return { stepPage + 1, 2 };
    }

    void changePage(int delta) override
    {
        stepPage = (stepPage + delta + 2) % 2;
    }

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void onDeactivate() override
    {
        // Restore standard utility row 3 pad labels when leaving StepSeqView
        gridState.pads[8][3].label = "&icon::arrowLeft::filled";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };
        gridState.pads[9][3].label = "&icon::arrowRight::filled";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };
        gridState.pads[10][3].label = "Oct-";
        gridState.pads[10][3].color = { 100, 120, 255, 255 };
        gridState.pads[11][3].label = "Oct+";
        gridState.pads[11][3].color = { 100, 120, 255, 255 };

        for (int i = 0; i < TOTAL_ENCODERS; ++i) {
            gridState.setEncoderBg(i, { 0, 0, 0, 0 });
        }

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < PAD_COLS; ++c) {
                gridState.pads[c][r].selected = false;
                gridState.pads[c][r].pressed = false;
                gridState.pads[c][r].active = false;
            }
        }
    }

    void updatePadLeds() override
    {
        // Utility Row 3 pads on StepSeqView: Pad Z is Str-, Pad X is Str+, Pad C is empty, Pad V is Gen.
        gridState.pads[8][3].label = "Str-";
        gridState.pads[8][3].color = { 255, 160, 40, 255 };
        gridState.pads[9][3].label = "Str+";
        gridState.pads[9][3].color = { 255, 160, 40, 255 };
        gridState.pads[10][3].label = "";
        gridState.pads[10][3].color = { 35, 45, 60, 255 };
        gridState.pads[11][3].label = "Gen.";
        gridState.pads[11][3].color = { 255, 160, 40, 255 };

        int trkIdx = studio.selTrack;
        if (trkIdx < 0 || trkIdx >= MAX_TRACKS) return;

        auto& track = studio.tracks[trkIdx];
        uint32_t currentStep = studio.currentStep % SEQ_STEPS;
        int selStep = (studio.selStep >= 0) ? studio.selStep : 0;

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int stepIdx = (stepPage * 32) + (r * DYNAMIC_PAD_COLS + c);
                auto& pad = gridState.pads[c][r];

                if (stepIdx < (int)track->sequence.size()) {
                    bool isActive = track->sequence[stepIdx].active;
                    bool isSelected = (stepIdx == selStep);
                    pad.active = isActive;
                    pad.selected = isSelected;
                    pad.pressed = false;
                    pad.label = std::to_string(stepIdx + 1);

                    if (studio.isPlaying && stepIdx == (int)currentStep) {
                        pad.color = { 255, 255, 255, 255 };
                    } else if (isActive) {
                        pad.color = track->themeColor;
                    } else {
                        pad.color = { 35, 45, 60, 255 };
                    }
                }
            }
        }
    }

    void updateEncoderLabels() override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        Color c = t->themeColor;
        Color genBg = { 35, 45, 65, 255 }; // Distinct background color for generator parameters

        // Encoders 0..3: Sequencer Timing & Global Track Params
        gridState.setEncoder(0, "BPM", studio.bpm, 20.0f, 300.0f, 1.0f, std::to_string((int)studio.bpm).c_str(), c);
        gridState.setEncoderBg(0, { 0, 0, 0, 0 });

        gridState.setEncoder(1, "Track", (float)(trk + 1), 1.0f, 8.0f, 1.0f, ("T" + std::to_string(trk + 1)).c_str(), c);
        gridState.setEncoderBg(1, { 0, 0, 0, 0 });

        int selStep = (studio.selStep >= 0) ? studio.selStep : 0;
        auto& step = t->sequence[selStep];

        gridState.setEncoder(2, "Length", step.len, 0.25f, 16.0f, 0.25f, nullptr, c);
        gridState.setEncoderBg(2, { 0, 0, 0, 0 });

        gridState.setEncoder(3, "Stretch", (float)t->genLen, 4.0f, 128.0f, 1.0f, std::to_string(t->genLen).c_str(), c);
        gridState.setEncoderBg(3, { 0, 0, 0, 0 });

        // Encoders 4..7: Step Params
        gridState.setEncoder(4, "Step", (float)(selStep + 1), 1.0f, 64.0f, 1.0f, ("S" + std::to_string(selStep + 1)).c_str(), c);
        gridState.setEncoderBg(4, { 0, 0, 0, 0 });

        gridState.setEncoder(5, "Note", (float)step.note, 12.0f, 108.0f, 1.0f, std::to_string(step.note).c_str(), c);
        gridState.setEncoderBg(5, { 0, 0, 0, 0 });

        gridState.setEncoder(6, "Velo", step.velocity * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
        gridState.setEncoderBg(6, { 0, 0, 0, 0 });

        gridState.setEncoder(7, "Prob", step.condition * 100.0f, 0.0f, 100.0f, 10.0f, nullptr, c, "%");
        gridState.setEncoderBg(7, { 0, 0, 0, 0 });

        // Encoders 8..11: Generator Engine & Generator Params (with distinct background color)
        const char* genEngineName = t->genEngine == 0 ? "Kick" : t->genEngine == 1 ? "Bass" : "Drum";
        gridState.setEncoder(8, "Gen", (float)t->genEngine, 0.0f, 2.0f, 1.0f, genEngineName, c);
        gridState.setEncoderBg(8, genBg);

        if (t->genEngine == 0) { // Kick
            gridState.setEncoder(9, "Tribe Vel", t->genParams[0] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(9, genBg);

            gridState.setEncoder(10, "Ghost Dens", t->genParams[1] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(10, genBg);

            gridState.setEncoder(11, "End Rumble", t->genParams[2] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(11, genBg);
        } else if (t->genEngine == 1) { // Bass
            const char* scaleName = t->genParams[0] < 0.25f ? "Min Pent" : t->genParams[0] < 0.5f ? "Nat Minor" : t->genParams[0] < 0.75f ? "Dorian" : "Acid Chrom";
            gridState.setEncoder(9, "Scale", t->genParams[0] * 100.0f, 0.0f, 100.0f, 5.0f, scaleName, c);
            gridState.setEncoderBg(9, genBg);

            gridState.setEncoder(10, "Density", t->genParams[1] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(10, genBg);

            gridState.setEncoder(11, "Slide/Oct", t->genParams[2] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(11, genBg);
        } else { // Drum
            const char* styleName = t->genParams[0] < 0.2f ? "Snare" : t->genParams[0] < 0.4f ? "Hat" : t->genParams[0] < 0.6f ? "Clap" : t->genParams[0] < 0.8f ? "Perc" : "Mixed";
            gridState.setEncoder(9, "Style", t->genParams[0] * 100.0f, 0.0f, 100.0f, 5.0f, styleName, c);
            gridState.setEncoderBg(9, genBg);

            gridState.setEncoder(10, "Ghost/Flam", t->genParams[1] * 100.0f, 0.0f, 100.0f, 5.0f, nullptr, c, "%");
            gridState.setEncoderBg(10, genBg);

            const char* intName = t->genParams[2] < 0.3f ? "Sparse" : t->genParams[2] < 0.7f ? "Medium" : "Dense";
            gridState.setEncoder(11, "Interval", t->genParams[2] * 100.0f, 0.0f, 100.0f, 5.0f, intName, c);
            gridState.setEncoderBg(11, genBg);
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        // 2. 8-Track 64-Step Sequence Overview (zicXYv2 style with Note pitch & Length)
        int seqTopY = y + 6;
        int badgeW = 28;
        int gridX = x + badgeW + 2;
        int gridW = w - badgeW - 2;
        float stepW = (float)gridW / (float)SEQ_STEPS;
        int rowH = 34;
        int rulerH = 14;

        // Step Ruler & Bar Numbers (Bar 1..4, Page highlights)
        d.filledRect({ gridX, seqTopY }, { gridW, rulerH - 2 }, { .color = { 20, 26, 36, 255 } });

        // Highlight active page steps range (P1: steps 0..31, P2: steps 32..63) in ruler
        int pageStartStep = stepPage * 32;
        int pageStartX = gridX + (int)(pageStartStep * stepW);
        int pageW = (int)(32 * stepW);
        d.filledRect({ pageStartX, seqTopY + rulerH - 4 }, { pageW, 2 }, { .color = studio.tracks[studio.selTrack]->themeColor });

        for (int b = 0; b < 4; ++b) {
            int barStep = b * 16;
            int bx = gridX + (int)(barStep * stepW);
            std::string barStr = "B" + std::to_string(b + 1);
            d.text({ bx + 2, seqTopY + 1 }, barStr, 8, { .color = { 130, 145, 170, 255 }, .font = &PoppinsLight_8 });
        }

        // Selected step indicator on the ruler
        if (studio.selStep >= 0 && studio.selStep < SEQ_STEPS) {
            int selSx = gridX + (int)(studio.selStep * stepW);
            int selNextSx = gridX + (int)((studio.selStep + 1) * stepW);
            int selSw = std::max(2, selNextSx - selSx);
            d.filledRect({ selSx, seqTopY + rulerH - 6 }, { selSw, 4 }, { .color = { 255, 255, 255, 255 } });
        }

        // Render 8 Track Sequence Rows
        int tracksStartY = seqTopY + rulerH;
        for (int t = 0; t < MAX_TRACKS; ++t) {
            if (t >= (int)studio.tracks.size() || !studio.tracks[t]) break;
            auto& trk = studio.tracks[t];
            int trkY = tracksStartY + t * rowH;

            // Track Label Badge
            bool isSelTrack = (t == studio.selTrack);
            Color badgeBg = isSelTrack ? trk->themeColor : Color { (uint8_t)(trk->themeColor.r / 3), (uint8_t)(trk->themeColor.g / 3), (uint8_t)(trk->themeColor.b / 3), 255 };
            d.filledRect({ x, trkY }, { badgeW, rowH - 2 }, { .color = badgeBg });

            if (isSelTrack) {
                d.rect({ x, trkY }, { badgeW, rowH - 2 }, { .color = { 255, 255, 255, 255 } });
            }

            std::string trkName = "T" + std::to_string(t + 1);
            Color textCol = isSelTrack ? getContrastTextColor(badgeBg) : trk->themeColor;
            d.textCentered({ x + badgeW / 2, trkY + (rowH - 2) / 2 - 4 }, trkName, 8, { .color = textCol, .font = &PoppinsLight_8 });

            // Pass 1: Draw step background boxes for all 64 steps
            for (int s = 0; s < SEQ_STEPS; ++s) {
                int sx = gridX + (int)(s * stepW);
                int nextSx = gridX + (int)((s + 1) * stepW);
                int sw = nextSx - sx - 1;
                if (sw < 1) sw = 1;

                bool isStepOnCurrentPage = (s >= pageStartStep && s < pageStartStep + 32);
                Color laneBg = isStepOnCurrentPage ? Color { 20, 26, 38, 255 } : Color { 13, 17, 24, 255 };
                d.filledRect({ sx, trkY }, { sw, rowH - 2 }, { .color = laneBg });
            }

            // Draw expanded background highlight for selected step on active track (with loop boundary wrapping)
            if (isSelTrack && studio.selStep >= 0 && studio.selStep < SEQ_STEPS) {
                int selS = studio.selStep;
                int selSx = gridX + (int)(selS * stepW);
                int selNextSx = gridX + (int)((selS + 1) * stepW);
                int selSw = std::max(1, selNextSx - selSx - 1);

                const auto& selStepObj = trk->sequence[selS];
                int totalLenPx = selSw;
                if (selStepObj.active) {
                    totalLenPx = std::max(selSw, (int)std::round(selStepObj.len * stepW));
                }

                int maxRight = gridX + gridW;
                int remaining = totalLenPx;
                int curX = selSx;

                while (remaining > 0) {
                    int space = maxRight - curX;
                    if (space <= 0) break;
                    int drawLen = std::min(remaining, space);
                    d.filledRect({ curX, trkY }, { drawLen, rowH - 2 }, { .color = Color { 50, 68, 98, 255 } });
                    remaining -= drawLen;
                    curX = gridX; // wrap around to grid start
                    if (drawLen == 0) break;
                    if (remaining > gridW) {
                        d.filledRect({ gridX, trkY }, { gridW, rowH - 2 }, { .color = Color { 50, 68, 98, 255 } });
                        remaining -= gridW;
                    }
                }
            }

            // Draw beat lines every 4 steps
            for (int s = 0; s < SEQ_STEPS; ++s) {
                if (s % 4 == 0) {
                    int sx = gridX + (int)(s * stepW);
                    Color beatCol = (s % 16 == 0) ? Color { 110, 125, 145, 120 } : Color { 50, 60, 78, 70 };
                    d.line({ sx - 1, trkY }, { sx - 1, trkY + rowH - 3 }, { .color = beatCol });
                }
            }

            // Pass 2: Render active note heads (WHITE) and note duration lines (velocity scaled, loop boundary wrapped) ON TOP of backgrounds
            for (int s = 0; s < SEQ_STEPS; ++s) {
                const auto& stepObj = trk->sequence[s];
                if (!stepObj.active) continue;

                int sx = gridX + (int)(s * stepW);

                // Map note pitch (24..96) to vertical pitch position within rowH (32px available)
                float clampedNote = std::clamp((float)stepObj.note, 24.0f, 96.0f);
                float nm = 1.0f - (clampedNote - 24.0f) / 72.0f; // 0.0 top, 1.0 bottom
                int marginY = 4;
                int ny = (trkY + rowH - 2 - marginY) - (int)(nm * (float)(rowH - 2 - marginY * 2));

                // Scale note line color/brightness by velocity
                float v = std::clamp(stepObj.velocity, 0.0f, 1.0f);
                Color lineCol = trk->themeColor;
                lineCol.r = (uint8_t)std::min(255, (int)(lineCol.r * (0.4f + 0.6f * v)));
                lineCol.g = (uint8_t)std::min(255, (int)(lineCol.g * (0.4f + 0.6f * v)));
                lineCol.b = (uint8_t)std::min(255, (int)(lineCol.b * (0.4f + 0.6f * v)));

                // Length line in pixels (stepObj.len * stepW) with loop boundary wrapping
                int totalLenPx = std::max(3, (int)std::round(stepObj.len * stepW));
                int maxRight = gridX + gridW;
                int remaining = totalLenPx;
                int curX = sx;
                bool isFirstSegment = true;

                while (remaining > 0) {
                    int space = maxRight - curX;
                    if (space <= 0) break;
                    int drawLen = std::min(remaining, space);

                    // Draw 2px thick note duration line segment
                    d.filledRect({ curX, ny - 1 }, { drawLen, 2 }, { .color = lineCol });

                    if (isFirstSegment) {
                        // Active step note head: WHITE scaled by velocity at start position
                        uint8_t whiteV = (uint8_t)std::min(255, (int)(180 + 75 * v));
                        Color headCol = { whiteV, whiteV, whiteV, 255 };
                        d.filledCircle({ curX + 2, ny }, 2, { .color = headCol });
                        isFirstSegment = false;
                    }

                    remaining -= drawLen;
                    curX = gridX; // wrap back to grid start (left edge)
                    if (drawLen == 0) break;
                    if (remaining > gridW) {
                        d.filledRect({ gridX, ny - 1 }, { gridW, 2 }, { .color = lineCol });
                        remaining -= gridW;
                    }
                }
            }
        }

        // Live Playhead Line during playback
        if (studio.isPlaying) {
            int curStep = studio.currentStep % SEQ_STEPS;
            int px = gridX + (int)(curStep * stepW);
            d.line({ px, tracksStartY }, { px, tracksStartY + 8 * rowH - 3 }, { .color = { 255, 255, 255, 255 } });
        }

        // 3. Dual Cyberpunk Visualizer (Bottom ~76px space)
        int animY = tracksStartY + 8 * rowH + 4;
        int animH = h - (animY - y) - 2; // ~74-76px height
        if (animH >= 30) {
            Color selTheme = studio.tracks[studio.selTrack]->themeColor;

            // Panel A: Live Audio Oscilloscope & Wave Stream (Left Panel, Width 330px)
            int waveW = 330;
            d.filledRect({ x, animY }, { waveW, animH }, { .color = { 14, 18, 26, 255 } });
            d.rect({ x, animY }, { waveW, animH }, { .color = { 35, 45, 60, 255 } });

            d.text({ x + 6, animY + 4 }, "WAVE / OSCILLOSCOPE", 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });

            // Render Oscilloscope / Synth Waveform
            int waveCenterY = animY + animH / 2 + 2;
            int waveAmplitude = (animH - 24) / 2;
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            float idlePhase = (nowMs % 3000) / 3000.0f * 2.0f * M_PI;

            std::vector<float> trkHistory;
            {
                std::lock_guard<std::mutex> hl(studio.tracks[studio.selTrack]->historyMtx);
                const auto& dequeHist = studio.tracks[studio.selTrack]->history;
                trkHistory.assign(dequeHist.begin(), dequeHist.end());
            }
            int historySize = (int)trkHistory.size();

            int prevX = x + 2;
            int prevY = waveCenterY;

            for (int px = 0; px < waveW - 4; px += 2) {
                float sampleVal = 0.0f;

                if (studio.isPlaying && historySize > 0) {
                    int histIdx = std::clamp((int)(px * ((float)historySize / (float)waveW)), 0, historySize - 1);
                    sampleVal = trkHistory[histIdx];
                } else {
                    // Animated idle wave breathing continuously
                    float normX = (float)px / (float)waveW;
                    sampleVal = 0.35f * std::sin(normX * 4.0f * M_PI + idlePhase) + 0.15f * std::sin(normX * 10.0f * M_PI - idlePhase * 1.5f);
                }

                int ptY = waveCenterY - (int)(sampleVal * (float)waveAmplitude);
                ptY = std::clamp(ptY, animY + 16, animY + animH - 4);

                int drawX = x + 2 + px;

                // Translucent wave fill under curve
                Color areaCol = { selTheme.r, selTheme.g, selTheme.b, 25 };
                if (ptY >= waveCenterY) {
                    d.filledRect({ drawX, waveCenterY }, { 2, ptY - waveCenterY + 1 }, { .color = areaCol });
                } else {
                    d.filledRect({ drawX, ptY }, { 2, waveCenterY - ptY + 1 }, { .color = areaCol });
                }

                // Vibrant wave line
                d.line({ prevX, prevY }, { drawX, ptY }, { .color = selTheme });
                prevX = drawX;
                prevY = ptY;
            }

            // Playhead Pulse line across wave stream
            if (studio.isPlaying) {
                int curStep = studio.currentStep % 32;
                int sweepX = x + 2 + (int)(curStep * ((waveW - 4) / 32.0f));
                d.line({ sweepX, animY + 16 }, { sweepX, animY + animH - 4 }, { .color = { 255, 255, 255, 200 } });
            }

            // Panel B: Animated Circular 16-Step Beat Radar Clock (Right Panel, Width w - 334)
            int radarX = x + 334;
            int radarW = w - 334;
            d.filledRect({ radarX, animY }, { radarW, animH }, { .color = { 14, 18, 26, 255 } });
            d.rect({ radarX, animY }, { radarW, animH }, { .color = { 35, 45, 60, 255 } });

            int centerX = radarX + radarW / 2;
            int centerY = animY + animH / 2;
            int radius = std::min((radarW - 16) / 2, (animH - 16) / 2);

            // 16 Step Radial Dots around dial
            const auto& selSeq = studio.tracks[studio.selTrack]->sequence;
            for (int s = 0; s < 16; ++s) {
                float angle = (s * 360.0f / 16.0f) - 90.0f;
                float rad = angle * M_PI / 180.0f;
                int dotX = centerX + (int)(radius * std::cos(rad));
                int dotY = centerY + (int)(radius * std::sin(rad));

                int stepOffset = (stepPage * 32) + s;
                bool isStepActive = (stepOffset < (int)selSeq.size() && selSeq[stepOffset].active);

                if (isStepActive) {
                    d.filledCircle({ dotX, dotY }, 2, { .color = selTheme });
                } else {
                    d.filledCircle({ dotX, dotY }, 1, { .color = { 50, 65, 85, 255 } });
                }
            }

            // Rotating Radar Sweep Arm
            int current16 = studio.isPlaying ? (studio.currentStep % 16) : 0;
            float sweepAngle = (current16 * 360.0f / 16.0f) - 90.0f;
            float sweepRad = sweepAngle * M_PI / 180.0f;
            int sweepX = centerX + (int)(radius * std::cos(sweepRad));
            int sweepY = centerY + (int)(radius * std::sin(sweepRad));

            d.line({ centerX, centerY }, { sweepX, sweepY }, { .color = { 255, 255, 255, 220 } });
            d.filledCircle({ centerX, centerY }, 2, { .color = { 255, 255, 255, 255 } });

            // Center Badge
            std::string radarBadge = "S" + std::to_string(current16 + 1);
            d.textCentered({ centerX, centerY - 4 }, radarBadge, 8, { .color = { 200, 215, 240, 255 }, .font = &PoppinsLight_8 });
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        if (!pressed) return;

        if (col < 0 || col >= DYNAMIC_PAD_COLS || row < 0 || row >= PAD_ROWS) return;

        int stepIdx = (stepPage * 32) + (row * DYNAMIC_PAD_COLS + col);
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        if (stepIdx >= (int)t->sequence.size()) return;

        // Select step
        studio.selStep = stepIdx;

        // If Shift is NOT held, toggle the step active status
        if (!gridState.utility.shiftActive) {
            t->sequence[stepIdx].active = !t->sequence[stepIdx].active;
            if (t->sequence[stepIdx].active) {
                std::lock_guard<std::mutex> lock(studio.audioMutex);
                if (t->engine) {
                    t->engine->noteOn(t->sequence[stepIdx].note, t->sequence[stepIdx].velocity);
                }
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        int selStep = (studio.selStep >= 0) ? studio.selStep : 0;
        auto& step = t->sequence[selStep];

        switch (encoderId) {
        case 1:
            studio.updateBpm(studio.bpm + delta);
            break;
        case 2:
            studio.selTrack = std::clamp(studio.selTrack + delta, 0, MAX_TRACKS - 1);
            gridState.utility.activeTrack = studio.selTrack;
            break;
        case 3:
            step.len = std::clamp(step.len + delta * 0.25f, 0.25f, 16.0f);
            break;
        case 4:
            if (delta < 0) {
                t->stretchSequence(true);
            } else if (delta > 0) {
                t->compressSequence(true);
            }
            break;
        case 5:
            studio.selStep = std::clamp(selStep + delta, 0, SEQ_STEPS - 1);
            break;
        case 6:
            step.note = std::clamp(step.note + delta, 12, 108);
            break;
        case 7:
            step.velocity = std::clamp(step.velocity + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 8:
            step.condition = std::clamp(step.condition + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 9:
            t->genEngine = std::clamp(t->genEngine + delta, 0, 2);
            break;
        case 10:
            t->genParams[0] = std::clamp(t->genParams[0] + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 11:
            t->genParams[1] = std::clamp(t->genParams[1] + delta * 0.05f, 0.0f, 1.0f);
            break;
        case 12:
            t->genParams[2] = std::clamp(t->genParams[2] + delta * 0.05f, 0.0f, 1.0f);
            break;
        }

        updatePadLeds();
        updateEncoderLabels();
    }
};
