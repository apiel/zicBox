#pragma once

#include "draw/draw.h"
#include "draw/fonts/PoppinsLight_12.h"
#include "draw/fonts/PoppinsLight_8.h"
#include "zicGridImpact/gridState.h"
#include "zicGridImpact/studio.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>

class MasterView {
private:
    static constexpr Color THEME_COLOR = { 255, 210, 0, 255 }; // Bright Master Gold
    float smoothVu[5] = { 0.0f };
    float peakHoldVal[5] = { 0.0f };
    float peakHoldDecay[5] = { 0.0f };
    float animTime = 0.0f;

public:
    void updateEncoders()
    {
        gridState.setEncoder(0, "KICK Lvl", studio.mixer.kickLevel, 0.0f, 1.0f, 0.01f, nullptr, THEME_COLOR);

        char s1Buf[16] = { 0 };
        if (studio.mixer.synth1Level > 1.0f) {
            snprintf(s1Buf, sizeof(s1Buf), "DRV +%.0f%%", (studio.mixer.synth1Level - 1.0f) * 100.0f);
            gridState.setEncoder(1, "SYN1 Lvl", studio.mixer.synth1Level, 0.0f, 2.0f, 0.01f, s1Buf, Color { 255, 110, 40, 255 });
        } else {
            gridState.setEncoder(1, "SYN1 Lvl", studio.mixer.synth1Level, 0.0f, 2.0f, 0.01f, nullptr, THEME_COLOR);
        }

        char s2Buf[16] = { 0 };
        if (studio.mixer.synth2Level > 1.0f) {
            snprintf(s2Buf, sizeof(s2Buf), "DRV +%.0f%%", (studio.mixer.synth2Level - 1.0f) * 100.0f);
            gridState.setEncoder(2, "SYN2 Lvl", studio.mixer.synth2Level, 0.0f, 2.0f, 0.01f, s2Buf, Color { 255, 110, 40, 255 });
        } else {
            gridState.setEncoder(2, "SYN2 Lvl", studio.mixer.synth2Level, 0.0f, 2.0f, 0.01f, nullptr, THEME_COLOR);
        }

        char chBuf[16] = { 0 };
        if (studio.mixer.chaosLevel > 1.0f) {
            snprintf(chBuf, sizeof(chBuf), "DRV +%.0f%%", (studio.mixer.chaosLevel - 1.0f) * 100.0f);
            gridState.setEncoder(3, "CHS Lvl", studio.mixer.chaosLevel, 0.0f, 2.0f, 0.01f, chBuf, Color { 255, 110, 40, 255 });
        } else {
            gridState.setEncoder(3, "CHS Lvl", studio.mixer.chaosLevel, 0.0f, 2.0f, 0.01f, nullptr, THEME_COLOR);
        }

        gridState.setEncoder(4, "MAST Vol", studio.mixer.volume, 0.0f, 1.0f, 0.02f, nullptr, THEME_COLOR);
        gridState.setEncoder(5, "DELAY Time", studio.mixer.delayTimeMs, 10.0f, 1000.0f, 10.0f, nullptr, THEME_COLOR, "ms");
        gridState.setEncoder(6, "DELAY Fdbk", studio.mixer.delayFeedback, 0.0f, 0.95f, 0.01f, nullptr, THEME_COLOR);
        gridState.setEncoder(7, "MAST Cutoff", studio.mixer.masterCutoff, 0.0f, 1.0f, 0.01f, nullptr, THEME_COLOR);

        gridState.setEncoder(8, "SCAT Crunch", studio.scatter.params[4][0], 0.0f, 1.0f, 0.02f, nullptr, THEME_COLOR);
        gridState.setEncoder(9, "SCAT Drive", studio.scatter.params[5][0], 0.0f, 1.0f, 0.02f, nullptr, THEME_COLOR);
        gridState.setEncoder(10, "SCAT Dist", studio.scatter.params[2][0], 0.0f, 1.0f, 0.02f, nullptr, THEME_COLOR);
        gridState.setEncoder(11, "SCAT Acid", studio.scatter.params[6][0], 0.0f, 1.0f, 0.02f, nullptr, THEME_COLOR);
    }

