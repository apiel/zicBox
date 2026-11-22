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

#include "AcidSequencer.h"
#include "core/Acid32Panel.h"

extern std::mutex audioMutex;
extern std::atomic<int> auditionNote;
extern std::atomic<bool> acidMuted;

// What the audio thread records for the UI
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

// Block accents: 0: OSC (Amber), 1: LFO/PITCH (Violet), 2: DUAL FILTER (Cyan), 3: AMP/FX (Red)
const sf::Color blockAccent[4] = {
    rgb(255, 176, 64),   // OSC (amber)
    rgb(172, 128, 255),  // LFO & PITCH (violet)
    rgb(56, 212, 255),   // DUAL FILTER (cyan)
    rgb(255, 86, 98)     // AMP & FX (red)
};

const sf::Color acidNeon = rgb(230, 255, 40); // Acid yellow
const sf::Color accentNote = rgb(255, 86, 98);
const sf::Color slideNote = rgb(56, 212, 255);
}

class EmuUi {
public:
    static constexpr float W = 1320.0f;
    static constexpr float H = 1236.0f;
    static constexpr float CARD_X = 24.0f, CELL_W = 150.0f, GAP = 36.0f;
    static constexpr float SCOPE_Y = 78.0f, SCOPE_H = 240.0f;
    static constexpr float CARDS_Y = 338.0f, ROW_PITCH = 130.0f, HEAD_S = 46.0f;
    static constexpr float CARD_H = HEAD_S + 2 * ROW_PITCH + 8.0f; // 2 rows x 4 cols per block
    static constexpr float ROLL_Y = CARDS_Y + 2 * CARD_H + 24.0f; // Piano roll at bottom
    static constexpr float ROLL_H = H - ROLL_Y - 26.0f; // Remaining height for roll & velocity

    struct Preset {
        const char* name;
        std::vector<std::pair<const char*, float>> values;
    };

    static const std::vector<Preset>& presets()
    {
        static const std::vector<Preset> p = {
            { "INIT 303", { { "color", 33 }, { "cutoff", 35 }, { "reso", 65 }, { "envMod", 65 }, { "fDecay", 260 }, { "accent", 80 }, { "decay", 350 }, { "sustain", 60 } } },
            { "DEVILFISH", { { "color", 50 }, { "cutoff", 30 }, { "reso", 88 }, { "drive", 70 }, { "dist", 45 }, { "distType", 0 }, { "fDecay", 170 }, { "glide", 65 }, { "hpfCut", 40 } } },
            { "HARDTEK UNISON", { { "color", 33 }, { "unison", 3 }, { "detune", 20 }, { "sub", 60 }, { "cutoff", 55 }, { "reso", 70 }, { "drive", 80 }, { "dist", 65 }, { "distType", 1 } } },
            { "LFO SQUELCH", { { "color", 40 }, { "cutoff", 40 }, { "reso", 80 }, { "lfoRate", 3.5f }, { "lfoToCut", 60 }, { "lfoToWave", 45 }, { "envMod", 50 } } },
            { "WAVEFOLD BASS", { { "color", 90 }, { "cutoff", 48 }, { "reso", 60 }, { "pDepth", 12 }, { "pDecay", 30 }, { "dist", 75 }, { "distType", 2 } } },
            { "METALLIC RING", { { "color", 33 }, { "cutoff", 60 }, { "reso", 75 }, { "ringMod", 65 }, { "ringRatio", 3.5f }, { "fDecay", 320 } } }
        };
        return p;
    }

    Acid32Panel::Panel& panel;
    AcidSequencer& seq;
    AcidCapture& cap;

    sf::Font font;
    int dragging = -1, hoverPot = -1, lastClicked = -1;
    float dragY = 0.0f, dragStart = 0.0f;

