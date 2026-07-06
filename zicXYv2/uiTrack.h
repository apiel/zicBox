#pragma once

#include <array>

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "helpers/format.h"
#include "helpers/midiNote.h"
#include "zicXYv2/draw.h"
#include "zicXYv2/drawPad.h"
#include "zicXYv2/project.h"
#include "zicXYv2/studio.h"
#include "zicXYv2/utils.h"
namespace UiTrack {

bool needsRedraw = true;
int paramsTopY = 0;
int waveformTopY = 0;

int Y_MARGIN = 2;

Rect loopRect = { { -1, -1 }, { -1, -1 } };
bool isDraggingLoop = false;
int dragStartX = 0;
float initialLoopStart = 0.0f;

int waveformH = 45;

const int paramsPerRow = 4;

int getMaxVisibleRows(const Track& trk)
{
    return trk.showWaveform ? 4 : 5;
}

int getTotalParamRows(const Track& trk)
{
    size_t totalParamCount = 4 + trk.engine->getParamCount();
    return ((int)totalParamCount + paramsPerRow - 1) / paramsPerRow;
}

int getStartRow(const Track& trk)
{
    int maxVisibleRows = getMaxVisibleRows(trk);
    int totalParamRows = getTotalParamRows(trk);
    int maxStartRow = std::max(0, totalParamRows - maxVisibleRows);
    return std::clamp(trk.encodersSelection - maxVisibleRows + 1, 0, maxStartRow);
}

void updateSelectionFromScroll(Track& trk, int visualRow, int col)
{
    int maxVisibleRows = getMaxVisibleRows(trk);
    if (visualRow < 0 || visualRow >= maxVisibleRows) return;
    if (col < 0 || col >= paramsPerRow) return;

    int totalParamRows = getTotalParamRows(trk);
    if (totalParamRows <= 0) return;

    int absoluteRow = getStartRow(trk) + visualRow;
    absoluteRow = std::clamp(absoluteRow, 0, totalParamRows - 1);

    if (trk.encodersSelection != absoluteRow) {
        trk.encodersSelection = absoluteRow;
    }
}

Rect historyRect = { { -1, -1 }, { -1, -1 } };

void drawWaveform(Draw& d, Track& trk, int x, int y, int w, int h)
{
    d.filledRect({ x, y }, { w, h }, { .color = darken(trk.themeColor, 0.9f) });

    float lStart = trk.engine->getLoopStart();
    float lLen = trk.engine->getLoopLength();

    int loopX = x + (int)(lStart * w);
    int loopW = (int)(lLen * w);

    if (lLen > 0) {
        Color loopColor = trk.themeColor;
        loopColor.a = 40; // Low alpha for the background area
        loopRect = { { loopX, y }, { loopW, h } };
        d.filledRect(loopRect.position, loopRect.size, { .color = loopColor });
    } else {
        loopRect = { { -1, -1 }, { -1, -1 } };
    }

    std::vector<Point> points;
    int centerY = y + (h / 2);
    float amplitude = h * 0.45f;

    for (int i = 0; i < w; i++) {
        float phase = (float)i / (float)w;
        float sample = trk.engine->draw(phase);
        int drawY = centerY - (int)(sample * amplitude);
        points.push_back({ x + i, drawY });
    }

    d.lines(points, { .color = trk.themeColor });

    if (lLen > 0) {
        Color markerColor = trk.themeColor;
        markerColor.a = 180;
        d.filledRect({ loopX, y }, { 1, h }, { .color = markerColor }); // Start line
        d.filledRect({ loopX + loopW - 1, y }, { 1, h }, { .color = markerColor }); // End line
    }

    d.rect({ x, y }, { w, h }, { .color = { 255, 255, 255, 20 } });
}

bool drawStatic(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY, Track& trk)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    auto now = std::chrono::steady_clock::now();

    currentY += Y_MARGIN;
    paramsTopY = currentY;

    Param* engineParams = trk.engine->getParams();
    size_t engineParamCount = trk.engine->getParamCount();
    size_t totalParamCount = 4 + engineParamCount;

    std::string engineLabel = engineRegistry[trk.currentEngineIdx].name;

