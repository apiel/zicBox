#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "Fm4Sequencer.h"
#include "core/Fm4Panel.h"

extern std::mutex audioMutex;
extern std::atomic<int> auditionNote;  // note to trigger, 0 = none

struct Fm4Capture {
    static constexpr int RING = 4096;
    float ring[RING] = {};
    int ringPos = 0;
    float peak = 0.0f;     // peak since the last render
    int note = 0;
    int hitId = 0;
    float livePeak = 0.0f; // smoothed output peak
};

namespace Theme {
inline sf::Color rgb(int r, int g, int b, int a = 255) { return sf::Color(r, g, b, a); }
const sf::Color bg = rgb(8, 9, 13);
const sf::Color card = rgb(17, 19, 25);
const sf::Color cardLine = rgb(31, 34, 44);
const sf::Color cell = rgb(26, 28, 36);
const sf::Color track = rgb(44, 47, 58);
const sf::Color cap = rgb(28, 30, 39);
const sf::Color text = rgb(234, 236, 242);
const sf::Color muted = rgb(126, 131, 148);
const sf::Color faint = rgb(68, 72, 88);

// Block accents: 0: OPERATOR FM (Violet), 1: OPERATOR ENVS (Amber), 2: AMP & FILTER (Cyan)
const sf::Color blockAccent[3] = {
    rgb(172, 128, 255),   // FM
    rgb(255, 176, 64),    // ENVS
    rgb(56, 212, 255),    // AMP & FILTER
};

// Operator colors: C, A, B1, B2
const sf::Color opC = rgb(56, 212, 255);
const sf::Color opA = rgb(172, 128, 255);
const sf::Color opB1 = rgb(255, 176, 64);
const sf::Color opB2 = rgb(255, 86, 98);

const sf::Color fmNeon = rgb(230, 255, 40); // acid yellow, for the transport / playhead
}

class EmuUi {
public:
    static constexpr float W = 1320.0f;
    static constexpr float H = 1240.0f;
    static constexpr float CARD_X = 24.0f, CELL_W = 150.0f, GAP = 36.0f;

    static constexpr float CARDS_Y = 266.0f, ROW_PITCH = 134.0f, HEAD_S = 44.0f;
    static constexpr float CARD_H2 = HEAD_S + 2.0f * ROW_PITCH + 8.0f; // 320: 2-row card
    static constexpr float CARD_H1 = HEAD_S + ROW_PITCH + 8.0f;        // 186: 1-row card
    static constexpr float STACK_GAP = 14.0f;

    static constexpr float SCOPE_Y = 82.0f, SCOPE_H = 170.0f;
    static constexpr float ROLL_Y = 800.0f;
    static constexpr float ROLL_H = H - ROLL_Y - 16.0f;

    struct Preset {
        const char* name;
        std::vector<std::pair<const char*, float>> values;
    };

    static const std::vector<Preset>& presets()
    {
        static const std::vector<Preset> p = {
            { "EPIANO", {
                { "alg", 1 }, { "ratioC", 0 }, { "ratioA", 3 }, { "ratioB", 35 },
                { "harm", 64 }, { "detune", 6 }, { "fdbk", 10 }, { "mix", 70 },
                { "leva", 90 }, { "atka", 0 }, { "deca", 60 }, { "enda", 20 },
                { "leve", 80 }, { "atkb", 0 }, { "decb", 55 }, { "endb", 0 },
                { "atk", 0 }, { "dec", 70 }, { "sus", 40 }, { "rel", 60 },
                { "cut", 110 }, { "reso", 10 }, { "drive", 20 }, { "trig", 0 } } },
            { "FM BASS", {
                { "alg", 4 }, { "ratioC", 0 }, { "ratioA", 2 }, { "ratioB", 3 },
                { "harm", 64 }, { "detune", 0 }, { "fdbk", 30 }, { "mix", 64 },
                { "leva", 110 }, { "atka", 0 }, { "deca", 45 }, { "enda", 30 },
                { "leve", 90 }, { "atkb", 0 }, { "decb", 40 }, { "endb", 0 },
                { "atk", 0 }, { "dec", 50 }, { "sus", 55 }, { "rel", 35 },
                { "cut", 95 }, { "reso", 30 }, { "drive", 55 }, { "trig", 0 } } },
            { "BELL", {
                { "alg", 6 }, { "ratioC", 0 }, { "ratioA", 9 }, { "ratioB", 60 },
                { "harm", 100 }, { "detune", 20 }, { "fdbk", 5 }, { "mix", 80 },
                { "leva", 70 }, { "atka", 0 }, { "deca", 80 }, { "enda", 0 },
                { "leve", 60 }, { "atkb", 0 }, { "decb", 90 }, { "endb", 0 },
                { "atk", 0 }, { "dec", 90 }, { "sus", 0 }, { "rel", 90 },
                { "cut", 120 }, { "reso", 0 }, { "drive", 0 }, { "trig", 1 } } },
            { "BRASS", {
                { "alg", 2 }, { "ratioC", 0 }, { "ratioA", 3 }, { "ratioB", 19 },
                { "harm", 70 }, { "detune", 25 }, { "fdbk", 15 }, { "mix", 70 },
                { "leva", 100 }, { "atka", 20 }, { "deca", 70 }, { "enda", 50 },
                { "leve", 90 }, { "atkb", 15 }, { "decb", 65 }, { "endb", 40 },
                { "atk", 15 }, { "dec", 70 }, { "sus", 80 }, { "rel", 50 },
                { "cut", 115 }, { "reso", 15 }, { "drive", 30 }, { "trig", 0 } } },
            { "METAL", {
                { "alg", 5 }, { "ratioC", 0 }, { "ratioA", 13 }, { "ratioB", 127 },
                { "harm", 127 }, { "detune", 50 }, { "fdbk", 90 }, { "mix", 100 },
                { "leva", 120 }, { "atka", 0 }, { "deca", 40 }, { "enda", 0 },
                { "leve", 120 }, { "atkb", 0 }, { "decb", 40 }, { "endb", 0 },
                { "atk", 0 }, { "dec", 45 }, { "sus", 20 }, { "rel", 40 },
                { "cut", 110 }, { "reso", 45 }, { "drive", 70 }, { "trig", 1 } } },
            { "PAD", {
                { "alg", 8 }, { "ratioC", 0 }, { "ratioA", 5 }, { "ratioB", 42 },
                { "harm", 80 }, { "detune", 35 }, { "fdbk", 0 }, { "mix", 60 },
                { "leva", 80 }, { "atka", 40 }, { "deca", 90 }, { "enda", 60 },
                { "leve", 80 }, { "atkb", 40 }, { "decb", 90 }, { "endb", 60 },
                { "atk", 50 }, { "dec", 100 }, { "sus", 85 }, { "rel", 100 },
                { "cut", 100 }, { "reso", 20 }, { "drive", 15 }, { "trig", 0 } } },
        };
        return p;
    }

    Fm4Panel::Panel& panel;
    Fm4Sequencer& seq;
    Fm4Capture& cap;

    sf::Font font;
    int dragging = -1, hoverPot = -1, lastClicked = -1;
    float dragY = 0.0f, dragStart = 0.0f;

    // Piano roll interaction
    int dragStepIdx = -1;
    int lastStepEdit = -1;
    bool draggingLen = false;
    int baseMidiNote = 36;       // C2 at the bottom
    int visibleSemitones = 25;   // two octaves
    int stepPage = 0;            // 0: steps 1-16, 1: steps 17-32

