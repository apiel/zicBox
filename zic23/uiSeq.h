#pragma once

#include <thread>

#include "helpers/clamp.h"
#include "helpers/enc.h"
#include "helpers/format.h"
#include "helpers/midiNote.h"
#include "zic23/studio.h"

static constexpr int STEP_H = 14; // sequencer step height
static constexpr int LANE_H = 18; // note-lane pixels below step

static int copyTrackIdx = -1, copyStepIdx = -1;
static Step copiedStep;

// Helper to trigger a non-blocking note preview (noteOn -> wait -> noteOff)
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

void stretchTrackSequence(Track& trk, bool setLen = false)
{
    if (trk.genLen <= 4) return;
    std::vector<Step> newSeq(SEQ_STEPS);
    for (int i = 0; i < 32; i++) {
        if (trk.sequence[i].active) {
            newSeq[i * 2] = trk.sequence[i];
            if (trk.type == TRACK_TYPE_SYNTH) {
                newSeq[i * 2].len *= 2.0f;
            }
        }
    }
    for (int i = 0; i < SEQ_STEPS; i++)
        trk.sequence[i] = newSeq[i];
    if (setLen) trk.genLen /= 2;
}

void compressTrackSequence(Track& trk, bool setLen = false)
{
    if (trk.genLen >= 128) return;
    std::vector<Step> newSeq(SEQ_STEPS);
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (trk.sequence[i].active) {
            int newIdx = i / 2;
            newSeq[newIdx] = trk.sequence[i];
            newSeq[newIdx].active = true;
            if (trk.type == TRACK_TYPE_SYNTH) {
                newSeq[newIdx].len = std::max(0.5f, newSeq[newIdx].len / 2.0f);
            }
        }
    }
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (i > 31) trk.sequence[i] = newSeq[i - 32];
        else trk.sequence[i] = newSeq[i];
    }
    if (setLen) trk.genLen *= 2;
}

void runGeneration(int trkIdx)
{
    Track& trk = *studio.tracks[trkIdx];
    if (trk.generate != nullptr) {
        trk.generate(trk.sequence);
        // Apply stretch logic based on display state (64 is native)
        if (trk.genLen == 32) stretchTrackSequence(trk);
        else if (trk.genLen == 16) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 8) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 4) {
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
            stretchTrackSequence(trk);
        } else if (trk.genLen == 128) {
            compressTrackSequence(trk);
        }

        studio.selTrack = trkIdx;
        studio.selStep = 0;
    }
}

void editStep(Step& step, StepEditMode mode, int scaled)
{
    if (mode == StepEditMode::EDIT_NOTE) step.note = CLAMP(step.note + scaled, 0, (int)MIDI_LAST_NOTE);
    else if (mode == StepEditMode::EDIT_VELO) step.velocity = CLAMP(step.velocity + (scaled * 0.05f), 0.0f, 1.0f);
    else if (mode == StepEditMode::EDIT_PROB) step.condition = CLAMP(step.condition + (scaled * 0.01f), 0.0f, 1.0f);
    else if (mode == StepEditMode::EDIT_LEN) step.len = CLAMP(step.len + (scaled * 0.5f), 0.5f, 64.5f);
}

void duplicateTrackSequence(Track& trk)
{
    int lastActive = -1;
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (trk.sequence[i].active) lastActive = i;
    }

    if (lastActive == -1) return;

    int currentLen = 0;
    if (lastActive < 4) currentLen = 4;
    else if (lastActive < 8) currentLen = 8;
    else if (lastActive < 16) currentLen = 16;
    else if (lastActive < 32) currentLen = 32;
    else return; // Already at max or filled beyond 32

    for (int i = 0; i < currentLen; i++) {
        if (currentLen + i < SEQ_STEPS) {
            trk.sequence[currentLen + i] = trk.sequence[i];
        }
    }
}

void deleteTrackSequence(Track& trk)
{
    int lastActive = -1;
    for (int i = 0; i < SEQ_STEPS; i++) {
        if (trk.sequence[i].active) lastActive = i;
    }

    if (lastActive == -1) return;

    int i = 0;
    if (lastActive < 4) i = 0;
    else if (lastActive < 8) i = 4;
    else if (lastActive < 16) i = 8;
    else if (lastActive < 32) i = 16;
    else i = 32;

    for (; i < SEQ_STEPS; i++) {
        trk.sequence[i].active = false;
    }
}

