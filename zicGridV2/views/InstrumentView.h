#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <mutex>
#include <string>
#include <vector>

#include "draw/draw.h"
#include "zicGridV2/ViewManager.h"
#include "zicGridV2/gridState.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/uiComponents.h"

class InstrumentView : public View {
private:
    int baseNote = 48;
    int currentPage = 0;
    float notePulseLevel = 0.0f;
    uint8_t lastTriggeredNote = 0;
    int lastSeqStep = -1;

public:
    InstrumentView() : View("INSTRUMENT & SYNTH") {}

    int getTotalPages() const
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        if (t && t->engine) {
            size_t paramCount = t->engine->getParamCount();
            if (paramCount == 0) return 1;
            return (int)((paramCount + TOTAL_ENCODERS - 1) / TOTAL_ENCODERS);
        }
        return 1;
    }

    std::pair<int, int> getViewPageInfo() const override
    {
        int totalPages = getTotalPages();
        int pageIdx = std::min(currentPage + 1, totalPages);
        return { pageIdx, totalPages };
    }

    void onTrackSelect(int trk, bool isSameTrack) override
    {
        if (isSameTrack) {
            int totalPages = getTotalPages();
            if (totalPages > 1) {
                currentPage = (currentPage + 1) % totalPages;
            }
        } else {
            currentPage = 0;
        }
    }

    void changePage(int delta) override
    {
        int totalPages = getTotalPages();
        if (totalPages > 1) {
            currentPage = (currentPage + delta + totalPages) % totalPages;
        }
    }

    void onActivate() override
    {
        updatePadLeds();
        updateEncoderLabels();
    }

    void updatePadLeds() override
    {
        int octave = gridState.utility.currentOctave;
        baseNote = 12 + octave * 12;

        int activeTrk = studio.selTrack;
        Color theme = studio.tracks[activeTrk]->themeColor;

        for (int r = 0; r < PAD_ROWS; ++r) {
            for (int c = 0; c < DYNAMIC_PAD_COLS; ++c) {
                int noteOffset = (3 - r) * DYNAMIC_PAD_COLS + c;
                int note = baseNote + noteOffset;

                auto& pad = gridState.pads[c][r];
                pad.note = (uint8_t)note;
                pad.selected = false;

                bool isRoot = (note % 12 == 0);
                bool isAccidental = (note % 12 == 1 || note % 12 == 3 || note % 12 == 6 || note % 12 == 8 || note % 12 == 10);

                if (pad.pressed) {
                    pad.color = { 255, 255, 255, 255 };
                } else if (isRoot) {
                    pad.color = { 255, 200, 50, 255 };
                } else if (isAccidental) {
                    pad.color = { 60, 70, 100, 255 };
                } else {
                    pad.color = theme;
                }

                pad.label = getNoteName(note);
            }
        }
    }

    void updateEncoderLabels() override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];
        Color c = t->themeColor;

        if (gridState.utility.shiftActive) {
            gridState.setEncoder(0, "Synth", t->currentEngineIdx, 0, ENGINE_REGISTRY_COUNT - 1, 1, engineRegistry[t->currentEngineIdx].name, c);
            gridState.setEncoder(1, "Volume", (int)(t->volume * 100.0f), 0, 100, 1, nullptr, c, "%");

            for (int i = 2; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoderParam(i, Param{}, c);
            }
            return;
        }

        int totalPages = getTotalPages();
        if (currentPage >= totalPages) {
            currentPage = 0;
        }

        if (t && t->engine) {
            size_t paramCount = t->engine->getParamCount();
            auto* params = t->engine->getParams();
            for (int i = 0; i < TOTAL_ENCODERS; ++i) {
                int actualParamIdx = currentPage * TOTAL_ENCODERS + i;
                if ((size_t)actualParamIdx < paramCount && params) {
                    gridState.setEncoderParam(i, params[actualParamIdx], c);
                } else {
                    gridState.setEncoderParam(i, Param{}, c);
                }
            }
        } else {
            for (int i = 0; i < TOTAL_ENCODERS; ++i) {
                gridState.setEncoderParam(i, Param{}, c);
            }
        }
    }

    void render(Draw& d, int x, int y, int w, int h) override
    {
        int selTrkIdx = studio.selTrack;
        auto& trk = studio.tracks[selTrkIdx];
        Color themeColor = trk->themeColor;

        // 1. View Title Header
        std::string titleStr = "VIEW: INST & KEYBOARD - T" + std::to_string(selTrkIdx + 1) + " (" + engineRegistry[trk->currentEngineIdx].name + ")";
        d.text({ x + 4, y + 2 }, titleStr, 8, { .color = themeColor, .font = &PoppinsLight_8 });

        // Draw Page Indicator Dots on the right side of the View Title Badge
        auto [pageIdx, totalPages] = getViewPageInfo();
        if (totalPages > 1) {
            int dotW = 5;
            int dotH = 3;
            int gap = 2;
            int totalDotsW = totalPages * dotW + (totalPages - 1) * gap;
            int dotsX = x + w - 6 - totalDotsW;
            int dotsY = y + (16 - dotH) / 2;
            for (int p = 0; p < totalPages; p++) {
                Color dotCol = (p + 1 == pageIdx) ? themeColor : Color { 60, 72, 95, 255 };
                d.filledRect({ dotsX + p * (dotW + gap), dotsY }, { dotW, dotH }, { .color = dotCol });
            }
        }

        if (!trk || !trk->engine) return;

        // 2. Upper Main Canvas: Adaptive Engine Waveform / Response Curve & Polyphonic Playheads
        int canvasY = y + 18;
        int canvasH = 200;
        int canvasW = w;

        // Background box & vibrant frame outline
        d.filledRect({ x, canvasY }, { canvasW, canvasH }, { .color = { 12, 16, 24, 255 } });
        d.rect({ x, canvasY }, { canvasW, canvasH }, { .color = { 35, 45, 60, 255 } });

        float lStart = trk->engine->getLoopStart();
        float lLen = trk->engine->getLoopLength();
        bool isSampleEngine = engineRegistry[trk->currentEngineIdx].showWaveform || (lLen > 0.0f);

        // Highlight Sample Loop Region if applicable (zicXYv2 style)
        if (isSampleEngine && lLen > 0.0f) {
            int loopX = x + 2 + (int)(lStart * (canvasW - 4));
            int loopW = std::max(2, (int)(lLen * (canvasW - 4)));
            Color loopBg = { themeColor.r, themeColor.g, themeColor.b, 40 };
            d.filledRect({ loopX, canvasY + 2 }, { loopW, canvasH - 4 }, { .color = loopBg });

            Color markerCol = { themeColor.r, themeColor.g, themeColor.b, 200 };
            d.filledRect({ loopX, canvasY + 2 }, { 1, canvasH - 4 }, { .color = markerCol });
            d.filledRect({ std::min(x + canvasW - 3, loopX + loopW - 1), canvasY + 2 }, { 1, canvasH - 4 }, { .color = markerCol });

            // Loop Info Badges
            std::string loopStr = "LOOP " + std::to_string((int)(lStart * 100)) + "% - " + std::to_string((int)((lStart + lLen) * 100)) + "%";
            d.text({ x + 6, canvasY + 4 }, loopStr, 8, { .color = { 180, 195, 220, 255 }, .font = &PoppinsLight_8 });
        } else {
            std::string engineTypeStr = isSampleEngine ? "SAMPLE WAVEFORM" : "SYNTH RESPONSE CURVE";
            d.text({ x + 6, canvasY + 4 }, engineTypeStr, 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });
        }

        // Render Engine Waveform / Response Curve using engine->draw(x)
        int centerY = canvasY + canvasH / 2;
        int maxAmplitude = (canvasH - 24) / 2;
        int drawW = canvasW - 4;
        int prevX = x + 2;
        int prevY = centerY;

        for (int px = 0; px < drawW; px += 2) {
            float phase = (float)px / (float)drawW;
            float sampleVal = trk->engine->draw(phase);
            int ptY = centerY - (int)(sampleVal * (float)maxAmplitude);
            ptY = std::clamp(ptY, canvasY + 16, canvasY + canvasH - 4);
            int drawX = x + 2 + px;

            // Translucent wave area fill
            Color fillCol = { themeColor.r, themeColor.g, themeColor.b, 25 };
            if (ptY >= centerY) {
                d.filledRect({ drawX, centerY }, { 2, ptY - centerY + 1 }, { .color = fillCol });
            } else {
                d.filledRect({ drawX, ptY }, { 2, centerY - ptY + 1 }, { .color = fillCol });
            }

            // Vibrant wave line
            d.line({ prevX, prevY }, { drawX, ptY }, { .color = themeColor });
            prevX = drawX;
            prevY = ptY;
        }

        // Render Live Polyphonic Voice Playheads
        int voiceCount = trk->engine->getVoiceCount();
        int activeVoiceCount = 0;

        for (int v = 0; v < voiceCount; ++v) {
            float ph = trk->engine->getPlayhead(v);
            if (ph >= 0.0f && ph <= 1.0f) {
                activeVoiceCount++;
                int playheadX = x + 2 + (int)(ph * (canvasW - 4));
                playheadX = std::clamp(playheadX, x + 2, x + canvasW - 4);

                // Laser playhead line
                d.filledRect({ playheadX, canvasY + 16 }, { 2, canvasH - 20 }, { .color = { 255, 255, 255, 240 } });
                d.filledCircle({ playheadX + 1, canvasY + 16 }, 3, { .color = { 255, 255, 255, 255 } });
            }
        }

        // Auto-trigger note pulse on sequencer step hit
        if (studio.isPlaying) {
            int curStep = studio.currentStep % SEQ_STEPS;
            if (curStep != lastSeqStep) {
                lastSeqStep = curStep;
                if (curStep < (int)trk->sequence.size() && trk->sequence[curStep].active) {
                    notePulseLevel = 1.0f;
                    lastTriggeredNote = trk->sequence[curStep].note;
                }
            }
        }

        // Note Trigger Pulse Shockwave Animation (Sleek 2-ring pulse)
        notePulseLevel = std::max(0.0f, notePulseLevel - 0.045f);
        if (notePulseLevel > 0.01f) {
            int cx = x + canvasW / 2;
            int cy = canvasY + canvasH / 2;
            for (int r = 0; r < 2; r++) {
                float pFactor = notePulseLevel - (r * 0.25f);
                if (pFactor > 0.0f) {
                    int radius = (int)(22.0f + (1.0f - pFactor) * 32.0f + r * 6);
                    uint8_t alpha = (uint8_t)(pFactor * 160.0f);
                    d.circle({ cx, cy }, radius, { .color = { themeColor.r, themeColor.g, themeColor.b, alpha } });
                }
            }
        }

        // 3. Lower Section: Dual Visualizer Panels (Oscilloscope & Context Monitor)
        int panelY = canvasY + canvasH + 4;
        int panelH = h - (panelY - y) - 2;

        if (panelH >= 40) {
            // Panel A: Real-Time Audio Output Oscilloscope (Left, Width ~276px)
            int waveW = 276;
            d.filledRect({ x, panelY }, { waveW, panelH }, { .color = { 14, 18, 26, 255 } });
            d.rect({ x, panelY }, { waveW, panelH }, { .color = { 35, 45, 60, 255 } });

            d.text({ x + 6, panelY + 4 }, "LIVE OSCILLOSCOPE", 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });

            std::vector<float> trkHistory;
            {
                std::lock_guard<std::mutex> hl(trk->historyMtx);
                trkHistory.assign(trk->history.begin(), trk->history.end());
            }
            int historySize = (int)trkHistory.size();

            int oscCenterY = panelY + panelH / 2 + 2;
            int oscAmp = (panelH - 20) / 2;
            int prevOscX = x + 2;
            int prevOscY = oscCenterY;

            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
            float idlePhase = (nowMs % 2500) / 2500.0f * 2.0f * M_PI;

            for (int px = 0; px < waveW - 4; px += 2) {
                float sampleVal = 0.0f;
                if (historySize > 0) {
                    int histIdx = std::clamp((int)(px * ((float)historySize / (float)waveW)), 0, historySize - 1);
                    sampleVal = trkHistory[histIdx];
                } else {
                    float normX = (float)px / (float)waveW;
                    sampleVal = 0.25f * std::sin(normX * 4.0f * M_PI + idlePhase);
                }

                int ptY = oscCenterY - (int)(sampleVal * (float)oscAmp);
                ptY = std::clamp(ptY, panelY + 16, panelY + panelH - 4);
                int drawX = x + 2 + px;

                Color areaCol = { themeColor.r, themeColor.g, themeColor.b, 20 };
                if (ptY >= oscCenterY) {
                    d.filledRect({ drawX, oscCenterY }, { 2, ptY - oscCenterY + 1 }, { .color = areaCol });
                } else {
                    d.filledRect({ drawX, ptY }, { 2, oscCenterY - ptY + 1 }, { .color = areaCol });
                }

                d.line({ prevOscX, prevOscY }, { drawX, ptY }, { .color = themeColor });
                prevOscX = drawX;
                prevOscY = ptY;
            }

            // Panel B: Engine Context & Performance Monitor (Right Panel, Width w - 280)
            int monitorX = x + 280;
            int monitorW = w - 280;
            d.filledRect({ monitorX, panelY }, { monitorW, panelH }, { .color = { 14, 18, 26, 255 } });
            d.rect({ monitorX, panelY }, { monitorW, panelH }, { .color = { 35, 45, 60, 255 } });

            int mCenterX = monitorX + monitorW / 2;
            int mCenterY = panelY + panelH / 2;

            if (isSampleEngine) {
                // Sampler Context Display
                d.text({ monitorX + 6, panelY + 4 }, "VOICE & SAMPLE", 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });

                std::string vStr = "VOICES: " + std::to_string(activeVoiceCount) + " / " + std::to_string(voiceCount);
                d.text({ monitorX + 8, panelY + 22 }, vStr, 8, { .color = { 220, 235, 255, 255 }, .font = &PoppinsLight_8 });

                uint8_t dispNote = (trk->playingNote > 0) ? trk->playingNote : lastTriggeredNote;
                std::string nStr = "NOTE: " + (dispNote > 0 ? getNoteName(dispNote) : "---");
                d.text({ monitorX + 8, panelY + 38 }, nStr, 8, { .color = themeColor, .font = &PoppinsLight_8 });

                // Voice Playhead Progress Gauge
                int gaugeX = monitorX + 8;
                int gaugeY = panelY + panelH - 18;
                int gaugeW = monitorW - 16;
                d.filledRect({ gaugeX, gaugeY }, { gaugeW, 8 }, { .color = { 24, 30, 42, 255 } });
                d.rect({ gaugeX, gaugeY }, { gaugeW, 8 }, { .color = { 50, 62, 85, 255 } });

                float activePh = 0.0f;
                for (int v = 0; v < voiceCount; ++v) {
                    float p = trk->engine->getPlayhead(v);
                    if (p >= 0.0f) { activePh = p; break; }
                }
                int fillW = (int)(activePh * (gaugeW - 2));
                if (fillW > 0) {
                    d.filledRect({ gaugeX + 1, gaugeY + 1 }, { fillW, 6 }, { .color = themeColor });
                }
            } else if (trk->engine->getNameXY() != nullptr) {
                // XY Performance Pad Monitor
                std::string xyName = trk->engine->getNameXY();
                d.text({ monitorX + 6, panelY + 4 }, xyName, 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });

                IEngine::XY xyPos = trk->engine->getXY();
                int padBoxW = std::min(monitorW - 16, panelH - 24);
                int padBoxX = mCenterX - padBoxW / 2;
                int padBoxY = panelY + 18;

                d.rect({ padBoxX, padBoxY }, { padBoxW, padBoxW }, { .color = { 60, 75, 100, 255 } });
                d.line({ padBoxX + padBoxW / 2, padBoxY }, { padBoxX + padBoxW / 2, padBoxY + padBoxW }, { .color = { 35, 45, 60, 255 } });
                d.line({ padBoxX, padBoxY + padBoxW / 2 }, { padBoxX + padBoxW, padBoxY + padBoxW / 2 }, { .color = { 35, 45, 60, 255 } });

                int posX = padBoxX + (int)(xyPos.x * padBoxW);
                int posY = padBoxY + (int)((1.0f - xyPos.y) * padBoxW);
                d.filledCircle({ posX, posY }, 3, { .color = themeColor });
            } else {
                // 16-Step Pitch / Beat Radar Clock
                d.text({ monitorX + 6, panelY + 4 }, "BEAT RADAR", 8, { .color = { 110, 125, 150, 255 }, .font = &PoppinsLight_8 });

                int radius = std::min((monitorW - 16) / 2, (panelH - 22) / 2);
                int current16 = studio.isPlaying ? (studio.currentStep % 16) : 0;

                for (int s = 0; s < 16; ++s) {
                    float angle = (s * 360.0f / 16.0f) - 90.0f;
                    float rad = angle * M_PI / 180.0f;
                    int dotX = mCenterX + (int)(radius * std::cos(rad));
                    int dotY = mCenterY + (int)(radius * std::sin(rad));

                    if (s == current16) {
                        d.filledCircle({ dotX, dotY }, 3, { .color = { 255, 255, 255, 255 } });
                    } else if (s % 4 == 0) {
                        d.filledCircle({ dotX, dotY }, 2, { .color = themeColor });
                    } else {
                        d.filledCircle({ dotX, dotY }, 1, { .color = { 50, 65, 85, 255 } });
                    }
                }

                float sweepAngle = (current16 * 360.0f / 16.0f) - 90.0f;
                float sweepRad = sweepAngle * M_PI / 180.0f;
                int sweepX = mCenterX + (int)(radius * std::cos(sweepRad));
                int sweepY = mCenterY + (int)(radius * std::sin(sweepRad));
                d.line({ mCenterX, mCenterY }, { sweepX, sweepY }, { .color = { themeColor.r, themeColor.g, themeColor.b, 180 } });
            }
        }
    }

    void handleDynamicPadPress(int col, int row, bool pressed) override
    {
        auto& pad = gridState.pads[col][row];
        pad.pressed = pressed;

        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (pressed) {
            notePulseLevel = 1.0f;
            lastTriggeredNote = pad.note;
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            t->engine->noteOn(pad.note, 0.9f);
        } else {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            t->engine->noteOff(pad.note);
        }

        updatePadLeds();
    }

    void handleEncoder(int encoderId, int delta) override
    {
        int trk = studio.selTrack;
        auto& t = studio.tracks[trk];

        if (gridState.utility.shiftActive) {
            if (encoderId == 1) { // Synth engine selection
                int nextEngine = std::clamp((int)t->currentEngineIdx + delta, 0, ENGINE_REGISTRY_COUNT - 1);
                if (nextEngine != t->currentEngineIdx) {
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    t->setEngine(nextEngine);
                    currentPage = 0;
                }
            } else if (encoderId == 2) { // Track volume
                t->volume = std::clamp(t->volume + delta * 0.05f, 0.0f, 1.0f);
            }
        } else if (t && t->engine) {
            int pIdx = encoderId - 1;
            if (pIdx >= 0 && pIdx < TOTAL_ENCODERS) {
                int actualParamIdx = currentPage * TOTAL_ENCODERS + pIdx;
                if ((size_t)actualParamIdx < t->engine->getParamCount()) {
                    auto& p = t->engine->getParams()[actualParamIdx];
                    p.set(p.value + delta * p.step);
                }
            }
        }

        updatePadLeds();
        updateEncoderLabels();
    }

private:
    std::string getNoteName(int note)
    {
        static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int octave = (note / 12) - 1;
        return std::string(names[note % 12]) + std::to_string(octave);
    }
};

