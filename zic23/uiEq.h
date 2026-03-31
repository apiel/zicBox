#pragma once

#include "zic23/studio.h"

// Spectrum strip (drawn next to the VU meter, same row as the track name)
static constexpr int SPEC_W = 350; // width of spectrum strip in px

// EQ editor zone (below the sequencer grid)
static constexpr int EQ_ZONE_H = 120; // total height of the EQ editor area
static constexpr int EQ_TRACK_W = 80; // left column: track selector buttons
static constexpr float EQ_DB_RANGE = 12.f;
static constexpr int EQ_DOT_R = 7; // dot hit radius

static int eqActiveTrack = 0;

// EQ drag
static bool eqDragging = false;
static int eqDragBand = -1;

// Spectrum strip rects — set during drawStaticUI, read by updateSpectrumPixels
// One per track: absolute pixel rect of the spectrum strip
static sf::IntRect specRects[MAX_TRACKS];

// EQ canvas rect — set during drawStaticUI, shared with mouse handler and pixel updater
static sf::IntRect eqCanvasRect; // the curve drawing area (absolute pixels)
static float eqCanvasY_f; // float version for sub-pixel accuracy
static float eqCanvasH_f;

// Track selector button rects (inside EQ zone)
static sf::IntRect eqTrackBtnRects[MAX_TRACKS];

void drawEqUI(Draw& d, sf::Vector2u size, int currentY)
{
    int eqZoneTop = currentY;
    int eqZoneBot = eqZoneTop + EQ_ZONE_H;

    d.filledRect({ 0, eqZoneTop }, { (int)size.x, EQ_ZONE_H }, { .color = { 10, 10, 14 } });

    // Track selector column (left)
    int btnH = EQ_ZONE_H / MAX_TRACKS - 2;
    for (int t = 0; t < MAX_TRACKS; t++) {
        int btnY = eqZoneTop + t * (btnH + 2);
        bool active = (t == eqActiveTrack);
        Color col = studio.tracks[t]->themeColor;
        Color bg = active
            ? Color { (uint8_t)(col.r / 2), (uint8_t)(col.g / 2), (uint8_t)(col.b / 2) }
            : Color { 25, 25, 30 };
        eqTrackBtnRects[t] = { MARGIN, btnY, EQ_TRACK_W - 4, btnH };
        d.filledRect({ eqTrackBtnRects[t].left, eqTrackBtnRects[t].top },
            { eqTrackBtnRects[t].width, eqTrackBtnRects[t].height }, { .color = bg });
        if (active) d.rect({ eqTrackBtnRects[t].left, eqTrackBtnRects[t].top },
            { eqTrackBtnRects[t].width, eqTrackBtnRects[t].height }, { .color = col });
        d.text({ eqTrackBtnRects[t].left + 6, btnY + btnH / 2 - 5 },
            studio.tracks[t]->engine->getName(), 8, { .color = col, .font = &PoppinsLight_8 });
    }

    // EQ curve canvas
    int eqCx = specRects[0].left;
    int eqCw = SPEC_W; // same width as spectrum strip
    int eqCy = eqZoneTop + 10;
    int eqCh = EQ_ZONE_H - 20;

    eqCanvasRect = sf::IntRect(eqCx, eqCy, eqCw, eqCh);
    eqCanvasY_f = (float)eqCy;
    eqCanvasH_f = (float)eqCh;

    d.filledRect({ eqCx, eqCy }, { eqCw, eqCh }, { .color = { 8, 8, 12 } });
    d.rect({ eqCx, eqCy }, { eqCw, eqCh }, { .color = { 45, 45, 58 } });

    Color gridCol = { 30, 30, 40 }, gridLbl = { 65, 65, 80 };
    for (int db : { -12, -6, 0, 6, 12 }) {
        int y = eqCy + eqCh / 2 - (int)((float)db / EQ_DB_RANGE * (eqCh / 2));
        d.line({ eqCx, y }, { eqCx + eqCw, y }, { .color = db == 0 ? Color { 50, 50, 65 } : gridCol });
        d.text({ eqCx - 38, y - 4 }, (db > 0 ? "+" : "") + std::to_string(db) + "dB", 8, { .color = gridLbl, .font = &PoppinsLight_8 });
    }

    for (float f : { 100.f, 500.f, 1000.f, 5000.f, 10000.f }) {
        float t = logf(f / 20.f) / logf(1000.f);
        int x = eqCx + (int)(eqCw * t);
        d.line({ x, eqCy }, { x, eqCy + eqCh }, { .color = gridCol });
    }

    // Active track EQ curve
    {
        auto& eq = studio.tracks[eqActiveTrack]->eq;
        Color col = studio.tracks[eqActiveTrack]->themeColor;

        // Ghost curves for other tracks (very faint)
        for (int t = 0; t < MAX_TRACKS; t++) {
            if (t == eqActiveTrack) continue;
            Color gc = studio.tracks[t]->themeColor;
            gc.r = (uint8_t)(8 + gc.r * 0.20f);
            gc.g = (uint8_t)(8 + gc.g * 0.20f);
            gc.b = (uint8_t)(12 + gc.b * 0.20f);
            auto pts = studio.tracks[t]->eq.curvePoints(
                (float)eqCx, (float)eqCy, (float)eqCw, (float)eqCh, EQ_DB_RANGE, SAMPLE_RATE, 150);
            for (int i = 1; i < (int)pts.size(); i++)
                d.line({ (int)pts[i - 1].first, (int)pts[i - 1].second },
                    { (int)pts[i].first, (int)pts[i].second }, { .color = gc });
        }

        // Active curve (full color)
        auto pts = eq.curvePoints((float)eqCx, (float)eqCy, (float)eqCw, (float)eqCh,
            EQ_DB_RANGE, SAMPLE_RATE, 250);
        for (int i = 1; i < (int)pts.size(); i++)
            d.line({ (int)pts[i - 1].first, (int)pts[i - 1].second },
                { (int)pts[i].first, (int)pts[i].second }, { .color = col });

        // Dots + labels
        const char* bLbl[] = { "Lo", "Mid", "Hi" };
        for (int b = 0; b < EQ::NUM_BANDS; b++) {
            auto [px, py] = eq.dotPos(b, (float)eqCx, (float)eqCy, (float)eqCw, (float)eqCh, EQ_DB_RANGE);
            d.filledCircle({ (int)px, (int)py }, EQ_DOT_R, { .color = col });
            d.circle({ (int)px, (int)py }, EQ_DOT_R, { .color = { 230, 230, 240 } });
            d.text({ (int)px - 8, (int)py + EQ_DOT_R + 2 }, bLbl[b], 8, { .color = { 150, 150, 165 }, .font = &PoppinsLight_8 });
        }

        // Crossover lines
        auto fmtHz = [](float f) -> std::string {
            if (f >= 1000.f) {
                std::ostringstream s;
                s << std::fixed << std::setprecision(1) << f / 1000.f << "k";
                return s.str();
            }
            return std::to_string((int)f) + "Hz";
        };
        int xLo = (int)EQ::freqToPx(eq.crossoverLow, (float)eqCx, (float)eqCw);
        int xHi = (int)EQ::freqToPx(eq.crossoverHigh, (float)eqCx, (float)eqCw);
        d.line({ xLo, eqCy }, { xLo, eqCy + eqCh }, { .color = { 70, 70, 90 } });
        d.line({ xHi, eqCy }, { xHi, eqCy + eqCh }, { .color = { 70, 70, 90 } });
        d.text({ xLo + 2, eqCy + 2 }, fmtHz(eq.crossoverLow), 8, { .color = { 110, 110, 130 }, .font = &PoppinsLight_8 });
        d.text({ xHi + 2, eqCy + 2 }, fmtHz(eq.crossoverHigh), 8, { .color = { 110, 110, 130 }, .font = &PoppinsLight_8 });
    }
}