    void handleEncoder(int idx, int delta)
    {
        float step = gridState.encoders[idx].step;
        float change = delta * step;
        switch (idx) {
        case 0:
            studio.mixer.kickLevel = std::clamp(studio.mixer.kickLevel + change, 0.0f, 1.0f);
            break;
        case 1:
            studio.mixer.synth1Level = std::clamp(studio.mixer.synth1Level + change, 0.0f, 2.0f);
            break;
        case 2:
            studio.mixer.synth2Level = std::clamp(studio.mixer.synth2Level + change, 0.0f, 2.0f);
            break;
        case 3:
            studio.mixer.chaosLevel = std::clamp(studio.mixer.chaosLevel + change, 0.0f, 2.0f);
            break;
        case 4:
            studio.mixer.volume = std::clamp(studio.mixer.volume + change, 0.0f, 1.0f);
            break;
        case 5:
            studio.mixer.delayTimeMs = std::clamp(studio.mixer.delayTimeMs + change, 10.0f, 1000.0f);
            break;
        case 6:
            studio.mixer.delayFeedback = std::clamp(studio.mixer.delayFeedback + change, 0.0f, 0.95f);
            break;
        case 7:
            studio.mixer.masterCutoff = std::clamp(studio.mixer.masterCutoff + change, 0.0f, 1.0f);
            break;
        case 8:
            studio.scatter.params[4][0] = std::clamp(studio.scatter.params[4][0] + change, 0.0f, 1.0f);
            break;
        case 9:
            studio.scatter.params[5][0] = std::clamp(studio.scatter.params[5][0] + change, 0.0f, 1.0f);
            break;
        case 10:
            studio.scatter.params[2][0] = std::clamp(studio.scatter.params[2][0] + change, 0.0f, 1.0f);
            break;
        case 11:
            studio.scatter.params[6][0] = std::clamp(studio.scatter.params[6][0] + change, 0.0f, 1.0f);
            break;
        }
        updateEncoders();
    }

