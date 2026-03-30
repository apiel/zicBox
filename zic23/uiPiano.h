#pragma once

#include "helpers/midiNote.h"
#include "zic23/studio.h"
#include "zic23/uiSeq.h"

sf::IntRect pianoRollCloseRect;
sf::IntRect pianoRollProgressRect;

int dragStepIdx = -1;
int dragNote = -1;

const int PIANO_ROLL_MIN_NOTE = 36; // C2
const int PIANO_ROLL_MAX_NOTE = 84; // C6
const int PIANO_ROLL_NOTE_COUNT = PIANO_ROLL_MAX_NOTE - PIANO_ROLL_MIN_NOTE;

void drawPianoRoll(Draw& d, sf::Vector2u size)
{
    if (studio.pianoRollTrack == -1) return;

    Track& trk = *studio.tracks[studio.pianoRollTrack];
    int W = (int)size.x, H = (int)size.y;
    int margin = 40;
    int pW = W - margin * 2, pH = H - margin * 2;
    int pX = margin, pY = margin;

    // Background
    d.filledRect({ pX, pY }, { pW, pH }, { .color = { 25, 25, 30 } });
    d.rect({ pX, pY }, { pW, pH }, { .color = trk.themeColor });

    // Header & Close Button
    d.text({ pX + 10, pY + 10 }, trk.engine->getName(), 16, { .color = { 150, 150, 150 }, .font = &PoppinsLight_16 });
    pianoRollCloseRect = { pX + pW - 60, pY + 10, 50, 20 };
    d.filledRect({ pianoRollCloseRect.left, pianoRollCloseRect.top }, { 50, 20 }, { .color = { 200, 50, 50 } });
    d.text({ pianoRollCloseRect.left + 8, pianoRollCloseRect.top + 4 }, "CLOSE", 8, { .color = { 255, 255, 255 }, .font = &PoppinsLight_8 });

    // Grid Settings
    int gridX = pX + 40; // Space for note names
    int gridY = pY + 40;
    int gridW = pW - 60;
    int gridH = pH - 60;
    float cellW = (float)gridW / SEQ_STEPS;
    float cellH = (float)gridH / PIANO_ROLL_NOTE_COUNT;

    // Draw Grid
    for (int i = 0; i < PIANO_ROLL_NOTE_COUNT; i++) {
        int note = PIANO_ROLL_MAX_NOTE - i;
        int rowY = gridY + (int)(i * cellH);

        // Key labels and row background
        bool isBlack = isBlackKey(note);
        if (isBlack) d.filledRect({ gridX, rowY }, { gridW, (int)cellH - 1 }, { .color = Color { 0, 0, 0 } });
        d.filledRect({ pX + 5, rowY }, { 30, (int)cellH - 1 }, { .color = isBlack ? Color { 40, 40, 45 } : Color { 255, 255, 255 } });
        d.text({ pX + 7, rowY + 2 }, MIDI_NOTES_STR[note], 8, { .color = isBlack ? Color { 200, 200, 200 } : Color { 20, 20, 20 }, .font = &PoppinsLight_8 });

        d.line({ gridX, rowY }, { gridX + gridW, rowY }, { .color = { 45, 45, 50 } });
    }

    for (int s = 0; s <= SEQ_STEPS; s++) {
        int colX = gridX + (int)(s * cellW);
        d.line({ colX, gridY }, { colX, gridY + gridH }, { .color = (s % 4 == 0) ? Color { 80, 80, 90 } : Color { 45, 45, 50 } });
    }

    // Draw Existing Notes
    for (int s = 0; s < SEQ_STEPS; s++) {
        Step& step = trk.sequence[s];
        if (step.active && step.note >= PIANO_ROLL_MIN_NOTE && step.note <= PIANO_ROLL_MAX_NOTE) {
            int noteIdx = PIANO_ROLL_MAX_NOTE - step.note;
            int nX = gridX + (int)(s * cellW) + 1;
            int nY = gridY + (int)(noteIdx * cellH) + 1;
            int nW = (int)(cellW * step.len) - 2;

            if (step.velocity < 1.0f) {
                Color c = trk.themeColor;
                c.a = 100;
                d.filledRect({ nX, nY }, { nW, (int)cellH - 2 }, { .color = c });
            }

            int totalH = (int)cellH - 2;
            int veloH = (int)(totalH * step.velocity);
            int offsetH = totalH - veloH;

            d.filledRect({ nX, nY + offsetH }, { nW, veloH }, { .color = trk.themeColor });
            d.rect({ nX, nY }, { nW, totalH }, { .color = { 100, 100, 100 } });
        }
    }

    pianoRollProgressRect = { gridX, gridY + gridH + 5, gridW, 10 };
    d.filledRect({ pianoRollProgressRect.left, pianoRollProgressRect.top }, { pianoRollProgressRect.width, pianoRollProgressRect.height }, { .color = { 20, 20, 25 } });
}