    float scrollY = 0.0f;
    bool draggingScroll = false;
    float scrollDragStart = 0.0f;

    sf::FloatRect editNoteRect, editLenRect, editVeloRect, editSlideRect, editAccentRect;

    sf::Clock clock, frameClock, flashClock;
    int flashPreset = -1;

    float shown[Fm4Panel::POTS];
    float peakHold = 0.0f;
    float glow[Fm4Panel::BLOCKS] = {};

    EmuUi(Fm4Panel::Panel& p, Fm4Sequencer& s, Fm4Capture& c)
        : panel(p)
        , seq(s)
        , cap(c)
    {
        const char* paths[] = {
            "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Regular.ttf",
            "../../../draw/fonts/Roboto-Regular.ttf",
            "../../draw/fonts/Roboto-Regular.ttf",
            "../draw/fonts/Roboto-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        };
        for (auto p : paths) {
            if (font.loadFromFile(p)) break;
        }
        for (int i = 0; i < Fm4Panel::POTS; ++i) {
            shown[i] = panel.norm(i);
        }
        stepPage = (seq.patternLength > 16) ? 1 : 0;
    }

    // ------------------------------------------------------------------ layout
    static float colX(int col) { return CARD_X + col * CELL_W + (col >= 4 ? GAP : 0.0f); }

    static sf::FloatRect blockRect(int b)
    {
        const Fm4Panel::Block& blk = Fm4Panel::block(b);
        float y = CARDS_Y + (blk.row >= 2 ? (CARD_H2 + STACK_GAP) : 0.0f);
        float h = HEAD_S + blk.rows * ROW_PITCH + 8.0f;
        // A block that spans the centre gap is one GAP wider than cols * CELL_W.
        float w = blk.cols * CELL_W + ((blk.col < 4 && blk.col + blk.cols > 4) ? GAP : 0.0f);
        return { colX(blk.col), y, w, h };
    }

    static float rowY(int pot)
    {
        int b = Fm4Panel::blockOf(pot);
        const Fm4Panel::Block& blk = Fm4Panel::block(b);
        return blockRect(b).top + HEAD_S + (Fm4Panel::potRow(pot) - blk.row) * ROW_PITCH;
    }

    static sf::Vector2f knobCenter(int pot)
    {
        return { colX(Fm4Panel::potCol(pot)) + CELL_W * 0.5f, rowY(pot) + 54.0f };
    }

    static sf::FloatRect knobCell(int pot)
    {
        auto ctr = knobCenter(pot);
        return { ctr.x - CELL_W * 0.5f, ctr.y - 40.0f, CELL_W, 112.0f };
    }

    static sf::FloatRect playBtn() { return { 220.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect trigBtn() { return { 286.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect bpmBox()  { return { 356.0f, 14.0f, 126.0f, 44.0f }; }
    static sf::FloatRect tuneBox() { return { 490.0f, 14.0f, 126.0f, 44.0f }; }
    static sf::FloatRect presetRect(int i) { return { 640.0f + i * 78.0f, 14.0f, 72.0f, 44.0f }; }
    static sf::FloatRect diceBtn() { return { 640.0f + 6 * 78.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect meterBox() { return { W - 24.0f - 90.0f, 14.0f, 90.0f, 44.0f }; }

    static sf::FloatRect scopeRect() { return { CARD_X, SCOPE_Y, 700.0f, SCOPE_H }; }
    static sf::FloatRect algoRect() { return { CARD_X + 712.0f, SCOPE_Y, W - 2.0f * CARD_X - 712.0f, SCOPE_H }; }

    static sf::FloatRect pianoRollRect() { return { CARD_X, ROLL_Y, W - 2.0f * CARD_X, ROLL_H }; }

    static sf::FloatRect pianoGridRect()
    {
        sf::FloatRect pr = pianoRollRect();
        return { pr.left + 54.0f, pr.top + 34.0f, pr.width - 72.0f, pr.height - 42.0f };
    }

    sf::FloatRect scrollBarRect()
    {
        sf::FloatRect pr = pianoRollRect();
        return { pr.left + pr.width - 12.0f, pr.top + 34.0f, 8.0f, pr.height - 42.0f };
    }

    sf::FloatRect scrollThumbRect()
    {
        sf::FloatRect bar = scrollBarRect();
        int totalNotes = MAX_SCROLL - MIN_SCROLL + 1;
        int scrollable = std::max(1, totalNotes - visibleSemitones);
        float thumbH = std::max(20.0f, bar.height * (float)visibleSemitones / (float)totalNotes);
        float t = (float)(MAX_SCROLL - visibleSemitones - baseMidiNote) / (float)scrollable;
        t = std::clamp(t, 0.0f, 1.0f);
        return { bar.left, bar.top + t * (bar.height - thumbH), bar.width, thumbH };
    }

    // ------------------------------------------------------------------ input
    void mousePressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);

        if (button == sf::Mouse::Left) {
            if (playBtn().contains(p)) { seq.toggle(); return; }
            if (trigBtn().contains(p)) { auditionNote.store(60); return; }
            if (bpmBox().contains(p)) { stepBpm(p.x > bpmBox().left + 63.0f ? 1.0f : -1.0f, shift); return; }
            if (tuneBox().contains(p)) { stepTune(p.x > tuneBox().left + 63.0f ? 1 : -1, shift); return; }

            for (int i = 0; i < (int)presets().size(); ++i) {
                if (presetRect(i).contains(p)) { applyPreset(i); return; }
            }
            if (diceBtn().contains(p)) { randomize(); return; }

            sf::FloatRect pr = pianoRollRect();
            sf::FloatRect page0 { pr.left + 190.0f, pr.top + 7.0f, 68.0f, 22.0f };
            sf::FloatRect page1 { pr.left + 262.0f, pr.top + 7.0f, 68.0f, 22.0f };
            if (page0.contains(p)) { stepPage = 0; seq.patternLength = 16; return; }
            if (page1.contains(p)) { stepPage = 1; seq.patternLength = 32; return; }

            for (int pot = 0; pot < Fm4Panel::POTS; ++pot) {
                if (knobCell(pot).contains(p)) {
                    dragging = pot;
                    dragY = p.y;
                    dragStart = panel.norm(pot);
                    lastClicked = pot;
                    return;
                }
            }

            if (scrollBarRect().contains(p) || scrollThumbRect().contains(p)) {
                draggingScroll = true;
                scrollDragStart = p.y;
                return;
            }

            if (lastStepEdit >= 0 && lastStepEdit < Fm4Sequencer::STEPS) {
                Fm4Step& st = seq.steps[lastStepEdit];
                if (editSlideRect.contains(p)) { st.slide = !st.slide; return; }
                if (editAccentRect.contains(p)) { st.accent = !st.accent; return; }
            }

            sf::FloatRect grid = pianoGridRect();
            if (grid.contains(p)) {
                float cellW = grid.width / 16.0f;
                float cellH = grid.height / (float)visibleSemitones;
                int col = std::clamp((int)((p.x - grid.left) / cellW), 0, 15);
                int stepIdx = stepPage * 16 + col;
                int noteFromBottom = (int)((grid.top + grid.height - p.y) / cellH);
                int clickedNote = std::clamp(baseMidiNote + noteFromBottom, 0, 127);

                int found = -1;
                for (int s = 0; s < Fm4Sequencer::STEPS; ++s) {
                    const Fm4Step& st = seq.steps[s];
                    if (!st.on || st.note != clickedNote) continue;
                    float sStart = (float)(s % 16);
                    float sEnd = sStart + st.length;
                    float clickCol = (p.x - grid.left) / cellW;
                    if (s / 16 == stepPage && clickCol >= sStart && clickCol < sEnd) {
                        found = s;
                        break;
                    }
                }

                if (found != -1) {
                    if (shift) {
                        seq.steps[found].slide = !seq.steps[found].slide;
                    } else {
                        dragStepIdx = found;
                        lastStepEdit = found;
                        float noteRightX = grid.left + (found % 16 + seq.steps[found].length) * cellW;
                        draggingLen = std::abs(p.x - noteRightX) < 14.0f;
                        if (!draggingLen) auditionNote.store(seq.steps[found].note);
                    }
                } else {
                    seq.steps[stepIdx].on = true;
                    seq.steps[stepIdx].note = (uint8_t)clickedNote;
                    seq.steps[stepIdx].length = 0.85f;
                    seq.steps[stepIdx].velocity = 0.8f;
                    dragStepIdx = stepIdx;
                    lastStepEdit = stepIdx;
                    draggingLen = false;
                    auditionNote.store(clickedNote);
                }
                return;
            }
        } else if (button == sf::Mouse::Right) {
            for (int pot = 0; pot < Fm4Panel::POTS; ++pot) {
                if (knobCell(pot).contains(p)) { panel.reset(pot); return; }
            }

            sf::FloatRect grid = pianoGridRect();
            if (grid.contains(p)) {
                float cellW = grid.width / 16.0f;
                float cellH = grid.height / (float)visibleSemitones;
                int noteFromBottom = (int)((grid.top + grid.height - p.y) / cellH);
                int clickedNote = std::clamp(baseMidiNote + noteFromBottom, 0, 127);
                for (int s = 0; s < Fm4Sequencer::STEPS; ++s) {
                    Fm4Step& st = seq.steps[s];
                    if (!st.on || st.note != clickedNote) continue;
                    float sStart = (float)(s % 16);
                    float sEnd = sStart + st.length;
                    float clickCol = (p.x - grid.left) / cellW;
                    if (s / 16 == stepPage && clickCol >= sStart && clickCol < sEnd) {
                        if (shift) st.accent = !st.accent;
                        else { st.on = false; if (lastStepEdit == s) lastStepEdit = -1; }
                        return;
                    }
                }
            }
        }
    }

    void mouseReleased(sf::Mouse::Button button = sf::Mouse::Left)
    {
        if (button == sf::Mouse::Left) {
            dragging = -1;
            dragStepIdx = -1;
            draggingLen = false;
            draggingScroll = false;
        }
    }

    void mouseMoved(sf::Vector2f p, bool shift = false)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        hoverPot = -1;
        for (int pot = 0; pot < Fm4Panel::POTS; ++pot) {
            if (knobCell(pot).contains(p)) { hoverPot = pot; break; }
        }

        if (draggingScroll) {
            int scrollable = std::max(1, (MAX_SCROLL - MIN_SCROLL + 1) - visibleSemitones);
            sf::FloatRect bar = scrollBarRect();
            float t = std::clamp((p.y - bar.top) / bar.height, 0.0f, 1.0f);
            baseMidiNote = std::clamp(MAX_SCROLL - visibleSemitones - (int)(t * scrollable), MIN_SCROLL, MAX_SCROLL - visibleSemitones);
            return;
        }

        if (dragging >= 0) {
            float speed = shift ? 0.0008f : 0.004f;
            panel.set(dragging, dragStart + (dragY - p.y) * speed);
        }

        if (dragStepIdx >= 0 && dragStepIdx < Fm4Sequencer::STEPS) {
            Fm4Step& st = seq.steps[dragStepIdx];
            sf::FloatRect grid = pianoGridRect();
            float cellW = grid.width / 16.0f;
            float cellH = grid.height / (float)visibleSemitones;

            if (draggingLen) {
                float stepLeftX = grid.left + (dragStepIdx % 16) * cellW;
                st.length = std::clamp((p.x - stepLeftX) / cellW, 0.2f, 8.0f);
            } else {
                int noteFromBottom = (int)((grid.top + grid.height - p.y) / cellH);
                int newNote = std::clamp(baseMidiNote + noteFromBottom, 0, 127);
                if (st.note != newNote) { st.note = (uint8_t)newNote; auditionNote.store(newNote); }
            }
        }
    }

    void wheel(sf::Vector2f p, float delta, bool shift = false, bool alt = false)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        for (int pot = 0; pot < Fm4Panel::POTS; ++pot) {
            if (knobCell(pot).contains(p)) {
                panel.tweak(pot, delta * (shift ? 0.005f : 0.02f));
                return;
            }
        }

        if (bpmBox().contains(p)) { stepBpm(delta > 0 ? 1.0f : -1.0f, shift); return; }
        if (tuneBox().contains(p)) { stepTune(delta > 0 ? 1 : -1, shift); return; }

        if (lastStepEdit >= 0 && lastStepEdit < Fm4Sequencer::STEPS) {
            Fm4Step& st = seq.steps[lastStepEdit];
            if (editNoteRect.contains(p)) {
                st.note = (uint8_t)std::clamp((int)st.note + (delta > 0 ? 1 : -1) * (shift ? 12 : 1), 0, 127);
                auditionNote.store(st.note);
                return;
            }
            if (editLenRect.contains(p)) {
                st.length = std::clamp(st.length + (delta > 0 ? 0.1f : -0.1f), 0.1f, 8.0f);
                return;
            }
            if (editVeloRect.contains(p)) {
                st.velocity = std::clamp(st.velocity + (delta > 0 ? 0.05f : -0.05f), 0.05f, 1.0f);
                return;
            }
        }

        sf::FloatRect grid = pianoGridRect();
        if (scrollBarRect().contains(p)) {
            baseMidiNote = std::clamp(baseMidiNote + (delta > 0 ? 1 : -1) * (shift ? 12 : 1), MIN_SCROLL, MAX_SCROLL - visibleSemitones);
            return;
        }
        if (grid.contains(p)) {
            float cellW = grid.width / 16.0f;
            float cellH = grid.height / (float)visibleSemitones;
            float mouseCol = (p.x - grid.left) / cellW;
            int noteFromBottom = (int)((grid.top + grid.height - p.y) / cellH);
            int noteUnderMouse = std::clamp(baseMidiNote + noteFromBottom, 0, 127);

            for (int s = 0; s < Fm4Sequencer::STEPS; ++s) {
                Fm4Step& st = seq.steps[s];
                if (!st.on || st.note != noteUnderMouse) continue;
                float sStart = (float)(s % 16);
                float sEnd = sStart + st.length;
                if (s / 16 == stepPage && mouseCol >= sStart && mouseCol < sEnd) {
                    lastStepEdit = s;
                    if (alt) st.slide = !st.slide;
                    else if (shift) st.length = std::clamp(st.length + (delta > 0 ? 0.1f : -0.1f), 0.1f, 8.0f);
                    else st.velocity = std::clamp(st.velocity + (delta > 0 ? 0.05f : -0.05f), 0.05f, 1.0f);
                    return;
                }
            }

            baseMidiNote = std::clamp(baseMidiNote + (delta > 0 ? 1 : -1) * (shift ? 12 : 1), MIN_SCROLL, MAX_SCROLL - visibleSemitones);
        }
    }

    void key(sf::Keyboard::Key k)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        if (k == sf::Keyboard::Space) {
            seq.toggle();
        } else if (k == sf::Keyboard::Tab) {
            stepPage = 1 - stepPage;
            seq.patternLength = (stepPage + 1) * 16;
        } else if (k == sf::Keyboard::D) {
            randomize();
        } else if (k >= sf::Keyboard::Num1 && k <= sf::Keyboard::Num6) {
            applyPreset((int)(k - sf::Keyboard::Num1));
        } else if (lastStepEdit >= 0 && lastStepEdit < Fm4Sequencer::STEPS) {
            Fm4Step& st = seq.steps[lastStepEdit];
            if (k == sf::Keyboard::Up) { st.note = (uint8_t)std::clamp((int)st.note + 1, 0, 127); auditionNote.store(st.note); }
            else if (k == sf::Keyboard::Down) { st.note = (uint8_t)std::clamp((int)st.note - 1, 0, 127); auditionNote.store(st.note); }
            else if (k == sf::Keyboard::Left) st.length = std::clamp(st.length - 0.1f, 0.1f, 8.0f);
            else if (k == sf::Keyboard::Right) st.length = std::clamp(st.length + 0.1f, 0.1f, 8.0f);
        }
    }

    void stepBpm(float d, bool fine) { seq.bpm = std::clamp(seq.bpm + (fine ? 0.1f : 1.0f) * d, 60.0f, 220.0f); }
    void stepTune(int d, bool oct)   { seq.transpose = std::clamp(seq.transpose + (oct ? 12 : 1) * d, -24, 24); }

    void applyPreset(int i)
    {
        if (i < 0 || i >= (int)presets().size()) return;
        for (int pot = 0; pot < Fm4Panel::POTS; ++pot) panel.reset(pot);
        for (const auto& kv : presets()[i].values) {
            for (int pot = 0; pot < Fm4Panel::POTS; ++pot) {
                if (!strcmp(panel.param(pot).key, kv.first)) {
                    panel.param(pot).set(kv.second);
                    break;
                }
            }
        }
        flashPreset = i;
        flashClock.restart();
    }

    void randomize()
    {
        // Keep the algorithm and the ratios musical; randomize the colour.
        for (int pot = 0; pot < Fm4Panel::POTS; ++pot) {
            if (pot == 0 || pot == 1 || pot == 2 || pot == 3 || pot == 23) continue;
            panel.set(pot, (float)rand() / (float)RAND_MAX);
        }
        flashPreset = -2;
        flashClock.restart();
    }

    // ------------------------------------------------------------------ draw
    void draw(sf::RenderTarget& target, sf::Vector2f mouse)
    {
        float dt = frameClock.restart().asSeconds();
        animate(dt);

        target.clear(Theme::bg);
        drawTopBar(target, mouse);
        drawScope(target, scopeRect());
        drawAlgo(target, algoRect());
        drawTransportBar(target);

        for (int b = 0; b < Fm4Panel::BLOCKS; ++b) drawBlock(target, b);
        for (int pot = 0; pot < Fm4Panel::POTS; ++pot) drawKnob(target, pot, Theme::blockAccent[Fm4Panel::blockOf(pot)]);
        drawSequencer(target, mouse);
        if (hoverPot >= 0 && dragging < 0) drawHelp(target, hoverPot);
    }

private:
    enum Align { LEFT, CENTER, RIGHT };
    static constexpr int MIN_SCROLL = 24;  // C1
    static constexpr int MAX_SCROLL = 84;  // C6

    static sf::Color alpha(sf::Color c, int a) { return sf::Color(c.r, c.g, c.b, (sf::Uint8)std::clamp(a, 0, 255)); }
    static sf::Color mix(sf::Color a, sf::Color b, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return sf::Color((sf::Uint8)(a.r + (b.r - a.r) * t),
                         (sf::Uint8)(a.g + (b.g - a.g) * t),
                         (sf::Uint8)(a.b + (b.b - a.b) * t),
                         (sf::Uint8)(a.a + (b.a - a.a) * t));
    }

    static sf::Color opColor(int op)
    {
        switch (op) {
        case 0: return Theme::opC;
        case 1: return Theme::opA;
        case 2: return Theme::opB1;
        default: return Theme::opB2;
        }
    }

    static const char* opName(int op)
    {
        static const char* n[4] = { "C", "A", "B1", "B2" };
        return n[op];
    }

    static std::string midiName(int midi)
    {
        static const char* names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        char buf[16];
        snprintf(buf, sizeof(buf), "%s%d", names[(midi % 12 + 12) % 12], midi / 12 - 1);
        return buf;
    }

    static bool isBlackKey(int midi)
    {
        int k = midi % 12;
        return k == 1 || k == 3 || k == 6 || k == 8 || k == 10;
    }

    void label(sf::RenderTarget& t, const std::string& s, float x, float y, int size, sf::Color c, Align a = CENTER, bool bold = false, float spacing = 1.0f)
    {
        sf::Text txt(s, font, size);
        txt.setFillColor(c);
        txt.setLetterSpacing(spacing);
        if (bold) txt.setStyle(sf::Text::Bold);
        auto b = txt.getLocalBounds();
        float ox = a == LEFT ? b.left : (a == CENTER ? b.left + b.width * 0.5f : b.left + b.width);
        txt.setOrigin(std::round(ox), std::round(b.top + b.height * 0.5f));
        txt.setPosition(std::round(x), std::round(y));
        t.draw(txt);
    }

    float textWidth(const std::string& s, int size, bool bold = false, float spacing = 1.0f)
    {
        sf::Text txt(s, font, size);
        txt.setLetterSpacing(spacing);
        if (bold) txt.setStyle(sf::Text::Bold);
        return txt.getLocalBounds().width;
    }

    void rrect(sf::RenderTarget& t, sf::FloatRect r, float rad, sf::Color fill, sf::Color line = sf::Color::Transparent)
    {
        sf::ConvexShape s;
        const int seg = 6;
        s.setPointCount(4 * (seg + 1));
        const float cx[4] = { r.left + r.width - rad, r.left + r.width - rad, r.left + rad, r.left + rad };
        const float cy[4] = { r.top + rad, r.top + r.height - rad, r.top + r.height - rad, r.top + rad };
        int k = 0;
        for (int c = 0; c < 4; ++c) {
            for (int i = 0; i <= seg; ++i) {
                float ang = ((float)c * 90.0f + (float)i * 90.0f / seg) * 3.14159265f / 180.0f - 1.5707963f;
                s.setPoint(k++, { cx[c] + rad * std::cos(ang), cy[c] + rad * std::sin(ang) });
            }
        }
        s.setFillColor(fill);
        if (line.a) {
            s.setOutlineColor(line);
            s.setOutlineThickness(-1.0f);
        }
        t.draw(s);
    }

    void disc(sf::RenderTarget& t, float x, float y, float r, sf::Color c)
    {
        sf::CircleShape s(r, 32);
        s.setOrigin(r, r);
        s.setPosition(x, y);
        s.setFillColor(c);
        t.draw(s);
    }

    void hline(sf::RenderTarget& t, float x, float y, float w, sf::Color c, float th = 1.0f)
    {
        sf::RectangleShape r({ w, th });
        r.setPosition(x, y);
        r.setFillColor(c);
        t.draw(r);
    }

    void vline(sf::RenderTarget& t, float x, float y, float h, sf::Color c, float th = 1.0f)
    {
        sf::RectangleShape r({ th, h });
        r.setPosition(x, y);
        r.setFillColor(c);
        t.draw(r);
    }

    void arc(sf::RenderTarget& t, float cx, float cy, float r, float thick, float a0, float a1, sf::Color c, bool caps = true)
    {
        if (a1 < a0) std::swap(a0, a1);
        if (a1 - a0 < 0.01f) return;
        int n = std::max(2, (int)((a1 - a0) / 4.0f) + 1);
        sf::VertexArray va(sf::TriangleStrip, 2 * (n + 1));
        for (int i = 0; i <= n; ++i) {
            float a = (a0 + (a1 - a0) * i / n) * 3.14159265f / 180.0f;
            float s = std::sin(a), co = std::cos(a);
            va[2 * i] = sf::Vertex({ cx + (r + thick * 0.5f) * s, cy - (r + thick * 0.5f) * co }, c);
            va[2 * i + 1] = sf::Vertex({ cx + (r - thick * 0.5f) * s, cy - (r - thick * 0.5f) * co }, c);
        }
        t.draw(va);
        if (caps) {
            for (float a : { a0, a1 }) {
                float rad = a * 3.14159265f / 180.0f;
                disc(t, cx + r * std::sin(rad), cy - r * std::cos(rad), thick * 0.5f, c);
            }
        }
    }

    void arrow(sf::RenderTarget& t, float x1, float y1, float x2, float y2, sf::Color c, float th = 2.0f)
    {
        float dx = x2 - x1, dy = y2 - y1;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 2.0f) return;
        float ang = std::atan2(dy, dx);
        sf::RectangleShape line({ len, th });
        line.setOrigin(0.0f, th * 0.5f);
        line.setPosition(x1, y1);
        line.setRotation(ang * 180.0f / 3.14159265f);
        line.setFillColor(c);
        t.draw(line);

        sf::ConvexShape head(3);
        head.setPoint(0, { x2, y2 });
        head.setPoint(1, { x2 - 9.0f * std::cos(ang - 0.42f), y2 - 9.0f * std::sin(ang - 0.42f) });
        head.setPoint(2, { x2 - 9.0f * std::cos(ang + 0.42f), y2 - 9.0f * std::sin(ang + 0.42f) });
        head.setFillColor(c);
        t.draw(head);
    }

    void animate(float dt)
    {
        float k = 1.0f - std::exp(-dt * 30.0f);
        for (int i = 0; i < Fm4Panel::POTS; ++i) shown[i] += (panel.norm(i) - shown[i]) * k;
        cap.livePeak += (std::clamp(cap.peak, 0.0f, 1.0f) - cap.livePeak) * std::min(1.0f, dt * 5.0f);
        peakHold = std::max(peakHold, cap.peak);
        cap.peak = 0.0f;

        const Fm4& e = panel.engine;
        float fmAct = 0.0f;
        for (int j = 0; j < Fm4::NUM_OPS; ++j) fmAct += std::clamp(e.liveOp[j], 0.0f, 1.0f);
        fmAct = std::clamp(fmAct * 0.5f, 0.0f, 1.0f);
        glow[0] = std::max(fmAct, glow[0] - dt * 3.0f);
        glow[1] = std::max(std::clamp(e.liveEnvA + e.liveEnvB, 0.0f, 1.0f) * 0.6f, glow[1] - dt * 3.0f);
        glow[2] = std::max(std::clamp(e.liveAmp, 0.0f, 1.0f) * 0.6f, glow[2] - dt * 3.0f);
    }

    // ------------------------------------------------------------------ top bar
    void drawTopBar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "zicStudio", CARD_X, 30.0f, 23, Theme::text, LEFT, true, 0.5f);
        label(t, "Fm4", CARD_X + 116.0f, 30.0f, 23, Theme::fmNeon, LEFT, true, 0.5f);
        label(t, "4-OPERATOR FM SYNTHESIZER", CARD_X, 52.0f, 9, Theme::muted, LEFT, false, 2.2f);