int drawMixerUI(Draw& d, sf::Vector2u size, int currentY)
{
    const int winW = (int)size.x;
    const int muteW = 25, volW = 70, genW = 25, editW = 25, sp = 5;
    const int mixW = muteW + sp + volW + sp + sp + genW + sp + editW + 10;
    const int stepW = (winW - (MARGIN * 2 + mixW)) / 64;

    for (int i = 0; i < MAX_TRACKS; i++) {
        Track& trk = *studio.tracks[i];

        // Existing Mute and Vol...
        trk.muteRect = { MARGIN, currentY, muteW, STEP_H };
        d.filledRect({ trk.muteRect.left, trk.muteRect.top }, { muteW, STEP_H }, { .color = trk.isMuted ? Color { 200, 50, 50 } : Color { 40, 40, 45 } });
        d.text({ trk.muteRect.left + 8, trk.muteRect.top + 1 }, "M", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        trk.volRect = { trk.muteRect.left + muteW + sp, currentY, volW, STEP_H };
        d.filledRect({ trk.volRect.left, trk.volRect.top }, { volW, STEP_H }, { .color = { 40, 40, 45 } });
        d.filledRect({ trk.volRect.left, trk.volRect.top + STEP_H / 2 - 2 }, { (int)(volW * trk.volume), 4 }, { .color = trk.themeColor });

        // Generate Button
        trk.genRect = { trk.volRect.left + volW + sp, currentY, genW, STEP_H };
        d.filledRect({ trk.genRect.left, trk.genRect.top }, { genW, STEP_H }, { .color = { 60, 60, 75 } });
        d.textCentered({ trk.genRect.left + genW / 2, trk.genRect.top + 1 }, "G" + std::to_string(trk.genLen), 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        trk.editRect = { trk.genRect.left + genW + sp, currentY, editW, STEP_H };
        d.filledRect({ trk.editRect.left, trk.editRect.top }, { editW, STEP_H }, { .color = { 80, 80, 100 } });
        d.textCentered({ trk.editRect.left + editW / 2, trk.editRect.top + 1 }, "E", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

        int gx = trk.editRect.left + editW + 10;
        for (int s = 0; s < SEQ_STEPS; s++)
            trk.stepRects[s] = { gx + s * stepW, currentY, stepW - 1, STEP_H / 2 + LANE_H };
        currentY += 26;
    }
    return currentY;
}

int drawSeqUI(Draw& d, sf::Vector2u size, int currentY)
{
    if (studio.selTrack != -1 && studio.selStep != -1) {
        auto& s = studio.tracks[studio.selTrack]->sequence[studio.selStep];
        int eY = currentY;
        d.text({ MARGIN, eY }, "EDIT T" + std::to_string(studio.selTrack + 1) + " S" + std::to_string(studio.selStep + 1), 8,
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
        currentY += 16;
    }

    int sw = studio.tracks[0]->stepRects[0].width + 1;
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            auto& step = trk->sequence[s];
            if (step.active) {
                float nm = 1.f - (float)(CLAMP(step.note, 24, 96) - 24) / 72.f;
                int ny = r.top + r.height - LANE_H + 1 + (int)(nm * LANE_H);
                d.line({ r.left, ny }, { r.left + (int)(step.len * (sw - 1)) - 1, ny }, { .color = trk->themeColor });
            }
        }
    }

    return currentY;
}

void updateSequencerPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    int sw = studio.tracks[0]->stepRects[0].width + 1;
    int ph = (int)((studio.currentStep + studio.sampleCounter.load() / studio.samplesPerStep) * sw);
    int gx = studio.tracks[0]->stepRects[0].left;
    for (int t = 0; t < MAX_TRACKS; t++) {
        auto& trk = studio.tracks[t];
        for (int s = 0; s < SEQ_STEPS; s++) {
            auto& r = trk->stepRects[s];
            int h = r.height - LANE_H;
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

void handelSeqEventKeyPressed(sf::RenderWindow& window, sf::Event& event, bool& static_needs_redraw)
{
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
                studio.tracks[studio.selTrack]->genLen = studio.tracks[copyTrackIdx]->genLen;
            }
            static_needs_redraw = true;
        }
    }
}

void handelSeqEventMousePressed(sf::RenderWindow& window, sf::Event& event, bool& static_needs_redraw)
{
    int mx = event.mouseButton.x, my = event.mouseButton.y;
    if (event.mouseButton.button == sf::Mouse::Middle) {
        studio.stepEditMode = static_cast<StepEditMode>((studio.stepEditMode + 1) % MODE_COUNT);
        static_needs_redraw = true;
    }
    for (int t = 0; t < (int)studio.tracks.size(); t++) {
        auto& trk = studio.tracks[t];
        if (trk->editRect.contains(mx, my)) {
            studio.pianoRollTrack = t;
            static_needs_redraw = true;
        }
        if (trk->genRect.contains(mx, my)) {
            runGeneration(t);
            static_needs_redraw = true;
        }
        if (trk->muteRect.contains(mx, my)) {
            trk->isMuted = !trk->isMuted;
            static_needs_redraw = true;
        }
        for (int s = 0; s < SEQ_STEPS; s++)
            if (trk->stepRects[s].contains(mx, my)) {
                studio.selTrack = t;
                studio.selStep = s;
                if (event.mouseButton.button == sf::Mouse::Left && !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                    trk->sequence[s].active = !trk->sequence[s].active;
                    if (trk->sequence[s].active && !studio.isPlaying) triggerPreview(*trk, trk->sequence[s].note, trk->sequence[s].velocity);
                }
                static_needs_redraw = true;
            }
    }
}

void handelSeqEventMouseWheelScrolled(sf::RenderWindow& window, sf::Event& event, bool& static_needs_redraw)
{
    int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
    float delta = event.mouseWheelScroll.delta;
    uint32_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    bool handled = false;
    for (int t = 0; t < MAX_TRACKS && !handled; t++) {
        auto& trk = studio.tracks[t];
        if (trk->lenBtnRect.contains(mx, my)) {
            if (delta > 0) compressTrackSequence(*trk, true);
            else stretchTrackSequence(*trk, true);
            static_needs_redraw = true;
            handled = true;
            break;
        }
        for (int s = 0; s < SEQ_STEPS; s++)
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
    if (!handled) {
        // TODO move bpm and param set in main.cpp
        if (studio.bpmRect.contains(mx, my)) {
            int scaled = encGetScaledDirection(delta, now, lastBpmTick);
            lastBpmTick = now;
            studio.bpm = std::clamp(studio.bpm + (scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 5.0f : 0.5f)), 20.0f, 300.0f);
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
                    trk->volume = std::clamp(trk->volume + scaled * (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.05f : 0.01f), 0.f, 1.f);
                    static_needs_redraw = true;
                } else if (trk->trackBounds.contains(mx, my)) {
                    const int winW = (int)window.getSize().x;
                    const int cW = (winW - MARGIN * 2) / 8;
                    int pIdx = ((my - (trk->trackBounds.top + TRACK_H)) / ROW_H) * 8 + (mx - MARGIN) / cW;
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