    // Piano roll interaction
    int draggingNoteStep = -1;
    bool resizingLength = false;
    bool draggingVelocity = false;
    int baseMidiNote = 24; // C1 at bottom
    int visibleSemitones = 25; // 2 octaves + 1 note (C1 to C3)
    int stepPage = 0; // 0: steps 0..15, 1: steps 16..31

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
        for (int i = 0; i < Acid32Panel::POTS; ++i) {
            shown[i] = panel.norm(i);
        }
    }

    // ------------------------------------------------------------------ layout helpers
    static float colX(int col) { return CARD_X + col * CELL_W + (col >= 4 ? GAP : 0.0f); }
    static sf::FloatRect blockRect(int b)
    {
        auto& blk = Acid32Panel::block(b);
        float y = CARDS_Y + (blk.row == 2 ? (CARD_H + 16.0f) : 0.0f);
        return { colX(blk.col), y, blk.cols * CELL_W, CARD_H };
    }
    static float rowY(int pot)
    {
        int b = Acid32Panel::blockOf(pot);
        auto& blk = Acid32Panel::block(b);
        return blockRect(b).top + HEAD_S + (Acid32Panel::potRow(pot) - blk.row) * ROW_PITCH;
    }
    static sf::Vector2f knobCenter(int pot)
    {
        return { colX(Acid32Panel::potCol(pot)) + CELL_W * 0.5f, rowY(pot) + 52.0f };
    }
    static sf::FloatRect knobCell(int pot)
    {
        auto ctr = knobCenter(pot);
        return { ctr.x - CELL_W * 0.5f, ctr.y - 40.0f, CELL_W, 110.0f };
    }

    // Top buttons
    static sf::FloatRect playBtn() { return { CARD_X + 230.0f, 16.0f, 44.0f, 44.0f }; }
    static sf::FloatRect trigBtn() { return { CARD_X + 282.0f, 16.0f, 44.0f, 44.0f }; }
    static sf::FloatRect bpmBox()  { return { CARD_X + 338.0f, 16.0f, 126.0f, 44.0f }; }
    static sf::FloatRect tuneBox() { return { CARD_X + 472.0f, 16.0f, 114.0f, 44.0f }; }
    static sf::FloatRect diceBtn() { return { CARD_X + 594.0f, 16.0f, 44.0f, 44.0f }; }
    static sf::FloatRect pageBtn() { return { CARD_X + 646.0f, 16.0f, 68.0f, 44.0f }; }
    static sf::FloatRect presetRect(int i) { return { W - CARD_X - (6 - i) * 98.0f, 16.0f, 90.0f, 44.0f }; }

    // ------------------------------------------------------------------ input
    void mousePressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        if (button == sf::Mouse::Left) {
            if (playBtn().contains(p)) { seq.toggle(); return; }
            if (trigBtn().contains(p)) { auditionNote = 36 + seq.transpose; return; }
            if (bpmBox().contains(p))  { seq.bpm = std::clamp(seq.bpm + (p.x > bpmBox().left + 63 ? 1.0f : -1.0f) * (shift ? 10.0f : 1.0f), 60.0f, 300.0f); return; }
            if (tuneBox().contains(p)) { seq.transpose = std::clamp(seq.transpose + (p.x > tuneBox().left + 57 ? 1 : -1) * (shift ? 12 : 1), -24, 24); return; }
            if (diceBtn().contains(p)) { dice(); return; }
            if (pageBtn().contains(p)) { stepPage = (stepPage == 0) ? 1 : 0; return; }

            for (int i = 0; i < (int)presets().size(); ++i) {
                if (presetRect(i).contains(p)) { applyPreset(i); return; }
            }

            // Piano roll click check
            handlePianoRollMouse(p, button, shift, true);
        } else if (button == sf::Mouse::Right) {
            handlePianoRollMouse(p, button, shift, true);
        }

        if (button != sf::Mouse::Left) return;

        // Knobs
        for (int i = 0; i < Acid32Panel::POTS; ++i) {
            if (!knobCell(i).contains(p)) continue;
            if (clock.getElapsedTime().asMilliseconds() < 350 && lastClicked == i) {
                panel.reset(i);
                dragging = -1;
            } else {
                dragging = i;
                dragY = p.y;
                dragStart = panel.norm(i);
            }
            lastClicked = i;
            clock.restart();
            return;
        }
    }

    void mouseReleased()
    {
        dragging = -1;
        draggingNoteStep = -1;
        resizingLength = false;
        draggingVelocity = false;
    }

    void mouseMoved(sf::Vector2f p, bool shift)
    {
        hoverPot = -1;
        for (int i = 0; i < Acid32Panel::POTS; ++i) {
            if (knobCell(i).contains(p)) hoverPot = i;
        }

        std::lock_guard<std::mutex> lock(audioMutex);
        if (dragging >= 0) {
            panel.set(dragging, dragStart + (dragY - p.y) / (shift ? 900.0f : 220.0f));
        } else if (draggingNoteStep >= 0) {
            handlePianoRollMouse(p, sf::Mouse::Left, shift, false);
        }
    }

    void wheel(sf::Vector2f p, float delta, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        for (int i = 0; i < Acid32Panel::POTS; ++i) {
            if (!knobCell(i).contains(p)) continue;
            panel.set(i, panel.norm(i) + (delta > 0 ? 1 : -1) * (shift ? 0.002f : 0.01f));
            return;
        }
        if (bpmBox().contains(p)) {
            seq.bpm = std::clamp(seq.bpm + (delta > 0 ? 1.0f : -1.0f) * (shift ? 10.0f : 1.0f), 60.0f, 300.0f);
        } else if (tuneBox().contains(p)) {
            seq.transpose = std::clamp(seq.transpose + (delta > 0 ? 1 : -1) * (shift ? 12 : 1), -24, 24);
        } else if (p.y >= ROLL_Y) {
            // Scroll piano roll pitch window
            baseMidiNote = std::clamp(baseMidiNote + (delta > 0 ? 1 : -1) * (shift ? 12 : 1), 12, 60);
        }
    }

    void key(sf::Keyboard::Key k)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        if (k >= sf::Keyboard::Num1 && k <= sf::Keyboard::Num6) applyPreset(k - sf::Keyboard::Num1);
        else if (k == sf::Keyboard::D) dice();
        else if (k == sf::Keyboard::Tab) stepPage = (stepPage == 0) ? 1 : 0;
        else if (k == sf::Keyboard::Up) baseMidiNote = std::min(baseMidiNote + 12, 60);
        else if (k == sf::Keyboard::Down) baseMidiNote = std::max(baseMidiNote - 12, 12);
    }

    void applyPreset(int i)
    {
        for (int p = 0; p < Acid32Panel::POTS; ++p) panel.reset(p);
        for (auto& kv : presets()[i].values) {
            for (int p = 0; p < Acid32Panel::POTS; ++p) {
                if (!strcmp(panel.param(p).key, kv.first)) panel.param(p).set(kv.second);
            }
        }
        flashPreset = i;
        flashClock.restart();
        if (!seq.playing) auditionNote = 36 + seq.transpose;
    }

    void dice()
    {
        auto r = [&]() { return (float)rand() / RAND_MAX; };
        for (int p = 0; p < Acid32Panel::POTS; ++p) {
            const char* k = panel.param(p).key;
            float v;
            if (!strcmp(k, "cutoff")) v = 0.25f + 0.5f * r();
            else if (!strcmp(k, "reso")) v = 0.4f + 0.5f * r();
            else if (!strcmp(k, "drive")) v = 0.3f + 0.6f * r();
            else if (!strcmp(k, "envMod")) v = 0.4f + 0.5f * r();
            else if (!strcmp(k, "accent")) v = 0.5f + 0.5f * r();
            else if (!strcmp(k, "fDecay") || !strcmp(k, "decay")) v = 0.1f + 0.4f * r();
            else if (panel.isCentered(p)) v = 0.5f + (r() - 0.5f) * 0.8f;
            else v = r() * (r() < 0.4f ? 0.0f : 1.0f);
            panel.set(p, v);
        }
        flashPreset = -2;
        flashClock.restart();
        if (!seq.playing) auditionNote = 36 + seq.transpose;
    }

    // ------------------------------------------------------------------ drawing
    void draw(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        float dt = frameClock.restart().asSeconds();
        dt = std::min(dt, 0.1f);
        animate(dt);

        t.clear(Theme::bg);

        drawTopBar(t, mouse);
        drawScopeAndAnalyzer(t, { CARD_X, SCOPE_Y, W - 2 * CARD_X, SCOPE_H });

        for (int b = 0; b < Acid32Panel::BLOCKS; ++b) drawBlock(t, b);
        drawFlowChevrons(t);
        for (int i = 0; i < Acid32Panel::POTS; ++i) drawKnob(t, i, Theme::blockAccent[Acid32Panel::blockOf(i)]);

        drawPianoRoll(t, mouse);

        if (hoverPot >= 0 && dragging < 0) drawHelp(t, hoverPot);
    }