        // Transport
        rrect(t, playBtn(), 12.0f, seq.playing ? Theme::fmNeon : (playBtn().contains(mouse) ? sf::Color(40, 43, 54) : Theme::card), seq.playing ? Theme::fmNeon : Theme::cardLine);
        label(t, seq.playing ? "STOP" : "PLAY", playBtn().left + playBtn().width * 0.5f, playBtn().top + 22.0f, 11, seq.playing ? sf::Color(14, 14, 18) : Theme::text, CENTER, true);

        rrect(t, trigBtn(), 12.0f, trigBtn().contains(mouse) ? sf::Color(40, 43, 54) : Theme::card, Theme::cardLine);
        label(t, "TRIG", trigBtn().left + trigBtn().width * 0.5f, trigBtn().top + 22.0f, 11, Theme::text, CENTER, true);

        char buf[32];
        snprintf(buf, sizeof(buf), "%d", (int)std::round(seq.bpm));
        rrect(t, bpmBox(), 12.0f, Theme::card, Theme::cardLine);
        label(t, buf, bpmBox().left + 48.0f, bpmBox().top + 22.0f, 20, Theme::text, CENTER, true);
        label(t, "BPM", bpmBox().left + 74.0f, bpmBox().top + 25.0f, 9, Theme::muted, LEFT, false, 1.5f);
        label(t, "-", bpmBox().left + 14.0f, bpmBox().top + 21.0f, 20, Theme::muted);
        label(t, "+", bpmBox().left + bpmBox().width - 14.0f, bpmBox().top + 21.0f, 20, Theme::muted);

