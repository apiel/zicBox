#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <pthread.h>
#include <sstream>
#include <thread>
#include <vector>

#include "helpers/enc.h"
#include "helpers/format.h"
#include "helpers/midiNote.h"
#include "zic23/audioWorker.h"
#include "audio/Eq.h"
#include "zic23/generator.h"
#include "zic23/studio.h"
#include "zic23/ui.h"

// ----------------------------------------------------------------
// Layout
// ----------------------------------------------------------------
const int laneH = 18;

// EQ modal geometry (all in window pixels)
static const int EQ_MODAL_TOP = 50;
static const int EQ_MODAL_X = 35;
static const int EQ_PAD_L = 55; // left margin inside modal (room for dB labels)
static const int EQ_PAD_R = 10;
static const int EQ_CURVE_Y = 46; // top of EQ curve canvas, relative to modal top
static const int EQ_CURVE_H = 170;
static const float EQ_DB_RANGE = 12.f;
static const int EQ_DOT_R = 8; // hit radius for drag

// Spectrum strips sit below the EQ canvas inside the modal
// Each track gets a strip of SPEC_STRIP_H pixels
static const int SPEC_TOP_PAD = 18; // gap between EQ canvas bottom and first strip
static const int SPEC_STRIP_H = 14; // height of each track's frequency strip
static const int SPEC_STRIP_GAP = 2; // gap between strips

// ----------------------------------------------------------------
// Global UI state
// ----------------------------------------------------------------
bool showHelp = false;
bool showEQ = false;
int eqActiveTrack = 0; // which track's EQ is being edited

bool eqDragging = false;
int eqDragBand = -1;

sf::IntRect helpBtnRect, helpCloseRect;

// Track draw order for EQ (last = drawn on top / active)
std::vector<int> eqTrackOrder;

// Clipboard
int copyTrackIdx = -1, copyStepIdx = -1;
Step copiedStep;

// Rects updated each draw, used by mouse handlers
struct EQRects {
    int canvX, canvY, canvW; // EQ curve canvas (absolute pixels)
    int specX, specBaseY, specW; // spectrum area origin
    sf::IntRect legendRects[MAX_TRACKS];
    sf::IntRect closeRect;
};
static EQRects eqR;

// ----------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------
void triggerPreview(Track& trk, int note, float velocity, int durationMs = 200)
{
    {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        trk.engine->noteOn(note, velocity);
    }
    std::thread([&trk, note, durationMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(durationMs));
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        trk.engine->noteOff(note);
    }).detach();
}

// Compute absolute EQ canvas coords from window size
void computeEQRects(sf::Vector2u winSize)
{
    int modalW = (int)winSize.x - EQ_MODAL_X * 2;
    eqR.canvX = EQ_MODAL_X + EQ_PAD_L;
    eqR.canvY = EQ_MODAL_TOP + EQ_CURVE_Y;
    eqR.canvW = modalW - EQ_PAD_L - EQ_PAD_R;
    eqR.specX = eqR.canvX;
    eqR.specW = eqR.canvW;
    eqR.specBaseY = eqR.canvY + EQ_CURVE_H + SPEC_TOP_PAD;

    // Close button
    eqR.closeRect = { EQ_MODAL_X + modalW - 58, EQ_MODAL_TOP + 8, 50, 16 };

    // Legend swatches — one per track, below all strips
    int legY = eqR.specBaseY + MAX_TRACKS * (SPEC_STRIP_H + SPEC_STRIP_GAP) + 10;
    int legX = eqR.canvX;
    for (int t = 0; t < MAX_TRACKS; t++) {
        eqR.legendRects[t] = { legX, legY, 22, 10 };
        legX += 60;
    }
}

