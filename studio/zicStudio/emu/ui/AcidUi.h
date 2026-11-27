#pragma once


#include <SFML/Graphics.hpp>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "studio/zicStudioAcid/emu/AcidSequencer.h"
#include "studio/zicStudioAcid/core/Acid32Panel.h"
#include "helpers/midiNote.h"

inline const char* getMidiNoteName(int midi)
{
    if (midi >= 0 && midi < 132) return MIDI_NOTES_STR[midi];
    return "---";
}

extern std::mutex audioMutex;
extern std::atomic<int> auditionNote;
extern std::atomic<bool> acidMuted;

namespace acid_ui {

struct AcidCapture {
    static constexpr int RING = 4096;
    float ring[RING] = {};
    int ringPos = 0;

    static constexpr int N = 1200;
    float lo[N] = {}, hi[N] = {};
    int count = 0;
    int note = 36;
    int hitId = 0;

    static constexpr int TRAIL = 96;
    float trailPre[TRAIL] = {}, trailPost[TRAIL] = {};
    int trailPos = 0, trailDiv = 0;

    float livePre = 0.0f;
    float livePost = 0.0f;
    float liveCutoff = 440.0f;
    float liveReso = 0.0f;
    float liveEnvAmp = 0.0f;
    float liveEnvFilter = 0.0f;
    float liveLfoVal = 0.0f;
};

namespace Theme {
inline sf::Color rgb(int r, int g, int b, int a = 255) { return sf::Color(r, g, b, a); }
const sf::Color bg = rgb(9, 10, 14);
const sf::Color card = rgb(17, 19, 25);
const sf::Color cardLine = rgb(31, 34, 44);
const sf::Color cell = rgb(26, 28, 36);
const sf::Color track = rgb(44, 47, 58);
const sf::Color cap = rgb(28, 30, 39);
const sf::Color text = rgb(234, 236, 242);
const sf::Color muted = rgb(126, 131, 148);
const sf::Color faint = rgb(68, 72, 88);

// Block accents: 0: OSC (Amber), 1: FILTER (Cyan), 2: ENVELOPES (Violet), 3: SPATIAL FX (Red)
const sf::Color blockAccent[4] = {
    rgb(255, 176, 64),   // OSC (amber)
    rgb(56, 212, 255),   // FILTER (cyan)
    rgb(172, 128, 255),  // ENVELOPES (violet)
    rgb(255, 86, 98)     // SPATIAL FX (red)
};

const sf::Color acidNeon = rgb(230, 255, 40); // Acid yellow
const sf::Color accentNote = rgb(255, 86, 98);
const sf::Color slideNote = rgb(56, 212, 255);
}

class EmuUi {
public:
    bool showTopBar = true;
    bool showSequencer = true;
    static constexpr float W = 1320.0f;
    static constexpr float H = 1236.0f;
    static constexpr float PANEL_TOP = 60.0f;
    static constexpr float PANEL_BOTTOM = 802.0f;
    static constexpr float CARD_X = 24.0f, CELL_W = 150.0f, GAP = 36.0f;

    // Compact Scope at top (58px high)
    static constexpr float SCOPE_Y = 68.0f, SCOPE_H = 58.0f;

    // Knob blocks: EXACT SAME SIZING AS zicStudioKick!
    // ROW_PITCH = 134px, HEAD_S = 44px, knob radius R = 27px, base disc = 36px
    static constexpr float CARDS_Y = 136.0f, ROW_PITCH = 134.0f, HEAD_S = 44.0f;
    static constexpr float CARD_H = HEAD_S + 2 * ROW_PITCH + 8.0f; // 320px
    static constexpr float STACK_GAP = 14.0f;