        snprintf(buf, sizeof(buf), "%+d", seq.transpose);
        rrect(t, tuneBox(), 12.0f, Theme::card, Theme::cardLine);
        label(t, buf, tuneBox().left + 48.0f, tuneBox().top + 22.0f, 20, Theme::text, CENTER, true);
        label(t, "TUNE", tuneBox().left + 74.0f, tuneBox().top + 25.0f, 9, Theme::muted, LEFT, false, 1.5f);
        label(t, "-", tuneBox().left + 14.0f, tuneBox().top + 21.0f, 20, Theme::muted);
        label(t, "+", tuneBox().left + tuneBox().width - 14.0f, tuneBox().top + 21.0f, 20, Theme::muted);

        // Presets + dice
        float flash = flashClock.getElapsedTime().asSeconds();
        for (int i = 0; i < (int)presets().size(); ++i) {
            auto r = presetRect(i);
            bool hot = r.contains(mouse);
            bool lit = flashPreset == i && flash < 0.6f;
            sf::Color acc = Theme::blockAccent[i % 3];
            rrect(t, r, 12.0f, lit ? alpha(acc, 200) : (hot ? sf::Color(40, 43, 54) : Theme::card), lit ? acc : Theme::cardLine);
            label(t, presets()[i].name, r.left + r.width * 0.5f, r.top + 22.0f, 9, lit ? sf::Color(14, 14, 18) : (hot ? Theme::text : Theme::muted), CENTER, true, 1.2f);
            label(t, std::to_string(i + 1), r.left + 7.0f, r.top + 8.0f, 7, Theme::faint, LEFT);
        }

