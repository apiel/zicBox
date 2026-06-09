#pragma once

#include "draw/utils/inRect.h"
#include "helpers/enc.h"
#include "helpers/format.h"
#include "helpers/midiNote.h"
#include "zicRack/draw.h"
#include "zicRack/drawPad.h"
#include "zicRack/project.h"
#include "zicRack/studio.h"
#include "zicRack/utils.h"
namespace UiTrack {

bool needsRedraw = true;
int paramsTopY = 0;
int waveformTopY = 0;

int Y_MARGIN = 6;

Rect loopRect = { { -1, -1 }, { -1, -1 } };
bool isDraggingLoop = false;
int dragStartX = 0;
float initialLoopStart = 0.0f;

bool xyDragging = false;
Rect xyRect;

Rect seqRect = { { -1, -1 }, { -1, -1 } };
Rect editNoteRect, editLenRect, editVeloRect, editProbRect;
int currentSeqPage = 0;
int stepsPerRow = 16;
int rows = 4; // 4 rows * 16 steps = 64 steps
int lastStepEdit = -1;
int waveformH = 50;

Rect historyRect = { { -1, -1 }, { -1, -1 } };

Rect clipsRect = { { -1, -1 }, { -1, -1 } };

void drawClips(Draw& d, Track& trk, Rect rect)
{
    // MAX_CLIP_COUNT
    int cellW = rect.size.w / MAX_CLIP_COUNT;

    d.filledRect(rect.position, rect.size, { .color = d.styles.colors.background });

    for (int i = 0; i < MAX_CLIP_COUNT; i++) {
        int sx = rect.position.x + i * cellW;

        Color bg = { 45, 45, 50 };
        Color textColor = { 120, 120, 130 };

        Clip& clip = trk.clips[i];
        if (trk.activeClipIdx == i) {
            bg = trk.themeColor;
            textColor = { 255, 255, 255 };
        } else if (clip.saved) {
            bg = trk.themeColor;
            bg.a = 128;
            textColor = { 200, 200, 210 };
        }

        d.filledRect({ sx + 1, rect.position.y }, { cellW - 2, rect.size.h }, { .color = bg });
        d.text({ sx + 4, rect.position.y + 2 }, std::to_string(i + 1), 8, { .color = textColor, .font = &PoppinsLight_8 });
    }
}

void drawSequencer(Draw& d, Track& trk, Rect rect)
{
    int cellW = rect.size.w / stepsPerRow;
    int cellH = rect.size.h / rows;

    d.filledRect(rect.position, rect.size, { .color = d.styles.colors.background });

    if (trk.sequence.size() < (size_t)SEQ_STEPS) {
        trk.sequence.resize(SEQ_STEPS);
    }

    float activeLen = 0.0f;
    // Scan backwards from the end of the sequence to find the last active step and see overlaping len
    for (int i = SEQ_STEPS - 1; i >= 0; i--) {
        if (trk.sequence[i].active) {
            float spillOverLen = ((float)i + trk.sequence[i].len) - (float)SEQ_STEPS;
            if (spillOverLen > 0.0f) {
                activeLen = spillOverLen;
            }
            break;
        }
    }

    for (int stepIdx = 0; stepIdx < SEQ_STEPS; stepIdx++) {
        int row = stepIdx / stepsPerRow;
        int col = stepIdx % stepsPerRow;

        int sx = rect.position.x + col * cellW;
        int sy = rect.position.y + row * cellH;

        Step& step = trk.sequence[stepIdx];

        bool isBeatHighlight = col % 4 != 0;
        Color stepBg = isBeatHighlight ? Color { 45, 45, 50 } : Color { 35, 35, 40 };
        Color labelColor = Color { 120, 120, 130 };
        Color halfStepBg = stepBg;
        bool restoreHalfStep = false;

        int cX = sx + 1;
        int cW = cellW - 2;
        bool isUnderSustainTrail = (float)stepIdx < activeLen;
        if (step.active) {
            stepBg = trk.themeColor;
            stepBg.a = 100 + (int)(step.velocity * 155.0f);
            labelColor = Color { 255, 255, 255 };
            if (isUnderSustainTrail && stepIdx > 0) {
                cX -= 1;
                cW += 1;
            }

            activeLen = (float)stepIdx + step.len;
        } else if (isUnderSustainTrail) {
            stepBg = trk.themeColor;
            stepBg.a = 130;
            labelColor = Color { 200, 200, 200 };
            restoreHalfStep = ((float)stepIdx + 0.5f) >= activeLen;
            cX -= 1;
            cW += 1;
        }

        d.filledRect({ cX, sy + 1 }, { cW, cellH - 2 }, { .color = stepBg });
        if (restoreHalfStep) {
            d.filledRect({ sx + 1 + cellW / 2, sy + 1 }, { cellW / 2 - 1, cellH - 2 }, { .color = halfStepBg });
        }

        std::string label = std::to_string(stepIdx + 1);
        d.text({ sx + 4, sy + 2 }, label, 8, { .color = labelColor, .font = &PoppinsLight_8 });

        if (step.active) {
            d.text({ sx + 4, sy + 14 }, MIDI_NOTES_STR[step.note], 8, { .color = { 255, 255, 255, 180 }, .font = &PoppinsLight_8 });
            d.text({ sx + 4, sy + 26 }, std::to_string((int)(step.condition * 100)) + "%", 8, { .color = { 255, 255, 255, 180 }, .font = &PoppinsLight_8 });

            int velBarW = (int)((cellW - 8) * step.velocity);
            d.filledRect({ sx + 4, sy + cellH - 6 }, { velBarW, 2 }, { .color = { 255, 255, 255, 180 } });
        }
    }

    int editX = rect.position.x;
    int editY = rect.position.y + 4 * cellH + 4;

    d.filledRect({ editX, editY }, { editX + rect.size.w - 260, 20 }, { .color = d.styles.colors.background });
    if (lastStepEdit != -1) {
        Step& s = trk.sequence[lastStepEdit];

        d.text({ editX + rect.size.w - 260, editY }, "STEP: " + std::to_string(lastStepEdit + 1), 8, { .color = trk.themeColor, .font = &PoppinsLight_8 });

        editNoteRect = { { editX + rect.size.w - 220, editY }, { 55, 15 } };
        d.text(editNoteRect.position, "NOTE: " + std::string(MIDI_NOTES_STR[s.note]), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        editLenRect = { { editX + rect.size.w - 160, editY }, { 45, 15 } };
        d.text(editLenRect.position, "LEN: " + fToString(s.len, 1), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        editVeloRect = { { editX + rect.size.w - 110, editY }, { 45, 15 } };
        d.text(editVeloRect.position, "VEL: " + std::to_string((int)(s.velocity * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        editProbRect = { { editX + rect.size.w - 50, editY }, { 45, 15 } };
        d.text(editProbRect.position, "PROB: " + std::to_string((int)(s.condition * 100)) + "%", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });
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

void drawParams(Draw& d, Param* params, size_t paramCount, int winW, int winH, int colW, int paramsTopY, int paramsPerRow, int& currentY, Color& themeColor, uint8_t encodersSelection)
{

    // Calculate total visual slots needed (rounded up to a full row of 8)
    size_t totalSlots = ((paramCount + (ENCODER_COUNT - 1)) / ENCODER_COUNT) * ENCODER_COUNT;

    int totalParamRows = ((int)totalSlots + paramsPerRow - 1) / paramsPerRow;
    int totalParamH = totalParamRows * UiDraw::ROW_H;
    d.filledRect({ MARGIN, paramsTopY }, { winW - (MARGIN * 2), totalParamH }, { .color = d.styles.colors.background });

    // Variables to capture the edges of the active 2x4 group
    int minX = winW, minY = winH;
    int maxX = 0, maxY = 0;
    bool hasActiveGroup = false;

    for (size_t visualIdx = 0; visualIdx < totalSlots; visualIdx++) {
        int row = (int)visualIdx / paramsPerRow;
        int col = (int)visualIdx % paramsPerRow;

        // --- SORTING MAPPING ---
        int blockRow = row / 2; // Which vertical pair of pages we are on (0 = A/B, 1 = C/D)
        int subRow = row % 2; // Top row (0) or bottom row (1) of the physical 2x4 layout
        int blockSide = col / 4; // Left page (0) or right page (1) in the row
        int subCol = col % 4; // Physical encoder column (0 to 3)

        size_t p = (blockRow * 16) + (blockSide * 8) + (subRow * 4) + subCol;

        if (p >= paramCount) continue;

        int x = MARGIN + col * colW;
        int y = paramsTopY + row * UiDraw::ROW_H;
        currentY = y;

        Color bgColor = lighten(d.styles.colors.quaternary, 0.2);
        Color pColor = darken(themeColor, 0.4f);

        bool isActiveGroup = (int)(p / ENCODER_COUNT) == encodersSelection;
        if (isActiveGroup) {
            bgColor = darken(d.styles.colors.quaternary, 0.1);
            pColor = themeColor;

            // Update bounds for the big rectangle
            hasActiveGroup = true;
            if (x < minX) minX = x;
            if (y < minY) minY = y;
            // colW - 2 and ROW_H - 2 match the inner dimensions inside drawParam
            if (x + colW - 2 > maxX) maxX = x + colW - 2;
            if (y + UiDraw::ROW_H - 2 > maxY) maxY = y + UiDraw::ROW_H - 2;
        }

        UiDraw::param(d, params[p], colW, winW, x, y, bgColor, pColor);
    }

    if (hasActiveGroup) {
        d.rect({ minX, minY }, { (maxX - minX), (maxY - minY) }, { .color = { 90, 90, 90 } });
    }
}

bool drawStatic(Draw& d, const int winW, const int winH, bool needFullRedraw, int currentY, Track& trk)
{
    if (!needsRedraw && !needFullRedraw) return false;
    needsRedraw = false;

    const int paramsPerRow = 8;
    const int colW = (winW - MARGIN * 2) / paramsPerRow;
    auto now = std::chrono::steady_clock::now();

    currentY += Y_MARGIN;
    paramsTopY = currentY;

    Param* params = trk.engine->getParams();
    size_t paramCount = trk.engine->getParamCount();
    drawParams(d, params, paramCount, winW, winH, colW, paramsTopY, paramsPerRow, currentY, trk.themeColor, trk.encodersSelection);

    currentY += UiDraw::ROW_H + 5;

    if (trk.showWaveform) {
        waveformTopY = currentY;
        int headerW = winW - (MARGIN * 2);
        drawWaveform(d, trk, MARGIN, currentY, headerW, waveformH);
        currentY += waveformH + 5;
    }

    int totalW = winW - (MARGIN * 2);
    int padH = winH - 50 - currentY;
    const char* nameXY = trk.engine->getNameXY();
    if (nameXY != nullptr) {
        int padW = colW * 2.5f;
        xyRect = { { MARGIN, currentY }, { padW, padH } };
        IEngine::XY xy = trk.engine->getXY();
        drawPad(d, xyRect, nameXY, trk.themeColor, xy.x, 1.0f - xy.y);

        int seqW = totalW - padW - MARGIN;
        seqRect = { { MARGIN + padW + MARGIN, currentY }, { seqW, padH } };
        clipsRect = { { MARGIN + padW + MARGIN, currentY + padH + 16 }, { seqW, 32 } };
    } else {
        xyRect = { { -1, -1 }, { -1, -1 } };
        seqRect = { { MARGIN, currentY }, { totalW, padH } };
        clipsRect = { { MARGIN, currentY + padH + 16 }, { totalW, 32 } };
    }

    drawSequencer(d, trk, seqRect);
    d.text({ clipsRect.position.x + 1, clipsRect.position.y - 11 }, "CLIPS:", 8, { .color = { 150, 150, 160 }, .font = &PoppinsLight_8 });
    drawClips(d, trk, clipsRect);

    historyRect = { { MARGIN, currentY + padH + 4 }, { WAVE_HISTORY, 40 } };

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
            int sy = seqRect.position.y + r * cellH + cellH - 2;

            Color originalColor = lastStepColor;
            d.line({ sx, sy }, { sx + cellW - 2, sy }, { originalColor });
            rendered = true;
        }
        lastActiveStep = -1;

        if (currentStep >= 0) {
            int r = currentStep / stepsPerRow;
            int c = currentStep % stepsPerRow;
            int sx = seqRect.position.x + c * cellW + 1;
            int sy = seqRect.position.y + r * cellH + cellH - 2;

            lastStepColor = d.getPixel({ sx, sy });
            d.line({ sx, sy }, { sx + cellW - 2, sy }, { { 255, 255, 255 } });

            lastActiveStep = currentStep;
            rendered = true;
        }
    }

    return rendered;
}

bool historyCleaned = false;
bool drawPlayheadHistory(Draw& d, Track& trk)
{
    bool rendered = false;
    std::lock_guard<std::mutex> lk(trk.historyMtx);
    for (int x = 0; x < WAVE_HISTORY; x++) {
        if (trk.history[x]) {
            int bH = (int)(std::min(trk.history[x], 1.f) * (historyRect.size.h / 2));
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
        // clear history as well
        for (int x = 0; x < WAVE_HISTORY; x++)
            trk.history[x] = 0;
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

    if (needFullRedraw) {
        lastActiveStep = -1;
    }

    rendered |= drawStatic(d, winW, winH, needFullRedraw, currentY, trk);
    if (trk.showWaveform) {
        rendered |= drawPlayhead(d, winW);
    }
    rendered |= drawSequencePlayhead(d, trk);

    rendered |= drawPlayheadHistory(d, trk);

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
                if (trk.sequence[stepIdx].active) {
                    lastStepEdit = stepIdx;
                    Step& step = trk.sequence[stepIdx];
                    triggerPreview(trk, step.note, step.velocity);
                } else {
                    lastStepEdit = -1;
                }
                needsRedraw = true;
            }
        }
        return;
    }

    if (inRect(clipsRect, position)) {
        int x = clipsRect.size.w - (position.x - clipsRect.position.x);
        int clipIdx = MAX_CLIP_COUNT - 1 - (x / (clipsRect.size.w / MAX_CLIP_COUNT));
        saveClip(trk, trk.activeClipIdx);
        trk.activeClipIdx = clipIdx;
        loadClip(trk, trk.activeClipIdx);
        needsRedraw = true;
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

    const int paramsPerRow = 8;
    const int cW = (winW - MARGIN * 2) / paramsPerRow;

    int row = (position.y - paramsTopY) / UiDraw::ROW_H;
    int col = (position.x - MARGIN) / cW;

    if (row >= 0 && col >= 0 && col < paramsPerRow) {
        // Apply the layout mapping step to find the parameter index
        int blockRow = row / 2;
        int subRow = row % 2;
        int blockSide = col / 4;
        int subCol = col % 4;

        size_t finalPIdx = (blockRow * 16) + (blockSide * 8) + (subRow * 4) + subCol;

        if (finalPIdx < trk.engine->getParamCount()) {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            Param& p = trk.engine->getParams()[finalPIdx];

            int scaled = encGetScaledDirection(delta, now, trk.lastShiftTicks[finalPIdx]);
            trk.lastShiftTicks[finalPIdx] = now;

            p.inc(scaled * (shifted ? 5.f : 1.f));

            // --- AUTOMATIC GROUP SELECTION SWITCH ---
            // Calculate which 8-parameter block this index belongs to
            uint8_t targetGroup = (uint8_t)(finalPIdx / 8);
            if (trk.encodersSelection != targetGroup) {
                trk.encodersSelection = targetGroup;
            }

            trk.activeParamIdx = finalPIdx;
            trk.lastEditTime = std::chrono::steady_clock::now();
            needsRedraw = true;
            return true;
        }
    }

    if (lastStepEdit != -1) {
        Step& s = trk.sequence[lastStepEdit];
        bool handled = false;
        int sc = delta > 0 ? 1 : -1;

        if (inRect(editNoteRect, position)) {
            editStep(s, EDIT_NOTE, sc);
            triggerPreview(trk, s.note, s.velocity);
            handled = true;
        } else if (inRect(editLenRect, position)) {
            editStep(s, EDIT_LEN, sc);
            handled = true;
        } else if (inRect(editVeloRect, position)) {
            editStep(s, EDIT_VELO, sc);
            handled = true;
        } else if (inRect(editProbRect, position)) {
            editStep(s, EDIT_PROB, sc);
            handled = true;
        }

        if (handled) {
            needsRedraw = true;
            return true;
        }
    }

    return false;
}
}