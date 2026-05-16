#pragma once

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "zicRack/drawPad.h"
#include "zicRack/studio.h"

namespace UiTrack {

bool needsRedraw = true;
int paramsTopY = 0;

static constexpr int ROW_H = 36; // param panel row height

Rect loopRect = { { -1, -1 }, { -1, -1 } };
bool isDraggingLoop = false;
int dragStartX = 0;
float initialLoopStart = 0.0f;

bool xyDragging = false;
Rect xyRect;

Rect seqRect = { { -1, -1 }, { -1, -1 } };
int currentSeqPage = 0;
int stepsPerRow = 16;
int rows = 4; // 4 rows * 16 steps = 64 steps

void drawSequencer(Draw& d, Track& trk, Rect rect)
{
    // // Background frame
    // d.filledRect(rect.position, rect.size, { .color = darken(d.styles.colors.quaternary, 0.15f) });
    // d.rect(rect.position, rect.size, { .color = { 255, 255, 255, 20 } });

    int cellW = rect.size.w / stepsPerRow;
    int cellH = rect.size.h / rows;

    for (int stepIdx = 0; stepIdx < SEQ_STEPS; stepIdx++) {
        int row = stepIdx / stepsPerRow;
        int col = stepIdx % stepsPerRow;

        int sx = rect.position.x + col * cellW;
        int sy = rect.position.y + row * cellH;

        // Ensure the sequence vector is properly sized
        if (trk.sequence.size() <= (size_t)stepIdx) {
            trk.sequence.resize(SEQ_STEPS);
        }

        Step& step = trk.sequence[stepIdx];

        // Determine base visual coloring (alternate colors every 4 steps for rhythm grouping)
        // bool isBeatHighlight = (col / 4) % 2 == 0;
        bool isBeatHighlight = col % 4 != 0;
        Color stepBg = isBeatHighlight ? Color { 45, 45, 50 } : Color { 35, 35, 40 };

        if (step.active) {
            // Mix track theme color with velocity for brightness response
            stepBg = trk.themeColor;
            stepBg.a = 100 + (int)(step.velocity * 155.0f);
        }

        // Render the pad cell
        d.filledRect({ sx + 1, sy + 1 }, { cellW - 2, cellH - 2 }, { .color = stepBg });

        // Render step text marker (Step number)
        std::string label = std::to_string(stepIdx + 1);
        d.text({ sx + 4, sy + 2 }, label, 8, { .color = step.active ? Color { 255, 255, 255 } : Color { 120, 120, 130 }, .font = &PoppinsLight_8 });

        // Small indicator bar showing velocity value inside active steps
        if (step.active) {
            int velBarW = (int)((cellW - 8) * step.velocity);
            d.filledRect({ sx + 4, sy + cellH - 6 }, { velBarW, 2 }, { .color = { 255, 255, 255, 180 } });
        }
    }
}

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

void drawGraph(Draw& d, Track& trk, Param& param, const int colW, int x, int y, Color& bgColor)
{
    std::vector<Point> points;
    int innerW = colW - 10;
    for (int gx = 0; gx < innerW; gx++) {
        float phase = (float)gx / (float)innerW;
        float sVal = param.getGraphPoint(phase);
        int centerY = y + (ROW_H / 2) + 4;
        int drawY = centerY - (int)(sVal * (ROW_H / 5.0f));
        points.push_back({ x + 4 + gx, drawY });
    }
    Color c = trk.themeColor;
    d.lines(points, { .color = c });
    c.a = 50;
    d.filledPolygon(points, { .color = c });
}

void drawParam(Draw& d, Track& trk, Param* params, size_t& p, const int colW, const int winW, int x, int y, Color& bgColor, const std::chrono::steady_clock::time_point& now)
{
    d.filledRect({ x, y }, { colW - 2, ROW_H - 2 }, { .color = bgColor });
    d.text({ x + 4, y + 2 }, params[p].label, 12, { .color = d.styles.colors.text, .font = &PoppinsLight_12 });

    std::stringstream ss;
    if (params[p].string) {
        ss << params[p].string;
    } else {
        ss << std::fixed << std::setprecision(params[p].precision) << params[p].value << params[p].unit;
    }

    d.text({ x + 4, y + 16 }, ss.str(), 8, { .color = { 170, 170, 180 }, .font = &PoppinsLight_8, .maxWidth = colW - 8 });

    float range = params[p].max - params[p].min;
    float pct = (params[p].value - params[p].min) / (range <= 0 ? 1.f : range);
    int bX = x + 4, bY = y + ROW_H - 8, bW = colW - 10;

    if (params[p].graph != nullptr) {
        drawGraph(d, trk, params[p], colW, x, y, bgColor);
    } else if (params[p].type & VALUE_CENTERED) {
        int mid = bX + bW / 2;
        int fw = (int)((bW / 2) * (params[p].value / (params[p].max == 0 ? 1.0f : params[p].max)));

        d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 50, 50, 50 } }); // background

        if (fw < 0) d.filledRect({ mid + fw, bY }, { std::abs(fw), 3 }, { .color = trk.themeColor });
        else d.filledRect({ mid, bY }, { fw, 3 }, { .color = trk.themeColor });

        d.filledRect({ mid, bY - 1 }, { 1, 5 }, { .color = { 100, 100, 100 } });
    } else {
        d.filledRect({ bX, bY }, { bW, 3 }, { .color = { 50, 50, 50 } }); // background
        d.filledRect({ bX, bY }, { (int)(bW * pct), 3 }, { .color = trk.themeColor });
    }
}