    std::vector<Param> params;
    params.reserve(totalParamCount);
    params.push_back({ .key = "engine", .label = "Engine", .string = engineLabel.data(), .value = (float)trk.currentEngineIdx, .min = 0.0f, .max = ENGINE_REGISTRY_COUNT - 1, .type = VALUE_STRING, .precision = 0 });
    params.push_back({ .key = "trkvol", .label = "Volume", .unit = "%", .value = trk.volume * 100.0f, .min = 0.0f, .max = 100.0f, .step = 1.0f, .type = VALUE_BASIC, .precision = 0 });
    params.push_back({});
    params.push_back({});

    for (size_t i = 0; i < engineParamCount; i++) {
        params.push_back(engineParams[i]);
    }

    if (trk.lastShiftTicks.size() < totalParamCount) {
        trk.lastShiftTicks.resize(totalParamCount, 0);
    }

    currentY += UiDraw::params(d, params.data(), params.size(), winW, winH, paramsTopY, paramsPerRow, trk.themeColor, trk.encodersSelection, trk.showWaveform ? 4 : 5);

    currentY += 5;

    if (trk.showWaveform) {
        waveformTopY = currentY;
        int headerW = winW - (MARGIN * 2);
        drawWaveform(d, trk, MARGIN, currentY, headerW, waveformH);
        currentY += waveformH + 5;
    }

    historyRect = { { MARGIN, currentY }, { WAVE_HISTORY, 16 } };

    return true;
}

struct SavedPixels {
    Color pixels[2][4]; // [width][height]
    Point pos;
    bool active = false;
};
std::map<int, SavedPixels> playheadBuffer;

bool drawPlayhead(Draw& d, const int winW)
{
    bool rendered = false; // Track if we modified the display this frame

    // RESTORE
    for (auto& [id, buf] : playheadBuffer) {
        if (buf.active) {
            for (int dx = 0; dx < 2; dx++) {
                for (int dy = 0; dy < 4; dy++) {
                    d.pixel({ buf.pos.x + dx, buf.pos.y + dy }, buf.pixels[dx][dy]);
                }
            }
            buf.active = false;
            rendered = true; // We erased a playhead, so the screen changed!
        }
    }

    if (studio.tracks[studio.selTrack] == nullptr) return rendered;
    Track& trk = *studio.tracks[studio.selTrack];

    int headerW = winW - (MARGIN * 2);
    int voiceCount = trk.engine->getVoiceCount();

    for (int i = 0; i < voiceCount; i++) {
        float ph = trk.engine->getPlayhead(i);
        if (ph < 0.0f || ph > 1.0f) continue;

        int px = MARGIN + (int)(ph * (headerW - 2));

        int py = waveformTopY + ((i + 1) * waveformH) / (voiceCount + 1);
        py -= 2;

        Point currentPos = { px, py };
        SavedPixels& buf = playheadBuffer[i];
        buf.pos = currentPos;
        buf.active = true;

        // Save the current screen pixels to restore them at the next redraw
        for (int dx = 0; dx < 2; dx++) {
            for (int dy = 0; dy < 4; dy++) {
                buf.pixels[dx][dy] = d.getPixel({ currentPos.x + dx, currentPos.y + dy });
            }
        }

        // Draw the 2x4 white playhead, do not use fillRect to be accurate with restore...
        for (int dx = 0; dx < 2; dx++) {
            for (int dy = 0; dy < 4; dy++) {
                d.pixel({ currentPos.x + dx, currentPos.y + dy }, { { 255, 255, 255 } });
            }
        }

        rendered = true; // We drew a new playhead, so the screen changed!
    }

    // Return true if we either erased an old playhead or drew a new one
    return rendered;
}

