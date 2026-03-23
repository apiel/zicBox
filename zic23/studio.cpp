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
// Layout constants
// ----------------------------------------------------------------
const int laneH = 18;

// EQ modal canvas (EQ curve)
static const int EQ_MODAL_TOP = 60;
static const int EQ_MODAL_X = 40;
static const int EQ_CANVAS_X = 60; // relative to modal left
static const int EQ_CANVAS_Y = 50; // relative to modal top
static const int EQ_CANVAS_H = 180;
static const float EQ_DB_RANGE = 12.0f;
static const int EQ_DOT_R = 7;

// Spectrum canvas sits below EQ curve inside the same modal
static const int SPEC_CANVAS_Y = EQ_CANVAS_Y + EQ_CANVAS_H + 30; // relative to modal top
static const int SPEC_CANVAS_H = 110;

// ----------------------------------------------------------------
// Global state
// ----------------------------------------------------------------
bool showHelp = false;
bool showEQ = false;
sf::IntRect helpBtnRect;
sf::IntRect helpCloseRect;

// EQ drag
bool eqDragging = false;
int eqDragTrack = -1;
int eqDragBand = -1;

// Draw order for EQ curves + spectrum (back = drawn last = on top)
// Initialised in main() after studio is ready
std::vector<int> eqTrackOrder; // e.g. {0,1,2,3,4,5}

// Clipboard
int copyTrackIdx = -1;
int copyStepIdx = -1;
Step copiedStep;

// ----------------------------------------------------------------
// Helpers
// ----------------------------------------------------------------

// Absolute canvas rects (recomputed each draw, also used by mouse handlers)
struct EQLayout {
    int canvX, canvY, canvW; // EQ curve canvas
    int specX, specY, specW; // spectrum canvas
};

EQLayout getEQLayout(sf::Vector2u winSize)
{
    int modalW = (int)winSize.x - EQ_MODAL_X * 2;
    int innerW = modalW - EQ_CANVAS_X * 2;
    EQLayout l;
    l.canvX = EQ_MODAL_X + EQ_CANVAS_X;
    l.canvY = EQ_MODAL_TOP + EQ_CANVAS_Y;
    l.canvW = innerW;
    l.specX = l.canvX;
    l.specY = EQ_MODAL_TOP + SPEC_CANVAS_Y;
    l.specW = innerW;
    return l;
}

// ----------------------------------------------------------------
// triggerPreview
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