bool drawStatic(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY, Track& trk)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    int waveformH = 60;
    int headerW = winW - (MARGIN * 2);
    drawWaveform(d, trk, MARGIN, currentY, headerW, waveformH);

    paramsTopY = currentY + waveformH + MARGIN;

    Param* params = trk.engine->getParams();
    size_t paramCount = trk.engine->getParamCount();

    Color bgColor = darken(d.styles.colors.quaternary, 0.1);
    int visualIdx = 0;
    for (size_t p = 0; p < paramCount; p++, visualIdx++) {
        int x = MARGIN + ((int)visualIdx % paramsPerRow) * colW;
        int y = paramsTopY + ((int)visualIdx / paramsPerRow) * ROW_H;
        currentY = y;

        bool isADSR = (p + 3 < paramCount) && (params[p].module == MODULE_ENV_ADSR && params[p + 3].module == MODULE_ENV_ADSR); // could add && params[p + 1].module == MODULE_ENV_ADSR && params[p + 2].module == MODULE_ENV_ADSR && but i guess it s enough..

        drawParam(d, trk, params, p, colW, winW, x, y, bgColor, now);
    }
    currentY += ROW_H + 10;

    int totalW = winW - (MARGIN * 2);
    int padH = winH - 30 - currentY;
    const char* nameXY = trk.engine->getNameXY();
    if (nameXY != nullptr) {
        int padW = colW * 2.5f;
        xyRect = { { MARGIN, currentY }, { padW, padH } };
        IEngine::XY xy = trk.engine->getXY();
        drawPad(d, xyRect, nameXY, trk.themeColor, xy.x, 1.0f - xy.y);

        // Position Sequencer right next to the XY pad using remaining 5 columns
        int seqW = totalW - padW - MARGIN;
        seqRect = { { MARGIN + padW + MARGIN, currentY }, { seqW, padH } };
        drawSequencer(d, trk, seqRect);
    } else {
        xyRect = { { -1, -1 }, { -1, -1 } };
        // Sequencer takes up full width if no XY Pad is registered
        seqRect = { { MARGIN, currentY }, { totalW, padH } };
        drawSequencer(d, trk, seqRect);
    }

    return true;
}
struct SavedPixels {
    Color pixels[2][4]; // [width][height]
    Point pos;
    bool active = false;
};
std::map<int, SavedPixels> playheadBuffer;