        auto d = diceBtn();
        bool lit = flashPreset == -2 && flash < 0.6f;
        rrect(t, d, 12.0f, lit ? Theme::fmNeon : (d.contains(mouse) ? sf::Color(40, 43, 54) : Theme::card), Theme::cardLine);
        label(t, "DICE", d.left + d.width * 0.5f, d.top + 22.0f, 10, lit ? sf::Color(14, 14, 18) : Theme::fmNeon, CENTER, true);

        // Output meter
        auto m = meterBox();
        rrect(t, m, 12.0f, Theme::card, Theme::cardLine);
        label(t, "OUT", m.left + 10.0f, m.top + 22.0f, 8, Theme::muted, LEFT, false, 2.0f);
        float lvl = std::clamp(cap.livePeak * 1.4f, 0.0f, 1.0f);
        sf::FloatRect bar { m.left + 36.0f, m.top + 17.0f, m.width - 46.0f, 10.0f };
        rrect(t, bar, 5.0f, Theme::cell);
        if (lvl > 0.01f) rrect(t, { bar.left, bar.top, bar.width * lvl, bar.height }, 5.0f, lvl > 0.9f ? Theme::opB2 : Theme::fmNeon);
    }

    void drawTransportBar(sf::RenderTarget& t)
    {
        // A slim readout under the top bar: the current algorithm + operator activity.
        char buf[64];
        int alg = (int)std::lround(panel.param(0).value);
        snprintf(buf, sizeof(buf), "ALG %d  /  4 OPERATORS  /  MONO VOICE", alg + 1);
        label(t, buf, CARD_X, 74.0f, 8, Theme::faint, LEFT, false, 1.6f);
    }

    void drawScope(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 14.0f, Theme::card, Theme::cardLine);
        float mid = r.top + r.height * 0.5f;
        hline(t, r.left + 12.0f, mid, r.width - 24.0f, sf::Color(255, 255, 255, 15));

        const int N = 300;
        sf::VertexArray va(sf::LineStrip, N);
        int pos = cap.ringPos;
        for (int i = 0; i < N; ++i) {
            int k = (pos + i * (Fm4Capture::RING / N)) % Fm4Capture::RING;
            float val = std::clamp(cap.ring[k], -1.0f, 1.0f);
            float x = r.left + 14.0f + (r.width - 28.0f) * (float)i / (float)(N - 1);
            float y = mid - val * (r.height * 0.44f);
            va[i] = sf::Vertex({ x, y }, Theme::fmNeon);
        }
        t.draw(va);

        const Fm4& e = panel.engine;
        char buf[96];
        snprintf(buf, sizeof(buf), "ENV A %.2f   ENV B %.2f   AMP %.2f", e.liveEnvA, e.liveEnvB, e.liveAmp);
        label(t, buf, r.left + 16.0f, r.top + 14.0f, 8, Theme::faint, LEFT, false, 1.2f);
        label(t, "MONO FM OUTPUT", r.left + r.width - 16.0f, r.top + 14.0f, 8, Theme::faint, RIGHT, false, 1.2f);
    }

    // ------------------------------------------------------------------ algorithm visualizer
    static constexpr float ALG_POS[8][4][2] = {
        // order: C, A, B1, B2
        { { 0.28f, 0.80f }, { 0.28f, 0.18f }, { 0.72f, 0.55f }, { 0.72f, 0.18f } }, // 1: A>C, B2>B1, A>B1
        { { 0.28f, 0.80f }, { 0.28f, 0.18f }, { 0.72f, 0.80f }, { 0.72f, 0.18f } }, // 2: A>C, B2>B1
        { { 0.20f, 0.82f }, { 0.20f, 0.18f }, { 0.80f, 0.58f }, { 0.80f, 0.18f } }, // 3: A>C, A>B2>B1
        { { 0.78f, 0.84f }, { 0.26f, 0.62f }, { 0.74f, 0.38f }, { 0.24f, 0.15f } }, // 4: B2>B1>A>C
        { { 0.74f, 0.82f }, { 0.26f, 0.18f }, { 0.26f, 0.82f }, { 0.74f, 0.18f } }, // 5: A>B1, B2>C
        { { 0.18f, 0.82f }, { 0.18f, 0.15f }, { 0.80f, 0.66f }, { 0.50f, 0.40f } }, // 6: A>B2>B1
        { { 0.28f, 0.68f }, { 0.28f, 0.18f }, { 0.72f, 0.30f }, { 0.72f, 0.72f } }, // 7: A>C, B1 & B2 alone
        { { 0.28f, 0.66f }, { 0.28f, 0.24f }, { 0.72f, 0.24f }, { 0.72f, 0.66f } }, // 8: all parallel
    };

    void drawAlgo(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 14.0f, Theme::card, Theme::cardLine);

        int alg = std::clamp((int)std::lround(panel.param(0).value), 0, 7);
        char buf[48];
        snprintf(buf, sizeof(buf), "ALGORITHM %d / 8", alg + 1);
        label(t, buf, r.left + 16.0f, r.top + 17.0f, 11, Theme::blockAccent[0], LEFT, true, 2.4f);

        // Routing summary
        std::string desc;
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                if ((Fm4::ALG_MOD[alg][j] >> k) & 1u) {
                    if (!desc.empty()) desc += "   ";
                    desc += opName(k);
                    desc += " > ";
                    desc += opName(j);
                }
            }
        }
        if (desc.empty()) desc = "all four in parallel";
        label(t, desc, r.left + 16.0f, r.top + 33.0f, 8, Theme::muted, LEFT, false, 1.0f);

        sf::FloatRect area { r.left + 24.0f, r.top + 46.0f, r.width - 48.0f, r.height - 80.0f };
        const float R = 16.0f;

        auto nodeXY = [&](int op) {
            return sf::Vector2f(area.left + ALG_POS[alg][op][0] * area.width,
                                area.top + ALG_POS[alg][op][1] * area.height);
        };

        const Fm4& e = panel.engine;

        // Arrows first (so the nodes sit on top)
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                if (!((Fm4::ALG_MOD[alg][j] >> k) & 1u)) continue;
                sf::Vector2f a = nodeXY(k), b = nodeXY(j);
                float dx = b.x - a.x, dy = b.y - a.y;
                float len = std::max(1.0f, std::sqrt(dx * dx + dy * dy));
                dx /= len;
                dy /= len;
                float act = std::clamp(std::sqrt(std::max(0.0f, e.liveOp[k])) * 1.2f, 0.0f, 1.0f);
                sf::Color c = mix(alpha(opColor(k), 110), opColor(k), act);
                arrow(t, a.x + dx * (R + 2.0f), a.y + dy * (R + 2.0f),
                      b.x - dx * (R + 4.0f), b.y - dy * (R + 4.0f), c, 2.0f);
            }
        }

        // Nodes
        for (int op = 0; op < 4; ++op) {
            sf::Vector2f p = nodeXY(op);
            float act = std::clamp(std::sqrt(std::max(0.0f, e.liveOp[op])) * 1.4f, 0.0f, 1.0f);
            sf::Color c = opColor(op);

            if (act > 0.02f) disc(t, p.x, p.y, R + 6.0f * act, alpha(c, (int)(55 * act)));

            bool xCar = ((Fm4::ALG_X[alg] >> op) & 1u) != 0;
            bool yCar = ((Fm4::ALG_Y[alg] >> op) & 1u) != 0;

            disc(t, p.x, p.y, R, mix(Theme::cap, c, 0.18f + 0.55f * act));
            sf::CircleShape ring(R, 32);
            ring.setOrigin(R, R);
            ring.setPosition(p.x, p.y);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(alpha(c, 150 + (int)(105 * act)));
            ring.setOutlineThickness(2.0f);
            t.draw(ring);

            label(t, opName(op), p.x, p.y + 1.0f, op == 0 || op == 1 ? 13 : 10, Theme::text, CENTER, true, 0.4f);

            // Carrier tag
            std::string tag;
            if (xCar) tag += "X";
            if (yCar) tag += "Y";
            if (!tag.empty()) {
                float tw = textWidth(tag, 7, true, 1.0f) + 8.0f;
                float tx = p.x + R + 3.0f;
                rrect(t, { tx, p.y - 5.0f, tw, 10.0f }, 4.0f, alpha(c, 220));
                label(t, tag, tx + tw * 0.5f, p.y, 7, sf::Color(12, 12, 16), CENTER, true, 1.0f);
            }
        }

        // Mix bar
        float mixN = panel.norm(7);
        sf::FloatRect mb { r.left + 44.0f, r.top + r.height - 22.0f, r.width - 88.0f, 4.0f };
        label(t, "X", mb.left - 12.0f, mb.top + 2.0f, 8, Theme::muted, CENTER, true);
        label(t, "Y", mb.left + mb.width + 12.0f, mb.top + 2.0f, 8, Theme::muted, CENTER, true);
        rrect(t, mb, 2.0f, Theme::track);
        rrect(t, { mb.left, mb.top, mb.width * mixN, mb.height }, 2.0f, Theme::blockAccent[0]);
        disc(t, mb.left + mb.width * mixN, mb.top + 2.0f, 3.5f, Theme::text);
        char mbuf[32];
        snprintf(mbuf, sizeof(mbuf), "MIX %d%%", (int)std::lround(mixN * 100.0f));
        label(t, mbuf, r.left + r.width - 16.0f, r.top + 17.0f, 8, Theme::faint, RIGHT, false, 1.2f);
    }

    // ------------------------------------------------------------------ knobs
    void drawBlock(sf::RenderTarget& t, int b)
    {
        auto r = blockRect(b);
        const Fm4Panel::Block& blk = Fm4Panel::block(b);
        sf::Color accent = Theme::blockAccent[b];
        float g = std::clamp(glow[b], 0.0f, 1.0f);

        rrect(t, r, 18.0f, Theme::card, mix(Theme::cardLine, accent, 0.55f + 0.45f * g));
        rrect(t, { r.left + 18.0f, r.top, r.width - 36.0f, 3.0f }, 1.5f, alpha(accent, 120 + (int)(135 * g)));
        if (g > 0.02f) rrect(t, { r.left + 18.0f, r.top - 2.0f, r.width - 36.0f, 7.0f }, 3.5f, alpha(accent, (int)(55 * g)));

        label(t, blk.name, r.left + 18.0f, r.top + 18.0f, 13, accent, LEFT, true, 3.0f);
        label(t, blk.subtitle, r.left + 18.0f, r.top + 33.0f, 9, Theme::muted, LEFT, false, 1.0f);

        for (int gi = 0; gi < Fm4Panel::GROUPS; ++gi) {
            const Fm4Panel::Group& grp = Fm4Panel::group(gi);
            if (Fm4Panel::blockOf(grp.firstPot) != b) continue;
            float x = colX(Fm4Panel::potCol(grp.firstPot));
            float y = rowY(grp.firstPot);
            sf::FloatRect pill { x + 6.0f, y + 6.0f, grp.count * CELL_W - 12.0f, ROW_PITCH - 16.0f };
            rrect(t, pill, 14.0f, sf::Color(255, 255, 255, 4), alpha(accent, 40));
            float tw = textWidth(grp.name, 8, true, 2.0f);
            rrect(t, { pill.left + 14.0f, pill.top - 7.0f, tw + 14.0f, 14.0f }, 6.0f, Theme::card);
            label(t, grp.name, pill.left + 21.0f, pill.top, 8, alpha(accent, 235), LEFT, true, 2.0f);
        }
    }

    void drawKnob(sf::RenderTarget& t, int pot, sf::Color accent)
    {
        auto ctr = knobCenter(pot);
        bool active = dragging == pot;
        bool hot = active || hoverPot == pot;
        float n = shown[pot];
        const float R = 27.0f;
        auto angle = [](float v) { return -135.0f + 270.0f * v; };

        disc(t, ctr.x, ctr.y, 36.0f, sf::Color(20, 22, 28));
        arc(t, ctr.x, ctr.y, R, 5.0f, -135.0f, 135.0f, Theme::track);

        // Algorithm knob: eight detent ticks
        if (pot == 0) {
            for (int s = 0; s < 8; ++s) {
                float a = angle((float)s / 7.0f) * 3.14159265f / 180.0f;
                float rr = (s <= (int)std::lround(n * 7.0f)) ? R + 5.0f : R + 5.0f;
                disc(t, ctr.x + rr * std::sin(a), ctr.y - rr * std::cos(a), 1.5f,
                     s == (int)std::lround(n * 7.0f) ? accent : Theme::faint);
            }
        } else {
            float a = angle(panel.defaultNorm(pot)) * 3.14159265f / 180.0f;
            disc(t, ctr.x + (R + 7.5f) * std::sin(a), ctr.y - (R + 7.5f) * std::cos(a), 1.5f, Theme::faint);
        }

        float from = panel.isCentered(pot) ? 0.0f : -135.0f;
        float to = angle(n);
        if (std::abs(to - from) > 0.5f) {
            arc(t, ctr.x, ctr.y, R, active ? 13.0f : 10.0f, from, to, sf::Color(accent.r, accent.g, accent.b, active ? 80 : 36), false);
            arc(t, ctr.x, ctr.y, R, 5.0f, from, to, accent);
        }

        disc(t, ctr.x, ctr.y, 19.0f, hot ? sf::Color(52, 56, 70) : Theme::cardLine);
        disc(t, ctr.x, ctr.y, 18.0f, Theme::cap);

        float a = angle(n) * 3.14159265f / 180.0f;
        sf::VertexArray ind(sf::Lines, 2);
        ind[0] = sf::Vertex({ ctr.x + 7.0f * std::sin(a), ctr.y - 7.0f * std::cos(a) }, Theme::text);
        ind[1] = sf::Vertex({ ctr.x + 16.0f * std::sin(a), ctr.y - 16.0f * std::cos(a) }, Theme::text);
        t.draw(ind);
        disc(t, ctr.x + 16.0f * std::sin(a), ctr.y - 16.0f * std::cos(a), 1.6f, hot ? accent : Theme::text);

        char buf[32];
        panel.formatValue(pot, buf, sizeof(buf));
        label(t, panel.param(pot).label, ctr.x, ctr.y + 43.0f, 12, hot ? Theme::text : sf::Color(176, 181, 196), CENTER, false, 0.6f);
        label(t, buf, ctr.x, ctr.y + 57.0f, 11, hot ? accent : Theme::muted, CENTER, hot, 0.4f);
    }

    // ------------------------------------------------------------------ sequencer
    void drawSequencer(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        (void)mouse;
        sf::FloatRect pr = pianoRollRect();
        rrect(t, pr, 16.0f, Theme::card, Theme::cardLine);

        label(t, "FM STEP SEQUENCER", pr.left + 18.0f, pr.top + 18.0f, 12, Theme::fmNeon, LEFT, true, 2.5f);

        sf::FloatRect page0 { pr.left + 190.0f, pr.top + 7.0f, 68.0f, 22.0f };
        sf::FloatRect page1 { pr.left + 262.0f, pr.top + 7.0f, 68.0f, 22.0f };
        rrect(t, page0, 6.0f, stepPage == 0 ? Theme::fmNeon : Theme::cell, Theme::cardLine);
        label(t, "1 - 16", page0.left + 34.0f, page0.top + 11.0f, 9, stepPage == 0 ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);
        rrect(t, page1, 6.0f, stepPage == 1 ? Theme::fmNeon : Theme::cell, Theme::cardLine);
        label(t, "17 - 32", page1.left + 34.0f, page1.top + 11.0f, 9, stepPage == 1 ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);

        if (lastStepEdit >= 0 && lastStepEdit < Fm4Sequencer::STEPS) {
            const Fm4Step& s = seq.steps[lastStepEdit];
            char buf[80];
            snprintf(buf, sizeof(buf), "STEP %d: %s  LEN %.1f  VEL %d%%",
                     lastStepEdit + 1, midiName(s.note).c_str(), s.length, (int)(s.velocity * 100.0f));
            float barX = pr.left + 350.0f;
            label(t, buf, barX, pr.top + 18.0f, 10, Theme::text, LEFT, true);

            editSlideRect = { barX + 270.0f, pr.top + 7.0f, 54.0f, 22.0f };
            rrect(t, editSlideRect, 6.0f, s.slide ? Theme::opC : Theme::cell, Theme::cardLine);
            label(t, "SLIDE", editSlideRect.left + 27.0f, editSlideRect.top + 11.0f, 8, s.slide ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);

            editAccentRect = { barX + 330.0f, pr.top + 7.0f, 58.0f, 22.0f };
            rrect(t, editAccentRect, 6.0f, s.accent ? Theme::opB2 : Theme::cell, Theme::cardLine);
            label(t, "ACCENT", editAccentRect.left + 29.0f, editAccentRect.top + 11.0f, 8, s.accent ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);
        } else {
            label(t, "Click to place  /  Drag pitch & length  /  Wheel velocity  /  Shift slide  /  Right-click delete",
                  pr.left + 350.0f, pr.top + 18.0f, 9, Theme::faint, LEFT, false, 0.6f);
        }

        label(t, "TAB page  /  SPACE play", pr.left + pr.width - 16.0f, pr.top + 18.0f, 9, Theme::faint, RIGHT, false, 0.6f);

        sf::FloatRect grid = pianoGridRect();
        rrect(t, grid, 8.0f, sf::Color(13, 14, 18), Theme::cardLine);

        float cellW = grid.width / 16.0f;
        float cellH = grid.height / (float)visibleSemitones;

        for (int row = 0; row < visibleSemitones; ++row) {
            int midi = baseMidiNote + (visibleSemitones - 1 - row);
            float y = grid.top + row * cellH;
            bool black = isBlackKey(midi);
            bool isC = (midi % 12) == 0;

            if (black) {
                sf::RectangleShape bgRow({ grid.width, cellH });
                bgRow.setPosition(grid.left, y);
                bgRow.setFillColor(sf::Color(18, 19, 25));
                t.draw(bgRow);
            }
            hline(t, grid.left, y + cellH, grid.width, isC ? sf::Color(255, 255, 255, 25) : sf::Color(255, 255, 255, 8));
            label(t, midiName(midi), grid.left - 6.0f, y + cellH * 0.5f, isC ? 9 : 8, isC ? Theme::fmNeon : (black ? Theme::faint : Theme::muted), RIGHT, isC);
        }

        int curPlayStep = seq.currentStep.load();
        for (int col = 0; col < 16; ++col) {
            float x = grid.left + col * cellW;
            bool downbeat = (col % 4) == 0;
            vline(t, x, grid.top, grid.height, downbeat ? sf::Color(255, 255, 255, 25) : sf::Color(255, 255, 255, 8));
            if (stepPage * 16 + col == curPlayStep) {
                sf::RectangleShape playCol({ cellW, grid.height });
                playCol.setPosition(x, grid.top);
                playCol.setFillColor(sf::Color(230, 255, 40, 30));
                t.draw(playCol);
            }
        }

        int startStep = stepPage * 16;
        for (int s = 0; s < Fm4Sequencer::STEPS; ++s) {
            const Fm4Step& step = seq.steps[s];
            if (!step.on) continue;

            float stepCol = (float)(s - startStep);
            if (stepCol + step.length <= 0.0f || stepCol >= 16.0f) continue;
            int noteOffset = step.note - baseMidiNote;
            if (noteOffset < 0 || noteOffset >= visibleSemitones) continue;

            float row = (float)(visibleSemitones - 1 - noteOffset);
            float nX = grid.left + stepCol * cellW + 1.0f;
            float nY = grid.top + row * cellH + 1.0f;
            float nW = std::max(6.0f, step.length * cellW - 2.0f);
            float totalH = std::max(4.0f, cellH - 2.0f);

            if (nX < grid.left) { nW -= (grid.left - nX); nX = grid.left; }
            if (nX + nW > grid.left + grid.width) nW = (grid.left + grid.width) - nX;

            sf::Color c = Theme::fmNeon;
            if (step.accent) c = Theme::opB2;
            else if (step.slide) c = Theme::opC;

            rrect(t, { nX, nY, nW, totalH }, 3.0f, alpha(c, 70));
            float veloH = std::clamp(totalH * step.velocity, 2.0f, totalH);
            rrect(t, { nX, nY + (totalH - veloH), nW, veloH }, 3.0f, c);
            bool selected = lastStepEdit == s;
            rrect(t, { nX, nY, nW, totalH }, 3.0f, sf::Color::Transparent, selected ? sf::Color::White : alpha(c, 180));
            if (step.slide && nW > 18.0f) label(t, ">", nX + nW - 8.0f, nY + totalH * 0.5f, 9, sf::Color(14, 14, 18), CENTER, true);
        }

        // Scrollbar
        sf::FloatRect bar = scrollBarRect();
        sf::FloatRect thumb = scrollThumbRect();
        rrect(t, bar, 4.0f, sf::Color(20, 22, 28), Theme::cardLine);
        rrect(t, thumb, 4.0f, Theme::fmNeon, Theme::fmNeon);
    }

    void drawHelp(sf::RenderTarget& t, int pot)
    {
        auto ctr = knobCenter(pot);
        std::string text = Fm4Panel::help(pot);
        sf::Color accent = Theme::blockAccent[Fm4Panel::blockOf(pot)];

        std::vector<std::string> lines;
        std::string cur;
        size_t start = 0;
        while (start < text.size()) {
            size_t sp = text.find(' ', start);
            std::string word = text.substr(start, sp == std::string::npos ? std::string::npos : sp - start);
            if (cur.size() + word.size() > 52 && !cur.empty()) { lines.push_back(cur); cur.clear(); }
            cur += (cur.empty() ? "" : " ") + word;
            if (sp == std::string::npos) break;
            start = sp + 1;
        }
        if (!cur.empty()) lines.push_back(cur);

        float w = 0;
        for (auto& l : lines) w = std::max(w, textWidth(l, 11));
        w += 28.0f;
        float h = 16.0f * lines.size() + 14.0f;
        float x = std::clamp(ctr.x - w * 0.5f, 10.0f, W - w - 10.0f);
        float y = ctr.y + 70.0f;
        if (y + h > H - 4.0f) y = ctr.y - 48.0f - h;

        rrect(t, { x, y, w, h }, 8.0f, sf::Color(28, 30, 40, 245), alpha(accent, 140));
        for (size_t i = 0; i < lines.size(); ++i) {
            label(t, lines[i], x + 14.0f, y + 14.0f + 16.0f * i, 11, Theme::text, LEFT);
        }
    }
};
