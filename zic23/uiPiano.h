#pragma once

#include "helpers/midiNote.h"
#include "zic23/studio.h"
#include "zic23/uiSeq.h"

sf::IntRect pianoRollCloseRect;
sf::IntRect pianoRollProgressRect;

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
            d.filledRect({ nX, nY }, { nW, (int)cellH - 2 }, { .color = trk.themeColor });
        }
    }

    pianoRollProgressRect = { gridX, gridY + gridH + 5, gridW, 10 };
    d.filledRect({ pianoRollProgressRect.left, pianoRollProgressRect.top }, { pianoRollProgressRect.width, pianoRollProgressRect.height }, { .color = { 20, 20, 25 } });
}

void handelPianoEvent(sf::RenderWindow& window, sf::Event& event, bool& static_needs_redraw)
{
    if (event.type == sf::Event::MouseButtonPressed) {
        int mx = event.mouseButton.x, my = event.mouseButton.y;
        if (pianoRollCloseRect.contains(mx, my)) {
            studio.pianoRollTrack = -1;
            static_needs_redraw = true;
            return;
        }

        // Grid logic
        int margin = 40;
        int gridX = margin + 40;
        int gridY = margin + 40;
        int gridW = (int)window.getSize().x - margin * 2 - 60;
        int gridH = (int)window.getSize().y - margin * 2 - 60;

        if (mx >= gridX && mx < gridX + gridW && my >= gridY && my < gridY + gridH) {
            int stepIdx = (int)((mx - gridX) / ((float)gridW / SEQ_STEPS));
            int note = PIANO_ROLL_MAX_NOTE - (int)((my - gridY) / ((float)gridH / PIANO_ROLL_NOTE_COUNT));

            Track& trk = *studio.tracks[studio.pianoRollTrack];
            if (trk.sequence[stepIdx].active && trk.sequence[stepIdx].note == note) {
                trk.sequence[stepIdx].active = false;
            } else {
                trk.sequence[stepIdx].active = true;
                trk.sequence[stepIdx].note = note;
                triggerPreview(trk, note, 0.8f);
            }
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