private:
    void animate(float dt)
    {
        float k = 1.0f - std::exp(-dt * 30.0f);
        for (int i = 0; i < Acid32Panel::POTS; ++i) shown[i] += (panel.norm(i) - shown[i]) * k;
        float act[4] = {
            cap.liveEnvAmp,
            std::abs(cap.liveLfoVal),
            cap.liveEnvFilter,
            std::min(1.0f, std::abs(cap.livePost) * 1.5f)
        };
        for (int b = 0; b < 4; ++b) glow[b] = std::max(act[b], glow[b] - dt * 2.5f);
    }

    // Primitives
    enum Align { LEFT, CENTER, RIGHT };
    static sf::Color alpha(sf::Color c, int a) { return sf::Color(c.r, c.g, c.b, (sf::Uint8)std::clamp(a, 0, 255)); }
    static sf::Color mix(sf::Color a, sf::Color b, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return sf::Color((sf::Uint8)(a.r + (b.r - a.r) * t), (sf::Uint8)(a.g + (b.g - a.g) * t), (sf::Uint8)(a.b + (b.b - a.b) * t), (sf::Uint8)(a.a + (b.a - a.a) * t));
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

    void rrect(sf::RenderTarget& t, sf::FloatRect r, float rad, sf::Color fill, sf::Color line = sf::Color::Transparent)
    {
        sf::ConvexShape s;
        const int seg = 5;
        s.setPointCount(4 * (seg + 1));
        const float cx[4] = { r.left + r.width - rad, r.left + r.width - rad, r.left + rad, r.left + rad };
        const float cy[4] = { r.top + rad, r.top + r.height - rad, r.top + r.height - rad, r.top + rad };
        int k = 0;
        for (int c = 0; c < 4; ++c)
            for (int i = 0; i <= seg; ++i) {
                float a = ((float)c * 90.0f + (float)i * 90.0f / seg) * 3.14159265f / 180.0f - 1.5707963f;
                s.setPoint(k++, { cx[c] + rad * std::cos(a), cy[c] + rad * std::sin(a) });
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
        sf::CircleShape s(r, 28);
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

    void curve(sf::RenderTarget& t, const std::vector<sf::Vector2f>& pts, sf::Color c, float th = 2.0f)
    {
        if (pts.size() < 2) return;
        sf::VertexArray va(sf::TriangleStrip, pts.size() * 2);
        for (size_t i = 0; i < pts.size(); ++i) {
            sf::Vector2f d = i + 1 < pts.size() ? pts[i + 1] - pts[i] : pts[i] - pts[i - 1];
            float len = std::max(1e-3f, std::sqrt(d.x * d.x + d.y * d.y));
            sf::Vector2f n(-d.y / len * th * 0.5f, d.x / len * th * 0.5f);
            va[2 * i] = sf::Vertex(pts[i] + n, c);
            va[2 * i + 1] = sf::Vertex(pts[i] - n, c);
        }
        t.draw(va);
    }

    void fillCurve(sf::RenderTarget& t, const std::vector<sf::Vector2f>& pts, float baseY, sf::Color top, sf::Color bottom)
    {
        if (pts.size() < 2) return;
        sf::VertexArray va(sf::TriangleStrip, pts.size() * 2);
        for (size_t i = 0; i < pts.size(); ++i) {
            va[2 * i] = sf::Vertex(pts[i], top);
            va[2 * i + 1] = sf::Vertex({ pts[i].x, baseY }, bottom);
        }
        t.draw(va);
    }

    static void fft(std::vector<std::complex<float>>& a)
    {
        int n = (int)a.size();
        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) std::swap(a[i], a[j]);
        }
        for (int len = 2; len <= n; len <<= 1) {
            float ang = -2.0f * 3.14159265f / len;
            std::complex<float> wl(std::cos(ang), std::sin(ang));
            for (int i = 0; i < n; i += len) {
                std::complex<float> w(1, 0);
                for (int j = 0; j < len / 2; ++j) {
                    auto u = a[i + j], v = a[i + j + len / 2] * w;
                    a[i + j] = u + v;
                    a[i + j + len / 2] = u - v;
                    w *= wl;
                }
            }
        }
    }

    // ------------------------------------------------------------------ Top Bar
    void drawTopBar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "zicStudio", 24, 28, 24, Theme::text, LEFT, true, 0.5f);
        label(t, "Acid32", 24 + 104, 28, 24, Theme::acidNeon, LEFT, true, 0.5f);
        label(t, "ACID / TEKNO LEAD BASS SYNTH", 24, 52, 9, Theme::muted, LEFT, false, 2.0f);

        // Play/Stop
        bool pHot = playBtn().contains(mouse);
        rrect(t, playBtn(), 12.0f, seq.playing ? Theme::acidNeon : (pHot ? sf::Color(40, 43, 54) : Theme::card), Theme::cardLine);
        label(t, seq.playing ? "STOP" : "PLAY", playBtn().left + 22.0f, playBtn().top + 22.0f, 10, seq.playing ? Theme::bg : Theme::text, CENTER, true);

        // Trig Audition
        bool tHot = trigBtn().contains(mouse);
        rrect(t, trigBtn(), 12.0f, tHot ? sf::Color(40, 43, 54) : Theme::card, Theme::cardLine);
        label(t, "TRIG", trigBtn().left + 22.0f, trigBtn().top + 22.0f, 10, Theme::acidNeon, CENTER, true);

        // BPM & Tune Steppers
        auto stepper = [&](sf::FloatRect b, const char* v, const char* cap) {
            rrect(t, b, 12.0f, Theme::card, Theme::cardLine);
            label(t, v, b.left + 48, b.top + 22, 20, Theme::text, CENTER, true, 0.5f);
            label(t, cap, b.left + 74, b.top + 24, 9, Theme::muted, LEFT, false, 1.5f);
            label(t, "-", b.left + 14, b.top + 21, 18, Theme::muted);
            label(t, "+", b.left + b.width - 14, b.top + 21, 18, Theme::muted);
        };
        char bBuf[16], tBuf[16];
        snprintf(bBuf, sizeof(bBuf), "%d", (int)std::round(seq.bpm));
        stepper(bpmBox(), bBuf, "BPM");
        snprintf(tBuf, sizeof(tBuf), "%+d", seq.transpose);
        stepper(tuneBox(), tBuf, "TUNE");

        // Dice (Randomize)
        bool dHot = diceBtn().contains(mouse);
        rrect(t, diceBtn(), 12.0f, dHot ? sf::Color(40, 43, 54) : Theme::card, Theme::cardLine);
        label(t, "DICE", diceBtn().left + 22.0f, diceBtn().top + 22.0f, 9, Theme::blockAccent[1], CENTER, true);

        // Page selector (1-16 / 17-32)
        bool pgHot = pageBtn().contains(mouse);
        rrect(t, pageBtn(), 12.0f, pgHot ? sf::Color(40, 43, 54) : Theme::card, Theme::cardLine);
        char pgBuf[16];
        snprintf(pgBuf, sizeof(pgBuf), "%s", stepPage == 0 ? "P 1-16" : "P 17-32");
        label(t, pgBuf, pageBtn().left + 34.0f, pageBtn().top + 22.0f, 10, Theme::text, CENTER, true);

        // Presets
        float flash = flashClock.getElapsedTime().asSeconds();
        for (int i = 0; i < (int)presets().size(); ++i) {
            auto r = presetRect(i);
            bool hot = r.contains(mouse);
            bool lit = flashPreset == i && flash < 0.6f;
            sf::Color acc = Theme::blockAccent[i % 4];
            rrect(t, r, 12.0f, lit ? alpha(acc, 200) : (hot ? sf::Color(40, 43, 54) : Theme::card), lit ? acc : Theme::cardLine);
            label(t, presets()[i].name, r.left + r.width * 0.5f, r.top + 22, 9, lit ? sf::Color(14, 14, 18) : (hot ? Theme::text : Theme::muted), CENTER, true, 1.2f);
        }
    }

    // ------------------------------------------------------------------ Scope & Analyzer
    void drawScopeAndAnalyzer(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 14.0f, Theme::card, Theme::cardLine);

        // Left section: Oscilloscope (width: 820px)
        float scopeW = 820.0f;
        sf::FloatRect sBox { r.left + 14.0f, r.top + 34.0f, scopeW, r.height - 48.0f };
        rrect(t, sBox, 8.0f, sf::Color(0, 0, 0, 90), Theme::cardLine);

        label(t, "LIVE OSCILLOSCOPE", sBox.left + 12.0f, r.top + 18.0f, 10, Theme::text, LEFT, true, 2.0f);
        label(t, "OUTPUT ANALYZER", r.left + scopeW + 40.0f, r.top + 18.0f, 10, Theme::text, LEFT, true, 2.0f);

        // Scope center line
        float midY = sBox.top + sBox.height * 0.5f;
        hline(t, sBox.left, midY, sBox.width, sf::Color(255, 255, 255, 15));

        // Waveform plot
        int N = AcidCapture::RING;
        int step = N / (int)sBox.width;
        std::vector<sf::Vector2f> wavePts;
        for (int x = 0; x < (int)sBox.width; ++x) {
            int idx = (cap.ringPos + x * step) % N;
            float s = cap.ring[idx];
            wavePts.push_back({ sBox.left + x, midY - s * (sBox.height * 0.42f) });
        }
        curve(t, wavePts, Theme::acidNeon, 1.8f);

        // Right section: Real-time Spectrum Analyzer
        sf::FloatRect aBox { r.left + scopeW + 28.0f, r.top + 34.0f, r.width - scopeW - 42.0f, r.height - 48.0f };
        drawSpectrum(t, aBox);
    }

    void drawSpectrum(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 8.0f, sf::Color(0, 0, 0, 90), Theme::cardLine);
        const int N = 2048;
        static std::vector<std::complex<float>> buf(N);
        int pos = cap.ringPos;
        for (int i = 0; i < N; ++i) {
            float w = 0.5f - 0.5f * std::cos(2.0f * 3.14159265f * i / (N - 1));
            buf[i] = cap.ring[(pos + i) % AcidCapture::RING] * w;
        }
        fft(buf);

        const int COLS = 160;
        std::vector<sf::Vector2f> pts, hold;
        auto fy = [&](float db) { return r.top + r.height - 6 - std::clamp((db + 72.0f) / 72.0f, 0.0f, 1.0f) * (r.height - 18); };

        for (int c = 0; c < COLS; ++c) {
            float f0 = 20.0f * std::pow(16000.0f / 20.0f, (float)c / COLS);
            float f1 = 20.0f * std::pow(16000.0f / 20.0f, (float)(c + 1) / COLS);
            int k0 = std::max(1, (int)(f0 / 44100.0f * N));
            int k1 = std::max(k0 + 1, (int)(f1 / 44100.0f * N) + 1);
            float p = 0;
            for (int k = k0; k < k1 && k < N / 2; ++k) p = std::max(p, std::norm(buf[k]));
            float db = 10.0f * std::log10(p / (N * N * 0.0625f) + 1e-12f);
            peakHold[c] = std::max(db, peakHold[c] - 0.4f);
            float x = r.left + 6 + (r.width - 12) * (c + 0.5f) / COLS;
            pts.push_back({ x, fy(db) });
            hold.push_back({ x, fy(peakHold[c]) });
        }

        fillCurve(t, pts, r.top + r.height - 6, alpha(Theme::blockAccent[2], 120), alpha(Theme::blockAccent[2], 10));
        curve(t, pts, Theme::blockAccent[2], 1.5f);
        curve(t, hold, sf::Color(255, 255, 255, 70), 1.0f);
    }

    // ------------------------------------------------------------------ Blocks & Knobs
    void drawBlock(sf::RenderTarget& t, int b)
    {
        auto r = blockRect(b);
        auto& blk = Acid32Panel::block(b);
        sf::Color accent = Theme::blockAccent[b];

        rrect(t, r, 16.0f, Theme::card, mix(Theme::cardLine, accent, glow[b] * 0.5f));
        rrect(t, { r.left + 16, r.top, r.width - 32, 3 }, 1.5f, alpha(accent, 120 + (int)(135 * glow[b])));

        label(t, blk.name, r.left + 18, r.top + 20, 13, accent, LEFT, true, 3.0f);
        label(t, blk.subtitle, r.left + 18, r.top + 34, 9, Theme::muted, LEFT, false, 1.0f);

        // Group capsules
        for (int g = 0; g < Acid32Panel::GROUPS; ++g) {
            auto& grp = Acid32Panel::group(g);
            if (Acid32Panel::blockOf(grp.firstPot) != b) continue;
            float x = colX(Acid32Panel::potCol(grp.firstPot)), y = rowY(grp.firstPot);
            sf::FloatRect pill { x + 6, y + 4, grp.count * CELL_W - 12, ROW_PITCH - 18 };
            rrect(t, pill, 12.0f, sf::Color(255, 255, 255, 4), alpha(accent, 35));
            label(t, grp.name, pill.left + 16.0f, pill.top, 8, alpha(accent, 230), LEFT, true, 1.8f);
        }
    }

    void drawFlowChevrons(sf::RenderTarget& t)
    {
        auto chevron = [&](float x, float y, sf::Color c) {
            sf::ConvexShape s(3);
            s.setPoint(0, { x - 5, y - 8 });
            s.setPoint(1, { x + 5, y });
            s.setPoint(2, { x - 5, y + 8 });
            s.setFillColor(c);
            t.draw(s);
        };
        float gX = colX(4) - GAP * 0.5f;
        float yTop = CARDS_Y + CARD_H * 0.5f;
        float yBot = CARDS_Y + CARD_H + 16.0f + CARD_H * 0.5f;

        // Top: OSC -> LFO
        chevron(gX, yTop, alpha(Theme::blockAccent[1], 200));
        // Bottom: FILTER -> AMP/FX
        chevron(gX, yBot, alpha(Theme::blockAccent[3], 200));
    }

    void drawKnob(sf::RenderTarget& t, int pot, sf::Color accent)
    {
        auto ctr = knobCenter(pot);
        bool active = dragging == pot;
        bool hot = active || hoverPot == pot;
        float n = shown[pot];
        bool centered = panel.isCentered(pot);
        const float R = 26.0f;
        auto angle = [](float v) { return -135.0f + 270.0f * v; };

        disc(t, ctr.x, ctr.y, 35.0f, sf::Color(20, 22, 28));
        arc(t, ctr.x, ctr.y, R, 5.0f, -135.0f, 135.0f, Theme::track);

        // default position indicator
        float defA = angle(panel.defaultNorm(pot)) * 3.14159265f / 180.0f;
        disc(t, ctr.x + (R + 7.0f) * std::sin(defA), ctr.y - (R + 7.0f) * std::cos(defA), 1.5f, Theme::faint);

        float from = centered ? 0.0f : -135.0f;
        float to = angle(n);
        if (std::abs(to - from) > 0.5f) {
            arc(t, ctr.x, ctr.y, R, active ? 12.0f : 9.0f, from, to, sf::Color(accent.r, accent.g, accent.b, active ? 80 : 36), false);
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
        label(t, panel.param(pot).label, ctr.x, ctr.y + 41.0f, 11, hot ? Theme::text : sf::Color(176, 181, 196), CENTER, false, 0.5f);
        label(t, buf, ctr.x, ctr.y + 54.0f, 10, hot ? accent : Theme::muted, CENTER, hot, 0.4f);
    }

    void drawHelp(sf::RenderTarget& t, int pot)
    {
        auto ctr = knobCenter(pot);
        std::string text = Acid32Panel::help(pot);
        sf::Color accent = Theme::blockAccent[Acid32Panel::blockOf(pot)];

        sf::FloatRect box { std::clamp(ctr.x - 170.0f, 20.0f, W - 360.0f), ctr.y + 65.0f, 340.0f, 32.0f };
        rrect(t, box, 8.0f, sf::Color(28, 30, 40, 245), alpha(accent, 140));
        label(t, text, box.left + 14.0f, box.top + 16.0f, 10, Theme::text, LEFT);
    }

    // ------------------------------------------------------------------ Piano Roll Grid
    void drawPianoRoll(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        sf::FloatRect r { CARD_X, ROLL_Y, W - 2 * CARD_X, ROLL_H };
        rrect(t, r, 16.0f, Theme::card, Theme::cardLine);

        // Header
        label(t, "ACID PIANO ROLL SEQUENCER", r.left + 18.0f, r.top + 16.0f, 12, Theme::acidNeon, LEFT, true, 2.0f);
        label(t, "LEFT CLICK: Place / Move Note   DRAG RIGHT EDGE: Note Length   RIGHT CLICK: Slide / Accent Toggle   BOTTOM: Velocity", r.left + 230.0f, r.top + 16.0f, 10, Theme::muted, LEFT, false, 1.0f);

        float keyW = 48.0f;
        float rollX = r.left + keyW;
        float rollW = r.width - keyW - 14.0f;
        float velH = 46.0f;
        float rollY = r.top + 32.0f;
        float rollH = r.height - 38.0f - velH;

        float colW = rollW / 16.0f; // 16 steps per page
        float rowH = rollH / (float)visibleSemitones;

        const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

        // Draw Piano Keys on left & horizontal grid lines
        for (int rIdx = 0; rIdx < visibleSemitones; ++rIdx) {
            int midi = baseMidiNote + (visibleSemitones - 1 - rIdx);
            int nIdx = midi % 12;
            int oct = (midi / 12) - 1;
            bool isBlack = (nIdx == 1 || nIdx == 3 || nIdx == 6 || nIdx == 8 || nIdx == 10);
            bool isRoot = (nIdx == 0);

            float y = rollY + rIdx * rowH;

            // Key on left
            sf::RectangleShape kBox({ keyW - 2.0f, rowH - 1.0f });
            kBox.setPosition(r.left + 12.0f, y);
            kBox.setFillColor(isRoot ? alpha(Theme::acidNeon, 45) : (isBlack ? sf::Color(18, 20, 26) : sf::Color(34, 37, 48)));
            t.draw(kBox);

            char nBuf[16];
            snprintf(nBuf, sizeof(nBuf), "%s%d", noteNames[nIdx], oct);
            label(t, nBuf, r.left + 28.0f, y + rowH * 0.5f, 9, isRoot ? Theme::acidNeon : (isBlack ? Theme::muted : Theme::text), CENTER, isRoot);

            // Row lane
            sf::RectangleShape rLane({ rollW, rowH - 1.0f });
            rLane.setPosition(rollX, y);
            rLane.setFillColor(isBlack ? sf::Color(14, 15, 20) : sf::Color(20, 22, 28));
            t.draw(rLane);
        }

        // Vertical step columns & downbeat markers
        int activeStep = seq.currentStep.load();
        int stepOffset = stepPage * 16;

        for (int c = 0; c < 16; ++c) {
            float x = rollX + c * colW;
            bool isDownbeat = (c % 4 == 0);

            sf::RectangleShape vLine({ 1.0f, rollH });
            vLine.setPosition(x, rollY);
            vLine.setFillColor(isDownbeat ? Theme::cardLine : sf::Color(255, 255, 255, 6));
            t.draw(vLine);

            // Active step cursor highlight
            int absStep = stepOffset + c;
            if (absStep == activeStep) {
                sf::RectangleShape cursor({ colW, rollH + velH });
                cursor.setPosition(x, rollY);
                cursor.setFillColor(sf::Color(230, 255, 40, 25));
                t.draw(cursor);
            }
        }

        // Draw Notes & Slide Connections
        for (int c = 0; c < 16; ++c) {
            int absStep = stepOffset + c;
            const AcidStep& s = seq.steps[absStep];
            if (!s.on) continue;

            int semiOffset = (int)s.note - baseMidiNote;
            if (semiOffset < 0 || semiOffset >= visibleSemitones) continue;

            int rIdx = visibleSemitones - 1 - semiOffset;
            float nx = rollX + c * colW;
            float ny = rollY + rIdx * rowH;
            float nw = std::max(colW * s.length - 2.0f, 6.0f);

            sf::Color noteColor = s.accent ? Theme::accentNote : (s.slide ? Theme::slideNote : Theme::acidNeon);

            // Note body
            rrect(t, { nx + 1.0f, ny + 1.0f, nw, rowH - 2.0f }, 4.0f, noteColor, sf::Color(255, 255, 255, 120));

            // Slide arrow indicator
            if (s.slide) {
                sf::ConvexShape arrow(3);
                arrow.setPoint(0, { nx + nw - 10.0f, ny + rowH * 0.5f - 4.0f });
                arrow.setPoint(1, { nx + nw - 2.0f,  ny + rowH * 0.5f });
                arrow.setPoint(2, { nx + nw - 10.0f, ny + rowH * 0.5f + 4.0f });
                arrow.setFillColor(sf::Color(14, 14, 18));
                t.draw(arrow);
            }

            // Accent dot
            if (s.accent) {
                disc(t, nx + 6.0f, ny + rowH * 0.5f, 2.5f, sf::Color(14, 14, 18));
            }
        }

        // Velocity & Attribute strip at bottom
        float vY = rollY + rollH + 6.0f;
        hline(t, rollX, vY, rollW, Theme::cardLine);

        for (int c = 0; c < 16; ++c) {
            int absStep = stepOffset + c;
            const AcidStep& s = seq.steps[absStep];
            float cx = rollX + c * colW + colW * 0.5f;

            if (s.on) {
                float vBarH = s.velocity * (velH - 12.0f);
                sf::RectangleShape vBar({ 4.0f, vBarH });
                vBar.setPosition(cx - 2.0f, vY + velH - 6.0f - vBarH);
                vBar.setFillColor(s.accent ? Theme::accentNote : Theme::acidNeon);
                t.draw(vBar);
                disc(t, cx, vY + velH - 6.0f - vBarH, 3.5f, s.accent ? Theme::accentNote : Theme::acidNeon);
            } else {
                disc(t, cx, vY + velH - 8.0f, 1.5f, Theme::faint);
            }
        }
    }

    void handlePianoRollMouse(sf::Vector2f p, sf::Mouse::Button button, bool shift, bool isInitialPress)
    {
        sf::FloatRect r { CARD_X, ROLL_Y, W - 2 * CARD_X, ROLL_H };
        float keyW = 48.0f;
        float rollX = r.left + keyW;
        float rollW = r.width - keyW - 14.0f;
        float velH = 46.0f;
        float rollY = r.top + 32.0f;
        float rollH = r.height - 38.0f - velH;

        float colW = rollW / 16.0f;
        float rowH = rollH / (float)visibleSemitones;
        float vY = rollY + rollH + 6.0f;

        // Check velocity strip
        if (p.x >= rollX && p.x <= rollX + rollW && p.y >= vY && p.y <= vY + velH) {
            int stepInPage = (int)((p.x - rollX) / colW);
            int absStep = std::clamp(stepPage * 16 + stepInPage, 0, AcidSequencer::STEPS - 1);
            float vel = std::clamp((vY + velH - 6.0f - p.y) / (velH - 12.0f), 0.1f, 1.0f);
            seq.steps[absStep].velocity = vel;
            draggingVelocity = true;
            return;
        }

        // Check note matrix
        if (p.x >= rollX && p.x <= rollX + rollW && p.y >= rollY && p.y <= rollY + rollH) {
            int stepInPage = (int)((p.x - rollX) / colW);
            int row = (int)((p.y - rollY) / rowH);
            int absStep = std::clamp(stepPage * 16 + stepInPage, 0, AcidSequencer::STEPS - 1);
            row = std::clamp(row, 0, visibleSemitones - 1);
            int midi = baseMidiNote + (visibleSemitones - 1 - row);

            AcidStep& s = seq.steps[absStep];

            if (button == sf::Mouse::Right) {
                // Right click toggles: Normal -> Slide -> Accent -> Slide+Accent -> Normal
                if (s.on) {
                    if (!s.slide && !s.accent) {
                        s.slide = true;
                    } else if (s.slide && !s.accent) {
                        s.slide = false;
                        s.accent = true;
                    } else if (!s.slide && s.accent) {
                        s.slide = true;
                    } else {
                        s.slide = false;
                        s.accent = false;
                    }
                }
                return;
            }

            if (isInitialPress) {
                // Check if clicking near right edge of existing note to resize length
                float noteX = rollX + stepInPage * colW;
                float noteW = colW * s.length;
                if (s.on && std::abs(p.x - (noteX + noteW)) < 8.0f) {
                    resizingLength = true;
                    draggingNoteStep = absStep;
                    return;
                }

                if (s.on && s.note == midi) {
                    s.on = false; // toggle off
                } else {
                    s.on = true;
                    s.note = (uint8_t)midi;
                    s.length = 0.85f;
                    auditionNote = midi + seq.transpose;
                    draggingNoteStep = absStep;
                }
            } else if (resizingLength && draggingNoteStep == absStep) {
                float startX = rollX + (absStep % 16) * colW;
                float len = std::clamp((p.x - startX) / colW, 0.2f, 4.0f);
                s.length = len;
            }
        }
    }
};
