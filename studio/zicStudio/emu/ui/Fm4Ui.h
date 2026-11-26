#pragma once


#include <SFML/Graphics.hpp>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "studio/zicStudioFm4/emu/Fm4Sequencer.h"
#include "studio/zicStudioFm4/core/Fm4Panel.h"

extern std::mutex audioMutex;
extern std::atomic<int> auditionNote;

namespace fm4_ui {

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

// Block accents: FM, ENVS, AMP, FILTER, LFO, FX
const sf::Color blockAccent[6] = {
    rgb(172, 128, 255),   // OPERATOR FM
    rgb(255, 176, 64),    // OPERATOR ENVS
    rgb(160, 235, 90),    // AMP
    rgb(56, 212, 255),    // FILTER
    rgb(255, 110, 200),   // LFO
    rgb(96, 148, 255),    // FX
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
    bool showTopBar = true;
    bool showSequencer = true;
    static constexpr float W = 1160.0f;
    static constexpr float H = 1180.0f;
    static constexpr float PANEL_TOP = 58.0f;
    static constexpr float PANEL_BOTTOM = 938.0f;
    static constexpr float CARD_X = 20.0f, CELL_W = 135.5f, GAP = 36.0f, COL_GAP = 14.0f;

    static constexpr float CARDS_Y = 256.0f, ROW_PITCH = 118.0f, HEAD_S = 50.0f;
    static constexpr float CARD_H2 = HEAD_S + 2.0f * ROW_PITCH + 8.0f; // 294: 2-row card
    static constexpr float CARD_H1 = HEAD_S + ROW_PITCH + 8.0f;        // 176: 1-row card
    static constexpr float STACK_GAP = 12.0f;

    static constexpr float SCOPE_Y = 66.0f, SCOPE_H = 180.0f;
    static constexpr float ROLL_Y = 938.0f;
    static constexpr float ROLL_H = H - ROLL_Y - 14.0f;

    // Four pages of eight knobs (2 rows x 4). A wide window lays the pages out
    // side by side; a narrow one folds them into a 2 x 2 grid.
    static constexpr int PAGES = 4;
    static constexpr int PAGE_POTS = 8;
    static constexpr int PAGE_COLS = 4;
    static constexpr int PAGE_ROWS = 2;
    static constexpr float W_CELL = 96.0f, W_GAP = 32.0f;

    struct Page {
        const char* name;
        const char* subtitle;
        int pots[PAGE_POTS];
        int accent;
    };
    static const Page& page(int p)
    {
        static const Page pages[PAGES] = {
            { "OPERATOR FM",   "algorithm, ratios, harmonics, detune", { 0, 1, 2, 3, 4, 5, 6, 7 },          0 },
            { "AMP & LFO",     "amp envelope, LFO rate, depth, shape",  { 16, 17, 18, 19, 24, 25, 26, 27 },  2 },
            { "OPERATOR ENVS", "the A and B operator envelopes",        { 8, 9, 10, 11, 12, 13, 14, 15 },    1 },
            { "FILTER & FX",   "low-pass, drive, delay and reverb",     { 20, 21, 22, 23, 28, 29, 30, 31 },  3 },
        };
        return pages[p];
    }

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
                { "cut", 110 }, { "reso", 10 }, { "drive", 20 }, { "trig", 0 },
                { "lfoRate", 64 }, { "lfoDepth", 0 }, { "lfoShape", 0 }, { "lfoDest", 0 },
                { "dlySend", 18 }, { "revSend", 34 }, { "distAmt", 0 }, { "crushAmt", 0 } } },
            { "FM BASS", {
                { "alg", 4 }, { "ratioC", 0 }, { "ratioA", 2 }, { "ratioB", 3 },
                { "harm", 64 }, { "detune", 0 }, { "fdbk", 30 }, { "mix", 64 },
                { "leva", 110 }, { "atka", 0 }, { "deca", 45 }, { "enda", 30 },
                { "leve", 90 }, { "atkb", 0 }, { "decb", 40 }, { "endb", 0 },
                { "atk", 0 }, { "dec", 50 }, { "sus", 55 }, { "rel", 35 },
                { "cut", 95 }, { "reso", 30 }, { "drive", 55 }, { "trig", 0 },
                { "lfoRate", 64 }, { "lfoDepth", 0 }, { "lfoShape", 0 }, { "lfoDest", 0 },
                { "dlySend", 10 }, { "revSend", 6 }, { "distAmt", 0 }, { "crushAmt", 0 } } },
            { "BELL", {
                { "alg", 6 }, { "ratioC", 0 }, { "ratioA", 9 }, { "ratioB", 60 },
                { "harm", 100 }, { "detune", 20 }, { "fdbk", 5 }, { "mix", 80 },
                { "leva", 70 }, { "atka", 0 }, { "deca", 80 }, { "enda", 0 },
                { "leve", 60 }, { "atkb", 0 }, { "decb", 90 }, { "endb", 0 },
                { "atk", 0 }, { "dec", 90 }, { "sus", 0 }, { "rel", 90 },
                { "cut", 120 }, { "reso", 0 }, { "drive", 0 }, { "trig", 1 },
                { "lfoRate", 64 }, { "lfoDepth", 0 }, { "lfoShape", 0 }, { "lfoDest", 0 },
                { "dlySend", 24 }, { "revSend", 58 }, { "distAmt", 0 }, { "crushAmt", 0 } } },
            { "BRASS", {
                { "alg", 2 }, { "ratioC", 0 }, { "ratioA", 3 }, { "ratioB", 19 },
                { "harm", 70 }, { "detune", 25 }, { "fdbk", 15 }, { "mix", 70 },
                { "leva", 100 }, { "atka", 20 }, { "deca", 70 }, { "enda", 50 },
                { "leve", 90 }, { "atkb", 15 }, { "decb", 65 }, { "endb", 40 },
                { "atk", 15 }, { "dec", 70 }, { "sus", 80 }, { "rel", 50 },
                { "cut", 115 }, { "reso", 15 }, { "drive", 30 }, { "trig", 0 },
                { "lfoRate", 72 }, { "lfoDepth", 12 }, { "lfoShape", 0 }, { "lfoDest", 0 },
                { "dlySend", 12 }, { "revSend", 28 }, { "distAmt", 0 }, { "crushAmt", 0 } } },
            { "METAL", {
                { "alg", 5 }, { "ratioC", 0 }, { "ratioA", 13 }, { "ratioB", 127 },
                { "harm", 127 }, { "detune", 50 }, { "fdbk", 90 }, { "mix", 100 },
                { "leva", 120 }, { "atka", 0 }, { "deca", 40 }, { "enda", 0 },
                { "leve", 120 }, { "atkb", 0 }, { "decb", 40 }, { "endb", 0 },
                { "atk", 0 }, { "dec", 45 }, { "sus", 20 }, { "rel", 40 },
                { "cut", 110 }, { "reso", 45 }, { "drive", 70 }, { "trig", 1 },
                { "lfoRate", 64 }, { "lfoDepth", 0 }, { "lfoShape", 0 }, { "lfoDest", 0 },
                { "dlySend", 40 }, { "revSend", 18 }, { "distAmt", 0 }, { "crushAmt", 0 } } },
            { "PAD", {
                { "alg", 8 }, { "ratioC", 0 }, { "ratioA", 5 }, { "ratioB", 42 },
                { "harm", 80 }, { "detune", 35 }, { "fdbk", 0 }, { "mix", 60 },
                { "leva", 80 }, { "atka", 40 }, { "deca", 90 }, { "enda", 60 },
                { "leve", 80 }, { "atkb", 40 }, { "decb", 90 }, { "endb", 60 },
                { "atk", 50 }, { "dec", 100 }, { "sus", 85 }, { "rel", 100 },
                { "cut", 100 }, { "reso", 20 }, { "drive", 15 }, { "trig", 0 },
                { "lfoRate", 30 }, { "lfoDepth", 22 }, { "lfoShape", 0 }, { "lfoDest", 1 },
                { "dlySend", 20 }, { "revSend", 60 }, { "distAmt", 0 }, { "crushAmt", 0 } } },
        };
        return p;
    }

    Fm4Panel::Panel& panel;
    Fm4Sequencer& seq;
    Fm4Capture& cap;

    sf::Font font;
    int dragging = -1, hoverPot = -1, lastClicked = -1;
    float dragY = 0.0f, dragStart = 0.0f, dragStartValue = 0.0f;

    // Piano roll interaction
    int dragStepIdx = -1;
    int lastStepEdit = -1;
    bool draggingLen = false;
    int baseMidiNote = 33;
    int visibleSemitones = 19;
    int stepPage = 0;

    bool draggingScroll = false;
    float scrollDragStart = 0.0f;

    sf::FloatRect editNoteRect, editLenRect, editVeloRect, editSlideRect, editAccentRect;

    sf::Clock clock, frameClock, flashClock;
    int flashPreset = -1;
    float animTime = 0.0f;

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
    bool wideLayout = true;
    float wideCell = W_CELL;
    void setWideLayout(bool w, float availWidth = 0.0f)
    {
        wideLayout = w;
        if (w && availWidth > 0.0f) {
            // Grow the cells to use a large window, but keep the panel 1:1.
            float fit = (availWidth - 2.0f * CARD_X - (PAGES - 1) * W_GAP) / (float)(PAGES * PAGE_COLS);
            wideCell = std::clamp(fit, W_CELL, 132.0f);
        } else {
            wideCell = W_CELL;
        }
    }

    float cellW() const { return wideLayout ? wideCell : CELL_W; }
    float gap() const { return wideLayout ? W_GAP : GAP; }
    float blockW() const { return PAGE_COLS * cellW(); }

    float panelWidth() const
    {
        return 2.0f * CARD_X + (wideLayout ? PAGES * blockW() + (PAGES - 1) * gap()
                                          : 2 * blockW() + gap());
    }
    float panelBottom() const
    {
        // leave a strip under the cards for the hover help line
        return CARDS_Y + (wideLayout ? CARD_H2 : 2.0f * CARD_H2 + STACK_GAP) + 30.0f;
    }
    float panelHeight() const { return panelBottom() - PANEL_TOP; }

    int pageOf(int pot) const
    {
        for (int p = 0; p < PAGES; ++p)
            for (int s = 0; s < PAGE_POTS; ++s)
                if (page(p).pots[s] == pot) return p;
        return 0;
    }

    int pageSlot(int pot) const
    {
        for (int p = 0; p < PAGES; ++p)
            for (int s = 0; s < PAGE_POTS; ++s)
                if (page(p).pots[s] == pot) return s;
        return 0;
    }

    sf::FloatRect pageRect(int p) const
    {
        int col = wideLayout ? p : (p / 2);
        int row = wideLayout ? 0 : (p % 2);
        float x = CARD_X + col * (blockW() + gap());
        float y = CARDS_Y + row * (CARD_H2 + STACK_GAP);
        return { x, y, blockW(), CARD_H2 };
    }

    float rowY(int pot) const
    {
        return pageRect(pageOf(pot)).top + HEAD_S + (pageSlot(pot) / PAGE_COLS) * ROW_PITCH;
    }

    sf::Vector2f knobCenter(int pot) const
    {
        auto r = pageRect(pageOf(pot));
        int slot = pageSlot(pot);
        return { r.left + (slot % PAGE_COLS) * cellW() + cellW() * 0.5f,
                 r.top + HEAD_S + (slot / PAGE_COLS) * ROW_PITCH + 46.0f };
    }

    sf::FloatRect knobCell(int pot) const
    {
        auto ctr = knobCenter(pot);
        return { ctr.x - cellW() * 0.5f, ctr.y - 38.0f, cellW(), 104.0f };
    }

    static sf::FloatRect playBtn() { return { 200.0f, 12.0f, 56.0f, 40.0f }; }
    static sf::FloatRect trigBtn() { return { 262.0f, 12.0f, 56.0f, 40.0f }; }
    static sf::FloatRect bpmBox()  { return { 330.0f, 12.0f, 112.0f, 40.0f }; }
    static sf::FloatRect tuneBox() { return { 448.0f, 12.0f, 112.0f, 40.0f }; }
    static sf::FloatRect presetRect(int i) { return { 574.0f + i * 62.0f, 12.0f, 56.0f, 40.0f }; }
    static sf::FloatRect diceBtn() { return { 956.0f, 12.0f, 52.0f, 40.0f }; }
    static sf::FloatRect meterBox() { return { W - 20.0f - 116.0f, 12.0f, 116.0f, 40.0f }; }

    sf::FloatRect algoRect() const { return { CARD_X, SCOPE_Y, 540.0f, SCOPE_H }; }
    sf::FloatRect scopeRect() const { return { CARD_X + 556.0f, SCOPE_Y, panelWidth() - 2.0f * CARD_X - 556.0f, SCOPE_H }; }

    static sf::FloatRect pianoRollRect() { return { CARD_X, ROLL_Y, W - 2.0f * CARD_X, ROLL_H }; }

    static sf::FloatRect pianoGridRect()
    {
        sf::FloatRect pr = pianoRollRect();
        return { pr.left + 54.0f, pr.top + 36.0f, pr.width - 72.0f, pr.height - 64.0f };
    }

    static sf::FloatRect scrollBarRect()
    {
        sf::FloatRect pr = pianoRollRect();
        return { pr.left + pr.width - 12.0f, pr.top + 36.0f, 8.0f, pr.height - 64.0f };
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
            if (bpmBox().contains(p)) { stepBpm(p.x > bpmBox().left + 56.0f ? 1.0f : -1.0f, shift); return; }
            if (tuneBox().contains(p)) { stepTune(p.x > tuneBox().left + 56.0f ? 1 : -1, shift); return; }

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
                    dragStartValue = panel.param(pot).value;
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
            if (panel.isDetent(dragging)) {
                // encoder feel: each 16 px of travel is one detent
                Param& prm = panel.param(dragging);
                int steps = (int)std::lround((dragY - p.y) / 16.0f);
                float v = dragStartValue + (float)steps * prm.step;
                panel.setValue(dragging, std::clamp(v, prm.min, prm.max));
            } else {
                float speed = shift ? 0.0008f : 0.004f;
                panel.set(dragging, dragStart + (dragY - p.y) * speed);
            }
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
                if (panel.isDetent(pot)) {
                    // one wheel notch = one detent (this is what the Alg knob wants)
                    Param& prm = panel.param(pot);
                    float v = prm.value + (delta > 0 ? prm.step : -prm.step);
                    panel.setValue(pot, std::clamp(v, prm.min, prm.max));
                } else {
                    panel.tweak(pot, delta * (shift ? 0.005f : 0.02f));
                }
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
        // Keep the algorithm, the trigger mode and the sends musical; randomize the colour.
        for (int pot = 0; pot < Fm4Panel::POTS; ++pot) {
            if (pot == 0 || pot == 23) continue;
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
        if (showTopBar) drawTopBar(target, mouse);
        drawAlgo(target, algoRect());
        drawScope(target, scopeRect());

        for (int b = 0; b < PAGES; ++b) drawBlock(target, b);
        for (int pot = 0; pot < Fm4Panel::POTS; ++pot) drawKnob(target, pot, Theme::blockAccent[page(pageOf(pot)).accent]);
        if (showSequencer) drawSequencer(target, mouse);
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
        head.setPoint(1, { x2 - 8.0f * std::cos(ang - 0.42f), y2 - 8.0f * std::sin(ang - 0.42f) });
        head.setPoint(2, { x2 - 8.0f * std::cos(ang + 0.42f), y2 - 8.0f * std::sin(ang + 0.42f) });
        head.setFillColor(c);
        t.draw(head);
    }

    void polyline(sf::RenderTarget& t, const std::vector<sf::Vector2f>& pts, sf::Color c, float th = 1.5f)
    {
        if (pts.size() < 2) return;
        sf::VertexArray va(sf::LineStrip, pts.size());
        for (size_t i = 0; i < pts.size(); ++i) va[i] = sf::Vertex(pts[i], c);
        t.draw(va);
    }

    void fillUnder(sf::RenderTarget& t, const std::vector<sf::Vector2f>& pts, float bottom, sf::Color c)
    {
        if (pts.size() < 2) return;
        sf::VertexArray va(sf::TriangleStrip, pts.size() * 2);
        for (size_t i = 0; i < pts.size(); ++i) {
            va[2 * i] = sf::Vertex(pts[i], c);
            va[2 * i + 1] = sf::Vertex({ pts[i].x, bottom }, c);
        }
        t.draw(va);
    }

    void animate(float dt)
    {
        animTime += dt;
        float k = 1.0f - std::exp(-dt * 30.0f);
        for (int i = 0; i < Fm4Panel::POTS; ++i) shown[i] += (panel.norm(i) - shown[i]) * k;
        cap.livePeak += (std::clamp(cap.peak, 0.0f, 1.0f) - cap.livePeak) * std::min(1.0f, dt * 5.0f);
        peakHold = std::max(peakHold, cap.peak);
        cap.peak = 0.0f;

        const Fm4& e = panel.engine;
        float fmAct = 0.0f;
        for (int j = 0; j < Fm4::NUM_OPS; ++j) fmAct += std::clamp(e.liveOp[j], 0.0f, 1.0f);
        fmAct = std::clamp(fmAct * 0.5f, 0.0f, 1.0f);
        float envAct = std::clamp(e.liveEnvA + e.liveEnvB, 0.0f, 1.0f) * 0.6f;
        float ampAct = std::clamp(e.liveAmp, 0.0f, 1.0f);
        float lfoAct = std::clamp(std::fabs(e.liveLfo) * panel.norm(25), 0.0f, 1.0f) * 0.8f;
        float fx = std::clamp(std::fabs(e.liveDelay) + std::fabs(e.liveReverb), 0.0f, 1.0f);
        glow[0] = std::max(fmAct, glow[0] - dt * 3.0f);                            // OPERATOR FM
        glow[1] = std::max(std::max(ampAct * 0.7f, lfoAct), glow[1] - dt * 3.0f);  // AMP & LFO
        glow[2] = std::max(envAct, glow[2] - dt * 3.0f);                           // OPERATOR ENVS
        glow[3] = std::max(std::max(ampAct * 0.5f, std::clamp(fx * 2.0f, 0.0f, 1.0f)), glow[3] - dt * 2.0f); // FILTER & FX
    }

    // ------------------------------------------------------------------ top bar
    void drawTopBar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "zicStudio", CARD_X, 26.0f, 22, Theme::text, LEFT, true, 0.4f);
        label(t, "Fm4", CARD_X + 112.0f, 26.0f, 22, Theme::fmNeon, LEFT, true, 0.4f);
        label(t, "4-OPERATOR FM SYNTHESIZER", CARD_X, 48.0f, 8, Theme::muted, LEFT, false, 2.0f);

        rrect(t, playBtn(), 11.0f, seq.playing ? Theme::fmNeon : (playBtn().contains(mouse) ? sf::Color(40, 43, 54) : Theme::card), seq.playing ? Theme::fmNeon : Theme::cardLine);
        label(t, seq.playing ? "STOP" : "PLAY", playBtn().left + playBtn().width * 0.5f, playBtn().top + 20.0f, 10, seq.playing ? sf::Color(14, 14, 18) : Theme::text, CENTER, true);

        rrect(t, trigBtn(), 11.0f, trigBtn().contains(mouse) ? sf::Color(40, 43, 54) : Theme::card, Theme::cardLine);
        label(t, "TRIG", trigBtn().left + trigBtn().width * 0.5f, trigBtn().top + 20.0f, 10, Theme::text, CENTER, true);

        char buf[32];
        snprintf(buf, sizeof(buf), "%d", (int)std::round(seq.bpm));
        rrect(t, bpmBox(), 11.0f, Theme::card, Theme::cardLine);
        label(t, buf, bpmBox().left + 46.0f, bpmBox().top + 20.0f, 18, Theme::text, CENTER, true);
        label(t, "BPM", bpmBox().left + 70.0f, bpmBox().top + 23.0f, 8, Theme::muted, LEFT, false, 1.4f);
        label(t, "-", bpmBox().left + 12.0f, bpmBox().top + 19.0f, 16, Theme::muted);
        label(t, "+", bpmBox().left + bpmBox().width - 12.0f, bpmBox().top + 19.0f, 16, Theme::muted);

        snprintf(buf, sizeof(buf), "%+d", seq.transpose);
        rrect(t, tuneBox(), 11.0f, Theme::card, Theme::cardLine);
        label(t, buf, tuneBox().left + 46.0f, tuneBox().top + 20.0f, 18, Theme::text, CENTER, true);
        label(t, "TUNE", tuneBox().left + 70.0f, tuneBox().top + 23.0f, 8, Theme::muted, LEFT, false, 1.4f);
        label(t, "-", tuneBox().left + 12.0f, tuneBox().top + 19.0f, 16, Theme::muted);
        label(t, "+", tuneBox().left + tuneBox().width - 12.0f, tuneBox().top + 19.0f, 16, Theme::muted);

        float flash = flashClock.getElapsedTime().asSeconds();
        for (int i = 0; i < (int)presets().size(); ++i) {
            auto r = presetRect(i);
            bool hot = r.contains(mouse);
            bool lit = flashPreset == i && flash < 0.6f;
            sf::Color acc = Theme::blockAccent[i % 6];
            rrect(t, r, 11.0f, lit ? alpha(acc, 200) : (hot ? sf::Color(40, 43, 54) : Theme::card), lit ? acc : Theme::cardLine);
            label(t, presets()[i].name, r.left + r.width * 0.5f, r.top + 20.0f, 8, lit ? sf::Color(14, 14, 18) : (hot ? Theme::text : Theme::muted), CENTER, true, 1.0f);
            label(t, std::to_string(i + 1), r.left + 6.0f, r.top + 7.0f, 6, Theme::faint, LEFT);
        }

        auto d = diceBtn();
        bool lit = flashPreset == -2 && flash < 0.6f;
        rrect(t, d, 11.0f, lit ? Theme::fmNeon : (d.contains(mouse) ? sf::Color(40, 43, 54) : Theme::card), Theme::cardLine);
        label(t, "DICE", d.left + d.width * 0.5f, d.top + 20.0f, 9, lit ? sf::Color(14, 14, 18) : Theme::fmNeon, CENTER, true);

        auto m = meterBox();
        rrect(t, m, 11.0f, Theme::card, Theme::cardLine);
        label(t, "OUT", m.left + 9.0f, m.top + 20.0f, 7, Theme::muted, LEFT, false, 2.0f);
        float lvl = std::clamp(cap.livePeak * 1.4f, 0.0f, 1.0f);
        sf::FloatRect bar { m.left + 32.0f, m.top + 15.0f, m.width - 41.0f, 10.0f };
        rrect(t, bar, 5.0f, Theme::cell);
        if (lvl > 0.01f) rrect(t, { bar.left, bar.top, bar.width * lvl, bar.height }, 5.0f, lvl > 0.9f ? Theme::opB2 : Theme::fmNeon);
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
            float y = mid - val * (r.height * 0.42f);
            va[i] = sf::Vertex({ x, y }, Theme::fmNeon);
        }
        t.draw(va);

        const Fm4& e = panel.engine;
        char buf[96];
        snprintf(buf, sizeof(buf), "ENV A %.2f   ENV B %.2f   AMP %.2f", e.liveEnvA, e.liveEnvB, e.liveAmp);
        label(t, buf, r.left + 16.0f, r.top + 13.0f, 8, Theme::faint, LEFT, false, 1.1f);
        label(t, "MONO FM OUTPUT", r.left + r.width - 16.0f, r.top + 13.0f, 8, Theme::faint, RIGHT, false, 1.1f);
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
        label(t, buf, r.left + 16.0f, r.top + 17.0f, 11, Theme::blockAccent[0], LEFT, true, 2.2f);

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
        label(t, desc, r.left + 16.0f, r.top + 31.0f, 8, Theme::muted, LEFT, false, 1.0f);

        sf::FloatRect area { r.left + 16.0f, r.top + 40.0f, r.width - 32.0f, r.height - 64.0f };
        const float NODE = 40.0f, HALF = NODE * 0.5f;

        auto nodeXY = [&](int op) {
            return sf::Vector2f(area.left + ALG_POS[alg][op][0] * area.width,
                                area.top + ALG_POS[alg][op][1] * area.height);
        };

        const Fm4& e = panel.engine;

        // Modulation paths: a faint line, flowing signal dots, and a head at the target.
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                if (!((Fm4::ALG_MOD[alg][j] >> k) & 1u)) continue;
                sf::Vector2f a = nodeXY(k), b = nodeXY(j);
                float dx = b.x - a.x, dy = b.y - a.y;
                float len = std::max(1.0f, std::sqrt(dx * dx + dy * dy));
                dx /= len;
                dy /= len;
                float t0 = HALF + 6.0f, t1 = len - HALF - 8.0f;
                if (t1 <= t0) continue;

                float lvl = std::clamp(std::sqrt(std::max(0.0f, e.liveOp[k])) * 1.3f, 0.0f, 1.0f);
                sf::Color base = opColor(k);

                sf::RectangleShape line({ t1 - t0, 1.5f });
                line.setOrigin(0.0f, 0.75f);
                line.setPosition(a.x + dx * t0, a.y + dy * t0);
                line.setRotation(std::atan2(dy, dx) * 180.0f / 3.14159265f);
                line.setFillColor(alpha(base, 45 + (int)(70 * lvl)));
                t.draw(line);

                for (int d = 0; d < 3; ++d) {
                    float u = std::fmod(animTime * 0.5f + d / 3.0f + (float)(j * 4 + k) * 0.09f, 1.0f);
                    float dd = t0 + (t1 - t0) * u;
                    disc(t, a.x + dx * dd, a.y + dy * dd, 2.0f, alpha(base, 110 + (int)(145 * lvl)));
                }

                float ang = std::atan2(dy, dx);
                sf::Vector2f tip { b.x - dx * (HALF + 2.0f), b.y - dy * (HALF + 2.0f) };
                sf::ConvexShape head(3);
                head.setPoint(0, tip);
                head.setPoint(1, { tip.x - 8.0f * std::cos(ang - 0.42f), tip.y - 8.0f * std::sin(ang - 0.42f) });
                head.setPoint(2, { tip.x - 8.0f * std::cos(ang + 0.42f), tip.y - 8.0f * std::sin(ang + 0.42f) });
                head.setFillColor(alpha(base, 120 + (int)(135 * lvl)));
                t.draw(head);
            }
        }

        // Operator nodes: the live output sets the fill and glow, the envelope sets the ring.
        for (int op = 0; op < 4; ++op) {
            sf::Vector2f p = nodeXY(op);
            float out = std::clamp(e.liveOp[op], 0.0f, 1.0f);
            float act = std::clamp(std::sqrt(out) * 1.6f, 0.0f, 1.0f);
            float env = op == 1 ? e.liveEnvA : (op >= 2 ? e.liveEnvB : act);
            sf::Color c = opColor(op);

            if (act > 0.02f) disc(t, p.x, p.y, HALF + 10.0f * act, alpha(c, (int)(50 * act)));

            if (op >= 1) {
                float span = 360.0f * std::clamp(env, 0.0f, 1.0f);
                if (span > 1.0f) arc(t, p.x, p.y, HALF + 5.0f, 2.5f, -90.0f, -90.0f + span, alpha(c, 200), false);
            }

            sf::FloatRect nr { p.x - HALF, p.y - HALF, NODE, NODE };
            rrect(t, nr, 11.0f, mix(Theme::cap, c, 0.16f + 0.55f * act));
            rrect(t, nr, 11.0f, sf::Color::Transparent, alpha(c, 150 + (int)(105 * act)));
            label(t, opName(op), p.x, p.y - 2.0f, op < 2 ? 13 : 10, Theme::text, CENTER, true, 0.4f);

            float bw = (NODE - 12.0f) * std::clamp(out * 2.2f, 0.0f, 1.0f);
            if (bw > 1.0f) rrect(t, { p.x - HALF + 6.0f, p.y + HALF - 8.0f, bw, 3.0f }, 1.5f, alpha(c, 230));

            bool xCar = ((Fm4::ALG_X[alg] >> op) & 1u) != 0;
            bool yCar = ((Fm4::ALG_Y[alg] >> op) & 1u) != 0;
            std::string tag;
            if (xCar) tag += "X";
            if (yCar) tag += "Y";
            if (!tag.empty()) {
                float tw = textWidth(tag, 7, true, 0.6f) + 8.0f;
                float tx = p.x + HALF + 2.0f;
                rrect(t, { tx, p.y - 5.0f, tw, 10.0f }, 4.0f, alpha(c, 220));
                label(t, tag, tx + tw * 0.5f, p.y, 7, sf::Color(12, 12, 16), CENTER, true, 0.6f);
            }
        }

        char mbuf[32];
        snprintf(mbuf, sizeof(mbuf), "MIX %d%%", (int)std::lround(panel.norm(7) * 100.0f));
        label(t, mbuf, r.left + r.width - 16.0f, r.top + 17.0f, 8, Theme::faint, RIGHT, false, 1.1f);
    }

    // ------------------------------------------------------------------ card header graphs
    sf::FloatRect headGraph(int b) const
    {
        auto r = pageRect(b);
        float gw = std::min(150.0f, r.width - 180.0f);
        if (gw < 60.0f) gw = 60.0f;
        return { r.left + r.width - gw - 14.0f, r.top + 9.0f, gw, HEAD_S - 18.0f };
    }

    static float lfoShape(int shape, float p)
    {
        switch (shape) {
        case 1: return 1.0f - 4.0f * std::fabs(p - 0.5f);
        case 2: return 2.0f * p - 1.0f;
        case 3: return p < 0.5f ? 1.0f : -1.0f;
        default: return std::sin(6.2831853f * p);
        }
    }

    void drawHarmWave(sf::RenderTarget& t, sf::FloatRect r)
    {
        float harmN = panel.norm(4);
        int op = harmN < 0.5f ? 0 : 1;
        std::vector<sf::Vector2f> pts;
        const int N = 64;
        float mid = r.top + r.height * 0.5f;
        for (int i = 0; i <= N; ++i) {
            float ph = (float)i / N * 2.0f;
            float v = panel.engine.wavePreview(op, ph - std::floor(ph));
            pts.push_back({ r.left + r.width * i / N, mid - v * r.height * 0.42f });
        }
        hline(t, r.left, mid, r.width, sf::Color(255, 255, 255, 12));
        polyline(t, pts, Theme::blockAccent[0], 1.5f);
        label(t, op == 0 ? "C" : "A", r.left + 1.0f, r.top + 3.0f, 6, Theme::faint, LEFT);
    }

    void drawEnvPair(sf::RenderTarget& t, sf::FloatRect r)
    {
        drawEnvCurve(t, r, 9, 10, 11, Theme::opA, panel.engine.liveEnvA, panel.engine.liveStageA);
        drawEnvCurve(t, r, 13, 14, 15, Theme::opB2, panel.engine.liveEnvB, panel.engine.liveStageB);
    }

    void drawEnvCurve(sf::RenderTarget& t, sf::FloatRect r, int atkPot, int decPot, int endPot,
                      sf::Color c, float live, int stage)
    {
        float atk = 0.001f * std::exp2(panel.norm(atkPot) * 10.0f);
        float dec = 0.005f * std::exp2(panel.norm(decPot) * 9.0f);
        float end = panel.norm(endPot);
        const float T = 4.0f, tau = 0.02f, L = std::log1p(T / tau);
        auto yOf = [&](float lvl) { return r.top + r.height - std::clamp(lvl, 0.0f, 1.0f) * (r.height - 2.0f) - 1.0f; };
        auto xOf = [&](float time) {
            return r.left + std::clamp(std::log1p(std::max(0.0f, time) / tau) / L, 0.0f, 1.0f) * r.width;
        };

        std::vector<sf::Vector2f> pts;
        const int N = 48;
        for (int i = 0; i <= N; ++i) {
            float u = (float)i / N;
            float time = tau * (std::exp(u * L) - 1.0f);
            float lvl = time < atk ? time / atk : end + (1.0f - end) * std::exp(-(time - atk) / dec);
            pts.push_back({ r.left + u * r.width, yOf(lvl) });
        }
        fillUnder(t, pts, r.top + r.height, alpha(c, 20));
        polyline(t, pts, c, 1.5f);

        float lvl = std::clamp(live, 0.0f, 1.0f);
        float time;
        if (stage == 0) time = lvl * atk;
        else if (1.0f - end > 1e-3f) time = atk - dec * std::log(std::clamp((lvl - end) / (1.0f - end), 0.002f, 1.0f));
        else time = atk;
        disc(t, xOf(std::max(0.0f, time)), yOf(lvl), 2.5f, c);
    }

    void drawAdsr(sf::RenderTarget& t, sf::FloatRect r)
    {
        float sus = panel.norm(18);
        sf::Color c = Theme::blockAccent[2];
        auto yOf = [&](float lvl) { return r.top + r.height - std::clamp(lvl, 0.0f, 1.0f) * (r.height - 2.0f) - 1.0f; };

        std::vector<sf::Vector2f> pts;
        pts.push_back({ r.left, yOf(0.0f) });
        pts.push_back({ r.left + r.width * 0.14f, yOf(1.0f) });
        const int D = 14;
        for (int i = 1; i <= D; ++i) {
            float u = (float)i / D;
            float lvl = sus + (1.0f - sus) * std::exp(-u * 3.0f);
            pts.push_back({ r.left + r.width * (0.14f + 0.38f * u), yOf(lvl) });
        }
        pts.push_back({ r.left + r.width * 0.68f, yOf(sus) });
        for (int i = 1; i <= D; ++i) {
            float u = (float)i / D;
            float lvl = sus * std::exp(-u * 4.0f);
            pts.push_back({ r.left + r.width * (0.68f + 0.32f * u), yOf(lvl) });
        }
        fillUnder(t, pts, r.top + r.height, alpha(c, 20));
        polyline(t, pts, c, 1.5f);

        const Fm4& e = panel.engine;
        float lvl = std::clamp(e.liveAmp, 0.0f, 1.0f);
        float x;
        if (e.liveStageAmp == 0) {
            x = r.width * 0.14f * lvl;
        } else if (e.liveStageAmp == 1) {
            float den = std::max(1e-3f, 1.0f - sus);
            x = r.width * (0.14f + 0.38f * std::clamp((1.0f - lvl) / den, 0.0f, 1.0f));
        } else if (e.liveStageAmp == 2) {
            x = r.width * (0.68f + 0.32f * (1.0f - lvl));
        } else {
            x = r.width;
        }
        disc(t, r.left + x, yOf(lvl), 2.5f, c);
    }

    void drawFilter(sf::RenderTarget& t, sf::FloatRect r)
    {
        float cutN = panel.norm(20), resoN = panel.norm(21);
        float cutSt = -43.0f + 100.0f * cutN;
        float fc = 440.0f * std::exp2(cutSt / 12.0f);
        float k = 2.0f - 1.9f * resoN;
        sf::Color c = Theme::blockAccent[3];

        const int N = 56;
        std::vector<sf::Vector2f> pts;
        float logSpan = std::log2(18000.0f / 30.0f);
        auto yOf = [&](float lvl) { return r.top + r.height - std::clamp(lvl, 0.0f, 1.0f) * (r.height - 2.0f) - 1.0f; };
        for (int i = 0; i <= N; ++i) {
            float u = (float)i / N;
            float f = 30.0f * std::exp2(u * logSpan);
            float rr = f / fc;
            float mag = 1.0f / std::sqrt((1.0f - rr * rr) * (1.0f - rr * rr) + (k * rr) * (k * rr));
            pts.push_back({ r.left + u * r.width, yOf(std::clamp(mag, 0.0f, 1.8f) / 1.8f) });
        }
        fillUnder(t, pts, r.top + r.height, alpha(c, 20));
        polyline(t, pts, c, 1.5f);

        float cu = std::clamp(std::log2(fc / 30.0f) / logSpan, 0.0f, 1.0f);
        float cx = r.left + cu * r.width;
        vline(t, cx, r.top, r.height, alpha(c, 80));
        disc(t, cx, yOf(std::clamp(1.0f / k, 0.0f, 1.8f) / 1.8f), 2.2f, c);
    }

    void drawLfoScope(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 8.0f, sf::Color(13, 14, 18), Theme::cardLine);
        sf::Color c = Theme::blockAccent[4];
        int shape = (int)std::lround(panel.param(26).value);

        sf::FloatRect g { r.left + 4.0f, r.top + 3.0f, r.width - 8.0f, r.height - 6.0f };
        hline(t, g.left, g.top + g.height * 0.5f, g.width, sf::Color(255, 255, 255, 14));

        std::vector<sf::Vector2f> pts;
        const int N = 64;
        for (int i = 0; i <= N; ++i) {
            float u = (float)i / N;
            float p = u * 2.0f;
            float v = lfoShape(shape, p - std::floor(p));
            pts.push_back({ g.left + u * g.width, g.top + g.height * 0.5f - v * g.height * 0.42f });
        }
        polyline(t, pts, alpha(c, 180), 1.4f);

        const Fm4& e = panel.engine;
        float px = g.left + e.liveLfoPhase * g.width * 0.5f;
        float py = g.top + g.height * 0.5f - e.liveLfo * g.height * 0.42f;
        vline(t, px, g.top, g.height, alpha(c, 80));
        disc(t, px, py, 2.6f, c);
    }

    void drawFxMini(sf::RenderTarget& t, sf::FloatRect r)
    {
        sf::Color c = Theme::blockAccent[5];
        float dly = panel.norm(28), rev = panel.norm(29);
        float base = r.top + r.height - 2.0f;

        for (int i = 0; i < 4; ++i) {
            float x = r.left + 4.0f + i * (r.width * 0.45f / 3.0f);
            float h = (0.9f - 0.24f * i) * dly * (r.height - 4.0f);
            if (h > 0.5f) vline(t, x, base - h, h, alpha(c, 220 - i * 30), 2.5f);
        }

        std::vector<sf::Vector2f> pts;
        const int N = 24;
        for (int i = 0; i <= N; ++i) {
            float u = (float)i / N;
            float lvl = rev * std::exp(-u * 3.5f);
            pts.push_back({ r.left + r.width * (0.55f + 0.45f * u), base - lvl * (r.height - 4.0f) });
        }
        polyline(t, pts, alpha(c, 220), 1.6f);
        hline(t, r.left, base, r.width, alpha(c, 60));
        label(t, "DLY", r.left + 1.0f, r.top + 3.0f, 6, Theme::faint, LEFT, false, 1.2f);
        label(t, "REV", r.left + r.width - 1.0f, r.top + 3.0f, 6, Theme::faint, RIGHT, false, 1.2f);
    }

    // ------------------------------------------------------------------ cards & knobs
    void drawBlock(sf::RenderTarget& t, int b)
    {
        auto r = pageRect(b);
        auto& pg = page(b);
        sf::Color accent = Theme::blockAccent[pg.accent];
        float g = std::clamp(glow[b], 0.0f, 1.0f);

        rrect(t, r, 18.0f, Theme::card, mix(Theme::cardLine, accent, 0.55f + 0.45f * g));
        rrect(t, { r.left + 18.0f, r.top, r.width - 36.0f, 3.0f }, 1.5f, alpha(accent, 120 + (int)(135 * g)));
        if (g > 0.02f) rrect(t, { r.left + 18.0f, r.top - 2.0f, r.width - 36.0f, 7.0f }, 3.5f, alpha(accent, (int)(55 * g)));

        label(t, pg.name, r.left + 18.0f, r.top + 17.0f, 12, accent, LEFT, true, 2.6f);
        label(t, pg.subtitle, r.left + 18.0f, r.top + 32.0f, 8, Theme::muted, LEFT, false, 0.8f);

        for (int gi = 0; gi < Fm4Panel::GROUPS; ++gi) {
            const Fm4Panel::Group& grp = Fm4Panel::group(gi);
            if (pageOf(grp.firstPot) != b) continue;
            int slot = pageSlot(grp.firstPot);
            float x = r.left + (slot % PAGE_COLS) * cellW();
            float y = r.top + HEAD_S + (slot / PAGE_COLS) * ROW_PITCH;
            sf::FloatRect pill { x + 6.0f, y + 4.0f, grp.count * cellW() - 12.0f, ROW_PITCH - 12.0f };
            rrect(t, pill, 14.0f, sf::Color(255, 255, 255, 4), alpha(accent, 40));
            float tw = textWidth(grp.name, 8, true, 1.8f);
            rrect(t, { pill.left + 12.0f, pill.top - 7.0f, tw + 14.0f, 14.0f }, 6.0f, Theme::card);
            label(t, grp.name, pill.left + 19.0f, pill.top, 8, alpha(accent, 235), LEFT, true, 1.8f);
        }

        switch (b) {
        case 0: drawHarmWave(t, headGraph(b)); break;
        case 1: drawAdsr(t, headGraph(b)); break;
        case 2: drawEnvPair(t, headGraph(b)); break;
        case 3: drawFilter(t, headGraph(b)); break;
        default: break;
        }
    }

    void drawMiniWave(sf::RenderTarget& t, float cx, float cy, int shape, sf::Color c)
    {
        const float w = 9.0f, h = 5.0f;
        std::vector<sf::Vector2f> pts;
        const int N = 8;
        for (int i = 0; i <= N; ++i) {
            float u = (float)i / N;
            float x = cx - w * 0.5f + u * w;
            float v = 0.0f;
            switch (shape) {
            case 1: v = 1.0f - 4.0f * std::fabs(u - 0.5f); break;
            case 2:
                if (u < 0.82f) v = -1.0f + 2.0f * u / 0.82f;
                else v = -1.0f;
                break;
            case 3: v = u < 0.5f ? 1.0f : -1.0f; break;
            default: v = std::sin(6.2831853f * u); break;
            }
            pts.push_back({ x, cy - v * h * 0.5f });
        }
        polyline(t, pts, c, 1.0f);
    }

    void drawKnob(sf::RenderTarget& t, int pot, sf::Color accent)
    {
        auto ctr = knobCenter(pot);
        bool active = dragging == pot;
        bool hot = active || hoverPot == pot;
        float n = shown[pot];
        const float R = 25.0f;
        auto angle = [](float v) { return -135.0f + 270.0f * v; };

        disc(t, ctr.x, ctr.y, 33.0f, sf::Color(20, 22, 28));
        arc(t, ctr.x, ctr.y, R, 5.0f, -135.0f, 135.0f, Theme::track);

        if (panel.isDetent(pot)) {
            int count = (int)std::lround(panel.param(pot).max - panel.param(pot).min) + 1;
            int cur = (int)std::lround(panel.param(pot).value - panel.param(pot).min);
            for (int s = 0; s < count; ++s) {
                float a = angle((float)s / std::max(1, count - 1)) * 3.14159265f / 180.0f;
                bool on = s == cur;
                disc(t, ctr.x + (R + 6.5f) * std::sin(a), ctr.y - (R + 6.5f) * std::cos(a),
                     on ? 2.0f : 1.4f, on ? accent : Theme::faint);
            }
            if (pot == 26) {
                for (int s = 0; s < count; ++s) {
                    float a = angle((float)s / std::max(1, count - 1)) * 3.14159265f / 180.0f;
                    float ix = ctr.x + (R + 16.0f) * std::sin(a);
                    float iy = ctr.y - (R + 16.0f) * std::cos(a);
                    drawMiniWave(t, ix, iy, s, (s == cur) ? accent : Theme::faint);
                }
            }
        } else {
            float a = angle(panel.defaultNorm(pot)) * 3.14159265f / 180.0f;
            disc(t, ctr.x + (R + 7.0f) * std::sin(a), ctr.y - (R + 7.0f) * std::cos(a), 1.4f, Theme::faint);
        }

        float from = panel.isCentered(pot) ? 0.0f : -135.0f;
        float to = angle(n);
        if (std::abs(to - from) > 0.5f) {
            arc(t, ctr.x, ctr.y, R, active ? 12.0f : 9.0f, from, to, sf::Color(accent.r, accent.g, accent.b, active ? 80 : 34), false);
            arc(t, ctr.x, ctr.y, R, 5.0f, from, to, accent);
        }

        disc(t, ctr.x, ctr.y, 18.0f, hot ? sf::Color(52, 56, 70) : Theme::cardLine);
        disc(t, ctr.x, ctr.y, 17.0f, Theme::cap);

        float a = angle(n) * 3.14159265f / 180.0f;
        sf::VertexArray ind(sf::Lines, 2);
        ind[0] = sf::Vertex({ ctr.x + 6.0f * std::sin(a), ctr.y - 6.0f * std::cos(a) }, Theme::text);
        ind[1] = sf::Vertex({ ctr.x + 15.0f * std::sin(a), ctr.y - 15.0f * std::cos(a) }, Theme::text);
        t.draw(ind);
        disc(t, ctr.x + 15.0f * std::sin(a), ctr.y - 15.0f * std::cos(a), 1.5f, hot ? accent : Theme::text);

        char buf[32];
        panel.formatValue(pot, buf, sizeof(buf));
        label(t, panel.param(pot).label, ctr.x, ctr.y + 37.0f, 11, hot ? Theme::text : sf::Color(176, 181, 196), CENTER, false, 0.4f);
        label(t, buf, ctr.x, ctr.y + 50.0f, 10, hot ? accent : Theme::muted, CENTER, hot, 0.3f);
    }

    // ------------------------------------------------------------------ sequencer
    void drawSequencer(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        sf::FloatRect pr = pianoRollRect();
        rrect(t, pr, 18.0f, Theme::card, Theme::cardLine);
        rrect(t, { pr.left + 18.0f, pr.top, pr.width - 36.0f, 3.0f }, 1.5f, alpha(Theme::fmNeon, 110));

        label(t, "SEQUENCER", pr.left + 18.0f, pr.top + 18.0f, 12, Theme::text, LEFT, true, 2.6f);
        char buf[64];
        snprintf(buf, sizeof(buf), "%d STEPS", seq.patternLength);
        label(t, buf, pr.left + 118.0f, pr.top + 18.0f, 8, Theme::muted, LEFT, false, 1.4f);

        // page pills
        sf::FloatRect p0 { pr.left + 190.0f, pr.top + 7.0f, 68.0f, 22.0f };
        sf::FloatRect p1 { pr.left + 262.0f, pr.top + 7.0f, 68.0f, 22.0f };
        for (auto pp : { p0, p1 }) {
            bool active = (pp.left == p0.left) ? (stepPage == 0) : (stepPage == 1);
            rrect(t, pp, 8.0f, active ? alpha(Theme::fmNeon, 180) : (pp.contains(mouse) ? sf::Color(40, 43, 54) : Theme::card), Theme::cardLine);
        }
        label(t, "1-16", p0.left + p0.width * 0.5f, p0.top + 11.0f, 9, stepPage == 0 ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);
        label(t, "17-32", p1.left + p1.width * 0.5f, p1.top + 11.0f, 9, stepPage == 1 ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);

        // help line
        label(t, "LEFT draw  RIGHT delete  SHIFT slide  ALT accent  SCROLL note", pr.left + 340.0f, pr.top + 18.0f, 8, Theme::faint, LEFT, false, 0.9f);

        // edit boxes for the selected step
        if (lastStepEdit >= 0 && lastStepEdit < Fm4Sequencer::STEPS) {
            Fm4Step& st = seq.steps[lastStepEdit];
            editNoteRect = { pr.left + 620.0f, pr.top + 7.0f, 96.0f, 22.0f };
            editLenRect = { pr.left + 724.0f, pr.top + 7.0f, 80.0f, 22.0f };
            editVeloRect = { pr.left + 812.0f, pr.top + 7.0f, 80.0f, 22.0f };
            editSlideRect = { pr.left + 900.0f, pr.top + 7.0f, 56.0f, 22.0f };
            editAccentRect = { pr.left + 964.0f, pr.top + 7.0f, 64.0f, 22.0f };

            char nb[24];
            snprintf(nb, sizeof(nb), "%s", midiName(st.note).c_str());
            rrect(t, editNoteRect, 7.0f, editNoteRect.contains(mouse) ? sf::Color(40, 43, 54) : Theme::cell, Theme::cardLine);
            label(t, nb, editNoteRect.left + editNoteRect.width * 0.5f, editNoteRect.top + 11.0f, 10, Theme::text, CENTER, true);
            snprintf(nb, sizeof(nb), "LEN %.2f", st.length);
            rrect(t, editLenRect, 7.0f, editLenRect.contains(mouse) ? sf::Color(40, 43, 54) : Theme::cell, Theme::cardLine);
            label(t, nb, editLenRect.left + editLenRect.width * 0.5f, editLenRect.top + 11.0f, 9, Theme::text, CENTER);
            snprintf(nb, sizeof(nb), "VEL %d%%", (int)std::lround(st.velocity * 100.0f));
            rrect(t, editVeloRect, 7.0f, editVeloRect.contains(mouse) ? sf::Color(40, 43, 54) : Theme::cell, Theme::cardLine);
            label(t, nb, editVeloRect.left + editVeloRect.width * 0.5f, editVeloRect.top + 11.0f, 9, Theme::text, CENTER);
            rrect(t, editSlideRect, 7.0f, st.slide ? alpha(Theme::fmNeon, 180) : (editSlideRect.contains(mouse) ? sf::Color(40, 43, 54) : Theme::cell), Theme::cardLine);
            label(t, "SLIDE", editSlideRect.left + editSlideRect.width * 0.5f, editSlideRect.top + 11.0f, 8, st.slide ? sf::Color(14, 14, 18) : Theme::text, CENTER, true);
            rrect(t, editAccentRect, 7.0f, st.accent ? alpha(Theme::opB2, 200) : (editAccentRect.contains(mouse) ? sf::Color(40, 43, 54) : Theme::cell), Theme::cardLine);
            label(t, "ACCENT", editAccentRect.left + editAccentRect.width * 0.5f, editAccentRect.top + 11.0f, 8, st.accent ? sf::Color(14, 14, 18) : Theme::text, CENTER, true);
        }

        // keyboard on the left
        sf::FloatRect grid = pianoGridRect();
        float keyW = 50.0f;
        float cellH = grid.height / (float)visibleSemitones;
        for (int i = 0; i < visibleSemitones; ++i) {
            int note = baseMidiNote + i;
            float y = grid.top + grid.height - (i + 1) * cellH;
            bool black = isBlackKey(note);
            rrect(t, { pr.left + 2.0f, y, keyW, cellH }, 3.0f, black ? sf::Color(22, 24, 30) : sf::Color(210, 212, 220));
            if (note % 12 == 0) {
                char nbuf[16];
                snprintf(nbuf, sizeof(nbuf), "C%d", note / 12 - 1);
                label(t, nbuf, pr.left + 6.0f, y + cellH * 0.5f, 8, black ? Theme::muted : sf::Color(40, 42, 50), LEFT, true);
            }
        }

        // grid background
        rrect(t, grid, 8.0f, sf::Color(13, 14, 18), sf::Color::Transparent);
        // black-key row shading
        for (int i = 0; i < visibleSemitones; ++i) {
            int note = baseMidiNote + i;
            if (isBlackKey(note)) {
                float y = grid.top + grid.height - (i + 1) * cellH;
                sf::RectangleShape band({ grid.width, cellH });
                band.setPosition(grid.left, y);
                band.setFillColor(sf::Color(255, 255, 255, 7));
                t.draw(band);
            }
        }
        // beat lines + step numbers
        float cellW = grid.width / 16.0f;
        for (int col = 0; col <= 16; ++col) {
            float x = grid.left + col * cellW;
            vline(t, x, grid.top, grid.height, col % 4 == 0 ? sf::Color(255, 255, 255, 22) : sf::Color(255, 255, 255, 8));
        }
        for (int col = 0; col < 16; ++col) {
            char nbuf[4];
            snprintf(nbuf, sizeof(nbuf), "%d", col + 1);
            label(t, nbuf, grid.left + (col + 0.5f) * cellW, grid.top + grid.height + 9.0f, 7, Theme::faint, CENTER);
        }

        // notes
        int curStep = seq.currentStep.load();
        for (int s = 0; s < Fm4Sequencer::STEPS; ++s) {
            const Fm4Step& st = seq.steps[s];
            if (!st.on) continue;
            if (s / 16 != stepPage) continue;
            int noteIdx = st.note - baseMidiNote;
            if (noteIdx < 0 || noteIdx >= visibleSemitones) continue;

            float x = grid.left + (s % 16) * cellW + 1.0f;
            float y = grid.top + grid.height - (noteIdx + 1) * cellH + 1.0f;
            float w = std::max(4.0f, st.length * cellW - 2.0f);
            float h = cellH - 2.0f;
            sf::Color c = st.accent ? Theme::fmNeon : Theme::blockAccent[0];
            rrect(t, { x, y, w, h }, 4.0f, alpha(c, 90 + (int)(80 * st.velocity)));
            rrect(t, { x, y + h - std::max(1.0f, 2.0f * st.velocity), w, std::max(1.0f, 2.0f * st.velocity) }, 1.0f, alpha(c, 220));
            if (st.slide) {
                sf::ConvexShape tri(3);
                tri.setPoint(0, { x + w, y + h * 0.5f });
                tri.setPoint(1, { x + w - 5.0f, y + 2.0f });
                tri.setPoint(2, { x + w - 5.0f, y + h - 2.0f });
                tri.setFillColor(c);
                t.draw(tri);
            }
            if (s == lastStepEdit) {
                rrect(t, { x - 1.0f, y - 1.0f, w + 2.0f, h + 2.0f }, 5.0f, sf::Color::Transparent, Theme::text);
            }
        }

        // playhead
        if (curStep >= 0 && curStep / 16 == stepPage) {
            float x = grid.left + (curStep % 16) * cellW;
            sf::RectangleShape ph({ cellW, grid.height });
            ph.setPosition(x, grid.top);
            ph.setFillColor(sf::Color(230, 255, 40, 30));
            t.draw(ph);
            vline(t, x, grid.top, grid.height, alpha(Theme::fmNeon, 150));
        }

        // scrollbar
        sf::FloatRect bar = scrollBarRect();
        rrect(t, bar, 4.0f, Theme::cell);
        rrect(t, scrollThumbRect(), 4.0f, Theme::muted);
    }

    // ------------------------------------------------------------------ help tooltip
    void drawHelp(sf::RenderTarget& t, int pot)
    {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s  -  %s", panel.param(pot).label, Fm4Panel::help(pot));
        float tw = textWidth(buf, 9, false, 1.0f) + 24.0f;
        if (tw > panelWidth() - 40.0f) tw = panelWidth() - 40.0f;
        sf::FloatRect r { (panelWidth() - tw) * 0.5f, panelBottom() - 28.0f, tw, 22.0f };
        rrect(t, r, 11.0f, sf::Color(12, 13, 17, 230), Theme::blockAccent[page(pageOf(pot)).accent]);
        label(t, buf, r.left + 12.0f, r.top + 11.0f, 9, Theme::text, LEFT, false, 1.0f);
    }

};

// @@PART6@@

} // namespace fm4_ui