    void render(Draw& d, int x, int y, int w, int h)
    {
        animTime += 0.05f;

        int graphX = x;
        int graphY = y;
        int graphW = w;
        int graphH = h;

        d.filledRect({ graphX, graphY }, { graphW, graphH }, { .color = { 12, 14, 20, 255 } });
        d.rect({ graphX, graphY }, { graphW, graphH }, { .color = THEME_COLOR });

        const char* channelLabels[5] = { "KICK", "SYN1", "SYN2", "CHS", "MAIN" };
        Color channelColors[5] = {
            { 0, 195, 255, 255 }, // KICK: Electric Blue
            { 0, 240, 190, 255 }, // SYN1: Cyan
            { 215, 125, 255, 255 }, // SYN2: Purple
            { 255, 45, 85, 255 }, // CHS: Crimson
            THEME_COLOR // MAIN: Gold
        };
        float channelLevels[5] = { studio.mixer.kickLevel, studio.mixer.synth1Level, studio.mixer.synth2Level, studio.mixer.chaosLevel, studio.mixer.volume };

        float targetSignals[5] = {
            studio.mixer.peakKick.load(),
            studio.mixer.peakSynth1.load(),
            studio.mixer.peakSynth2.load(),
            studio.mixer.peakChaos.load(),
            studio.mixer.peakMaster.load()
        };

        for (int ch = 0; ch < 5; ch++) {
            float tgt = std::clamp(targetSignals[ch], 0.0f, 1.0f);
            if (tgt > smoothVu[ch]) {
                smoothVu[ch] += (tgt - smoothVu[ch]) * 0.40f;
            } else {
                smoothVu[ch] += (tgt - smoothVu[ch]) * 0.12f;
            }

            if (smoothVu[ch] >= peakHoldVal[ch]) {
                peakHoldVal[ch] = smoothVu[ch];
                peakHoldDecay[ch] = 0.5f;
            } else {
                if (peakHoldDecay[ch] > 0.0f) {
                    peakHoldDecay[ch] -= 0.04f;
                } else {
                    peakHoldVal[ch] = std::max(smoothVu[ch], peakHoldVal[ch] - 0.015f);
                }
            }
        }

        int totalStrips = 5;
        int stripW = (graphW - 16) / totalStrips;

        for (int ch = 0; ch < 5; ch++) {
            int colX = graphX + 8 + ch * stripW;
            Color themeCol = channelColors[ch];
            bool hasDrive = (ch == 1 || ch == 2 || ch == 3);
            float maxLvl = hasDrive ? 2.0f : 1.0f;
            float rawLvl = std::clamp(channelLevels[ch], 0.0f, maxLvl);
            bool isDriven = (hasDrive && rawLvl > 1.0f);
            bool isMuted = (ch == 0 && gridState.isKickMuted) || (ch == 1 && gridState.isSynth1Muted) || (ch == 2 && gridState.isSynth2Muted) || (ch == 3 && gridState.isChaosMuted);

            d.text({ colX + 2, graphY + 8 }, channelLabels[ch], 8, { .color = themeCol, .font = &PoppinsLight_8 });

            int fX = colX + 4;
            int fY = graphY + 30;
            int fW = 16;
            int fH = graphH - 80;

            Color faderBg = isMuted ? Color { 14, 16, 22, 255 } : Color { 20, 24, 34, 255 };
            Color faderBorder = isMuted ? Color { 35, 42, 56, 255 } : Color { 50, 60, 80, 255 };
            uint8_t cleanAlpha = isMuted ? 105 : 180;
            uint8_t driveAlpha = isMuted ? 115 : 230;

            d.filledRect({ fX, fY }, { fW, fH }, { .color = faderBg });
            d.rect({ fX, fY }, { fW, fH }, { .color = faderBorder });

            if (hasDrive) {
                // Draw 100% clean threshold marker line at 50% fader height
                int midY = fY + fH / 2;
                Color lineCol = isMuted ? Color { 100, 90, 40, 80 } : Color { 255, 180, 50, 140 };
                d.line({ fX, midY }, { fX + fW - 1, midY }, { .color = lineCol });

                float cleanPart = std::min(rawLvl, 1.0f) / 2.0f;
                int cleanFillH = (int)(fH * cleanPart);
                if (cleanFillH > 0) {
                    d.filledRect({ fX + 1, fY + fH - cleanFillH }, { fW - 2, cleanFillH }, { .color = { themeCol.r, themeCol.g, themeCol.b, cleanAlpha } });
                }

                if (rawLvl > 1.0f) {
                    float drivePart = (rawLvl - 1.0f) / 2.0f;
                    int driveFillH = (int)(fH * drivePart);
                    int driveY = fY + fH / 2 - driveFillH;
                    d.filledRect({ fX + 1, driveY }, { fW - 2, driveFillH }, { .color = Color { 255, 90, 35, driveAlpha } });
                }

                int totalFillH = (int)(fH * (rawLvl / 2.0f));
                int capY = fY + fH - totalFillH - 1;
                capY = std::clamp(capY, fY, fY + fH - 2);
                Color capCol = isMuted ? Color { 80, 85, 100, 140 } : (isDriven ? Color { 255, 120, 40, 255 } : Color { 245, 250, 255, 255 });
                d.filledRect({ fX - 2, capY }, { fW + 4, 3 }, { .color = capCol });
            } else {
                float lvl = std::clamp(rawLvl, 0.0f, 1.0f);
                int fillH = (int)(fH * lvl);
                if (fillH > 0) {
                    d.filledRect({ fX + 1, fY + fH - fillH }, { fW - 2, fillH }, { .color = { themeCol.r, themeCol.g, themeCol.b, cleanAlpha } });
                }
                int capY = fY + fH - fillH - 1;
                capY = std::clamp(capY, fY, fY + fH - 2);
                Color capCol = isMuted ? Color { 80, 85, 100, 140 } : Color { 245, 250, 255, 255 };
                d.filledRect({ fX - 2, capY }, { fW + 4, 3 }, { .color = capCol });
            }

            if (isMuted) {
                // Overlay dark semi-transparent mask over fader to make it noticeably faded/opaque
                d.filledRect({ fX + 1, fY + 1 }, { fW - 2, fH - 2 }, { .color = Color { 10, 12, 18, 140 } });
            }

            int vuX = colX + 26;
            int vuY = graphY + 30;
            int vuW = 14;
            int vuH = fH;

            Color vuBg = isMuted ? Color { 10, 12, 18, 255 } : Color { 14, 18, 26, 255 };
            Color vuBorder = isMuted ? Color { 30, 36, 48, 255 } : Color { 45, 55, 75, 255 };
            d.filledRect({ vuX, vuY }, { vuW, vuH }, { .color = vuBg });
            d.rect({ vuX, vuY }, { vuW, vuH }, { .color = vuBorder });

            float sigVal = std::clamp(smoothVu[ch], 0.0f, 1.0f);
            int actVuH = (int)((vuH - 2) * sigVal);

            if (actVuH > 0) {
                for (int py = 0; py < actVuH; py++) {
                    float normY = (float)py / (float)(vuH - 2);
                    uint8_t alpha = (uint8_t)(140 + normY * 115.0f);

                    Color segCol = {
                        (uint8_t)std::min(255, (int)(themeCol.r * (0.85f + normY * 0.35f))),
                        (uint8_t)std::min(255, (int)(themeCol.g * (0.85f + normY * 0.35f))),
                        (uint8_t)std::min(255, (int)(themeCol.b * (0.85f + normY * 0.35f))),
                        alpha
                    };

                    if (sigVal > 0.90f && py >= actVuH - 3) {
                        segCol = Color { 255, 90, 70, 255 };
                    }

                    d.line({ vuX + 1, vuY + vuH - 2 - py }, { vuX + vuW - 2, vuY + vuH - 2 - py }, { .color = segCol });
                }

                for (int gy = vuY + vuH - 5; gy > vuY + 1; gy -= 4) {
                    d.line({ vuX + 1, gy }, { vuX + vuW - 2, gy }, { .color = Color { 10, 14, 20, 180 } });
                }
            }

            float pkVal = std::clamp(peakHoldVal[ch], 0.0f, 1.0f);
            if (pkVal > 0.02f) {
                int pkY = vuY + vuH - 2 - (int)((vuH - 3) * pkVal);
                pkY = std::clamp(pkY, vuY + 1, vuY + vuH - 2);
                Color pkCol = (pkVal > 0.90f) ? Color { 255, 80, 60, 255 } : Color { 245, 250, 255, 240 };
                d.line({ vuX + 1, pkY }, { vuX + vuW - 2, pkY }, { .color = pkCol });
            }
        }

        // Scope Waveform Ribbon at the bottom
        int scopeY = graphY + graphH - 18;
        int innerW = graphW - 24;
        std::vector<Point> scopeWave;

        for (int gx = 0; gx < innerW; gx += 2) {
            float t = (float)gx / (float)innerW;
            float waveK = std::sin(t * 18.0f + animTime * 6.0f) * (studio.mixer.kickLevel * 6.0f);
            float waveS1 = std::sin(t * 32.0f + animTime * 10.0f) * (studio.mixer.synth1Level * 5.0f);
            float waveS2 = std::sin(t * 48.0f + animTime * 14.0f) * (studio.mixer.synth2Level * 5.0f);
            float waveCh = std::sin(t * 64.0f + animTime * 18.0f) * (studio.mixer.chaosLevel * 5.0f);

            float combined = (waveK + waveS1 + waveS2 + waveCh) * (0.5f + studio.mixer.volume * 0.5f);
            combined = std::clamp(combined, -14.0f, 14.0f);

            scopeWave.push_back({ graphX + 12 + gx, scopeY + (int)combined });
        }

        d.lines(scopeWave, { .color = THEME_COLOR, .thickness = 1 });
    }
};