bool historyCleaned = false;
bool drawPlayheadHistory(Draw& d, Track& trk)
{
    bool rendered = false;
    std::array<float, WAVE_HISTORY> historySnapshot {};
    {
        std::lock_guard<std::mutex> lk(trk.historyMtx);
        for (int i = 0; i < WAVE_HISTORY; ++i) {
            historySnapshot[i] = trk.history[i];
        }
    }

    for (int x = 0; x < WAVE_HISTORY; x++) {
        if (historySnapshot[x]) {
            int bH = (int)(std::min(historySnapshot[x], 1.f) * (historyRect.size.h / 2));
            if (bH != 0) {
                rendered = true;
                historyCleaned = false;
            }
            for (int y = 0; y < historyRect.size.h; y++) {
                bool w = std::abs(y - historyRect.size.h / 2) <= bH;
                d.pixel({ historyRect.position.x + x, historyRect.position.y + y }, { .color = (w ? trk.themeColor : d.styles.colors.background) });
            }
        }
    }
    if (!rendered && !historyCleaned) {
        d.filledRect(historyRect.position, historyRect.size, { .color = d.styles.colors.background });
        historyCleaned = true;
        rendered = true;
    }
    return rendered;
}

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (studio.tracks[studio.selTrack] == nullptr) return false;
    Track& trk = *studio.tracks[studio.selTrack];

    bool rendered = false;

    rendered |= drawStatic(d, winW, winH, needFullRedraw, currentY, trk);
    if (trk.showWaveform) {
        rendered |= drawPlayhead(d, winW);
    } else {
        rendered |= drawPlayheadHistory(d, trk);
    }

    return rendered;
}

void mouseButtonPressed(Point position)
{
    if (studio.currentView != ViewTrack || studio.tracks[studio.selTrack] == nullptr) return;
    Track& trk = *studio.tracks[studio.selTrack];

    if (loopRect.size.w > 0 && inRect(loopRect, position)) {
        isDraggingLoop = true;
        dragStartX = position.x;
        initialLoopStart = trk.engine->getLoopStart();
        return;
    }
}

void mouseMoved(Point position, const int winW)
{
    if (studio.currentView != ViewTrack || studio.tracks[studio.selTrack] == nullptr) return;
    Track& trk = *studio.tracks[studio.selTrack];

    if (isDraggingLoop) {
        int headerW = winW - (MARGIN * 2);
        if (headerW <= 0) return;

        // Calculate the horizontal drag offset in pixels and convert it to 0.0 -> 1.0 phase delta
        int deltaX = position.x - dragStartX;
        float deltaPhase = (float)deltaX / (float)headerW;
        float newLoopStart = initialLoopStart + deltaPhase;

        // Keep loopStart bounded so the loop end never extends beyond 1.0f
        float lLen = trk.engine->getLoopLength();
        float maxStart = 1.0f - lLen;
        if (maxStart < 0.0f) maxStart = 0.0f;

        if (newLoopStart < 0.0f) newLoopStart = 0.0f;
        if (newLoopStart > maxStart) newLoopStart = maxStart;

        // Thread-safe update to the engine
        {
            std::unique_lock<std::mutex> lock(studio.audioMutex, std::try_to_lock);
            if (!lock.owns_lock()) return;
            trk.engine->setLoopStart(newLoopStart);
        }

        needsRedraw = true;
    }
}

void mouseButtonReleased()
{
    isDraggingLoop = false;
}