// Draw spectrum strip directly to pixels buffer for optimization
void updateSpectrumPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        // Run FFT on this track's post-EQ ring buffer
        if (!trk->spectrum.compute(SAMPLE_RATE)) continue;

        const auto& cols = trk->spectrum.columns;
        const auto& sr = specRects[t];
        Color col = trk->themeColor;

        // Clear strip to dark background
        for (int y = 0; y < sr.height; y++) {
            for (int x = 0; x < sr.width; x++) {
                size_t idx = ((sr.top + y) * stride + sr.left + x) * 4;
                if (idx + 2 < pixels.size()) {
                    pixels[idx] = 8;
                    pixels[idx + 1] = 8;
                    pixels[idx + 2] = 12;
                }
            }
        }

        if (!trk->spectrum.updated) continue;

        // Draw frequency bars bottom-aligned
        float colPxW = (float)sr.width / SPEC_COLS;
        for (int c = 0; c < SPEC_COLS; c++) {
            float norm = cols[c];
            int barH = std::max(0, std::min(sr.height, (int)(norm * sr.height)));
            int barX = sr.left + (int)(c * colPxW);
            int barW = std::max(1, (int)colPxW);

            for (int y = 0; y < barH; y++) {
                int py = sr.top + sr.height - 1 - y; // bottom-aligned
                float bright = 0.55f + 0.45f * ((float)y / std::max(1, barH));
                for (int x = 0; x < barW; x++) {
                    size_t idx = ((py)*stride + barX + x) * 4;
                    if (idx + 2 < pixels.size()) {
                        pixels[idx] = (uint8_t)(col.r * bright);
                        pixels[idx + 1] = (uint8_t)(col.g * bright);
                        pixels[idx + 2] = (uint8_t)(col.b * bright);
                    }
                }
            }
        }
    }
}