// ----------------------------------------------------------------
// drawHelpOverlay
// ----------------------------------------------------------------
void drawHelpOverlay(Draw& d, sf::Vector2u size)
{
    int winW = (int)size.x, winH = (int)size.y;
    int x = winW < 1000 ? 50 : 200, y = 50;
    int rectW = winW - x * 2, rectH = winH - y * 2;

    d.filledRect({ x, y }, { rectW, rectH }, { .color = { 20, 20, 25, 235 } });
    d.rect({ x, y }, { rectW, rectH }, { .color = { 200, 200, 205, 235 } });

    helpCloseRect = { x + rectW - 50, y + 10, 40, 12 };
    d.filledRect({ helpCloseRect.left, helpCloseRect.top },
        { helpCloseRect.width, helpCloseRect.height }, { .color = { 200, 50, 50 } });
    d.text({ helpCloseRect.left + 6, helpCloseRect.top + 2 }, "Close", 8,
        { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    d.text({ x + 10, y + 10 }, "KEYBOARD SHORTCUTS", 16,
        { .color = { 0, 180, 255 }, .font = &PoppinsLight_16 });
    y += 40;

    auto dk = [&](std::string key, std::string desc) {
        d.text({ x + 10, y }, key, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
        d.text({ x + 150, y }, desc, 12, { .color = { 180, 180, 190 }, .font = &PoppinsLight_12 });
        y += 24;
    };
    dk("SPACE", "Play / Stop Transport");
    dk("H", "Toggle this Help Menu");
    dk("E", "Toggle EQ Modal");
    y += 10;
    dk("1 - 6", "Trigger Note (Track 1-6)");
    dk("M + 1 - 6", "Toggle Mute Track");
    dk("G + 1 - 6", "Generate Pattern for Track");
    dk("SHIFT + 1 - 6", "Select Track");
    y += 10;
    dk("D", "Duplicate Sequence (Double length)");
    dk("DELETE", "De-duplicate Sequence / Delete Page");
    dk("- (Minus)", "Stretch x2 (Half-time)");
    dk("+ (Plus)", "Compress /2 (Double-time)");
    y += 10;
    dk("CTRL + C / V", "Copy/Paste Step or Track");
    dk("SCROLL", "Edit Parameter / Selected Step");
    dk("N/V/P/L + SCROLL", "Edit Note / Vel / Prob / Len");
    dk("MIDDLE CLICK", "Cycle Step Edit Mode");
}

// ----------------------------------------------------------------
// drawEQOverlay — EQ curves + spectrum stacked bars, shared modal
// ----------------------------------------------------------------
void drawEQOverlay(Draw& d, sf::Vector2u size)
{
    auto L = getEQLayout(size);
    int winW = (int)size.x;
    int winH = (int)size.y;
    int modalW = winW - EQ_MODAL_X * 2;
    int modalH = winH - EQ_MODAL_TOP * 2;

    // Modal background
    d.filledRect({ EQ_MODAL_X, EQ_MODAL_TOP }, { modalW, modalH },
        { .color = { 18, 18, 22, 242 } });
    d.rect({ EQ_MODAL_X, EQ_MODAL_TOP }, { modalW, modalH },
        { .color = { 80, 80, 95 } });

    d.text({ EQ_MODAL_X + 14, EQ_MODAL_TOP + 14 }, "EQ", 12,
        { .color = { 0, 180, 255 }, .font = &PoppinsLight_12 });
    d.text({ EQ_MODAL_X + 40, EQ_MODAL_TOP + 14 }, "— click a track swatch to bring it to front", 8,
        { .color = { 100, 100, 115 }, .font = &PoppinsLight_8 });

    // Close button
    studio.eqCloseRect = { EQ_MODAL_X + modalW - 60, EQ_MODAL_TOP + 10, 50, 16 };
    d.filledRect({ studio.eqCloseRect.left, studio.eqCloseRect.top },
        { studio.eqCloseRect.width, studio.eqCloseRect.height }, { .color = { 180, 50, 50 } });
    d.text({ studio.eqCloseRect.left + 8, studio.eqCloseRect.top + 2 }, "Close", 8,
        { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    Color gridCol = { 35, 35, 45 };
    Color gridLabelCol = { 75, 75, 90 };

    // ---- EQ CURVE CANVAS ----------------------------------------
    d.filledRect({ L.canvX, L.canvY }, { L.canvW, EQ_CANVAS_H }, { .color = { 10, 10, 14 } });
    d.rect({ L.canvX, L.canvY }, { L.canvW, EQ_CANVAS_H }, { .color = { 55, 55, 65 } });

    // dB grid
    for (int db : { -12, -6, 0, 6, 12 }) {
        int y = L.canvY + EQ_CANVAS_H / 2 - (int)((float)db / EQ_DB_RANGE * (EQ_CANVAS_H / 2));
        d.line({ L.canvX, y }, { L.canvX + L.canvW, y },
            { .color = db == 0 ? Color { 55, 55, 68 } : gridCol });
        std::string lbl = (db > 0 ? "+" : "") + std::to_string(db) + "dB";
        d.text({ L.canvX - 36, y - 4 }, lbl, 8, { .color = gridLabelCol, .font = &PoppinsLight_8 });
    }

    // Frequency grid (shared by both canvases — draw labels once between them)
    for (float f : { 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f }) {
        float t = logf(f / 20.f) / logf(1000.f);
        int x = L.canvX + (int)(L.canvW * t);
        d.line({ x, L.canvY }, { x, L.canvY + EQ_CANVAS_H }, { .color = gridCol });
        // Label sits between the two canvases
        std::string lbl = f >= 1000.f
            ? std::to_string((int)(f / 1000)) + "k"
            : std::to_string((int)f);
        d.text({ x - 8, L.canvY + EQ_CANVAS_H + 8 }, lbl, 8,
            { .color = gridLabelCol, .font = &PoppinsLight_8 });
    }

    // Draw EQ curves in eqTrackOrder (last = on top)
    for (int t : eqTrackOrder) {
        auto& trk = studio.tracks[t];
        Color col = trk->themeColor;
        auto pts = trk->eq.curvePoints(L.canvX, L.canvY, L.canvW, EQ_CANVAS_H, EQ_DB_RANGE, SAMPLE_RATE, 300);
        for (int i = 1; i < (int)pts.size(); i++)
            d.line({ (int)pts[i - 1].first, (int)pts[i - 1].second },
                { (int)pts[i].first, (int)pts[i].second },
                { .color = col });
    }

    // Draw control dots in eqTrackOrder (last = on top, so top track dots render last)
    for (int t : eqTrackOrder) {
        auto& trk = studio.tracks[t];
        Color col = trk->themeColor;
        for (int b = 0; b < EQ::NUM_BANDS; b++) {
            auto [px, py] = trk->eq.bandPointPos(b, L.canvX, L.canvY, L.canvW, EQ_CANVAS_H, EQ_DB_RANGE);
            d.filledCircle({ (int)px, (int)py }, EQ_DOT_R, { .color = col });
            d.circle({ (int)px, (int)py }, EQ_DOT_R, { .color = { 220, 220, 230 } });
        }
    }

    // ---- SPECTRUM CANVAS ----------------------------------------
    d.filledRect({ L.specX, L.specY }, { L.specW, SPEC_CANVAS_H }, { .color = { 8, 8, 12 } });
    d.rect({ L.specX, L.specY }, { L.specW, SPEC_CANVAS_H }, { .color = { 55, 55, 65 } });

    // Frequency grid lines on spectrum
    for (float f : { 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f }) {
        float t = logf(f / 20.f) / logf(1000.f);
        int x = L.specX + (int)(L.specW * t);
        d.line({ x, L.specY }, { x, L.specY + SPEC_CANVAS_H }, { .color = gridCol });
    }

    // Run FFT for all tracks and collect columns
    // columns[track][col] -> normalised height 0..1
    std::array<std::array<float, SPEC_COLS>, MAX_TRACKS> cols;
    for (int t = 0; t < MAX_TRACKS; t++) {
        studio.tracks[t]->spectrum.compute(SAMPLE_RATE);
        cols[t] = studio.tracks[t]->spectrum.columns;
    }

    // Draw stacked bars per column
    float colPxW = (float)L.specW / SPEC_COLS;
    for (int c = 0; c < SPEC_COLS; c++) {
        int barX = L.specX + (int)(c * colPxW);
        int barW = std::max(1, (int)colPxW - 1);

        // Sum total height across all tracks for this column
        float totalNorm = 0.f;
        for (int t = 0; t < MAX_TRACKS; t++)
            totalNorm += cols[t][c];

        // Cap at 1 so bars never exceed canvas height
        float scale = (totalNorm > 1.f) ? 1.f / totalNorm : 1.f;

        // Draw segments bottom-up in eqTrackOrder
        int bottomY = L.specY + SPEC_CANVAS_H;
        for (int t : eqTrackOrder) {
            float norm = cols[t][c] * scale;
            int segH = (int)(norm * SPEC_CANVAS_H);
            if (segH <= 0) continue;
            int segY = bottomY - segH;
            d.filledRect({ barX, segY }, { barW, segH },
                { .color = studio.tracks[t]->themeColor });
            bottomY = segY;
        }
    }

    // "SPECTRUM" label top-left of spectrum canvas
    d.text({ L.specX + 4, L.specY + 4 }, "SPECTRUM", 8,
        { .color = { 60, 60, 75 }, .font = &PoppinsLight_8 });

    // ---- LEGEND -------------------------------------------------
    // Swatches below the spectrum canvas. Click = bring to front.
    int legY = L.specY + SPEC_CANVAS_H + 14;
    int legX = L.specX;
    for (int t = 0; t < MAX_TRACKS; t++) {
        Color col = studio.tracks[t]->themeColor;
        // Highlight if this track is on top (last in order)
        bool isTop = (eqTrackOrder.back() == t);
        d.filledRect({ legX, legY }, { 22, 10 }, { .color = col });
        if (isTop)
            d.rect({ legX - 1, legY - 1 }, { 24, 12 }, { .color = { 255, 255, 255 } });
        d.text({ legX + 26, legY }, "T" + std::to_string(t + 1), 8,
            { .color = { 180, 180, 200 }, .font = &PoppinsLight_8 });

        // Store swatch rect in studio for hit-testing
        studio.eqLegendRects[t] = { legX, legY, 22, 10 };
        legX += 62;
    }
}

// ----------------------------------------------------------------
// drawStaticUI
// ----------------------------------------------------------------
void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    int winW = (int)size.x, margin = 10, rowH = 26;

    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    // Transport
    studio.transportRect = { margin, 4, 60, 17 };
    d.filledRect({ margin, 4 }, { 60, 17 },
        { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.text({ margin + 6, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8,
        { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // Help button
    helpBtnRect = { margin + 70, 4, 60, 17 };
    d.filledRect({ helpBtnRect.left, helpBtnRect.top }, { 60, 17 }, { .color = { 60, 60, 75 } });
    d.text({ helpBtnRect.left + 14, 7 }, "HELP", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // EQ button
    studio.eqHeaderBtnRect = { margin + 140, 4, 40, 17 };
    d.filledRect({ studio.eqHeaderBtnRect.left, studio.eqHeaderBtnRect.top }, { 40, 17 },
        { .color = showEQ ? Color { 0, 120, 180 } : Color { 40, 60, 75 } });
    d.text({ studio.eqHeaderBtnRect.left + 8, 7 }, "EQ", 8,
        { .color = { 200, 220, 255 }, .font = &PoppinsLight_8 });

    // BPM
    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - margin, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // Per-track param panels
    int currentY = 35;
    int paramsPerRow = 8, colW = (winW - margin * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;
        d.filledRect({ margin, currentY }, { colW / 2, 12 }, { .color = d.styles.colors.quaternary });
        d.text({ margin + 4, currentY + 1 }, trk.engine->getName(), 8,
            { .color = trk.themeColor, .font = &PoppinsLight_8 });

        trk.vuRect = sf::IntRect(margin + colW / 2 + 10, currentY - 2, WAVE_HISTORY, 16);
        currentY += 14;

        Param* params = trk.engine->getParams();
        for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
            int x = margin + ((int)p % paramsPerRow) * colW;
            int y = currentY + ((int)p / paramsPerRow) * rowH;
            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });
            d.text({ x + 4, y + 2 }, params[p].label, 12,
                { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

            if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(params[p].precision)
                   << params[p].value << params[p].unit;
                d.textRight({ x + colW - 6, y + 2 },
                    params[p].string ? params[p].string : ss.str(), 8,
                    { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
            }

            float range = params[p].max - params[p].min;
            float pct = (params[p].value - params[p].min) / (range <= 0 ? 1.f : range);
            int bW = colW - 10, bH = 3, bX = x + 4, bY = y + rowH - 8;
            if (params[p].type & VALUE_CENTERED) {
                int midX = bX + bW / 2;
                int fillW = (int)((bW / 2) * (params[p].value / (params[p].max)));
                if (fillW < 0) d.filledRect({ midX + fillW, bY }, { std::abs(fillW), bH }, { .color = trk.themeColor });
                else d.filledRect({ midX, bY }, { fillW, bH }, { .color = trk.themeColor });
                d.filledRect({ midX, bY - 1 }, { 1, bH + 2 }, { .color = { 100, 100, 100 } });
            } else {
                d.filledRect({ bX, bY }, { (int)(bW * pct), bH }, { .color = trk.themeColor });
            }
        }
        int sectionH = (((int)trk.engine->getParamCount() + 7) / 8) * rowH + 14;
        trk.trackBounds = sf::IntRect(margin, startY, winW - (margin * 2), sectionH);
        currentY += sectionH - 2;
    }

    currentY += 10;

    // Mixer row
    int muteW = 25, volW = 70, lenW = 18, genW = 18, eqW = 20, spacing = 5;
    int mixerW = muteW + spacing + volW + spacing + lenW + spacing + genW + spacing + eqW + 10;
    int stepW = (winW - (margin * 2 + mixerW)) / 64;
    int stepH = 14;

    for (int i = 0; i < MAX_TRACKS; i++) {
        Track& trk = *studio.tracks[i];

        trk.muteRect = { margin, currentY, muteW, stepH };
        d.filledRect({ trk.muteRect.left, trk.muteRect.top }, { muteW, stepH },
            { .color = trk.isMuted ? Color { 200, 50, 50 } : Color { 40, 40, 45 } });
        d.text({ trk.muteRect.left + 8, trk.muteRect.top + 1 }, "M", 8,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        trk.volRect = { trk.muteRect.left + muteW + spacing, currentY, volW, stepH };
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { volW, stepH }, { .color = { 40, 40, 45 } });
        d.filledRect({ trk.volRect.left, trk.volRect.top + stepH / 2 - 2 },
            { (int)(volW * trk.volume), 4 }, { .color = trk.themeColor });

        trk.lenBtnRect = { trk.volRect.left + volW + spacing, currentY, lenW, stepH };
        d.filledRect({ trk.lenBtnRect.left, trk.lenBtnRect.top }, { lenW, stepH }, { .color = { 50, 50, 60 } });
        d.text({ trk.lenBtnRect.left + 4, trk.lenBtnRect.top + 1 },
            std::to_string(trk.seqDisplayLen), 8, { .color = { 200, 200, 210 }, .font = &PoppinsLight_8 });

        trk.genRect = { trk.lenBtnRect.left + lenW + spacing, currentY, genW, stepH };
        d.filledRect({ trk.genRect.left, trk.genRect.top }, { genW, stepH }, { .color = { 60, 60, 75 } });
        d.text({ trk.genRect.left + 5, trk.genRect.top + 1 }, "G", 8,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        trk.eqBtnRect = { trk.genRect.left + genW + spacing, currentY, eqW, stepH };
        d.filledRect({ trk.eqBtnRect.left, trk.eqBtnRect.top }, { eqW, stepH }, { .color = { 40, 60, 75 } });
        d.text({ trk.eqBtnRect.left + 3, trk.eqBtnRect.top + 1 }, "EQ", 8,
            { .color = { 200, 220, 255 }, .font = &PoppinsLight_8 });

        int gridStartX = trk.eqBtnRect.left + eqW + 10;
        for (int s = 0; s < SEQ_STEPS; s++)
            trk.stepRects[s] = { gridStartX + s * stepW, currentY, stepW - 1, stepH / 2 + laneH };

        currentY += 26;
    }

    currentY += 4;

    // Step editor row
    if (studio.selTrack != -1 && studio.selStep != -1) {
        auto& s = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        int eY = currentY;
        d.text({ margin, eY }, "EDIT T" + std::to_string(studio.selTrack + 1) + " S" + std::to_string(studio.selStep + 1), 8,
            { .color = studio.tracks[studio.selTrack]->themeColor, .font = &PoppinsLight_8 });

        studio.editNoteRect = { 100, eY - 2, 80, 15 };
        d.text({ 100, eY }, "NOTE: " + std::string(MIDI_NOTES_STR[s.note]), 8,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        studio.editLenRect = { 200, eY - 2, 80, 15 };
        d.text({ 200, eY }, "LEN: " + fToString(s.len, 1) + "steps", 8,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        studio.editVeloRect = { 300, eY - 2, 80, 15 };
        d.text({ 300, eY }, "VEL: " + std::to_string((int)(s.velocity * 100)) + "%", 8,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        studio.editProbRect = { 400, eY - 2, 80, 15 };
        d.text({ 400, eY }, "PROB: " + std::to_string((int)(s.condition * 100)) + "%", 8,
            { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

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
        if (tr)
            d.filledRect({ tr->left, tr->top + tr->height }, { tr->width, 2 },
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
                int noteY = r.top + r.height - laneH + 1 + (int)(nm * laneH);
                int pxLen = (int)(step.len * (sw - 1)) - 1;
                d.line({ r.left, noteY }, { r.left + pxLen, noteY }, { .color = trk->themeColor });
            }
        }
    }

    // Overlays (EQ below help so help is always on top)
    if (showEQ) drawEQOverlay(d, size);
    if (showHelp) drawHelpOverlay(d, size);
}

// ----------------------------------------------------------------
// updateWaveforms
// ----------------------------------------------------------------
void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    if (showHelp || showEQ) return;
    for (auto& trkPtr : studio.tracks) {
        std::lock_guard<std::mutex> hLock(trkPtr->historyMtx);
        for (int x = 0; x < WAVE_HISTORY; x++) {
            int barH = (int)(std::min(trkPtr->history[x], 1.f) * (trkPtr->vuRect.height / 2));
            for (int y = 0; y < trkPtr->vuRect.height; y++) {
                size_t idx = ((trkPtr->vuRect.top + y) * stride + trkPtr->vuRect.left + x) * 4;
                bool isWave = std::abs(y - trkPtr->vuRect.height / 2) <= barH;
                pixels[idx] = isWave ? trkPtr->themeColor.r : 20;
                pixels[idx + 1] = isWave ? trkPtr->themeColor.g : 20;
                pixels[idx + 2] = isWave ? trkPtr->themeColor.b : 25;
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
    int stepW = studio.tracks[0]->stepRects[0].width + 1;
    double progressInStep = studio.sampleCounter.load() / studio.samplesPerStep;
    int phGX = (int)((studio.currentStep + progressInStep) * stepW);
    int gridStartX = studio.tracks[0]->stepRects[0].left;

    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            int h = r.height - laneH;
            Color c = trk->sequence[s].active
                ? trk->themeColor
                : (s % 4 == 0 ? Color { 55, 55, 60 } : Color { 45, 45, 50 });
            int selY = (studio.selTrack == t && studio.selStep == s) ? h - 3 : h;

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < r.width; x++) {
                    int gx = r.left + x;
                    size_t idx = ((r.top + y) * stride + gx) * 4;
                    if (studio.isPlaying && (gx == gridStartX + phGX || gx == gridStartX + phGX - 1))
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

    // Initialise track draw order
    for (int i = 0; i < MAX_TRACKS; i++)
        eqTrackOrder.push_back(i);

    bool static_needs_redraw = true;
    std::thread aThread(audioWorker, pcm_h);
    pthread_setname_np(aThread.native_handle(), "zicBox_Audio");

    while (window.isOpen() && keep_running) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect(0, 0,
                    (float)event.size.width, (float)event.size.height)));
                static_needs_redraw = true;
            }

            // Stop EQ drag on mouse release
            if (event.type == sf::Event::MouseButtonReleased) {
                eqDragging = false;
                eqDragTrack = -1;
                eqDragBand = -1;
            }

            // EQ drag update
            if (event.type == sf::Event::MouseMoved && showEQ && eqDragging) {
                auto L = getEQLayout(window.getSize());
                auto& eq = studio.tracks[eqDragTrack]->eq;
                eq.setBandFreqFromX(eqDragBand, (float)event.mouseMove.x, L.canvX, L.canvW);
                eq.setBandGainFromY(eqDragBand, (float)event.mouseMove.y, L.canvY, EQ_CANVAS_H, EQ_DB_RANGE);
                eq.recompute(SAMPLE_RATE);
                static_needs_redraw = true;
            }

            // Key released — note off
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

                // Copy/Paste
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

                // Track shortcuts 1–6
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

                // ---- EQ modal intercepts first ----
                if (showEQ) {
                    // Close
                    if (studio.eqCloseRect.contains(mx, my)) {
                        showEQ = false;
                        static_needs_redraw = true;
                        continue;
                    }

                    // Legend swatches — bring track to front (move to back of draw order)
                    bool hitLegend = false;
                    for (int t = 0; t < MAX_TRACKS; t++) {
                        if (studio.eqLegendRects[t].contains(mx, my)) {
                            eqTrackOrder.erase(std::remove(eqTrackOrder.begin(), eqTrackOrder.end(), t), eqTrackOrder.end());
                            eqTrackOrder.push_back(t);
                            static_needs_redraw = true;
                            hitLegend = true;
                            break;
                        }
                    }

                    if (!hitLegend) {
                        // EQ control dots — hit-test in reverse draw order (topmost first)
                        auto L = getEQLayout(window.getSize());
                        for (int oi = (int)eqTrackOrder.size() - 1; oi >= 0 && !eqDragging; oi--) {
                            int t = eqTrackOrder[oi];
                            for (int b = 0; b < EQ::NUM_BANDS; b++) {
                                auto [px, py] = studio.tracks[t]->eq.bandPointPos(
                                    b, L.canvX, L.canvY, L.canvW, EQ_CANVAS_H, EQ_DB_RANGE);
                                int dx = mx - (int)px, dy = my - (int)py;
                                if (dx * dx + dy * dy <= (EQ_DOT_R + 3) * (EQ_DOT_R + 3)) {
                                    eqDragging = true;
                                    eqDragTrack = t;
                                    eqDragBand = b;
                                    // Also bring that track to front
                                    eqTrackOrder.erase(std::remove(eqTrackOrder.begin(), eqTrackOrder.end(), t), eqTrackOrder.end());
                                    eqTrackOrder.push_back(t);
                                    break;
                                }
                            }
                        }
                    }
                    continue; // consume all clicks inside modal
                }

                // ---- Help modal intercepts ----
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

            // Mouse scroll
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
                            int scaled = delta > 0 ? 1 : -1;
                            auto& step = trk->sequence[s];
                            bool isNoteEdit = sf::Keyboard::isKeyPressed(sf::Keyboard::N)
                                || (studio.stepEditMode == EDIT_NOTE
                                    && !sf::Keyboard::isKeyPressed(sf::Keyboard::L)
                                    && !sf::Keyboard::isKeyPressed(sf::Keyboard::P)
                                    && !sf::Keyboard::isKeyPressed(sf::Keyboard::V));
                            if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) editStep(step, EDIT_LEN, scaled);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) editStep(step, EDIT_PROB, scaled);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) editStep(step, EDIT_VELO, scaled);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) editStep(step, EDIT_NOTE, scaled);
                            else editStep(step, studio.stepEditMode, scaled);
                            if (isNoteEdit && !studio.isPlaying)
                                triggerPreview(*trk, step.note, step.velocity);
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
                        int scaled = encGetScaledDirection(delta, now, lastBpmTick);
                        lastBpmTick = now;
                        studio.bpm = CLAMP(studio.bpm + scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 0.5f), 20.f, 300.f);
                        studio.updateClock();
                        static_needs_redraw = true;
                    } else if (studio.selTrack != -1 && studio.selStep != -1 && (studio.editNoteRect.contains(mx, my) || studio.editVeloRect.contains(mx, my) || studio.editProbRect.contains(mx, my) || studio.editLenRect.contains(mx, my))) {
                        auto& trk = studio.tracks[studio.selTrack];
                        auto& step = trk->sequence[studio.selStep];
                        int scaled = delta > 0 ? 1 : -1;
                        if (studio.editNoteRect.contains(mx, my)) {
                            editStep(step, EDIT_NOTE, scaled);
                            if (!studio.isPlaying) triggerPreview(*trk, step.note, step.velocity);
                        } else if (studio.editVeloRect.contains(mx, my)) editStep(step, EDIT_VELO, scaled);
                        else if (studio.editProbRect.contains(mx, my)) editStep(step, EDIT_PROB, scaled);
                        else if (studio.editLenRect.contains(mx, my)) editStep(step, EDIT_LEN, scaled);
                        static_needs_redraw = true;
                    } else {
                        for (auto& trk : studio.tracks) {
                            if (trk->volRect.contains(mx, my)) {
                                int scaled = encGetScaledDirection(delta, now, trk->lastVolShiftTick);
                                trk->lastVolShiftTick = now;
                                trk->volume = CLAMP(trk->volume + scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f), 0.f, 1.f);
                                static_needs_redraw = true;
                            } else if (trk->trackBounds.contains(mx, my)) {
                                int margin = 4;
                                int rowH = ((int)window.getSize().y - margin * 2) / 9;
                                int cW = ((int)window.getSize().x - margin * 2) / 8;
                                int pIdx = ((my - (trk->trackBounds.top + 14)) / rowH) * 8 + (mx - margin) / cW;
                                if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                                    Param& p = trk->engine->getParams()[pIdx];
                                    int scaled = encGetScaledDirection(delta, now, trk->lastShiftTicks[pIdx]);
                                    trk->lastShiftTicks[pIdx] = now;
                                    p.set(p.value + scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.f : 1.f));
                                    trk->activeParamIdx = pIdx;
                                    trk->lastEditTime = std::chrono::steady_clock::now();
                                    static_needs_redraw = true;
                                }
                            }
                        }
                    }
                }
            }
        } // end pollEvent

        if (static_needs_redraw) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawStaticUI(*drawer, winSize);
            for (unsigned int y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
            static_needs_redraw = false;
        }

        updateWaveforms(pixelBuffer, BUFFER_SIZE);
        updateSequencerPixels(pixelBuffer, BUFFER_SIZE);

        // Live spectrum redraw every frame when EQ modal is open
        if (showEQ) {
            sf::Vector2u winSize = window.getSize();
            drawer->setScreenSize({ (int)winSize.x, (int)winSize.y });
            drawEQOverlay(*drawer, winSize);
            for (unsigned int y = 0; y < winSize.y; y++)
                std::memcpy(&pixelBuffer[y * BUFFER_SIZE * 4], drawer->screenBuffer[y], winSize.x * 4);
        }

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