void onEncoder(int encoderId, int8_t direction, bool& needFullRedraw)
{
    if (studio.currentView != ViewTrack) return;
    if (direction == 0) return;
    if (studio.tracks[studio.selTrack] == nullptr) return;

    Track& trk = *studio.tracks[studio.selTrack];
    const int maxVisibleRows = getMaxVisibleRows(trk);
    size_t totalParamCount = 4 + trk.engine->getParamCount();
    int startRow = getStartRow(trk);

    int visualRow = std::clamp(trk.encodersSelection - startRow, 0, maxVisibleRows - 1);
    int col = std::clamp(encoderId - 1, 0, paramsPerRow - 1);

    uint32_t now = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
                       .count();

    size_t engineParamCount = trk.engine->getParamCount();
    if (trk.lastShiftTicks.size() < totalParamCount) {
        trk.lastShiftTicks.resize(totalParamCount, 0);
    }

    int absoluteRow = startRow + visualRow;
    size_t finalPIdx = (absoluteRow * paramsPerRow) + col;
    if (finalPIdx >= totalParamCount) return;

    int scaled = encGetScaledDirection(direction, now, trk.lastShiftTicks[finalPIdx]);
    trk.lastShiftTicks[finalPIdx] = now;

    if (finalPIdx == 0) {
        int currentEngineIdx = trk.currentEngineIdx;
        currentEngineIdx += scaled;
        currentEngineIdx = std::clamp(currentEngineIdx, 0, ENGINE_REGISTRY_COUNT - 1);

        if (currentEngineIdx != trk.currentEngineIdx) {
            std::unique_lock<std::mutex> lock(studio.audioMutex, std::try_to_lock);
            if (!lock.owns_lock()) return;
            trk.setEngine(currentEngineIdx);
            trk.lastShiftTicks.resize(4 + trk.engine->getParamCount(), 0);
            needFullRedraw = true;
        }
    } else if (finalPIdx == 1) {
        float newVol = trk.volume * 100.0f + scaled;
        newVol = std::clamp(newVol, 0.0f, 100.0f);
        trk.volume = newVol / 100.0f;
        needsRedraw = true;
    } else if (finalPIdx >= 4) {
        size_t engineParamIdx = finalPIdx - 4;
        if (engineParamIdx >= engineParamCount) return;

        std::unique_lock<std::mutex> lock(studio.audioMutex, std::try_to_lock);
        if (!lock.owns_lock()) return;
        Param& p = trk.engine->getParams()[engineParamIdx];
        p.inc(scaled);
        needsRedraw = true;
    }

    if (trk.encodersSelection != absoluteRow) {
        trk.encodersSelection = absoluteRow;
    }
    trk.activeParamIdx = (int)finalPIdx;
    trk.lastEditTime = std::chrono::steady_clock::now();
}

bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted, bool& needFullRedraw)
{
    if (studio.currentView != ViewTrack) return false;
    if (studio.tracks[studio.selTrack] == nullptr) return false;

    Track& trk = *studio.tracks[studio.selTrack];
    (void)now;
    (void)shifted;

    const int SB_WIDTH = 4;
    const int SB_GAP = 3;
    int usableWidth = winW - (MARGIN * 2) - (SB_WIDTH + SB_GAP);
    if (usableWidth <= 0) return false;

    int adjustedColW = usableWidth / paramsPerRow;
    if (adjustedColW <= 0) return false;

    int relX = position.x - MARGIN;
    if (relX < 0 || relX >= usableWidth) return false;
    int col = relX / adjustedColW;
    if (col < 0 || col >= paramsPerRow) return false;

    int relY = position.y - paramsTopY;
    int maxVisibleRows = getMaxVisibleRows(trk);
    int visualRow = relY / UiDraw::ROW_H;
    int totalParamRows = getTotalParamRows(trk);
    int startRow = getStartRow(trk);
    int absoluteRow = -1;
    if (totalParamRows > 0) {
        absoluteRow = std::clamp(startRow + visualRow, 0, totalParamRows - 1);
    }

    std::fprintf(stderr,
        "[UiTrack::mouseWheelScrolled] visualRow=%d absoluteRow=%d encodersSelection=%d relY=%d maxVisibleRows=%d, totalParamRows = %d startRow=%d\n",
        visualRow,
        absoluteRow,
        trk.encodersSelection,
        relY,
        maxVisibleRows,
        totalParamRows,
        startRow);
    std::fflush(stderr);

    // trk.encodersSelection = absoluteRow;

    onEncoder(col + 1, delta, needFullRedraw);
    return needsRedraw;
}

void keyPressed(int key, bool& needFullRedraw)
{
    if (studio.currentView != ViewTrack) return;

    if (key == KEY_F3) {
        Track& trk = *studio.tracks[studio.selTrack];
        trk.encodersSelection++;
        size_t totalParamCount = 4 + trk.engine->getParamCount();
        int totalParamRows = ((int)totalParamCount + paramsPerRow - 1) / paramsPerRow;
        if (trk.encodersSelection >= totalParamRows) trk.encodersSelection = totalParamRows - 1;
        needsRedraw = true;
    } else if (key == KEY_F2) {
        Track& trk = *studio.tracks[studio.selTrack];
        trk.encodersSelection--;
        if (trk.encodersSelection < 0) trk.encodersSelection = 0;
        needsRedraw = true;
    }
}
}