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
#include "zic23/generator.h"
#include "zic23/studio.h"
#include "zic23/ui.h"

const int laneH = 18;
bool showHelp = false;
bool showEQ   = false;
sf::IntRect helpBtnRect;
sf::IntRect helpCloseRect;

// EQ drag state
bool eqDragging  = false;
int  eqDragTrack = -1;
int  eqDragBand  = -1;

// Clipboard state
int copyTrackIdx = -1;
int copyStepIdx  = -1;
Step copiedStep;

// EQ canvas layout constants (shared between draw and mouse handlers)
static const int   EQ_CANVAS_X  = 60;
static const int   EQ_CANVAS_Y  = 100; // relative to modal top
static const int   EQ_CANVAS_W  = 860;
static const int   EQ_CANVAS_H  = 220;
static const float EQ_DB_RANGE  = 12.0f;
static const int   EQ_DOT_R     = 7;

// ---------------------------------------------------------------
// triggerPreview
// ---------------------------------------------------------------
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

// ---------------------------------------------------------------
// drawHelpOverlay
// ---------------------------------------------------------------
void drawHelpOverlay(Draw& d, sf::Vector2u size)
{
    int winW = (int)size.x, winH = (int)size.y;
    int x = winW < 1000 ? 50 : 200, y = 50;
    int rectH = winH - y * 2;
    int rectW = winW - x * 2;

    d.filledRect({ x, y }, { rectW, rectH }, { .color = { 20, 20, 25, 235 } });
    d.rect({ x, y }, { rectW, rectH }, { .color = { 200, 200, 205, 235 } });

    helpCloseRect = { x + rectW - 50, y + 10, 40, 12 };
    d.filledRect({ helpCloseRect.left, helpCloseRect.top }, { helpCloseRect.width, helpCloseRect.height }, { .color = { 200, 50, 50 } });
    d.text({ helpCloseRect.left + 6, helpCloseRect.top + 2 }, "Close", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    d.text({ x + 10, y + 10 }, "KEYBOARD SHORTCUTS", 16, { .color = { 0, 180, 255 }, .font = &PoppinsLight_16 });
    y += 40;

    auto drawKey = [&](std::string key, std::string desc) {
        d.text({ x + 10, y }, key, 12, { .color = { 255, 255, 255 }, .font = &PoppinsLight_12 });
        d.text({ x + 150, y }, desc, 12, { .color = { 180, 180, 190 }, .font = &PoppinsLight_12 });
        y += 24;
    };

    drawKey("SPACE", "Play / Stop Transport");
    drawKey("H", "Toggle this Help Menu");
    drawKey("E", "Toggle EQ Modal");
    y += 10;
    drawKey("1 - 6", "Trigger Note (Track 1-6)");
    drawKey("M + 1 - 6", "Toggle Mute Track");
    drawKey("G + 1 - 6", "Generate Pattern for Track");
    drawKey("SHIFT + 1 - 6", "Select Track");
    y += 10;
    drawKey("D", "Duplicate Sequence (Double length)");
    drawKey("DELETE", "De-duplicate Sequence / Delete Page");
    drawKey("- (Minus)", "Stretch x2 (Half-time)");
    drawKey("+ (Plus)", "Compress /2 (Double-time)");
    y += 10;
    drawKey("CTRL + C / V", "Copy/Paste Step or Track");
    drawKey("SCROLL", "Edit Parameter / Selected Step");
    drawKey("N / V / P / L + SCROLL", "Edit Note / Vel / Prob / Len");
    drawKey("MIDDLE CLICK", "Cycle Step Edit Mode");
}

// ---------------------------------------------------------------
// drawEQOverlay
// ---------------------------------------------------------------
void drawEQOverlay(Draw& d, sf::Vector2u size, int modalTop = 60)
{
    int winW   = (int)size.x;
    int modalX = 40;
    int modalW = winW - 80;
    int modalH = (int)size.y - modalTop * 2;

    d.filledRect({ modalX, modalTop }, { modalW, modalH }, { .color = { 18, 18, 22, 240 } });
    d.rect({ modalX, modalTop }, { modalW, modalH }, { .color = { 80, 80, 95 } });

    d.text({ modalX + 14, modalTop + 12 }, "EQ — all tracks", 12,
        { .color = { 180, 180, 200 }, .font = &PoppinsLight_12 });

    // Close button
    sf::IntRect closeR = { modalX + modalW - 60, modalTop + 8, 50, 16 };
    d.filledRect({ closeR.left, closeR.top }, { closeR.width, closeR.height }, { .color = { 180, 50, 50 } });
    d.text({ closeR.left + 8, closeR.top + 2 }, "Close", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
    studio.eqCloseRect = closeR;

    int canvX = modalX + EQ_CANVAS_X;
    int canvY = modalTop + EQ_CANVAS_Y;
    int canvW = std::min(EQ_CANVAS_W, modalW - EQ_CANVAS_X * 2);
    int canvH = EQ_CANVAS_H;

    // Canvas background
    d.filledRect({ canvX, canvY }, { canvW, canvH }, { .color = { 10, 10, 14 } });
    d.rect({ canvX, canvY }, { canvW, canvH }, { .color = { 55, 55, 65 } });

    // dB grid lines
    Color gridCol      = { 38, 38, 48 };
    Color gridLabelCol = { 80, 80, 95 };
    for (int db : { -12, -6, 0, 6, 12 }) {
        int y = canvY + canvH / 2 - (int)((float)db / EQ_DB_RANGE * (canvH / 2));
        d.line({ canvX, y }, { canvX + canvW, y },
            { .color = (db == 0 ? Color { 60, 60, 74 } : gridCol) });
        std::string label = (db > 0 ? "+" : "") + std::to_string(db) + "dB";
        d.text({ canvX - 38, y - 4 }, label, 8, { .color = gridLabelCol, .font = &PoppinsLight_8 });
    }

    // Frequency grid lines
    for (float f : { 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f }) {
        float t = log(f / 20.0f) / log(1000.0f);
        int x = canvX + (int)(canvW * t);
        d.line({ x, canvY }, { x, canvY + canvH }, { .color = gridCol });
        std::string label = (f >= 1000.f)
            ? std::to_string((int)(f / 1000)) + "k"
            : std::to_string((int)f);
        d.text({ x - 8, canvY + canvH + 6 }, label, 8, { .color = gridLabelCol, .font = &PoppinsLight_8 });
    }

    // Curves + control point dots per track
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk  = studio.tracks[t];
        Color col  = trk->themeColor;

        auto pts = trk->eq.curvePoints(canvX, canvY, canvW, canvH, EQ_DB_RANGE, SAMPLE_RATE, 300);
        for (int i = 1; i < (int)pts.size(); i++) {
            d.line(
                { (int)pts[i - 1].first, (int)pts[i - 1].second },
                { (int)pts[i].first,     (int)pts[i].second },
                { .color = col });
        }

        for (int b = 0; b < EQ::NUM_BANDS; b++) {
            auto [px, py] = trk->eq.bandPointPos(b, canvX, canvY, canvW, canvH, EQ_DB_RANGE);
            int ix = (int)px, iy = (int)py;
            d.filledCircle({ ix, iy }, EQ_DOT_R, { .color = col });
            d.circle({ ix, iy }, EQ_DOT_R, { .color = { 220, 220, 230 } });
        }
    }

    // Legend
    int legY = canvY + canvH + 26;
    int legX = canvX;
    for (int t = 0; t < MAX_TRACKS; t++) {
        Color col = studio.tracks[t]->themeColor;
        d.filledRect({ legX, legY }, { 18, 8 }, { .color = col });
        d.text({ legX + 22, legY - 1 }, "T" + std::to_string(t + 1), 8,
            { .color = { 180, 180, 200 }, .font = &PoppinsLight_8 });
        legX += 55;
    }

    // Band labels (use track 0 positions for x)
    const char* bandLabels[EQ::NUM_BANDS] = { "Low shelf", "250 Hz", "Mid", "4 kHz", "Hi shelf" };
    for (int b = 0; b < EQ::NUM_BANDS; b++) {
        auto [px, py] = studio.tracks[0]->eq.bandPointPos(b, canvX, canvY, canvW, canvH, EQ_DB_RANGE);
        d.text({ (int)px - 20, canvY + canvH + 8 }, bandLabels[b], 8,
            { .color = { 70, 70, 85 }, .font = &PoppinsLight_8 });
    }
}

// ---------------------------------------------------------------
// drawStaticUI
// ---------------------------------------------------------------
void drawStaticUI(Draw& d, sf::Vector2u size)
{
    d.clear();
    int winW = (int)size.x, margin = 10, rowH = 26;

    d.filledRect({ 0, 0 }, { winW, 25 }, { .color = d.styles.colors.quaternary });

    studio.transportRect = { margin, 4, 60, 17 };
    d.filledRect({ studio.transportRect.left, studio.transportRect.top }, { 60, 17 },
        { .color = studio.isPlaying ? Color { 200, 50, 50 } : Color { 50, 200, 50 } });
    d.text({ margin + 6, 7 }, studio.isPlaying ? "STOP" : "PLAY", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    helpBtnRect = { margin + 70, 4, 60, 17 };
    d.filledRect({ helpBtnRect.left, helpBtnRect.top }, { 60, 17 }, { .color = { 60, 60, 75 } });
    d.text({ helpBtnRect.left + 14, 7 }, "HELP", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // EQ button in header
    sf::IntRect eqHeaderBtn = { margin + 140, 4, 40, 17 };
    d.filledRect({ eqHeaderBtn.left, eqHeaderBtn.top }, { 40, 17 }, { .color = { 40, 60, 75 } });
    d.text({ eqHeaderBtn.left + 8, 7 }, "EQ", 8, { .color = { 200, 220, 255 }, .font = &PoppinsLight_8 });
    studio.eqHeaderBtnRect = eqHeaderBtn;

    std::stringstream bss;
    bss << "BPM: " << std::fixed << std::setprecision(1) << studio.bpm.load();
    studio.bpmRect = { winW - 100, 0, 90, 25 };
    d.textRight({ winW - margin, 6 }, bss.str(), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    int currentY = 35;
    int paramsPerRow = 8, colW = (winW - (margin * 2)) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    for (auto& trkPtr : studio.tracks) {
        Track& trk = *trkPtr;
        int startY = currentY;
        d.filledRect({ margin, currentY }, { colW / 2, 12 }, { .color = d.styles.colors.quaternary });
        d.text({ margin + 4, currentY + 1 }, trk.engine->getName(), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        trk.vuRect = sf::IntRect(margin + (colW / 2) + 10, currentY - 2, WAVE_HISTORY, 16);
        currentY += 14;

        Param* params = trk.engine->getParams();
        for (size_t p = 0; p < trk.engine->getParamCount(); p++) {
            int x = margin + ((int)p % paramsPerRow) * colW, y = currentY + ((int)p / paramsPerRow) * rowH;
            d.filledRect({ x, y }, { colW - 2, rowH - 2 }, { .color = d.styles.colors.quaternary });
            d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

            if (winW >= 900 || (trk.activeParamIdx == (int)p && std::chrono::duration_cast<std::chrono::milliseconds>(now - trk.lastEditTime).count() < 1500)) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(params[p].precision) << params[p].value << params[p].unit;
                d.textRight({ x + colW - 6, y + 2 }, params[p].string ? params[p].string : ss.str(), 8, { .color = { 120, 120, 130 }, .font = &PoppinsLight_8 });
            }

            float range    = params[p].max - params[p].min;
            float pct      = (params[p].value - params[p].min) / (range <= 0 ? 1.0f : range);
            int   barWidth = colW - 10, barHeight = 3;
            int   barX = x + 4, barY = y + rowH - 8;
            if (params[p].type & VALUE_CENTERED) {
                int midX = barX + (barWidth / 2);
                float offsetPct = (params[p].value / (params[p].max - 0));
                int fillW = (int)((barWidth / 2) * offsetPct);
                if (fillW < 0)
                    d.filledRect({ midX + fillW, barY }, { std::abs(fillW), barHeight }, { .color = trk.themeColor });
                else
                    d.filledRect({ midX, barY }, { fillW, barHeight }, { .color = trk.themeColor });
                d.filledRect({ midX, barY - 1 }, { 1, barHeight + 2 }, { .color = { 100, 100, 100 } });
            } else {
                d.filledRect({ barX, barY }, { (int)(barWidth * pct), barHeight }, { .color = trk.themeColor });
            }
        }
        int sectionHeight = (((trk.engine->getParamCount() + 7) / 8) * rowH) + 14;
        trk.trackBounds = sf::IntRect(margin, startY, winW - (margin * 2), sectionHeight);
        currentY += sectionHeight - 2;
    }

    currentY += 10;

    int muteW = 25, volW = 70, lenW = 18, genW = 18, eqW = 20, spacing = 5;
    int mixerTotalWidth = muteW + spacing + volW + spacing + lenW + spacing + genW + spacing + eqW + 10;
    int stepW = (winW - (margin * 2 + mixerTotalWidth)) / 64, stepH = 14;

    for (int i = 0; i < MAX_TRACKS; i++) {
        Track& trk = *studio.tracks[i];

        // Mute button
        trk.muteRect = { margin, currentY, muteW, stepH };
        d.filledRect({ trk.muteRect.left, trk.muteRect.top }, { muteW, stepH },
            { .color = trk.isMuted ? Color { 200, 50, 50 } : Color { 40, 40, 45 } });
        d.text({ trk.muteRect.left + 8, trk.muteRect.top + 1 }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        // Volume bar
        trk.volRect = { trk.muteRect.left + muteW + spacing, currentY, volW, stepH };
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { volW, stepH }, { .color = Color { 40, 40, 45 } });
        d.filledRect({ trk.volRect.left, trk.volRect.top + (stepH / 2) - 2 }, { (int)(volW * trk.volume), 4 }, { .color = trk.themeColor });

        // Length display
        trk.lenBtnRect = { trk.volRect.left + volW + spacing, currentY, lenW, stepH };
        d.filledRect({ trk.lenBtnRect.left, trk.lenBtnRect.top }, { lenW, stepH }, { .color = Color { 50, 50, 60 } });
        d.text({ trk.lenBtnRect.left + 4, trk.lenBtnRect.top + 1 }, std::to_string(trk.seqDisplayLen), 8, { .color = { 200, 200, 210 }, .font = &PoppinsLight_8 });

        // Generator button
        trk.genRect = { trk.lenBtnRect.left + lenW + spacing, currentY, genW, stepH };
        d.filledRect({ trk.genRect.left, trk.genRect.top }, { genW, stepH }, { .color = Color { 60, 60, 75 } });
        d.text({ trk.genRect.left + 5, trk.genRect.top + 1 }, "G", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        // EQ button (per-track, opens the shared EQ modal)
        trk.eqBtnRect = { trk.genRect.left + genW + spacing, currentY, eqW, stepH };
        d.filledRect({ trk.eqBtnRect.left, trk.eqBtnRect.top }, { eqW, stepH }, { .color = Color { 40, 60, 75 } });
        d.text({ trk.eqBtnRect.left + 3, trk.eqBtnRect.top + 1 }, "EQ", 8, { .color = { 200, 220, 255 }, .font = &PoppinsLight_8 });

        // Sequencer step rects
        int gridStartX = trk.eqBtnRect.left + eqW + 10;
        for (int s = 0; s < SEQ_STEPS; s++)
            trk.stepRects[s] = { gridStartX + (s * stepW), currentY, stepW - 1, stepH / 2 + laneH };

        currentY += 26;
    }

    currentY += 4;

    // Step editor row
    if (studio.selTrack != -1 && studio.selStep != -1) {
        int editorY = currentY;
        auto& s = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        d.text({ margin, editorY }, "EDIT T" + std::to_string(studio.selTrack + 1) + " S" + std::to_string(studio.selStep + 1), 8,
            { .color = studio.tracks[studio.selTrack]->themeColor, .font = &PoppinsLight_8 });

        studio.editNoteRect = { 100, editorY - 2, 80, 15 };
        d.text({ 100, editorY }, "NOTE: " + std::string(MIDI_NOTES_STR[s.note]), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        studio.editLenRect = { 200, editorY - 2, 80, 15 };
        d.text({ 200, editorY }, "LEN: " + fToString(s.len, 1) + "steps", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        studio.editVeloRect = { 300, editorY - 2, 80, 15 };
        d.text({ 300, editorY }, "VEL: " + std::to_string((int)(s.velocity * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        studio.editProbRect = { 400, editorY - 2, 80, 15 };
        d.text({ 400, editorY }, "PROB: " + std::to_string((int)(s.condition * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        sf::IntRect* targetRect = nullptr;
        switch (studio.stepEditMode) {
        case EDIT_NOTE: targetRect = &studio.editNoteRect; break;
        case EDIT_VELO: targetRect = &studio.editVeloRect; break;
        case EDIT_PROB: targetRect = &studio.editProbRect; break;
        case EDIT_LEN:  targetRect = &studio.editLenRect;  break;
        }
        if (targetRect)
            d.filledRect({ targetRect->left, targetRect->top + targetRect->height }, { targetRect->width, 2 },
                { .color = studio.tracks[studio.selTrack]->themeColor });
    }

    // Note lane lines
    int stepWidth  = studio.tracks[0]->stepRects[0].width + 1;

    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r    = trk->stepRects[s];
            auto& step = trk->sequence[s];
            if (step.active) {
                int   synthLaneY = r.top + r.height - laneH + 1;
                float noteMapped = 1.0f - (float)(CLAMP(step.note, 24, 96) - 24) / 72.0f;
                int   noteY      = synthLaneY + (int)(noteMapped * laneH);
                int   pixelLen   = (int)(step.len * (stepWidth - 1)) - 1;
                d.line({ r.left, noteY }, { r.left + pixelLen, noteY }, { .color = trk->themeColor });
            }
        }
    }

    // Overlays — EQ first, then help (help sits on top if both somehow true)
    if (showEQ)   drawEQOverlay(d, size);
    if (showHelp) drawHelpOverlay(d, size);
}

// ---------------------------------------------------------------
// updateWaveforms
// ---------------------------------------------------------------
void updateWaveforms(std::vector<sf::Uint8>& pixels, int stride)
{
    if (showHelp || showEQ) return;

    for (auto& trkPtr : studio.tracks) {
        std::lock_guard<std::mutex> hLock(trkPtr->historyMtx);
        for (int x = 0; x < WAVE_HISTORY; x++) {
            int barH = (int)(std::min(trkPtr->history[x], 1.0f) * (trkPtr->vuRect.height / 2));
            for (int y = 0; y < trkPtr->vuRect.height; y++) {
                size_t idx   = ((trkPtr->vuRect.top + y) * stride + (trkPtr->vuRect.left + x)) * 4;
                bool isWave  = std::abs(y - (trkPtr->vuRect.height / 2)) <= barH;
                pixels[idx]     = isWave ? trkPtr->themeColor.r : 20;
                pixels[idx + 1] = isWave ? trkPtr->themeColor.g : 20;
                pixels[idx + 2] = isWave ? trkPtr->themeColor.b : 25;
            }
        }
    }
}

// ---------------------------------------------------------------
// updateSequencerPixels
// ---------------------------------------------------------------
void updateSequencerPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    if (showHelp || showEQ) return;

    int    stepWidth        = studio.tracks[0]->stepRects[0].width + 1;
    double progressInStep   = studio.sampleCounter.load() / studio.samplesPerStep;
    int    playheadGlobalX  = (int)((studio.currentStep + progressInStep) * stepWidth);
    int    gridStartX       = studio.tracks[0]->stepRects[0].left;

    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            int h   = r.height - laneH;
            Color c = trk->sequence[s].active
                ? trk->themeColor
                : ((s % 4 == 0) ? Color { 55, 55, 60 } : Color { 45, 45, 50 });
            int drawSelectorY = (studio.selTrack == t && studio.selStep == s) ? h - 3 : h;

            for (int y = 0; y < h; y++) {
                for (int x = 0; x < r.width; x++) {
                    int    globalX = r.left + x;
                    size_t idx     = ((r.top + y) * stride + globalX) * 4;
                    if (studio.isPlaying && (globalX == gridStartX + playheadGlobalX || globalX == gridStartX + playheadGlobalX - 1)) {
                        pixels[idx] = pixels[idx + 1] = pixels[idx + 2] = 255;
                    } else if (y >= drawSelectorY) {
                        pixels[idx] = pixels[idx + 1] = pixels[idx + 2] = 255;
                    } else {
                        pixels[idx]     = c.r;
                        pixels[idx + 1] = c.g;
                        pixels[idx + 2] = c.b;
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------
// main
// ---------------------------------------------------------------
int main()
{
    snd_pcm_t* pcm_h = audioInit();
    pthread_setname_np(pthread_self(), "zicBox_UI");
    sf::RenderWindow window(sf::VideoMode(1080, 1080), "Zic23");
    window.setFramerateLimit(60);

    Styles appStyles = {
        .screen = { 1080, 1080 },
        .margin = 2,
        .colors = { { 15, 15, 18 }, { 255, 255, 255 }, { 120, 120, 130 }, { 0, 180, 255 }, { 10, 10, 12 }, { 28, 28, 32 }, { 35, 35, 40 } }
    };
    auto drawer = std::make_unique<Draw>(appStyles);
    sf::Texture screenTexture;
    screenTexture.create(BUFFER_SIZE, BUFFER_SIZE);
    sf::Sprite screenSprite(screenTexture);
    std::vector<sf::Uint8> pixelBuffer(BUFFER_SIZE * BUFFER_SIZE * 4, 15);

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

            // ---- Mouse released: stop EQ drag ----
            if (event.type == sf::Event::MouseButtonReleased) {
                eqDragging  = false;
                eqDragTrack = -1;
                eqDragBand  = -1;
            }

            // ---- Mouse moved: EQ drag update ----
            if (event.type == sf::Event::MouseMoved) {
                if (showEQ && eqDragging) {
                    int mx = event.mouseMove.x, my = event.mouseMove.y;

                    int canvX = 40 + EQ_CANVAS_X;
                    int canvY = 60 + EQ_CANVAS_Y;
                    int canvW = std::min(EQ_CANVAS_W, (int)window.getSize().x - 80 - EQ_CANVAS_X * 2);
                    int canvH = EQ_CANVAS_H;

                    auto& eq = studio.tracks[eqDragTrack]->eq;
                    eq.setBandFreqFromX(eqDragBand, (float)mx, canvX, canvW);
                    eq.setBandGainFromY(eqDragBand, (float)my, canvY, canvH, EQ_DB_RANGE);
                    eq.recompute(SAMPLE_RATE);
                    static_needs_redraw = true;
                }
            }

            // ---- Key released ----
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    int note   = (studio.selTrack == trkIdx && studio.selStep != -1)
                        ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                    studio.tracks[trkIdx]->engine->noteOff(note);
                }
            }

            // ---- Key pressed ----
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

                // Copy / Paste
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::RControl)) {
                    if (event.key.code == sf::Keyboard::C) {
                        if (studio.selTrack != -1 && studio.selStep != -1) {
                            copyTrackIdx = studio.selTrack;
                            copyStepIdx  = studio.selStep;
                            copiedStep   = studio.tracks[copyTrackIdx]->sequence[copyStepIdx];
                        }
                    }
                    if (event.key.code == sf::Keyboard::V) {
                        if (studio.selTrack != -1 && studio.selStep != -1 && copyTrackIdx != -1) {
                            if (copyTrackIdx == studio.selTrack) {
                                studio.tracks[studio.selTrack]->sequence[studio.selStep] = copiedStep;
                            } else {
                                for (int i = 0; i < SEQ_STEPS; i++)
                                    studio.tracks[studio.selTrack]->sequence[i] = studio.tracks[copyTrackIdx]->sequence[i];
                                studio.tracks[studio.selTrack]->volume        = studio.tracks[copyTrackIdx]->volume;
                                studio.tracks[studio.selTrack]->seqDisplayLen = studio.tracks[copyTrackIdx]->seqDisplayLen;
                            }
                            static_needs_redraw = true;
                        }
                    }
                }

                // Track shortcuts 1-6
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num6) {
                    int trkIdx = event.key.code - sf::Keyboard::Num1;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::G)) {
                        runGeneration(trkIdx);
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::M)) {
                        studio.tracks[trkIdx]->isMuted = !studio.tracks[trkIdx]->isMuted;
                        static_needs_redraw = true;
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        studio.selTrack = trkIdx;
                        studio.selStep  = 0;
                        static_needs_redraw = true;
                    } else {
                        int note = (studio.selTrack == trkIdx && studio.selStep != -1)
                            ? studio.tracks[trkIdx]->sequence[studio.selStep].note : 60;
                        std::lock_guard<std::mutex> lock(studio.audioMutex);
                        studio.tracks[trkIdx]->engine->noteOn(note, 1.0f);
                    }
                }
            }

            // ---- Mouse pressed ----
            if (event.type == sf::Event::MouseButtonPressed) {
                int mx = event.mouseButton.x, my = event.mouseButton.y;

                // EQ modal intercepts all clicks first
                if (showEQ) {
                    // Close button
                    if (studio.eqCloseRect.contains(mx, my)) {
                        showEQ = false;
                        static_needs_redraw = true;
                        continue;
                    }

                    int canvX = 40 + EQ_CANVAS_X;
                    int canvY = 60 + EQ_CANVAS_Y;
                    int canvW = std::min(EQ_CANVAS_W, (int)window.getSize().x - 80 - EQ_CANVAS_X * 2);
                    int canvH = EQ_CANVAS_H;

                    // Hit-test control dots
                    bool hitDot = false;
                    for (int t = 0; t < MAX_TRACKS && !hitDot; t++) {
                        for (int b = 0; b < EQ::NUM_BANDS && !hitDot; b++) {
                            auto [px, py] = studio.tracks[t]->eq.bandPointPos(b, canvX, canvY, canvW, canvH, EQ_DB_RANGE);
                            int dx = mx - (int)px, dy = my - (int)py;
                            if (dx * dx + dy * dy <= (EQ_DOT_R + 3) * (EQ_DOT_R + 3)) {
                                eqDragging  = true;
                                eqDragTrack = t;
                                eqDragBand  = b;
                                hitDot      = true;
                            }
                        }
                    }
                    // Consume all clicks inside the modal
                    continue;
                }

                // Help modal intercepts all clicks
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
                            studio.selStep  = s;
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

            // ---- Mouse scroll ----
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (showHelp || showEQ) continue;

                int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
                float delta = event.mouseWheelScroll.delta;
                uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();

                bool handled = false;
                for (int t = 0; t < MAX_TRACKS; t++) {
                    auto& trk = studio.tracks[t];
                    if (trk->lenBtnRect.contains(mx, my)) {
                        if (delta > 0) compressTrackSequence(*trk);
                        else           stretchTrackSequence(*trk);
                        static_needs_redraw = true;
                        handled = true;
                        break;
                    }
                    for (int s = 0; s < SEQ_STEPS; s++) {
                        if (trk->stepRects[s].contains(mx, my)) {
                            int scaled = (delta > 0) ? 1 : -1;
                            auto& step = trk->sequence[s];
                            bool isNoteEdit = sf::Keyboard::isKeyPressed(sf::Keyboard::N)
                                || (studio.stepEditMode == EDIT_NOTE
                                    && !sf::Keyboard::isKeyPressed(sf::Keyboard::L)
                                    && !sf::Keyboard::isKeyPressed(sf::Keyboard::P)
                                    && !sf::Keyboard::isKeyPressed(sf::Keyboard::V));

                            if      (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) editStep(step, EDIT_LEN,  scaled);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) editStep(step, EDIT_PROB, scaled);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) editStep(step, EDIT_VELO, scaled);
                            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::N)) editStep(step, EDIT_NOTE, scaled);
                            else                                                   editStep(step, studio.stepEditMode, scaled);

                            if (isNoteEdit && !studio.isPlaying)
                                triggerPreview(*trk, step.note, step.velocity);

                            studio.selTrack = t;
                            studio.selStep  = s;
                            static_needs_redraw = true;
                            handled = true;
                            break;
                        }
                    }
                    if (handled) break;
                }

                if (!handled) {
                    if (studio.bpmRect.contains(mx, my)) {
                        int scaled = encGetScaledDirection(delta, now, lastBpmTick);
                        lastBpmTick = now;
                        studio.bpm = CLAMP(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
                        studio.updateClock();
                        static_needs_redraw = true;
                    } else if (studio.selTrack != -1 && studio.selStep != -1
                        && (studio.editNoteRect.contains(mx, my)
                            || studio.editVeloRect.contains(mx, my)
                            || studio.editProbRect.contains(mx, my)
                            || studio.editLenRect.contains(mx, my))) {
                        auto& trk  = studio.tracks[studio.selTrack];
                        auto& step = trk->sequence[studio.selStep];
                        int scaled = (delta > 0) ? 1 : -1;
                        if (studio.editNoteRect.contains(mx, my)) {
                            editStep(step, EDIT_NOTE, scaled);
                            if (!studio.isPlaying) triggerPreview(*trk, step.note, step.velocity);
                        } else if (studio.editVeloRect.contains(mx, my)) editStep(step, EDIT_VELO, scaled);
                        else if (studio.editProbRect.contains(mx, my))    editStep(step, EDIT_PROB, scaled);
                        else if (studio.editLenRect.contains(mx, my))     editStep(step, EDIT_LEN,  scaled);
                        static_needs_redraw = true;
                    } else {
                        for (auto& trk : studio.tracks) {
                            if (trk->volRect.contains(mx, my)) {
                                int scaled = encGetScaledDirection(delta, now, trk->lastVolShiftTick);
                                trk->lastVolShiftTick = now;
                                trk->volume = CLAMP(trk->volume + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f)), 0.0f, 1.0f);
                                static_needs_redraw = true;
                            } else if (trk->trackBounds.contains(mx, my)) {
                                int pPerRow = 8, rH = 26, margin = 10;
                                int cW = ((int)window.getSize().x - (margin * 2)) / pPerRow;
                                int pIdx = ((my - (trk->trackBounds.top + 14)) / rH) * pPerRow + (mx - margin) / cW;
                                if (pIdx >= 0 && (size_t)pIdx < trk->engine->getParamCount()) {
                                    std::lock_guard<std::mutex> lock(studio.audioMutex);
                                    Param& p  = trk->engine->getParams()[pIdx];
                                    int scaled = encGetScaledDirection(delta, now, trk->lastShiftTicks[pIdx]);
                                    trk->lastShiftTicks[pIdx] = now;
                                    p.set(p.value + (scaled * p.step * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 1.0f)));
                                    trk->activeParamIdx = pIdx;
                                    trk->lastEditTime   = std::chrono::steady_clock::now();
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