    // Four pages of eight knobs (2 rows x 4). A wide window lays the pages out
    // side by side; a narrow one folds them into a 2 x 2 grid (filled down the
    // columns, so the signal flow still reads top to bottom, then left to right).
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
            { "OSCILLATOR",    "morphing wave, unison, sub-oscillator & slide", { 0, 1, 2, 3, 4, 5, 6, 7 },        0 },
            { "ENVELOPES",     "filter decay, punch envelope & amp ADSR",       { 16, 17, 18, 19, 20, 21, 22, 23 }, 2 },
            { "LADDER FILTER", "4-pole 24dB ladder, accent, HPF & modulation",  { 8, 9, 10, 11, 12, 13, 14, 15 },   1 },
            { "SPATIAL FX",    "overdrive, distortion, ping-pong delay & reverb", { 24, 25, 26, 27, 28, 29, 30, 31 }, 3 },
        };
        return pages[p];
    }

    // Piano Roll Sequencer
    static constexpr float ROLL_Y = CARDS_Y + 2 * CARD_H + STACK_GAP + 12.0f; // ~802px
    static constexpr float ROLL_H = H - ROLL_Y - 16.0f;                       // ~418px

    struct Preset {
        const char* name;
        std::vector<std::pair<const char*, float>> values;
    };

    static const std::vector<Preset>& presets()
    {
        static const std::vector<Preset> p = {
            { "INIT 303", { { "color", 33 }, { "cutoff", 45 }, { "reso", 65 }, { "envMod", 65 }, { "fDecay", 260 }, { "accent", 80 }, { "decay", 350 }, { "sustain", 60 } } },
            { "DEVILFISH", { { "color", 50 }, { "cutoff", 35 }, { "reso", 88 }, { "drive", 70 }, { "dist", 45 }, { "distType", 0 }, { "fDecay", 170 }, { "glide", 65 }, { "hpfCut", 40 } } },
            { "MENTAL TRIBE", { { "color", 33 }, { "cutoff", 52 }, { "reso", 75 }, { "drive", 60 }, { "dist", 40 }, { "delayTime", 320 }, { "delayFdbk", 65 }, { "delayMix", 45 }, { "revDecay", 70 }, { "revMix", 35 } } },
            { "HARDTEK UNISON", { { "color", 33 }, { "unison", 3 }, { "detune", 20 }, { "sub", 60 }, { "cutoff", 55 }, { "reso", 70 }, { "drive", 80 }, { "dist", 65 }, { "distType", 1 } } },
            { "LFO SQUELCH", { { "color", 40 }, { "cutoff", 40 }, { "reso", 80 }, { "lfoRate", 3.5f }, { "lfoToCut", 60 }, { "lfoToWave", 45 }, { "envMod", 50 } } },
            { "DARK SPACE", { { "color", 25 }, { "cutoff", 38 }, { "reso", 60 }, { "delayTime", 480 }, { "delayFdbk", 75 }, { "delayMix", 50 }, { "revDecay", 85 }, { "revDamp", 50 }, { "revMix", 45 } } }
        };
        return p;
    }

    Acid32Panel::Panel& panel;
    AcidSequencer& seq;
    AcidCapture& cap;

    sf::Font font, fontBold;
    bool haveBold = false;
    int dragging = -1, hoverPot = -1, lastClicked = -1;
    float dragY = 0.0f, dragStart = 0.0f;

    // Piano roll interaction (zic23 model)
    int dragStepIdx = -1;
    int lastStepEdit = -1;
    bool draggingLen = false;
    int baseMidiNote = 24;      // C1 bottom
    int visibleSemitones = 25;  // 2 octaves (C1 to C3)
    int stepPage = 0;           // 0: steps 0..15, 1: steps 16..31

    // Vertical scrollbar for piano roll pitch range
    float scrollY = 0.0f;       // 0.0 = top, 1.0 = bottom
    bool draggingScroll = false;
    float scrollDragStart = 0.0f;
    int scrollBaseStart = 24;

    // Quick edit rects (header bar of piano roll)
    sf::FloatRect editNoteRect;
    sf::FloatRect editLenRect;
    sf::FloatRect editVeloRect;
    sf::FloatRect editSlideRect;
    sf::FloatRect editAccentRect;

    sf::Clock clock, frameClock, flashClock;
    int flashPreset = -1;

    // Animation & smoothing
    float shown[Acid32Panel::POTS];
    float glow[4] = {};
    float peakHold[256] = {};

    EmuUi(Acid32Panel::Panel& panel, AcidSequencer& seq, AcidCapture& cap)
        : panel(panel)
        , seq(seq)
        , cap(cap)
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
        // Real bold face; sf::Text::Bold smears small glyphs (faux bold).
        const char* boldPaths[] = {
            "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Bold.ttf",
            "../../../draw/fonts/Roboto-Bold.ttf",
            "../../draw/fonts/Roboto-Bold.ttf",
            "../draw/fonts/Roboto-Bold.ttf",
            "draw/fonts/Roboto-Bold.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
        };
        for (auto p : boldPaths) {
            if (fontBold.loadFromFile(p)) { haveBold = true; break; }
        }
        for (int i = 0; i < Acid32Panel::POTS; ++i) {
            shown[i] = panel.norm(i);
        }
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
        return CARDS_Y + (wideLayout ? CARD_H : 2.0f * CARD_H + STACK_GAP) + 8.0f;
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
        float y = CARDS_Y + row * (CARD_H + STACK_GAP);
        return { x, y, blockW(), CARD_H };
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
                 r.top + HEAD_S + (slot / PAGE_COLS) * ROW_PITCH + 54.0f };
    }

    sf::FloatRect knobCell(int pot) const
    {
        auto ctr = knobCenter(pot);
        return { ctr.x - cellW() * 0.5f, ctr.y - 40.0f, cellW(), 112.0f };
    }

    static sf::FloatRect playBtn() { return { 236.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect trigBtn() { return { 302.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect bpmBox()  { return { 372.0f, 14.0f, 126.0f, 44.0f }; }
    static sf::FloatRect tuneBox() { return { 506.0f, 14.0f, 126.0f, 44.0f }; }
    static sf::FloatRect presetRect(int i) { return { 660.0f + i * 78.0f, 14.0f, 72.0f, 44.0f }; }
    static sf::FloatRect diceBtn() { return { 660.0f + 6 * 78.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect meterBox() { return { W - 24.0f - 90.0f, 14.0f, 90.0f, 44.0f }; }

    // Piano Roll Coordinates
    static sf::FloatRect pianoRollRect()
    {
        return { CARD_X, ROLL_Y, W - 2.0f * CARD_X, ROLL_H };
    }

    static sf::FloatRect pianoGridRect()
    {
        sf::FloatRect pr = pianoRollRect();
        return { pr.left + 54.0f, pr.top + 34.0f, pr.width - 64.0f, pr.height - 42.0f };
    }

    // Vertical scrollbar for piano roll pitch range
    sf::FloatRect scrollBarRect()
    {
        sf::FloatRect pr = pianoRollRect();
        return { pr.left + pr.width - 12.0f, pr.top + 34.0f, 8.0f, pr.height - 42.0f };
    }

    sf::FloatRect scrollThumbRect()
    {
        sf::FloatRect bar = scrollBarRect();
        int minNote = 12;
        int maxNote = 72;
        int totalNotes = maxNote - minNote + 1;   // 61
        int viewNotes = visibleSemitones;         // 25
        int scrollable = std::max(1, totalNotes - viewNotes);
        float thumbH = std::max(20.0f, bar.height * (float)viewNotes / (float)totalNotes);
        float t = scrollY; // 0..1
        float thumbY = bar.top + t * (bar.height - thumbH);
        return { bar.left, thumbY, bar.width, thumbH };
    }

    int scrollBaseFromY(float y)
    {
        sf::FloatRect bar = scrollBarRect();
        int minNote = 12;
        int maxNote = 72;
        int totalNotes = maxNote - minNote + 1;
        int viewNotes = visibleSemitones;
        int scrollable = std::max(1, totalNotes - viewNotes);
        float t = std::clamp((y - bar.top) / bar.height, 0.0f, 1.0f);
        // Inverted: top of bar = highest notes, bottom = lowest notes
        return std::clamp(maxNote - viewNotes - (int)(t * scrollable), minNote, maxNote - viewNotes);
    }

    // Input Handling
    void mousePressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);

        if (button == sf::Mouse::Left) {
            if (playBtn().contains(p)) { seq.toggle(); return; }
            if (trigBtn().contains(p)) { auditionNote.store(36); return; }
            if (bpmBox().contains(p)) { stepBpm(p.x > bpmBox().left + 63.0f ? 1.0f : -1.0f, shift); return; }
            if (tuneBox().contains(p)) { stepTune(p.x > tuneBox().left + 63.0f ? 1 : -1, shift); return; }

            for (int i = 0; i < (int)presets().size(); ++i) {
                if (presetRect(i).contains(p)) { applyPreset(i); return; }
            }
            if (diceBtn().contains(p)) { randomize(); return; }

            // Check knobs
            for (int pot = 0; pot < Acid32Panel::POTS; ++pot) {
                if (knobCell(pot).contains(p)) {
                    dragging = pot;
                    dragY = p.y;
                    dragStart = panel.norm(pot);
                    lastClicked = pot;
                    return;
                }
            }

            // Check scrollbar (piano roll vertical pitch scroll)
            if (scrollBarRect().contains(p)) {
                draggingScroll = true;
                scrollDragStart = p.y;
                scrollBaseStart = baseMidiNote;
                return;
            }
            if (scrollThumbRect().contains(p)) {
                draggingScroll = true;
                scrollDragStart = p.y;
                scrollBaseStart = baseMidiNote;
                return;
            }

            // Check Quick Edit Bar
            if (lastStepEdit >= 0 && lastStepEdit < AcidSequencer::STEPS) {
                AcidStep& st = seq.steps[lastStepEdit];
                if (editSlideRect.contains(p)) {
                    st.slide = !st.slide;
                    return;
                }
                if (editAccentRect.contains(p)) {
                    st.accent = !st.accent;
                    return;
                }
            }

            // Piano roll click
            sf::FloatRect grid = pianoGridRect();
            if (grid.contains(p)) {
                float cellW = grid.width / 16.0f;
                float cellH = grid.height / (float)visibleSemitones;
                int col = std::clamp((int)((p.x - grid.left) / cellW), 0, 15);
                int stepIdx = (stepPage * 16) + col;
                int noteFromBottom = (int)((grid.top + grid.height - p.y) / cellH);
                int clickedNote = std::clamp(baseMidiNote + noteFromBottom, 0, 127);

                // Check if clicking on an existing note
                int found = -1;
                for (int s = 0; s < AcidSequencer::STEPS; ++s) {
                    const AcidStep& st = seq.steps[s];
                    if (st.on && st.note == clickedNote) {
                        float sStart = (float)(s % 16);
                        float sEnd = sStart + st.length;
                        float clickCol = (p.x - grid.left) / cellW;
                        if ((s / 16 == stepPage) && clickCol >= sStart && clickCol < sEnd) {
                            found = s;
                            break;
                        }
                    }
                }

                if (found != -1) {
                    if (shift) {
                        // Shift+click: toggle slide
                        seq.steps[found].slide = !seq.steps[found].slide;
                    } else {
                        dragStepIdx = found;
                        lastStepEdit = found;
                        // Check if clicking near right edge -> drag length
                        float noteRightX = grid.left + (found % 16 + seq.steps[found].length) * cellW;
                        if (std::abs(p.x - noteRightX) < 14.0f) {
                            draggingLen = true;
                        } else {
                            draggingLen = false;
                            auditionNote.store(seq.steps[found].note);
                        }
                    }
                } else {
                    // Create note
                    seq.steps[stepIdx].on = true;
                    seq.steps[stepIdx].note = clickedNote;
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
            // Right-click knob -> reset
            for (int pot = 0; pot < Acid32Panel::POTS; ++pot) {
                if (knobCell(pot).contains(p)) {
                    panel.reset(pot);
                    return;
                }
            }

            // Right-click piano roll -> delete note or toggle accent
            sf::FloatRect grid = pianoGridRect();
            if (grid.contains(p)) {
                float cellW = grid.width / 16.0f;
                float cellH = grid.height / (float)visibleSemitones;
                int col = std::clamp((int)((p.x - grid.left) / cellW), 0, 15);
                int stepIdx = (stepPage * 16) + col;
                int noteFromBottom = (int)((grid.top + grid.height - p.y) / cellH);
                int clickedNote = std::clamp(baseMidiNote + noteFromBottom, 0, 127);

                for (int s = 0; s < AcidSequencer::STEPS; ++s) {
                    AcidStep& st = seq.steps[s];
                    if (st.on && st.note == clickedNote) {
                        float sStart = (float)(s % 16);
                        float sEnd = sStart + st.length;
                        float clickCol = (p.x - grid.left) / cellW;
                        if ((s / 16 == stepPage) && clickCol >= sStart && clickCol < sEnd) {
                            if (shift) {
                                st.accent = !st.accent;
                            } else {
                                st.on = false; // Delete note!
                                if (lastStepEdit == s) lastStepEdit = -1;
                            }
                            return;
                        }
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
        hoverPot = -1;
        for (int pot = 0; pot < Acid32Panel::POTS; ++pot) {
            if (knobCell(pot).contains(p)) {
                hoverPot = pot;
                break;
            }
        }

        if (draggingScroll) {
            float dy = scrollDragStart - p.y;
            int newBase = scrollBaseFromY(p.y);
            baseMidiNote = newBase;
            // sync scrollY with baseMidiNote (inverted: top = high notes)
            int minNote = 12;
            int maxNote = 72;
            int totalNotes = maxNote - minNote + 1;
            int viewNotes = visibleSemitones;
            int scrollable = std::max(1, totalNotes - viewNotes);
            scrollY = (float)(maxNote - viewNotes - baseMidiNote) / (float)scrollable;
            return;
        }

        if (dragging >= 0) {
            float dy = dragY - p.y;
            float speed = shift ? 0.0008f : 0.004f;
            panel.set(dragging, dragStart + dy * speed);
        }

        if (dragStepIdx >= 0 && dragStepIdx < AcidSequencer::STEPS) {
            AcidStep& st = seq.steps[dragStepIdx];
            sf::FloatRect grid = pianoGridRect();
            float cellW = grid.width / 16.0f;
            float cellH = grid.height / (float)visibleSemitones;

            if (draggingLen) {
                // Adjust note length horizontally only
                float stepLeftX = grid.left + (dragStepIdx % 16) * cellW;
                float currentLen = (p.x - stepLeftX) / cellW;
                st.length = std::clamp(currentLen, 0.2f, 8.0f);
            } else {
                // Drag note pitch vertically only (no length change)
                int noteFromBottom = (int)((grid.top + grid.height - p.y) / cellH);
                int newNote = std::clamp(baseMidiNote + noteFromBottom, 0, 127);
                if (st.note != newNote) {
                    st.note = newNote;
                    auditionNote.store(newNote);
                }
            }
        }
    }

    void mouseScrolled(sf::Vector2f p, float delta, bool shift, bool alt)
    {
        for (int pot = 0; pot < Acid32Panel::POTS; ++pot) {
            if (knobCell(pot).contains(p)) {
                float step = shift ? 0.005f : 0.02f;
                panel.tweak(pot, delta * step);
                return;
            }
        }

        if (bpmBox().contains(p)) {
            stepBpm(delta > 0 ? 1.0f : -1.0f, shift);
            return;
        }

        if (tuneBox().contains(p)) {
            stepTune(delta > 0 ? 1 : -1, shift);
            return;
        }

        // Quick edit bar wheel
        if (lastStepEdit >= 0 && lastStepEdit < AcidSequencer::STEPS) {
            AcidStep& st = seq.steps[lastStepEdit];
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

        // Mouse wheel over piano roll / scrollbar
        sf::FloatRect grid = pianoGridRect();
        sf::FloatRect sbar = scrollBarRect();
        if (sbar.contains(p)) {
            // Wheel over scrollbar scrolls pitch range
            baseMidiNote = std::clamp(baseMidiNote + (delta > 0 ? 1 : -1) * (shift ? 12 : 1), 12, 72 - visibleSemitones);
            int minNote = 12;
            int maxNote = 72;
            int totalNotes = maxNote - minNote + 1;
            int viewNotes = visibleSemitones;
            int scrollable = std::max(1, totalNotes - viewNotes);
            scrollY = (float)(maxNote - viewNotes - baseMidiNote) / (float)scrollable;
            return;
        }
        if (grid.contains(p)) {
            float cellW = grid.width / 16.0f;
            float cellH = grid.height / (float)visibleSemitones;
            float mouseCol = (p.x - grid.left) / cellW;
            int noteFromBottom = (int)((grid.top + grid.height - p.y) / cellH);
            int noteUnderMouse = std::clamp(baseMidiNote + noteFromBottom, 0, 127);

            // Check if hovering an existing note
            for (int s = 0; s < AcidSequencer::STEPS; ++s) {
                AcidStep& st = seq.steps[s];
                if (st.on && st.note == noteUnderMouse) {
                    float sStart = (float)(s % 16);
                    float sEnd = sStart + st.length;
                    if ((s / 16 == stepPage) && mouseCol >= sStart && mouseCol < sEnd) {
                        lastStepEdit = s;
                        if (alt) {
                            st.slide = !st.slide;
                        } else if (shift) {
                            st.length = std::clamp(st.length + (delta > 0 ? 0.1f : -0.1f), 0.1f, 8.0f);
                        } else {
                            st.velocity = std::clamp(st.velocity + (delta > 0 ? 0.05f : -0.05f), 0.05f, 1.0f);
                        }
                        return;
                    }
                }
            }

            // Otherwise, wheel scrolls visible piano roll pitch range
            baseMidiNote = std::clamp(baseMidiNote + (delta > 0 ? 1 : -1) * (shift ? 12 : 1), 12, 72 - visibleSemitones);
            int minNote = 12;
            int maxNote = 72;
            int totalNotes = maxNote - minNote + 1;
            int viewNotes = visibleSemitones;
            int scrollable = std::max(1, totalNotes - viewNotes);
            scrollY = (float)(maxNote - viewNotes - baseMidiNote) / (float)scrollable;
        }
    }

    void keyPressed(sf::Keyboard::Key key)
    {
        if (key == sf::Keyboard::Space) {
            seq.toggle();
        } else if (key == sf::Keyboard::Tab) {
            stepPage = 1 - stepPage;
        } else if (key == sf::Keyboard::D) {
            randomize();
        } else if (key >= sf::Keyboard::Num1 && key <= sf::Keyboard::Num6) {
            applyPreset(key - sf::Keyboard::Num1);
        } else if (lastStepEdit >= 0 && lastStepEdit < AcidSequencer::STEPS) {
            AcidStep& st = seq.steps[lastStepEdit];
            if (key == sf::Keyboard::Up) {
                st.note = (uint8_t)std::clamp((int)st.note + 1, 0, 127);
                auditionNote.store(st.note);
            } else if (key == sf::Keyboard::Down) {
                st.note = (uint8_t)std::clamp((int)st.note - 1, 0, 127);
                auditionNote.store(st.note);
            } else if (key == sf::Keyboard::Left) {
                st.length = std::clamp(st.length - 0.1f, 0.1f, 8.0f);
            } else if (key == sf::Keyboard::Right) {
                st.length = std::clamp(st.length + 0.1f, 0.1f, 8.0f);
            }
        }
    }

    void stepBpm(float d, bool fine) { seq.bpm = std::clamp(seq.bpm + (fine ? 0.1f : 1.0f) * d, 60.0f, 220.0f); }
    void stepTune(int d, bool oct)   { seq.transpose = std::clamp(seq.transpose + (oct ? 12 : 1) * d, -24, 24); }

    void applyPreset(int i)
    {
        if (i < 0 || i >= (int)presets().size()) return;
        const auto& p = presets()[i];
        for (int pot = 0; pot < Acid32Panel::POTS; ++pot) {
            panel.reset(pot);
        }
        for (const auto& kv : p.values) {
            for (int pot = 0; pot < Acid32Panel::POTS; ++pot) {
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
        for (int pot = 0; pot < Acid32Panel::POTS; ++pot) {
            // Keep tuning sane
            if (pot == 0 || pot == 1) continue;
            float r = (float)rand() / (float)RAND_MAX;
            panel.set(pot, r);
        }
        flashPreset = -2;
        flashClock.restart();
    }

    // Drawing
    void wheel(sf::Vector2f p, float delta, bool shift = false, bool alt = false)
    {
        mouseScrolled(p, delta, shift, alt);
    }

    void key(sf::Keyboard::Key k)
    {
        keyPressed(k);
    }

    void draw(sf::RenderTarget& target, sf::Vector2f mouse)
    {
        float dt = frameClock.restart().asSeconds();
        animate(dt);

        target.clear(Theme::bg);

        if (showTopBar) drawTopBar(target, mouse);
        drawScope(target, { CARD_X, SCOPE_Y, panelWidth() - 2.0f * CARD_X, SCOPE_H });

        for (int b = 0; b < PAGES; ++b) {
            drawBlock(target, b);
        }

        for (int pot = 0; pot < Acid32Panel::POTS; ++pot) {
            drawKnob(target, pot, Theme::blockAccent[page(pageOf(pot)).accent]);
        }

        drawFlow(target);
        if (showSequencer) drawSequencer(target, mouse);

        if (hoverPot >= 0 && dragging < 0) {
            drawHelp(target, hoverPot);
        }
    }

private:
    enum Align { LEFT, CENTER, RIGHT };

    static sf::Color alpha(sf::Color c, int a) { return sf::Color(c.r, c.g, c.b, (sf::Uint8)std::clamp(a, 0, 255)); }
    static sf::Color mix(sf::Color a, sf::Color b, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return sf::Color((sf::Uint8)(a.r + (b.r - a.r) * t),
                         (sf::Uint8)(a.g + (b.g - a.g) * t),
                         (sf::Uint8)(a.b + (b.b - a.b) * t),
                         (sf::Uint8)(a.a + (b.a - a.a) * t));
    }

    void label(sf::RenderTarget& t, const std::string& s, float x, float y, int size, sf::Color c, Align a = CENTER, bool bold = false, float spacing = 1.0f)
    {
        bool realBold = bold && haveBold;
        sf::Text txt(s, realBold ? fontBold : font, size);
        txt.setFillColor(c);
        txt.setLetterSpacing(spacing);
        if (bold && !realBold) txt.setStyle(sf::Text::Bold);
        auto b = txt.getLocalBounds();
        float ox = a == LEFT ? b.left : (a == CENTER ? b.left + b.width * 0.5f : b.left + b.width);
        txt.setOrigin(std::round(ox), std::round(b.top + b.height * 0.5f));
        txt.setPosition(std::round(x), std::round(y));
        t.draw(txt);
    }

    float textWidth(const std::string& s, int size, bool bold = false, float spacing = 1.0f)
    {
        bool realBold = bold && haveBold;
        sf::Text txt(s, realBold ? fontBold : font, size);
        txt.setLetterSpacing(spacing);
        if (bold && !realBold) txt.setStyle(sf::Text::Bold);
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

    void animate(float dt)
    {
        float k = 1.0f - std::exp(-dt * 30.0f);
        for (int i = 0; i < Acid32Panel::POTS; ++i) {
            shown[i] += (panel.norm(i) - shown[i]) * k;
        }
        float act[4] = {
            std::abs(cap.livePre) * 0.8f,   // OSCILLATOR
            cap.liveEnvFilter,              // LADDER FILTER
            cap.liveEnvAmp,                 // ENVELOPES
            std::min(1.0f, std::abs(cap.livePost) * 0.8f) // SPATIAL FX
        };
        // page order: OSC, ENV, FILTER, FX
        static const int map[PAGES] = { 0, 2, 1, 3 };
        for (int b = 0; b < PAGES; ++b) {
            glow[b] = std::max(act[map[b]], glow[b] - dt * 2.5f);
        }
    }

    void drawTopBar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "zicStudio", 24.0f, 30.0f, 23, Theme::text, LEFT, true, 0.5f);
        label(t, "Acid32", 24.0f + 116.0f, 30.0f, 23, Theme::acidNeon, LEFT, true, 0.5f);
        label(t, "303 / TEKNO BASS SYNTHESIZER WITH DELAY & REVERB", 24.0f, 52.0f, 9, Theme::muted, LEFT, false, 1.8f);

        // Controls
        rrect(t, playBtn(), 12.0f, seq.playing ? Theme::blockAccent[1] : (playBtn().contains(mouse) ? sf::Color(40, 43, 54) : Theme::card), seq.playing ? Theme::blockAccent[1] : Theme::cardLine);
        label(t, seq.playing ? "STOP" : "PLAY", playBtn().left + playBtn().width * 0.5f, playBtn().top + 22.0f, 11, seq.playing ? sf::Color(14, 14, 18) : Theme::text, CENTER, true);

        rrect(t, trigBtn(), 12.0f, auditionNote.load() >= 0 ? Theme::acidNeon : (trigBtn().contains(mouse) ? sf::Color(40, 43, 54) : Theme::card), Theme::cardLine);
        label(t, "TRIG", trigBtn().left + trigBtn().width * 0.5f, trigBtn().top + 22.0f, 11, auditionNote.load() >= 0 ? sf::Color(14, 14, 18) : Theme::text, CENTER, true);

        // Steppers
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
            sf::Color acc = Theme::blockAccent[i % 4];
            rrect(t, r, 12.0f, lit ? alpha(acc, 200) : (hot ? sf::Color(40, 43, 54) : Theme::card), lit ? acc : Theme::cardLine);
            label(t, presets()[i].name, r.left + r.width * 0.5f, r.top + 22.0f, 9, lit ? sf::Color(14, 14, 18) : (hot ? Theme::text : Theme::muted), CENTER, true, 1.4f);
            label(t, std::to_string(i + 1), r.left + 8.0f, r.top + 8.0f, 7, Theme::faint, LEFT);
        }

        auto d = diceBtn();
        bool lit = flashPreset == -2 && flash < 0.6f;
        rrect(t, d, 12.0f, lit ? Theme::acidNeon : (d.contains(mouse) ? sf::Color(40, 43, 54) : Theme::card), Theme::cardLine);
        label(t, "DICE", d.left + d.width * 0.5f, d.top + 22.0f, 10, lit ? sf::Color(14, 14, 18) : Theme::acidNeon, CENTER, true);

        // Meter Out
        auto m = meterBox();
        rrect(t, m, 12.0f, Theme::card, Theme::cardLine);
        label(t, "OUT", m.left + 10.0f, m.top + 22.0f, 8, Theme::muted, LEFT, false, 2.0f);
        float lvl = std::clamp(std::abs(cap.livePost) * 0.9f, 0.0f, 1.0f);
        sf::FloatRect bar { m.left + 36.0f, m.top + 17.0f, m.width - 46.0f, 10.0f };
        rrect(t, bar, 5.0f, Theme::cell);
        if (lvl > 0.01f) {
            rrect(t, { bar.left, bar.top, bar.width * lvl, bar.height }, 5.0f, lvl > 0.9f ? Theme::blockAccent[3] : Theme::acidNeon);
        }
    }

    void drawScope(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 12.0f, Theme::card, Theme::cardLine);
        float mid = r.top + r.height * 0.5f;
        hline(t, r.left + 10.0f, mid, r.width - 20.0f, sf::Color(255, 255, 255, 15));

        const int N = 256;
        sf::VertexArray va(sf::LineStrip, N);
        int pos = cap.ringPos;
        for (int i = 0; i < N; ++i) {
            int k = (pos + i * (AcidCapture::RING / N)) % AcidCapture::RING;
            float val = std::clamp(cap.ring[k], -1.0f, 1.0f);
            float x = r.left + 12.0f + (r.width - 24.0f) * (float)i / (float)(N - 1);
            float y = mid - val * (r.height * 0.44f);
            va[i] = sf::Vertex({ x, y }, Theme::acidNeon);
        }
        t.draw(va);

        char buf[64];
        snprintf(buf, sizeof(buf), "CUTOFF: %.0f Hz  ·  RESO: %.0f%%  ·  ENV AMP: %.2f", cap.liveCutoff, cap.liveReso, cap.liveEnvAmp);
        label(t, buf, r.left + 14.0f, r.top + 12.0f, 8, Theme::faint, LEFT, false, 1.2f);
        label(t, "STEREO SIGNAL OUTPUT", r.left + r.width - 14.0f, r.top + 12.0f, 8, Theme::faint, RIGHT, false, 1.2f);
    }

    void drawBlock(sf::RenderTarget& t, int b)
    {
        auto r = pageRect(b);
        auto& pg = page(b);
        sf::Color accent = Theme::blockAccent[pg.accent];

        rrect(t, r, 18.0f, Theme::card, mix(Theme::cardLine, accent, glow[b] * 0.5f));
        rrect(t, { r.left + 18.0f, r.top, r.width - 36.0f, 3.0f }, 1.5f, alpha(accent, 120 + (int)(135 * glow[b])));
        if (glow[b] > 0.02f) {
            rrect(t, { r.left + 18.0f, r.top - 2.0f, r.width - 36.0f, 7.0f }, 3.5f, alpha(accent, (int)(60 * glow[b])));
        }

        label(t, pg.name, r.left + 18.0f, r.top + 18.0f, 13, accent, LEFT, true, 3.0f);
        label(t, pg.subtitle, r.left + 18.0f, r.top + 33.0f, 9, Theme::muted, LEFT, false, 1.0f);

        // Group capsules
        for (int g = 0; g < Acid32Panel::GROUPS; ++g) {
            auto& grp = Acid32Panel::group(g);
            if (pageOf(grp.firstPot) != b) continue;
            int slot = pageSlot(grp.firstPot);
            float x = r.left + (slot % PAGE_COLS) * cellW();
            float y = r.top + HEAD_S + (slot / PAGE_COLS) * ROW_PITCH;
            sf::FloatRect pill { x + 6.0f, y + 6.0f, grp.count * cellW() - 12.0f, ROW_PITCH - 16.0f };
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
        bool centered = panel.isCentered(pot);
        const float R = 27.0f; // EXACT SAME RADIUS AS zicStudioKick!
        auto angle = [](float v) { return -135.0f + 270.0f * v; };

        disc(t, ctr.x, ctr.y, 36.0f, sf::Color(20, 22, 28));
        arc(t, ctr.x, ctr.y, R, 5.0f, -135.0f, 135.0f, Theme::track);

        // Default position tick
        {
            float a = angle(panel.defaultNorm(pot)) * 3.14159265f / 180.0f;
            disc(t, ctr.x + (R + 7.5f) * std::sin(a), ctr.y - (R + 7.5f) * std::cos(a), 1.5f, Theme::faint);
        }

        float from = centered ? 0.0f : -135.0f;
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

    void drawFlow(sf::RenderTarget& t)
    {
        auto chevron = [&](float x, float y, bool down, sf::Color c) {
            sf::ConvexShape s(3);
            if (down) {
                s.setPoint(0, { x - 9.0f, y - 6.0f });
                s.setPoint(1, { x, y + 6.0f });
                s.setPoint(2, { x + 9.0f, y - 6.0f });
            } else {
                s.setPoint(0, { x - 6.0f, y - 9.0f });
                s.setPoint(1, { x + 6.0f, y });
                s.setPoint(2, { x - 6.0f, y + 9.0f });
            }
            s.setFillColor(c);
            t.draw(s);
        };

        if (wideLayout) {
            for (int p = 0; p + 1 < PAGES; ++p) {
                auto r = pageRect(p);
                chevron(r.left + r.width + gap() * 0.5f, r.top + CARD_H * 0.5f, false,
                        alpha(Theme::blockAccent[page(p + 1).accent], 180));
            }
        } else {
            for (int p = 0; p + 1 < PAGES; p += 2) {
                auto r = pageRect(p);
                chevron(r.left + r.width * 0.5f, r.top + CARD_H + STACK_GAP * 0.5f, true,
                        alpha(Theme::blockAccent[page(p + 1).accent], 180));
            }
        }
    }

    void drawHelp(sf::RenderTarget& t, int pot)
    {
        auto ctr = knobCenter(pot);
        std::string text = Acid32Panel::help(pot);
        sf::Color accent = Theme::blockAccent[page(pageOf(pot)).accent];

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
        float x = std::clamp(ctr.x - w * 0.5f, 10.0f, panelWidth() - w - 10.0f);
        float y = ctr.y + 70.0f;
        if (y + h > panelBottom() - 4.0f) y = ctr.y - 48.0f - h;

        rrect(t, { x, y, w, h }, 8.0f, sf::Color(28, 30, 40, 245), alpha(accent, 140));
        for (size_t i = 0; i < lines.size(); ++i) {
            label(t, lines[i], x + 14.0f, y + 14.0f + 16.0f * i, 11, Theme::text, LEFT);
        }
    }

    // =========================================================================
    // PIANO ROLL SEQUENCER (zic23 model: velocity in note body, direct mouse)
    // =========================================================================
    void drawSequencer(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        sf::FloatRect pr = pianoRollRect();
        rrect(t, pr, 16.0f, Theme::card, Theme::cardLine);

        // Header
        label(t, "ACID PIANO ROLL", pr.left + 18.0f, pr.top + 18.0f, 12, Theme::acidNeon, LEFT, true, 2.5f);

        // Page selector tabs (Steps 1-16 / Steps 17-32)
        sf::FloatRect page0 { pr.left + 160.0f, pr.top + 7.0f, 68.0f, 22.0f };
        sf::FloatRect page1 { pr.left + 232.0f, pr.top + 7.0f, 68.0f, 22.0f };
        rrect(t, page0, 6.0f, stepPage == 0 ? Theme::acidNeon : Theme::cell, Theme::cardLine);
        label(t, "1 - 16", page0.left + 34.0f, page0.top + 11.0f, 9, stepPage == 0 ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);
        rrect(t, page1, 6.0f, stepPage == 1 ? Theme::acidNeon : Theme::cell, Theme::cardLine);
        label(t, "17 - 32", page1.left + 34.0f, page1.top + 11.0f, 9, stepPage == 1 ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);

        // Inspector info / Quick Edit Bar
        if (lastStepEdit >= 0 && lastStepEdit < AcidSequencer::STEPS) {
            const AcidStep& s = seq.steps[lastStepEdit];
            char buf[64];
            snprintf(buf, sizeof(buf), "STEP %d: %s (MIDI %d)  LEN: %.1f  VEL: %d%%",
                     lastStepEdit + 1, getMidiNoteName(s.note), s.note, s.length, (int)(s.velocity * 100.0f));

            float barX = pr.left + 320.0f;
            label(t, buf, barX, pr.top + 18.0f, 10, Theme::text, LEFT, true);

            // Quick toggle badges
            editSlideRect = { barX + 280.0f, pr.top + 7.0f, 54.0f, 22.0f };
            rrect(t, editSlideRect, 6.0f, s.slide ? Theme::slideNote : Theme::cell, Theme::cardLine);
            label(t, "SLIDE", editSlideRect.left + 27.0f, editSlideRect.top + 11.0f, 8, s.slide ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);

            editAccentRect = { barX + 340.0f, pr.top + 7.0f, 58.0f, 22.0f };
            rrect(t, editAccentRect, 6.0f, s.accent ? Theme::accentNote : Theme::cell, Theme::cardLine);
            label(t, "ACCENT", editAccentRect.left + 29.0f, editAccentRect.top + 11.0f, 8, s.accent ? sf::Color(14, 14, 18) : Theme::muted, CENTER, true);
        } else {
            label(t, "Click to place note · Drag note for pitch & length · Wheel adjusts velocity · Shift+Click slide · Right-click delete",
                  pr.left + 320.0f, pr.top + 18.0f, 9, Theme::faint, LEFT, false, 0.8f);
        }

        // Help hints at right
        label(t, "TAB: toggle 16/32 steps  ·  SPACE: play", pr.left + pr.width - 16.0f, pr.top + 18.0f, 9, Theme::faint, RIGHT, false, 0.8f);

        // Piano Grid
        sf::FloatRect grid = pianoGridRect();
        rrect(t, grid, 8.0f, sf::Color(13, 14, 18), Theme::cardLine);

        float cellW = grid.width / 16.0f;
        float cellH = grid.height / (float)visibleSemitones;

        // Draw Piano Keys on left & grid pitch lines
        for (int row = 0; row < visibleSemitones; ++row) {
            int midi = baseMidiNote + (visibleSemitones - 1 - row);
            float y = grid.top + row * cellH;
            bool isBlack = isBlackKey(midi);
            bool isC = (midi % 12) == 0;

            // Pitch row background
            if (isBlack) {
                sf::RectangleShape bgRow({ grid.width, cellH });
                bgRow.setPosition(grid.left, y);
                bgRow.setFillColor(sf::Color(18, 19, 25));
                t.draw(bgRow);
            }

            // Horizontal dividing line
            hline(t, grid.left, y + cellH, grid.width, isC ? sf::Color(255, 255, 255, 25) : sf::Color(255, 255, 255, 8));

            // Key label
            std::string name = getMidiNoteName(midi);
            label(t, name, grid.left - 6.0f, y + cellH * 0.5f, isC ? 9 : 8, isC ? Theme::acidNeon : (isBlack ? Theme::faint : Theme::muted), RIGHT, isC);
        }

        // Draw vertical 16th step lines
        int curPlayStep = seq.currentStep.load();
        for (int col = 0; col < 16; ++col) {
            float x = grid.left + col * cellW;
            bool isDownbeat = (col % 4) == 0;
            vline(t, x, grid.top, grid.height, isDownbeat ? sf::Color(255, 255, 255, 25) : sf::Color(255, 255, 255, 8));

            int stepNum = (stepPage * 16) + col;
            if (stepNum == curPlayStep) {
                // Playhead highlight
                sf::RectangleShape playCol({ cellW, grid.height });
                playCol.setPosition(x, grid.top);
                playCol.setFillColor(sf::Color(230, 255, 40, 30));
                t.draw(playCol);
            }
        }

        // DRAW NOTES (zic23 pattern: velocity fill inside note body!)
        int startStep = stepPage * 16;
        int endStep = startStep + 16;

        for (int s = 0; s < AcidSequencer::STEPS; ++s) {
            const AcidStep& step = seq.steps[s];
            if (!step.on) continue;

            // Check if note is visible on current 16-step page
            float stepCol = (float)(s - startStep);
            if (stepCol + step.length <= 0.0f || stepCol >= 16.0f) continue;

            int noteOffset = step.note - baseMidiNote;
            if (noteOffset < 0 || noteOffset >= visibleSemitones) continue;

            float row = (float)(visibleSemitones - 1 - noteOffset);
            float nX = grid.left + stepCol * cellW + 1.0f;
            float nY = grid.top + row * cellH + 1.0f;
            float nW = std::max(6.0f, step.length * cellW - 2.0f);
            float totalH = std::max(4.0f, cellH - 2.0f);

            // Clip horizontally to grid if spanning across boundaries
            if (nX < grid.left) {
                nW -= (grid.left - nX);
                nX = grid.left;
            }
            if (nX + nW > grid.left + grid.width) {
                nW = (grid.left + grid.width) - nX;
            }

            // Colors based on slide & accent
            sf::Color themeColor = Theme::acidNeon;
            if (step.accent) themeColor = Theme::accentNote;
            else if (step.slide) themeColor = Theme::slideNote;

            // Note body dimmed background (alpha = 80)
            rrect(t, { nX, nY, nW, totalH }, 3.0f, alpha(themeColor, 70));

            // Velocity fill (height proportional to velocity)
            float veloH = std::clamp(totalH * step.velocity, 2.0f, totalH);
            float offsetH = totalH - veloH;
            rrect(t, { nX, nY + offsetH, nW, veloH }, 3.0f, themeColor);

            // Note outline
            bool isSelected = (lastStepEdit == s);
            rrect(t, { nX, nY, nW, totalH }, 3.0f, sf::Color::Transparent, isSelected ? sf::Color::White : alpha(themeColor, 180));

            // Slide arrow indicator
            if (step.slide && nW > 18.0f) {
                label(t, ">", nX + nW - 8.0f, nY + totalH * 0.5f, 9, sf::Color(14, 14, 18), CENTER, true);
            }
        }

        // ---- Vertical scrollbar for piano roll pitch range ----
        // Sync scrollY with current baseMidiNote (inverted: top = high notes)
        {
            int minNote = 12;
            int maxNote = 72;
            int totalNotes = maxNote - minNote + 1;
            int viewNotes = visibleSemitones;
            int scrollable = std::max(1, totalNotes - viewNotes);
            scrollY = (float)(maxNote - viewNotes - baseMidiNote) / (float)scrollable;
        }
        sf::FloatRect bar = scrollBarRect();
        sf::FloatRect thumb = scrollThumbRect();
        rrect(t, bar, 4.0f, sf::Color(20, 22, 28), Theme::cardLine);
        rrect(t, thumb, 4.0f, Theme::acidNeon, Theme::acidNeon);
        // Scrollbar track ticks
        {
            int minNote = 12;
            int maxNote = 72;
            int totalNotes = maxNote - minNote + 1;
            int viewNotes = visibleSemitones;
            int scrollable = std::max(1, totalNotes - viewNotes);
            for (int n = minNote; n <= maxNote - viewNotes; n += 12) {
                float tpos = (float)(n - minNote) / (float)scrollable;
                float ty = bar.top + tpos * bar.height;
                hline(t, bar.left, ty, bar.width, sf::Color(255, 255, 255, 30), 1.0f);
                char lbl[16];
                snprintf(lbl, sizeof(lbl), "%d", n);
                label(t, lbl, bar.left + bar.width + 4.0f, ty, 7, Theme::faint, LEFT);
            }
        }
    }

    static bool isBlackKey(int midi)
    {
        int k = midi % 12;
        return (k == 1 || k == 3 || k == 6 || k == 8 || k == 10);
    }
};

} // namespace acid_ui