// ----------------------------------------------------------------
// drawHelpOverlay
// ----------------------------------------------------------------
void drawHelpOverlay(Draw& d, sf::Vector2u size)
{
    int winW = (int)size.x, winH = (int)size.y;
    int x = winW < 1000 ? 50 : 200, y = 50;
    int rW = winW - x * 2, rH = winH - y * 2;
    d.filledRect({ x, y }, { rW, rH }, { .color = { 20, 20, 25, 235 } });
    d.rect({ x, y }, { rW, rH }, { .color = { 200, 200, 205, 235 } });
    helpCloseRect = { x + rW - 50, y + 10, 40, 12 };
    d.filledRect({ helpCloseRect.left, helpCloseRect.top }, { 40, 12 }, { .color = { 200, 50, 50 } });
    d.text({ helpCloseRect.left + 6, helpCloseRect.top + 2 }, "Close", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    d.text({ x + 10, y + 10 }, "KEYBOARD SHORTCUTS", 16, { .color = { 0, 180, 255 }, .font = &PoppinsLight_16 });
    y += 40;
    auto dk = [&](std::string k, std::string v) {
        d.text({ x + 10, y }, k, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
        d.text({ x + 160, y }, v, 12, { .color = { 180, 180, 190 }, .font = &PoppinsLight_12 });
        y += 24;
    };
    dk("SPACE", "Play / Stop");
    dk("H", "Help");
    dk("E", "EQ modal");
    y += 8;
    dk("1-6", "Trigger note");
    dk("M+1-6", "Mute track");
    dk("G+1-6", "Generate");
    dk("SHIFT+1-6", "Select track");
    y += 8;
    dk("D", "Duplicate");
    dk("DELETE", "De-duplicate / delete page");
    dk("- / +", "Stretch / compress");
    y += 8;
    dk("CTRL+C/V", "Copy / paste step or track");
    dk("SCROLL", "Edit param / step");
    dk("N/V/P/L+SCROLL", "Note / vel / prob / len");
    dk("MIDDLE CLICK", "Cycle step edit mode");
}

// ----------------------------------------------------------------
// drawEQOverlay  — static part only (curves, dots, labels, legend)
// Spectrum strips are drawn separately into the pixel buffer.
// ----------------------------------------------------------------
void drawEQOverlay(Draw& d, sf::Vector2u size)
{
    computeEQRects(size);

    int winW = (int)size.x, winH = (int)size.y;
    int modalW = winW - EQ_MODAL_X * 2;
    // Modal height: enough to contain curve + strips + legend + padding
    int modalH = winH - EQ_MODAL_TOP * 2;

    // Panel
    d.filledRect({ EQ_MODAL_X, EQ_MODAL_TOP }, { modalW, modalH }, { .color = { 16, 16, 20, 245 } });
    d.rect({ EQ_MODAL_X, EQ_MODAL_TOP }, { modalW, modalH }, { .color = { 70, 70, 85 } });

    // Title + hint
    d.text({ EQ_MODAL_X + 14, EQ_MODAL_TOP + 12 }, "EQ", 12, { .color = { 0, 180, 255 }, .font = &PoppinsLight_12 });
    d.text({ EQ_MODAL_X + 38, EQ_MODAL_TOP + 14 },
        "drag dot: Y=gain  X=crossover   click swatch to edit track", 8,
        { .color = { 75, 75, 90 }, .font = &PoppinsLight_8 });

    // Close
    d.filledRect({ eqR.closeRect.left, eqR.closeRect.top }, { eqR.closeRect.width, eqR.closeRect.height }, { .color = { 170, 45, 45 } });
    d.text({ eqR.closeRect.left + 8, eqR.closeRect.top + 2 }, "Close", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // ---- EQ curve canvas ----------------------------------------
    int cx = eqR.canvX, cy = eqR.canvY, cw = eqR.canvW, ch = EQ_CURVE_H;
    Color gridCol = { 32, 32, 42 }, gridLbl = { 68, 68, 82 };

    d.filledRect({ cx, cy }, { cw, ch }, { .color = { 8, 8, 12 } });
    d.rect({ cx, cy }, { cw, ch }, { .color = { 50, 50, 62 } });

    // dB grid
    for (int db : { -12, -6, 0, 6, 12 }) {
        int y = cy + ch / 2 - (int)((float)db / EQ_DB_RANGE * (ch / 2));
        d.line({ cx, y }, { cx + cw, y }, { .color = db == 0 ? Color { 50, 50, 65 } : gridCol });
        d.text({ cx - 44, y - 4 }, (db > 0 ? "+" : "") + std::to_string(db) + "dB", 8, { .color = gridLbl, .font = &PoppinsLight_8 });
    }

    // Frequency grid + labels (shared visual axis with spectrum below)
    for (float f : { 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f }) {
        float t = logf(f / 20.f) / logf(1000.f);
        int x = cx + (int)(cw * t);
        // Full vertical line through EQ canvas
        d.line({ x, cy }, { x, cy + ch }, { .color = gridCol });
        // Tick label sits between EQ canvas and spectrum strips
        std::string lbl = f >= 1000.f ? std::to_string((int)(f / 1000)) + "k" : std::to_string((int)f);
        d.text({ x - 8, cy + ch + 4 }, lbl, 8, { .color = gridLbl, .font = &PoppinsLight_8 });
    }

    // Draw non-active track curves first (faded ghosts), then active on top
    // Build order: non-active tracks in eqTrackOrder, then active last
    auto drawCurve = [&](int t, bool active) {
        auto& trk = studio.tracks[t];
        Color col = trk->themeColor;
        if (!active) {
            // Faded: draw at low alpha by blending toward background
            col.r = (uint8_t)(8 + col.r * 0.12f);
            col.g = (uint8_t)(8 + col.g * 0.12f);
            col.b = (uint8_t)(12 + col.b * 0.12f);
        }
        auto pts = trk->eq.curvePoints(cx, cy, cw, ch, EQ_DB_RANGE, SAMPLE_RATE, 250);
        for (int i = 1; i < (int)pts.size(); i++)
            d.line({ (int)pts[i - 1].first, (int)pts[i - 1].second },
                { (int)pts[i].first, (int)pts[i].second },
                { .color = col });
    };

    for (int t : eqTrackOrder)
        if (t != eqActiveTrack) drawCurve(t, false);
    drawCurve(eqActiveTrack, true);

    // Active track dots only
    {
        auto& trk = studio.tracks[eqActiveTrack];
        Color col = trk->themeColor;
        for (int b = 0; b < EQ::NUM_BANDS; b++) {
            auto [px, py] = trk->eq.dotPos(b, cx, cy, cw, ch, EQ_DB_RANGE);
            d.filledCircle({ (int)px, (int)py }, EQ_DOT_R, { .color = col });
            d.circle({ (int)px, (int)py }, EQ_DOT_R, { .color = { 230, 230, 240 } });
            // Band label under dot
            const char* bLbl[EQ::NUM_BANDS] = { "Low", "Mid", "High" };
            d.text({ (int)px - 10, (int)py + EQ_DOT_R + 3 }, bLbl[b], 8, { .color = { 160, 160, 175 }, .font = &PoppinsLight_8 });
        }
    }

    // Crossover frequency labels on the X axis for active track
    {
        auto& eq = studio.tracks[eqActiveTrack]->eq;
        auto fmtHz = [](float f) -> std::string {
            if (f >= 1000.f) {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(1) << f / 1000.f << "k";
                return ss.str();
            }
            return std::to_string((int)f) + "Hz";
        };
        int xLo = (int)EQ::freqToPx(eq.crossoverLow, cx, cw);
        int xHi = (int)EQ::freqToPx(eq.crossoverHigh, cx, cw);
        d.line({ xLo, cy }, { xLo, cy + ch }, { .color = { 80, 80, 100 } });
        d.line({ xHi, cy }, { xHi, cy + ch }, { .color = { 80, 80, 100 } });
        d.text({ xLo + 3, cy + 4 }, fmtHz(eq.crossoverLow), 8, { .color = { 120, 120, 140 }, .font = &PoppinsLight_8 });
        d.text({ xHi + 3, cy + 4 }, fmtHz(eq.crossoverHigh), 8, { .color = { 120, 120, 140 }, .font = &PoppinsLight_8 });
    }

    // ---- Spectrum strip labels (left axis) ----------------------
    // Each strip: track name on the left, drawn by updateSpectrumPixels into pixel buffer
    int specY = eqR.specBaseY;
    for (int t = 0; t < MAX_TRACKS; t++) {
        Color col = studio.tracks[t]->themeColor;
        bool active = (t == eqActiveTrack);
        // Dim label for inactive tracks
        if (!active) {
            col.r /= 3;
            col.g /= 3;
            col.b /= 3;
        }
        d.text({ cx - 44, specY + 2 }, studio.tracks[t]->engine->getName(), 8,
            { .color = col, .font = &PoppinsLight_8 });
        // Border rect outline for the strip (spectrum pixels drawn into buffer separately)
        d.rect({ cx, specY }, { cw, SPEC_STRIP_H }, { .color = { 40, 40, 52 } });
        specY += SPEC_STRIP_H + SPEC_STRIP_GAP;
    }

    // ---- Legend (click to change active EQ track) ---------------
    int legX = eqR.canvX, legY = eqR.legendRects[0].top;
    for (int t = 0; t < MAX_TRACKS; t++) {
        Color col = studio.tracks[t]->themeColor;
        bool active = (t == eqActiveTrack);
        // Swatch
        d.filledRect({ legX, legY }, { 22, 10 }, { .color = col });
        if (active)
            d.rect({ legX - 2, legY - 2 }, { 26, 14 }, { .color = { 255, 255, 255 } });
        // Label
        Color lblCol = active ? Color { 220, 220, 235 } : Color { 90, 90, 105 };
        d.text({ legX + 26, legY }, studio.tracks[t]->engine->getName(), 8, { .color = lblCol, .font = &PoppinsLight_8 });
        legX += 60;
    }
}

// ----------------------------------------------------------------
// updateSpectrumPixels  — writes spectrum strips into pixel buffer
// Called every frame when EQ modal is open (like updateWaveforms)
// ----------------------------------------------------------------
void updateSpectrumPixels(std::vector<sf::Uint8>& pixels, int stride, sf::Vector2u winSize)
{
    // Run FFT for all tracks
    for (int t = 0; t < MAX_TRACKS; t++)
        studio.tracks[t]->spectrum.compute(SAMPLE_RATE);

    int specY = eqR.specBaseY;
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        Color col = trk->themeColor;
        bool active = (t == eqActiveTrack);

        // Dim inactive strips significantly
        uint8_t rr = active ? col.r : (uint8_t)(col.r * 0.25f);
        uint8_t gg = active ? col.g : (uint8_t)(col.g * 0.25f);
        uint8_t bb = active ? col.b : (uint8_t)(col.b * 0.25f);

        const auto& cols = trk->spectrum.columns;
        int stripTop = specY;
        int stripH = SPEC_STRIP_H;
        int cx = eqR.canvX;
        int cw = eqR.canvW;

        // Clear strip to background first
        for (int y = 0; y < stripH; y++) {
            for (int x = 0; x < cw; x++) {
                size_t idx = ((stripTop + y) * stride + cx + x) * 4;
                if (idx + 3 < pixels.size()) {
                    pixels[idx] = 8;
                    pixels[idx + 1] = 8;
                    pixels[idx + 2] = 12;
                }
            }
        }

        // Draw frequency bars (bottom-aligned within the strip)
        float colPxW = (float)cw / SPEC_COLS;
        for (int c = 0; c < SPEC_COLS; c++) {
            float norm = cols[c];
            int barH = std::max(0, std::min(stripH, (int)(norm * stripH)));
            int barX = cx + (int)(c * colPxW);
            int barW = std::max(1, (int)colPxW - 1);

            for (int y = 0; y < barH; y++) {
                int py = stripTop + stripH - 1 - y; // bottom-aligned
                for (int x = 0; x < barW; x++) {
                    int px = barX + x;
                    size_t idx = ((py)*stride + px) * 4;
                    if (idx + 3 < pixels.size()) {
                        // Brighter at peak (top of bar)
                        float bright = 0.6f + 0.4f * ((float)y / std::max(1, barH));
                        pixels[idx] = (uint8_t)(rr * bright);
                        pixels[idx + 1] = (uint8_t)(gg * bright);
                        pixels[idx + 2] = (uint8_t)(bb * bright);
                    }
                }
            }
        }

        specY += stripH + SPEC_STRIP_GAP;
    }
}

// ----------------------------------------------------------------
// drawStaticUI
// ----------------------------------------------------------------
void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    int winW = size.x, margin = 10, rowH = 26;

    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    // Transport
    studio.transportRect = { margin, 4, 60, 17 };
    d.filledRect({ margin, 4 }, { 60, 17 }, { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.text({ margin + 6, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // Help
    helpBtnRect = { margin + 70, 4, 60, 17 };
    d.filledRect({ helpBtnRect.left, helpBtnRect.top }, { 60, 17 }, { .color = { 60, 60, 75 } });
    d.text({ helpBtnRect.left + 14, 7 }, "HELP", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // EQ
    studio.eqHeaderBtnRect = { margin + 140, 4, 40, 17 };
    d.filledRect({ studio.eqHeaderBtnRect.left, studio.eqHeaderBtnRect.top }, { 40, 17 },
        { .color = showEQ ? Color { 0, 110, 170 } : Color { 40, 60, 75 } });
    d.text({ studio.eqHeaderBtnRect.left + 8, 7 }, "EQ", 8, { .color = { 200, 220, 255 }, .font = &PoppinsLight_8 });

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - margin, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // Engine param panels
    int currentY = 35, paramsPerRow = 8;
    int colW = (winW - margin * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;
        d.filledRect({ margin, currentY }, { colW / 2, 12 }, { .color = d.styles.colors.quaternary });
        d.text({ margin + 4, currentY + 1 }, trk.engine->getName(), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });
        trk.vuRect = sf::IntRect(margin + colW / 2 + 10, currentY - 2, WAVE_HISTORY, 16);
        currentY += 14;

        Param* params = trk.engine->getParams();
        for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
            int x = margin + ((int)p % paramsPerRow) * colW, y = currentY + ((int)p / paramsPerRow) * rowH;
            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });
            if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(params[p].precision) << params[p].value << params[p].unit;
                d.textRight({ x + colW - 6, y + 2 }, params[p].string ? params[p].string : ss.str(), 8,
                    { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
            }
            float range = params[p].max - params[p].min;
            float pct = (params[p].value - params[p].min) / (range <= 0 ? 1.f : range);
            int bX = x + 4, bY = y + rowH - 8, bW = colW - 10;
            if (params[p].type & VALUE_CENTERED) {
                int mid = bX + bW / 2, fw = (int)((bW / 2) * (params[p].value / params[p].max));
                if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = trk.themeColor });
                else d.filledRect({ mid, bY }, { fw, 3 }, { .color = trk.themeColor });
                d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = { 100, 100, 100 } });
            } else {
                d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = trk.themeColor });
            }
        }
        int sh = (((int)trk.engine->getParamCount() + 7) / 8) * rowH + 14;
        trk.trackBounds = sf::IntRect(margin, startY, winW - margin * 2, sh);
        currentY += sh - 2;
    }

    currentY += 10;

    // Mixer row
    int muteW = 25, volW = 70, lenW = 18, genW = 18, eqW = 20, sp = 5;
    int mixW = muteW + sp + volW + sp + lenW + sp + genW + sp + eqW + 10;
    int stepW = (winW - (margin * 2 + mixW)) / 64, stepH = 14;

    for (int i = 0; i < MAX_TRACKS; i++) {
        Track& trk = *studio.tracks[i];
        trk.muteRect = { margin, currentY, muteW, stepH };
        d.filledRect({ trk.muteRect.left, trk.muteRect.top }, { muteW, stepH },
            { .color = trk.isMuted ? Color { 200, 50, 50 } : Color { 40, 40, 45 } });
        d.text({ trk.muteRect.left + 8, trk.muteRect.top + 1 }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        trk.volRect = { trk.muteRect.left + muteW + sp, currentY, volW, stepH };
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { volW, stepH }, { .color = { 40, 40, 45 } });
        d.filledRect({ trk.volRect.left, trk.volRect.top + stepH / 2 - 2 }, { (int)(volW * trk.volume), 4 }, { .color = trk.themeColor });

        trk.lenBtnRect = { trk.volRect.left + volW + sp, currentY, lenW, stepH };
        d.filledRect({ trk.lenBtnRect.left, trk.lenBtnRect.top }, { lenW, stepH }, { .color = { 50, 50, 60 } });
        d.text({ trk.lenBtnRect.left + 4, trk.lenBtnRect.top + 1 }, std::to_string(trk.seqDisplayLen), 8,
            { .color = { 200, 200, 210 }, .font = &PoppinsLight_8 });

        trk.genRect = { trk.lenBtnRect.left + lenW + sp, currentY, genW, stepH };
        d.filledRect({ trk.genRect.left, trk.genRect.top }, { genW, stepH }, { .color = { 60, 60, 75 } });
        d.text({ trk.genRect.left + 5, trk.genRect.top + 1 }, "G", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        trk.eqBtnRect = { trk.genRect.left + genW + sp, currentY, eqW, stepH };
        d.filledRect({ trk.eqBtnRect.left, trk.eqBtnRect.top }, { eqW, stepH }, { .color = { 40, 60, 75 } });
        d.text({ trk.eqBtnRect.left + 3, trk.eqBtnRect.top + 1 }, "EQ", 8, { .color = { 200, 220, 255 }, .font = &PoppinsLight_8 });

        int gx = trk.eqBtnRect.left + eqW + 10;
        for (int s = 0; s < SEQ_STEPS; s++)
            trk.stepRects[s] = { gx + s * stepW, currentY, stepW - 1, stepH / 2 + laneH };
        currentY += 26;
    }

    currentY += 4;

    // Step editor
    if (studio.selTrack != -1 && studio.selStep != -1) {
        auto& s = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        int eY = currentY;
        d.text({ margin, eY }, "EDIT T" + std::to_string(studio.selTrack + 1) + " S" + std::to_string(studio.selStep + 1), 8,
            { .color = studio.tracks[studio.selTrack]->themeColor, .font = &PoppinsLight_8 });
        studio.editNoteRect = { 100, eY - 2, 80, 15 };
        d.text({ 100, eY }, "NOTE: " + std::string(MIDI_NOTES_STR[s.note]), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        studio.editLenRect = { 200, eY - 2, 80, 15 };
        d.text({ 200, eY }, "LEN: " + fToString(s.len, 1) + "steps", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        studio.editVeloRect = { 300, eY - 2, 80, 15 };
        d.text({ 300, eY }, "VEL: " + std::to_string((int)(s.velocity * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        studio.editProbRect = { 400, eY - 2, 80, 15 };
        d.text({ 400, eY }, "PROB: " + std::to_string((int)(s.condition * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
        sf::IntRect* tr = nullptr;
        switch (studio.stepEditMode) {
        case EDIT_NOTE:
            tr = &studio.editNoteRect;
            break;
        case EDIT_VELO:
            tr = &studio.editVeloRect;
            break;
        case EDIT_PROB:
            tr = &studio.editProbRect;
            break;
        case EDIT_LEN:
            tr = &studio.editLenRect;
            break;
        }
        if (tr) d.filledRect({ tr->left, tr->top + tr->height }, { tr->width, 2 },
            { .color = studio.tracks[studio.selTrack]->themeColor });
    }

    // Note lanes
    int sw = studio.tracks[0]->stepRects[0].width + 1;
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            auto& step = trk->sequence[s];
            if (step.active) {
                float nm = 1.f - (float)(CLAMP(step.note, 24, 96) - 24) / 72.f;
                int ny = r.top + r.height - laneH + 1 + (int)(nm * laneH);
                d.line({ r.left, ny }, { r.left + (int)(step.len * (sw - 1)) - 1, ny }, { .color = trk->themeColor });
            }
        }
    }

    if (showEQ) drawEQOverlay(d, size);
    if (showHelp) drawHelpOverlay(d, size);
}

// ----------------------------------------------------------------
// updateWaveforms
// ----------------------------------------------------------------
void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    if (showHelp || showEQ) return;
    for (auto& p : studio.tracks) {
        std::lock_guard<std::mutex> lk(p->historyMtx);
        for (int x = 0; x < WAVE_HISTORY; x++) {
            int bH = (int)(std::min(p->history[x], 1.f) * (p->vuRect.height / 2));
            for (int y = 0; y < p->vuRect.height; y++) {
                size_t idx = ((p->vuRect.top + y) * stride + p->vuRect.left + x) * 4;
                bool w = std::abs(y - p->vuRect.height / 2) <= bH;
                pixels[idx] = w ? p->themeColor.r : 20;
                pixels[idx + 1] = w ? p->themeColor.g : 20;
                pixels[idx + 2] = w ? p->themeColor.b : 25;
            }
        }
    }
}

// ----------------------------------------------------------------
// updateSequencerPixels
// ----------------------------------------------------------------
void updateSequencerPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    if (showHelp || showEQ) return;
    int sw = studio.tracks[0]->stepRects[0].width + 1;
    int ph = (int)((studio.currentStep + studio.sampleCounter.load() / studio.samplesPerStep) * sw);
    int gx = studio.tracks[0]->stepRects[0].left;
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            int h = r.height - laneH;
            Color c = trk->sequence[s].active ? trk->themeColor : (s % 4 == 0 ? Color { 55, 55, 60 } : Color { 45, 45, 50 });
            int selY = (studio.selTrack == t && studio.selStep == s) ? h - 3 : h;
            for (int y = 0; y < h; y++)
                for (int x = 0; x < r.width; x++) {
                    int gX = r.left + x;
                    size_t idx = ((r.top + y) * stride + gX) * 4;
                    if (studio.isPlaying && (gX == gx + ph || gX == gx + ph - 1))
                        pixels[idx] = pixels[idx + 1] = pixels[idx + 2] = 255;
                    else if (y >= selY)
                        pixels[idx] = pixels[idx + 1] = pixels[idx + 2] = 255;
                    else {
                        pixels[idx] = c.r;
                        pixels[idx + 1] = c.g;
                        pixels[idx + 2] = c.b;
                    }
                }
        }
    }
}

// ----------------------------------------------------------------
// main
// ----------------------------------------------------------------
int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");
    sf::RenderWindow window(sf::VideoMode(1080, 1080), "Zic23");
    window.setFramerateLimit(60);

    Styles appStyles = {
        .screen = { 1080, 1080 }, .margin = 2, .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

    for (int i = 0; i < MAX_TRACKS; i++)
        eqTrackOrder.push_back(i);
    eqActiveTrack = 0;

    bool static_needs_redraw = true;
    std::thread aThread(audioWorker, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicBox_Audio");

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0, (float)event.size.width, (float)event.size.height)));
                static_needs_redraw = true;
            }

            // Stop drag on release
            if (event.type == sf::Event::MouseButtonReleased) {
                eqDragging = false;
                eqDragBand = -1;
            }

            // EQ drag move
            if (event.type == sf::Event::MouseMoved && showEQ && eqDragging) {
                float mx = (float)event.mouseMove.x, my = (float)event.mouseMove.y;
                studio.tracks[eqActiveTrack]->eq.applyDrag(
                    eqDragBand, mx, my,
                    eqR.canvX, eqR.canvY, eqR.canvW, EQ_CURVE_H, EQ_DB_RANGE, SAMPLE_RATE);
                static_needs_redraw = true;
            }

            // Key released
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int ti = event.key.code - sf::Keyboard::Num1;
                    int note = (studio.selTrack == ti && studio.selStep != -1)
                        ? studio.tracks[ti]->sequence[studio.selStep].note
                        : 60;
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.tracks[ti]->engine->noteOff(note);
                }
            }

            // Key pressed
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::H) {
                    showHelp = !showHelp;
                    static_needs_redraw = true;
                }
                if (event.key.code == sf::Keyboard::E) {
                    showEQ = !showEQ;
                    static_needs_redraw = true;
                }
                if (event.key.code == sf::Keyboard::Space) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }

                if (studio.selTrack != -1) {
                    if (event.key.code == sf::Keyboard::D) {
                        duplicateTrackSequence(*studio.tracks[studio.selTrack]);
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::Delete) {
                        deleteTrackSequence(*studio.tracks[studio.selTrack]);
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::Dash || event.key.code == sf::Keyboard::Subtract) {
                        stretchTrackSequence(*studio.tracks[studio.selTrack], true);
                        static_needs_redraw = true;
                    }
                    if (event.key.code == sf::Keyboard::Equal || event.key.code == sf::Keyboard::Add) {
                        compressTrackSequence(*studio.tracks[studio.selTrack], true);
                        static_needs_redraw = true;
                    }
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
                    if (event.key.code == sf::Keyboard::C && studio.selTrack != -1 && studio.selStep != -1) {
                        copyTrackIdx = studio.selTrack;
                        copyStepIdx = studio.selStep;
                        copiedStep = studio.tracks[copyTrackIdx]->sequence[copyStepIdx];
                    }
                    if (event.key.code == sf::Keyboard::V && studio.selTrack != -1 && studio.selStep != -1 && copyTrackIdx != -1) {
                        if (copyTrackIdx == studio.selTrack) {
                            studio.tracks[studio.selTrack]->sequence[studio.selStep] = copiedStep;
                        } else {
                            for (int i = 0; i < SEQ_STEPS; i++)
                                studio.tracks[studio.selTrack]->sequence[i] = studio.tracks[copyTrackIdx]->sequence[i];
                            studio.tracks[studio.selTrack]->volume = studio.tracks[copyTrackIdx]->volume;
                            studio.tracks[studio.selTrack]->seqDisplayLen = studio.tracks[copyTrackIdx]->seqDisplayLen;
                        }
                        static_needs_redraw = true;
                    }
                }

                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int ti = event.key.code - sf::Keyboard::Num1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
                        runGeneration(ti);
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
                        studio.tracks[ti]->isMuted = !studio.tracks[ti]->isMuted;
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        studio.selTrack = ti;
                        studio.selStep = 0;
                        static_needs_redraw = true;
                    } else {
                        int note = (studio.selTrack == ti && studio.selStep != -1)
                            ? studio.tracks[ti]->sequence[studio.selStep].note
                            : 60;
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        studio.tracks[ti]->engine->noteOn(note, 1.f);
                    }
                }
            }

            // Mouse pressed
            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;

                if (showEQ) {
                    // Close
                    if (eqR.closeRect.contains(mx, my)) {
                        showEQ = false;
                        static_needs_redraw = true;
                        continue;
                    }

                    // Legend — change active EQ track
                    bool hitLeg = false;
                    for (int t = 0; t < MAX_TRACKS; t++) {
                        if (eqR.legendRects[t].contains(mx, my)) {
                            eqActiveTrack = t;
                            static_needs_redraw = true;
                            hitLeg = true;
                            break;
                        }
                    }

                    // Dot hit-test (active track only)
                    if (!hitLeg) {
                        auto& eq = studio.tracks[eqActiveTrack]->eq;
                        for (int b = 0; b < EQ::NUM_BANDS; b++) {
                            auto [px, py] = eq.dotPos(b, eqR.canvX, eqR.canvY, eqR.canvW, EQ_CURVE_H, EQ_DB_RANGE);
                            int dx = mx - (int)px, dy = my - (int)py;
                            if (dx * dx + dy * dy <= (EQ_DOT_R + 4) * (EQ_DOT_R + 4)) {
                                eqDragging = true;
                                eqDragBand = b;
                                break;
                            }
                        }
                    }
                    continue;
                }

                if (showHelp) {
                    if (helpCloseRect.contains(mx, my)) {
                        showHelp = false;
                        static_needs_redraw = true;
                    }
                    continue;
                }

                if (event.mouseButton.button == sf::Mouse::Middle) {
                    studio.stepEditMode = static_cast<StepEditMode>((studio.stepEditMode + 1) % MODE_COUNT);
                    static_needs_redraw = true;
                }
                if (studio.transportRect.contains(mx, my)) {
                    studio.isPlaying = !studio.isPlaying;
                    static_needs_redraw = true;
                }
                if (helpBtnRect.contains(mx, my)) {
                    showHelp = true;
                    static_needs_redraw = true;
                }
                if (studio.eqHeaderBtnRect.contains(mx, my)) {
                    showEQ = true;
                    static_needs_redraw = true;
                }

                for (int t = 0; t < (int)studio.tracks.size(); t++) {
                    auto& trk = studio.tracks[t];
                    if (trk->genRect.contains(mx, my)) {
                        runGeneration(t);
                        static_needs_redraw = true;
                    }
                    if (trk->muteRect.contains(mx, my)) {
                        trk->isMuted = !trk->isMuted;
                        static_needs_redraw = true;
                    }
                    if (trk->eqBtnRect.contains(mx, my)) {
                        eqActiveTrack = t;
                        showEQ = true;
                        static_needs_redraw = true;
                    }
                    for (int s = 0; s < SEQ_STEPS; s++) {
                        if (trk->stepRects[s].contains(mx, my)) {
                            studio.selTrack = t;
                            studio.selStep = s;
                            if (event.mouseButton.button == sf::Mouse::Left && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                                trk->sequence[s].active = !trk->sequence[s].active;
                                if (trk->sequence[s].active && !studio.isPlaying)
                                    triggerPreview(*trk, trk->sequence[s].note, trk->sequence[s].velocity);
                            }
                            static_needs_redraw = true;
                        }
                    }
                }
            }

            // Scroll
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (showHelp || showEQ) continue;
                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch())
                                   .count();
                bool handled = false;
                for (int t = 0; t < MAX_TRACKS && !handled; t++) {
                    auto& trk = studio.tracks[t];
                    if (trk->lenBtnRect.contains(mx, my)) {
                        if (delta > 0) compressTrackSequence(*trk);
                        else stretchTrackSequence(*trk);
                        static_needs_redraw = true;
                        handled = true;
                        break;
                    }
                    for (int s = 0; s < SEQ_STEPS; s++) {
                        if (trk->stepRects[s].contains(mx, my)) {
                            int sc = delta > 0 ? 1 : -1;
                            auto& step = trk->sequence[s];
                            bool isNote = sf::Keyboard::isKeyPressed(sf::Keyboard::N) || (studio.stepEditMode == EDIT_NOTE && !sf::Keyboard::isKeyPressed(sf::Keyboard::L) && !sf::Keyboard::isKeyPressed(sf::Keyboard::P) && !sf::Keyboard::isKeyPressed(sf::Keyboard::V));
                            if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) editStep(step, EDIT_LEN, sc);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) editStep(step, EDIT_PROB, sc);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) editStep(step, EDIT_VELO, sc);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) editStep(step, EDIT_NOTE, sc);
                            else editStep(step, studio.stepEditMode, sc);
                            if (isNote && !studio.isPlaying) triggerPreview(*trk, step.note, step.velocity);
                            studio.selTrack = t;
                            studio.selStep = s;
                            static_needs_redraw = true;
                            handled = true;
                            break;
                        }
                    }
                }
                if (!handled) {
                    if (studio.bpmRect.contains(mx, my)) {
                        int sc = encGetScaledDirection(delta, now, lastBpmTick);
                        lastBpmTick = now;
                        studio.bpm = CLAMP(studio.bpm + sc * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 0.5f), 20.f, 300.f);
                        studio.updateClock();
                        static_needs_redraw = true;
                    } else if (studio.selTrack != -1 && studio.selStep != -1 && (studio.editNoteRect.contains(mx, my) || studio.editVeloRect.contains(mx, my) || studio.editProbRect.contains(mx, my) || studio.editLenRect.contains(mx, my))) {
                        auto& trk = studio.tracks[studio.selTrack];
                        auto& step = trk->sequence[studio.selStep];
                        int sc = delta > 0 ? 1 : -1;
                        if (studio.editNoteRect.contains(mx, my)) {
                            editStep(step, EDIT_NOTE, sc);
                            if (!studio.isPlaying) triggerPreview(*trk, step.note, step.velocity);
                        } else if (studio.editVeloRect.contains(mx, my)) editStep(step, EDIT_VELO, sc);
                        else if (studio.editProbRect.contains(mx, my)) editStep(step, EDIT_PROB, sc);
                        else if (studio.editLenRect.contains(mx, my)) editStep(step, EDIT_LEN, sc);
                        static_needs_redraw = true;
                    } else {
                        for (auto& trk : studio.tracks) {
                            if (trk->volRect.contains(mx, my)) {
                                int sc = encGetScaledDirection(delta, now, trk->lastVolShiftTick);
                                trk->lastVolShiftTick = now;
                                trk->volume = CLAMP(trk->volume + sc * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f), 0.f, 1.f);
                                static_needs_redraw = true;
                            } else if (trk->trackBounds.contains(mx, my)) {
                                int margin = 4;
                                int rowH = (trk->trackBounds.height - margin * 2) / 8;
                                int winW = trk->trackBounds.width;
                                int cW = (winW - margin * 2) / 8;
                                int pIdx = ((my - (trk->trackBounds.top + 14)) / rowH) * 8 + (mx - margin) / cW;
                                if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                                    Param& p = trk->engine->getParams()[pIdx];
                                    int sc = encGetScaledDirection(delta, now, trk->lastShiftTicks[pIdx]);
                                    trk->lastShiftTicks[pIdx] = now;
                                    p.set(p.value + sc * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 1.f));
                                    trk->activeParamIdx = pIdx;
                                    trk->lastEditTime = std::chrono::steady_clock::now();
                                    static_needs_redraw = true;
                                }
                            }
                        }
                    }
                }
            }
        } // pollEvent

        if (static_needs_redraw) {
            sf::Vector2u ws = window.getSize();
            drawer->setScreenSize({ (int)ws.x, (int)ws.y });
            drawStaticUI(*drawer, ws);
            for (unsigned y = 0; y < ws.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], ws.x * 4);
            static_needs_redraw = false;
        }

        updateWaveforms(pixelBuffer, BUFFER_SIZE);
        updateSequencerPixels(pixelBuffer, BUFFER_SIZE);

        // Spectrum strips — live update every frame when EQ modal is open
        if (showEQ)
            updateSpectrumPixels(pixelBuffer, BUFFER_SIZE, window.getSize());

        screenTexture.update(pixelBuffer.data());
        window.clear();
        window.draw(screenSprite);
        window.display();
    }

    keep_running = false;
    aThread.join();
    snd_pcm_close(pcm_h);
    return 0;
}