void handelPianoEvent(sf::RenderWindow& window, sf::Event& event, bool& static_needs_redraw)
{
    const int margin = 40;
    const int gridX = margin + 40;
    const int gridY = margin + 40;
    const int gridW = (int)window.getSize().x - margin * 2 - 60;
    const int gridH = (int)window.getSize().y - margin * 2 - 60;
    const float cellW = (float)gridW / SEQ_STEPS;
    const float cellH = (float)gridH / PIANO_ROLL_NOTE_COUNT;

    if (event.type == sf::Event::MouseButtonPressed) {
        int mx = event.mouseButton.x, my = event.mouseButton.y;

        if (pianoRollCloseRect.contains(mx, my)) {
            studio.pianoRollTrack = -1;
            dragStepIdx = -1;
            static_needs_redraw = true;
            return;
        }

        if (mx >= gridX && mx < gridX + gridW && my >= gridY && my < gridY + gridH) {
            float clickStep = (float)(mx - gridX) / cellW;
            int noteAtClick = PIANO_ROLL_MAX_NOTE - (int)((my - gridY) / cellH);
            Track& trk = *studio.tracks[studio.pianoRollTrack];

            // 1. Scan to see if we are clicking on an existing note's body
            int foundIdx = -1;
            for (int s = 0; s < SEQ_STEPS; s++) {
                Step& step = trk.sequence[s];
                if (step.active && step.note == noteAtClick) {
                    if (clickStep >= (float)s && clickStep < ((float)s + step.len)) {
                        foundIdx = s;
                        break;
                    }
                }
            }

            // 2. Handle Deletion (Right Click OR Shift + Left Click)
            if (foundIdx != -1) {
                if (event.mouseButton.button == sf::Mouse::Right || sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                    trk.sequence[foundIdx].active = false;
                    dragStepIdx = -1;
                } else if (event.mouseButton.button == sf::Mouse::Left) {
                    // Start dragging existing note (Pitch/Length)
                    dragStepIdx = foundIdx;
                    triggerPreview(trk, trk.sequence[dragStepIdx].note, trk.sequence[dragStepIdx].velocity);
                }
            }
            // 3. Handle Creation (Left Click only)
            else if (event.mouseButton.button == sf::Mouse::Left) {
                int exactStep = (int)clickStep;
                trk.sequence[exactStep].active = true;
                trk.sequence[exactStep].note = noteAtClick;
                trk.sequence[exactStep].len = 1.0f;
                trk.sequence[exactStep].velocity = 0.8f;
                dragStepIdx = exactStep;
                triggerPreview(trk, noteAtClick, 0.8f);
            }
            static_needs_redraw = true;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            dragStepIdx = -1;
        }
    }

    if (event.type == sf::Event::MouseMoved && dragStepIdx != -1) {
        // ... (Keep your MouseMoved logic for vertical pitch and horizontal length) ...
        int mx = event.mouseMove.x, my = event.mouseMove.y;
        Track& trk = *studio.tracks[studio.pianoRollTrack];
        Step& step = trk.sequence[dragStepIdx];

        int noteAtMouse = PIANO_ROLL_MAX_NOTE - (int)((my - gridY) / cellH);
        noteAtMouse = std::clamp(noteAtMouse, PIANO_ROLL_MIN_NOTE, PIANO_ROLL_MAX_NOTE);

        if (step.note != noteAtMouse) {
            step.note = noteAtMouse;
            triggerPreview(trk, step.note, step.velocity);
        }

        float currentMouseStep = (float)(mx - gridX) / cellW;
        float newLen = std::max(0.5f, currentMouseStep - dragStepIdx + 0.8f);
        step.len = std::clamp(newLen, 0.5f, (float)(SEQ_STEPS - dragStepIdx));

        static_needs_redraw = true;
    }

    if (event.type == sf::Event::MouseWheelScrolled) {
        int mx = event.mouseWheelScroll.x, my = event.mouseWheelScroll.y;
        if (mx >= gridX && mx < gridX + gridW && my >= gridY && my < gridY + gridH) {
            float mouseStep = (float)(mx - gridX) / cellW;
            int noteUnderMouse = PIANO_ROLL_MAX_NOTE - (int)((my - gridY) / cellH);
            Track& trk = *studio.tracks[studio.pianoRollTrack];
            int sc = event.mouseWheelScroll.delta > 0 ? 1 : -1;

            for (int s = 0; s < SEQ_STEPS; s++) {
                Step& step = trk.sequence[s];
                if (step.active && step.note == noteUnderMouse) {
                    if (mouseStep >= (float)s && mouseStep < ((float)s + step.len)) {
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                            editStep(step, EDIT_VELO, sc);
                        } else {
                            editStep(step, EDIT_LEN, sc);
                        }
                        static_needs_redraw = true;
                        break;
                    }
                }
            }
        }
    }

    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Space) {
            studio.isPlaying = !studio.isPlaying;
            static_needs_redraw = true;
        }
        if (event.key.code == sf::Keyboard::Escape) {
            studio.pianoRollTrack = -1;
            dragStepIdx = -1;
            static_needs_redraw = true;
        }
    }
}

void updatePianoRollPixels(std::vector<sf::Uint8>& pixels, int stride)
{
    if (studio.pianoRollTrack == -1 || !studio.isPlaying) return;

    // Calculate normalized progress (0.0 to 1.0) across the sequence
    float progress = (studio.currentStep + (float)studio.sampleCounter.load() / studio.samplesPerStep) / SEQ_STEPS;
    int playheadX = (int)(progress * pianoRollProgressRect.width);

    for (int y = 0; y < pianoRollProgressRect.height; y++) {
        for (int x = 0; x < pianoRollProgressRect.width; x++) {
            size_t idx = ((pianoRollProgressRect.top + y) * stride + (pianoRollProgressRect.left + x)) * 4;

            if (x <= playheadX) {
                // Bright color for the "played" part (or track theme color)
                pixels[idx] = 250;
                pixels[idx + 1] = 250;
                pixels[idx + 2] = 250;
            } else {
                // Dark background for the "unplayed" part
                pixels[idx] = 40;
                pixels[idx + 1] = 40;
                pixels[idx + 2] = 45;
            }
        }
    }
}