bool drawPlayhead(Draw& d, const int winW, int currentY)
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

    int waveformH = 60;
    int headerW = winW - (MARGIN * 2);
    int voiceCount = trk.engine->getVoiceCount();

    for (int i = 0; i < voiceCount; i++) {
        float ph = trk.engine->getPlayhead(i);
        if (ph < 0.0f || ph > 1.0f) continue;

        int px = MARGIN + (int)(ph * (headerW - 2));

        int py = currentY + ((i + 1) * waveformH) / (voiceCount + 1);
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

int lastActiveStep = -1;
Color lastStepColor = { 0, 0, 0 };
bool drawSequencePlayhead(Draw& d, Track& trk)
{
    if (seqRect.size.w <= 0) return false;

    int cellW = seqRect.size.w / stepsPerRow;
    int cellH = seqRect.size.h / rows;

    bool rendered = false;
    int currentStep = studio.isPlaying ? (studio.currentStep % SEQ_STEPS) : -1;

    if (lastActiveStep != currentStep) {
        if (lastActiveStep >= 0 && lastActiveStep < SEQ_STEPS) {
            int r = lastActiveStep / stepsPerRow;
            int c = lastActiveStep % stepsPerRow;
            int sx = seqRect.position.x + c * cellW + 1;
            int sy = seqRect.position.y + r * cellH + 2;

            Color originalColor = lastStepColor;
            d.line({ sx, sy }, { sx + cellW - 2, sy }, { originalColor });
            rendered = true;
        }
        lastActiveStep = -1;

        if (currentStep >= 0) {
            int r = currentStep / stepsPerRow;
            int c = currentStep % stepsPerRow;
            int sx = seqRect.position.x + c * cellW + 1;
            int sy = seqRect.position.y + r * cellH + 2;

            lastStepColor = d.getPixel({ sx, sy});
            d.line({ sx, sy }, { sx + cellW - 2, sy }, { { 255, 255, 255 } });

            lastActiveStep = currentStep;
            rendered = true;
        }
    }

    return rendered;
}

bool draw(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY)
{
    if (studio.tracks[studio.selTrack] == nullptr) return false;
    Track& trk = *studio.tracks[studio.selTrack];

    bool rendered = false;

    if (needFullRedraw) {
        lastActiveStep = -1;
    }

    rendered |= drawStatic(d, winW, winH, needFullRedraw, currentY, trk);
    rendered |= drawPlayhead(d, winW, currentY);
    rendered |= drawSequencePlayhead(d, trk);

    return rendered;
}

void onXY(Point position, Track& trk)
{
    float x = position.x - xyRect.position.x;
    float y = position.y - xyRect.position.y;
    trk.engine->setXY({ x / xyRect.size.w, 1.0f - (y / xyRect.size.h) });
    needsRedraw = true;
}

void mouseButtonPressed(Point position)
{
    if (studio.currentView != ViewTrack || studio.tracks[studio.selTrack] == nullptr) return;
    Track& trk = *studio.tracks[studio.selTrack];

    if (inRect(xyRect, position)) {
        xyDragging = true;
        onXY(position, trk);
        return;
    }

    if (loopRect.size.w > 0 && inRect(loopRect, position)) {
        isDraggingLoop = true;
        dragStartX = position.x;
        initialLoopStart = trk.engine->getLoopStart();
        return;
    }

    if (seqRect.size.w > 0 && inRect(seqRect, position)) {
        int stepsPerRow = 16;
        int rows = 4;
        int cellW = seqRect.size.w / stepsPerRow;
        int cellH = seqRect.size.h / rows;

        int col = (position.x - seqRect.position.x) / cellW;
        int row = (position.y - seqRect.position.y) / cellH;

        if (col >= 0 && col < stepsPerRow && row >= 0 && row < rows) {
            int stepIdx = row * stepsPerRow + col;
            if (stepIdx >= 0 && stepIdx < SEQ_STEPS) {
                if (trk.sequence.size() <= (size_t)stepIdx) trk.sequence.resize(SEQ_STEPS);

                // Toggle state
                trk.sequence[stepIdx].active = !trk.sequence[stepIdx].active;
                needsRedraw = true;
            }
        }
        return;
    }
}

void mouseMoved(Point position, const int winW)
{
    if (studio.currentView != ViewTrack || studio.tracks[studio.selTrack] == nullptr) return;
    Track& trk = *studio.tracks[studio.selTrack];

    if (xyDragging) {
        onXY(position, trk);
    }

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
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            trk.engine->setLoopStart(newLoopStart);
        }

        needsRedraw = true;
    }
}

void mouseButtonReleased()
{
    isDraggingLoop = false;
    xyDragging = false;
}

bool mouseWheelScrolled(Point position, int delta, const int winW, uint32_t now, bool shifted)
{
    if (studio.currentView != ViewTrack) return false;

    if (studio.tracks[studio.selTrack] == nullptr) return false;
    Track& trk = *studio.tracks[studio.selTrack];

    const int cW = (winW - MARGIN * 2) / 8;

    int finalPIdx = ((position.y - paramsTopY) / ROW_H) * 8 + (position.x - MARGIN) / cW;

    if (finalPIdx >= 0 && (size_t)finalPIdx < trk.engine->getParamCount()) {
        std::lock_guard<std::mutex> lock(studio.audioMutex);
        Param& p = trk.engine->getParams()[finalPIdx];

        int scaled = encGetScaledDirection(delta, now, trk.lastShiftTicks[finalPIdx]);
        trk.lastShiftTicks[finalPIdx] = now;

        p.inc(scaled * (shifted ? 5.f : 1.f));

        trk.activeParamIdx = finalPIdx;
        trk.lastEditTime = std::chrono::steady_clock::now();
        needsRedraw = true;
        return true;
    }

    return false;
}
}