#pragma once

/**
 * Emulator only: SFML front panel (32 knobs) + minimal 64 step test sequencer.
 * All the "what does a pot do" knowledge lives in core/Kick32v4Panel.h.
 *
 * The panel is drawn as the signal flow: BODY + MOTION (left) > DRIVE > FILTER.
 *   - HIT: the waveform of the last hit over a time aligned spectrogram (log frequency), with the pitch curve
 *     (drop + bend) drawn on it, next to a live spectrum analyzer of the output
 *   - every card has a live preview: the body cycle, the motion curves (pitch, FM index, shift) over the decay,
 *     the drive transfer curve with the signal riding on it, the filterbank response breathing with its envelope
 *   - hover a knob: a help line tells what it does; presets and a dice to start somewhere
 */

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdio>
#include <mutex>
#include <string>
#include <vector>

#include "Sequencer.h"
#include "core/Kick32v4Panel.h"

extern std::mutex audioMutex;
extern std::atomic<int> kickTrigger; // set to 1 to audition a kick
extern std::atomic<bool> kickMuted; // held: sequencer kicks are skipped

// What the audio thread records for the UI (no lock: display only)
struct Capture {
    static constexpr int N = 1200; // min / max buckets of the last hit
    static constexpr float MS_PER_BUCKET = 0.5f; // 600 ms
    static constexpr int SAMPLES = 26460; // the same 600 ms, raw, for the spectrogram
    float lo[N] = {}, hi[N] = {};
    float samples[SAMPLES] = {};
    std::atomic<int> count { 0 }; // samples captured so far (-1 = idle, nothing yet)
    int note = 60;
    int hitId = 0;

    // live output ring (analyzer) and the drive in / out trail
    static constexpr int RING = 4096;
    float ring[RING] = {};
    int ringPos = 0;
    static constexpr int TRAIL = 96;
    float trailPre[TRAIL] = {}, trailPost[TRAIL] = {};
    int trailPos = 0, trailDiv = 0;
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
const sf::Color blockAccent[4] = { rgb(255, 176, 64), rgb(172, 128, 255), rgb(255, 86, 98), rgb(56, 212, 255) }; // BODY, MOTION, DRIVE, FILTER
const sf::Color kick = rgb(255, 176, 64);
}

class EmuUi {
public:
    static constexpr float W = 1320.0f;
    static constexpr float H = 1236.0f;
    static constexpr float CARD_X = 24.0f, CELL_W = 150.0f, GAP = 36.0f;
    static constexpr float HIT_Y = 78.0f, HIT_H = 250.0f;
    static constexpr float CARDS_Y = 346.0f, ROW_PITCH = 134.0f, HEAD_S = 48.0f, STACK_GAP = 16.0f;
    static constexpr float CARD_S_H = HEAD_S + 2 * ROW_PITCH + 8.0f;
    static constexpr float CARD_T_H = 2 * CARD_S_H + STACK_GAP; // DRIVE / FILTER
    static constexpr float BODY_H = HEAD_S + 3 * ROW_PITCH + 8.0f; // BODY (3 rows), MOTION takes the rest
    static constexpr float HEAD_T = CARD_T_H - 4 * ROW_PITCH - 8.0f;
    static constexpr float SEQ_Y = CARDS_Y + CARD_T_H + 22.0f;

    // emulator presets: a few starting points (the README ones). Emulator only, the module has no memory.
    struct Preset { const char* name; std::vector<std::pair<const char*, float>> values; };
    static const std::vector<Preset>& presets()
    {
        static const std::vector<Preset> p = {
            { "INIT", {} },
            { "HARDCORE", { { "drive", 75 }, { "character", 40 }, { "shape", 60 }, { "punch", -50 }, { "decay", 500 }, { "click", 35 }, { "clickTone", 30 }, { "filterDrive", 30 } } },
            { "TRIBE", { { "cutoff", 45 }, { "reso", 70 }, { "env", 60 }, { "cutoff2", 30 }, { "filterDrive", 60 }, { "bend", 25 }, { "decay", 600 }, { "shape", 55 }, { "drive", 45 } } },
            { "HARDTEK", { { "decay", 300 }, { "drop", 60 }, { "dropTime", 35 }, { "wave", 30 }, { "harm2", 40 }, { "feedback", 50 }, { "drive", 60 }, { "character", 35 }, { "punch", 30 }, { "cutoff", 70 }, { "env", 40 }, { "filterDrive", 40 }, { "noise", 25 }, { "noiseColor", 60 } } },
            { "METAL", { { "fmDepth", 45 }, { "fmRatio", 58 }, { "shift", 45 }, { "feedback", 60 }, { "drive", 60 }, { "character", 70 }, { "fold", 40 }, { "resonator", 30 }, { "env", 30 }, { "click", 40 }, { "clickTone", 80 } } },
            { "TEKNO", { { "sync", 35 }, { "skew", 30 }, { "vowel", 40 }, { "bend", -30 }, { "drive", 50 }, { "cutoff", 60 }, { "reso", 50 }, { "env", 50 }, { "envDecay", 45 }, { "decay", 420 } } },
        };
        return p;
    }

    EmuUi(Kick32v4Panel::Panel& panel, Sequencer& seq, Capture& cap)
        : panel(panel)
        , seq(seq)
        , cap(cap)
    {
        const char* paths[] = {
            "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Regular.ttf",
            "../../draw/fonts/Roboto-Regular.ttf", "../draw/fonts/Roboto-Regular.ttf", "draw/fonts/Roboto-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        };
        for (auto p : paths)
            if (font.loadFromFile(p)) break;
        sonoImg.create(SONO_COLS, SONO_ROWS, sf::Color(0, 0, 0, 0));
        sonoTex.create(SONO_COLS, SONO_ROWS);
        sonoTex.setSmooth(true);
        for (int i = 0; i < Kick32v4Panel::POTS; ++i) shown[i] = panel.norm(i);
    }

    // ------------------------------------------------------------------ layout
    static float colX(int col) { return CARD_X + col * CELL_W + (col >= 4 ? GAP : 0.0f) + (col >= 6 ? GAP : 0.0f); }
    static sf::FloatRect blockRect(int b)
    {
        auto& k = Kick32v4Panel::block(b);
        float y = CARDS_Y + (b == 1 ? BODY_H + STACK_GAP : 0.0f);
        float h = b == 0 ? BODY_H : (b == 1 ? CARD_T_H - BODY_H - STACK_GAP : CARD_T_H);
        return { colX(k.col), y, k.cols * CELL_W, h };
    }
    static float headH(int b) { return Kick32v4Panel::block(b).rows < 4 ? HEAD_S : HEAD_T; }
    static float rowY(int pot)
    {
        int b = Kick32v4Panel::blockOf(pot);
        auto& k = Kick32v4Panel::block(b);
        return blockRect(b).top + headH(b) + (Kick32v4Panel::potRow(pot) - k.row) * ROW_PITCH;
    }
    static sf::Vector2f knobCenter(int pot) { return { colX(Kick32v4Panel::potCol(pot)) + CELL_W * 0.5f, rowY(pot) + 54.0f }; }
    static sf::FloatRect knobCell(int pot)
    {
        auto ctr = knobCenter(pot);
        return { ctr.x - CELL_W * 0.5f, ctr.y - 40.0f, CELL_W, 112.0f };
    }
    static sf::FloatRect stepRect(int i)
    {
        int row = i / 16, col = i % 16;
        float gap = 6.0f, beatGap = 14.0f;
        float w = (W - 2 * CARD_X - 12 * gap - 3 * beatGap) / 16.0f;
        float x = CARD_X + col * (w + gap) + (col / 4) * (beatGap - gap);
        return { x, SEQ_Y + 24.0f + row * 40.0f, w, 34.0f };
    }
    static sf::FloatRect playBtn() { return { 236.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect trigBtn() { return { 302.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect bpmBox() { return { 372.0f, 14.0f, 126.0f, 44.0f }; }
    static sf::FloatRect tuneBox() { return { 506.0f, 14.0f, 126.0f, 44.0f }; }
    static sf::FloatRect presetRect(int i) { return { 660.0f + i * 78.0f, 14.0f, 72.0f, 44.0f }; }
    static sf::FloatRect diceBtn() { return { 660.0f + 6 * 78.0f, 14.0f, 58.0f, 44.0f }; }
    static sf::FloatRect meterBox() { return { W - 24 - 90, 14.0f, 90.0f, 44.0f }; }

    // ------------------------------------------------------------------ input (logical coordinates)
    void mousePressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        if (button == sf::Mouse::Left) {
            if (playBtn().contains(p)) { seq.toggle(); return; }
            if (trigBtn().contains(p)) { trigDown(); trigHeldByMouse = true; return; }
            if (bpmBox().contains(p)) { stepBpm(p.x > bpmBox().left + 63 ? 1.0f : -1.0f, shift); return; }
            if (tuneBox().contains(p)) { stepTune(p.x > tuneBox().left + 63 ? 1 : -1, shift); return; }
            if (diceBtn().contains(p)) { dice(); return; }
            for (int i = 0; i < (int)presets().size(); ++i)
                if (presetRect(i).contains(p)) { applyPreset(i); return; }
        }
        for (int i = 0; i < Sequencer::STEPS; ++i) {
            if (!stepRect(i).contains(p)) continue;
            if (button == sf::Mouse::Left) {
                seq.steps[i].on = !seq.steps[i].on;
                paintValue = seq.steps[i].on;
                painting = true;
            } else if (button == sf::Mouse::Right) {
                seq.steps[i].note = 0;
            }
            return;
        }
        if (button != sf::Mouse::Left) return;
        for (int i = 0; i < Kick32v4Panel::POTS; ++i) {
            if (!knobCell(i).contains(p)) continue;
            if (clock.getElapsedTime().asMilliseconds() < 350 && lastClicked == i) {
                panel.reset(i); // double click = default
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

    // Space / TRIG button: audition the kick, or mute the kick while the sequencer plays (held)
    void trigDown()
    {
        if (seq.playing) kickMuted = true;
        else kickTrigger = 1;
    }
    void trigUp() { kickMuted = false; }

    void stepBpm(float dir, bool shift) { seq.bpm = std::clamp(seq.bpm + dir * (shift ? 10.0f : 1.0f), 60.0f, 300.0f); }
    void stepTune(int dir, bool shift) { seq.transpose = std::clamp(seq.transpose + dir * (shift ? 12 : 1), -24, 24); }

    // presets: INIT resets everything, the others start from INIT (caller holds the lock)
    void applyPreset(int i)
    {
        for (int p = 0; p < Kick32v4Panel::POTS; ++p) panel.reset(p);
        for (auto& kv : presets()[i].values)
            for (int p = 0; p < Kick32v4Panel::POTS; ++p)
                if (!strcmp(panel.param(p).key, kv.first)) panel.param(p).set(kv.second);
        flashPreset = i;
        flashClock.restart();
        if (!seq.playing) kickTrigger = 1;
    }

    // dice: a random kick that stays musical (every pot within a sane window around its default)
    void dice()
    {
        auto r = [&]() { return (float)rand() / RAND_MAX; };
        for (int p = 0; p < Kick32v4Panel::POTS; ++p) {
            const char* k = panel.param(p).key;
            float v;
            if (!strcmp(k, "decay")) v = 0.03f + 0.4f * r() * r();
            else if (!strcmp(k, "cutoff")) v = 0.45f + 0.55f * r();
            else if (!strcmp(k, "cutoff2")) v = 0.35f * r() * r();
            else if (!strcmp(k, "drive")) v = 0.2f + 0.65f * r();
            else if (!strcmp(k, "drop")) v = 0.3f + 0.5f * r();
            else if (!strcmp(k, "filterFm") || !strcmp(k, "reso")) v = 0.6f * r() * r();
            else if (panel.isCentered(p)) v = 0.5f + (r() - 0.5f) * 1.2f * r();
            else v = r() * r() * (r() < 0.3f ? 0.0f : 1.0f); // most pots low, a few of them up
            panel.set(p, v);
        }
        flashPreset = -2;
        flashClock.restart();
        if (!seq.playing) kickTrigger = 1;
    }

    void mouseReleased()
    {
        if (trigHeldByMouse) { trigUp(); trigHeldByMouse = false; }
        dragging = -1;
        painting = false;
    }

    void mouseMoved(sf::Vector2f p, bool shift)
    {
        hoverPot = -1;
        for (int i = 0; i < Kick32v4Panel::POTS; ++i)
            if (knobCell(i).contains(p)) hoverPot = i;
        std::lock_guard<std::mutex> lock(audioMutex);
        if (dragging >= 0) {
            panel.set(dragging, dragStart + (dragY - p.y) / (shift ? 900.0f : 220.0f));
        } else if (painting) {
            for (int i = 0; i < Sequencer::STEPS; ++i)
                if (stepRect(i).contains(p)) seq.steps[i].on = paintValue;
        }
    }

    void wheel(sf::Vector2f p, float delta, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        for (int i = 0; i < Sequencer::STEPS; ++i) {
            if (!stepRect(i).contains(p)) continue;
            int d = (delta > 0 ? 1 : -1) * (shift ? 12 : 1);
            seq.steps[i].note = (int8_t)std::clamp((int)seq.steps[i].note + d, -24, 24);
            return;
        }
        for (int i = 0; i < Kick32v4Panel::POTS; ++i) {
            if (!knobCell(i).contains(p)) continue;
            panel.set(i, panel.norm(i) + (delta > 0 ? 1 : -1) * (shift ? 0.002f : 0.01f));
            return;
        }
        if (bpmBox().contains(p)) stepBpm(delta > 0 ? 1.0f : -1.0f, shift);
        else if (tuneBox().contains(p)) stepTune(delta > 0 ? 1 : -1, shift);
    }

    void key(sf::Keyboard::Key k)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        if (k >= sf::Keyboard::Num1 && k <= sf::Keyboard::Num6) applyPreset(k - sf::Keyboard::Num1);
        else if (k == sf::Keyboard::D) dice();
    }

    // ------------------------------------------------------------------ drawing
    void draw(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        float dt = frameClock.restart().asSeconds();
        dt = std::min(dt, 0.1f);
        animate(dt);
        t.clear(Theme::bg);
        drawTopBar(t, mouse);
        drawHit(t, { CARD_X, HIT_Y, W - 2 * CARD_X, HIT_H });
        for (int b = 0; b < Kick32v4Panel::BLOCKS; ++b) drawBlock(t, b);
        drawFlow(t);
        for (int i = 0; i < Kick32v4Panel::POTS; ++i) drawKnob(t, i, Theme::blockAccent[Kick32v4Panel::blockOf(i)]);
        drawSequencer(t, mouse);
        if (hoverPot >= 0 && dragging < 0) drawHelp(t, hoverPot);
    }

private:
    Kick32v4Panel::Panel& panel;
    Sequencer& seq;
    Capture& cap;

    sf::Font font;
    int dragging = -1, hoverPot = -1, lastClicked = -1;
    float dragY = 0.0f, dragStart = 0.0f;
    bool painting = false, paintValue = false, trigHeldByMouse = false;
    sf::Clock clock, frameClock, flashClock;
    int flashPreset = -1;

    // animation state
    float shown[Kick32v4Panel::POTS]; // knob positions eased toward the real value
    float glow[4] = {}; // card activity
    float liveOct = 0.0f; // filter offset, smoothed
    float peakHold[256] = {};

    // spectrogram of the last hit
    static constexpr int SONO_COLS = 180, SONO_ROWS = 112, SONO_FFT = 2048;
    sf::Image sonoImg;
    sf::Texture sonoTex;
    int sonoDone = 0, sonoHit = -1;

    // ---- primitives
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
        for (int c = 0; c < 4; ++c)
            for (int i = 0; i <= seg; ++i) {
                float a = ((float)c * 90.0f + (float)i * 90.0f / seg) * 3.14159265f / 180.0f;
                float ang = a - 1.5707963f;
                s.setPoint(k++, { cx[c] + rad * std::cos(ang), cy[c] + rad * std::sin(ang) });
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

    // degrees, 0 = up, clockwise
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

    // a polyline, th px thick (two offset strips)
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

    // area under a curve down to a base line
    void fill(sf::RenderTarget& t, const std::vector<sf::Vector2f>& pts, float baseY, sf::Color top, sf::Color bottom)
    {
        if (pts.size() < 2) return;
        sf::VertexArray va(sf::TriangleStrip, pts.size() * 2);
        for (size_t i = 0; i < pts.size(); ++i) {
            va[2 * i] = sf::Vertex(pts[i], top);
            va[2 * i + 1] = sf::Vertex({ pts[i].x, baseY }, bottom);
        }
        t.draw(va);
    }

    // ---- fft (radix 2, in place) for the spectrogram and the analyzer
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

    // log frequency axis 30 Hz .. 16 kHz
    static float fLo() { return 20.0f; }
    static float fHi() { return 16000.0f; }
    static float fNorm(float hz) { return std::log(std::max(hz, 1.0f) / fLo()) / std::log(fHi() / fLo()); }
    static float sr() { return 44100.0f; }

    // ---- animation
    void animate(float dt)
    {
        auto& e = panel.engine;
        float k = 1.0f - std::exp(-dt * 30.0f);
        for (int i = 0; i < Kick32v4Panel::POTS; ++i) shown[i] += (panel.norm(i) - shown[i]) * k;
        float act[4] = { e.envAmp(), std::max(e.envFm(), std::min(1.0f, std::abs(e.liveShiftHz) / 200.0f)), std::min(1.0f, std::abs(e.livePre) * 0.25f), e.envFilter() };
        for (int b = 0; b < 4; ++b) glow[b] = std::max(act[b], glow[b] - dt * 2.5f);
        float target = e.playing() ? e.liveFilterOct - e.noteOctave() : 0.0f;
        liveOct += (target - liveOct) * (1.0f - std::exp(-dt * 20.0f));
    }

    // ---- widgets
    enum Icon { PLAY, STOP, TRIG, MUTE, DICE };

    void icon(sf::RenderTarget& t, Icon kind, float x, float y, sf::Color c)
    {
        auto poly = [&](std::initializer_list<sf::Vector2f> pts) {
            sf::ConvexShape s(pts.size());
            int i = 0;
            for (auto& p : pts) s.setPoint(i++, { x + p.x, y + p.y });
            s.setFillColor(c);
            t.draw(s);
        };
        auto line = [&](sf::Vector2f a, sf::Vector2f b, float w) {
            sf::Vector2f d = b - a;
            float len = std::sqrt(d.x * d.x + d.y * d.y);
            sf::RectangleShape r({ len, w });
            r.setOrigin(0, w * 0.5f);
            r.setPosition(x + a.x, y + a.y);
            r.setRotation(std::atan2(d.y, d.x) * 180.0f / 3.14159265f);
            r.setFillColor(c);
            t.draw(r);
        };
        switch (kind) {
        case PLAY: poly({ { -6, -9 }, { 9, 0 }, { -6, 9 } }); break;
        case STOP: rrect(t, { x - 7, y - 7, 14, 14 }, 3.0f, c); break;
        case TRIG:
            disc(t, x, y, 4.0f, c);
            arc(t, x, y, 10.0f, 2.5f, 0.0f, 360.0f, c, false);
            break;
        case MUTE:
            poly({ { -11, -4 }, { -6, -4 }, { -6, 4 }, { -11, 4 } });
            poly({ { -6, -4 }, { 0, -9 }, { 0, 9 }, { -6, 4 } });
            line({ 4, -5 }, { 12, 5 }, 2.5f);
            line({ 4, 5 }, { 12, -5 }, 2.5f);
            break;
        case DICE:
            rrect(t, { x - 10, y - 10, 20, 20 }, 5.0f, sf::Color::Transparent, c);
            disc(t, x - 5, y - 5, 2.2f, c);
            disc(t, x + 5, y + 5, 2.2f, c);
            disc(t, x, y, 2.2f, c);
            disc(t, x + 5, y - 5, 2.2f, c);
            disc(t, x - 5, y + 5, 2.2f, c);
            break;
        }
    }

    void button(sf::RenderTarget& t, sf::FloatRect r, Icon kind, sf::Vector2f mouse, bool active, sf::Color accent)
    {
        bool hot = r.contains(mouse);
        sf::Color fill = active ? accent : (hot ? sf::Color(40, 43, 54) : Theme::card);
        sf::Color ink = active ? sf::Color(14, 14, 18) : Theme::text;
        rrect(t, r, 12.0f, fill, active ? accent : Theme::cardLine);
        icon(t, kind, r.left + r.width * 0.5f, r.top + r.height * 0.5f, ink);
    }

    void stepper(sf::RenderTarget& t, sf::FloatRect b, const char* value, const char* caption)
    {
        rrect(t, b, 12.0f, Theme::card, Theme::cardLine);
        label(t, value, b.left + 50, b.top + 22, 21, Theme::text, CENTER, true, 0.5f);
        label(t, caption, b.left + 78, b.top + 25, 9, Theme::muted, LEFT, false, 1.5f);
        label(t, "-", b.left + 14, b.top + 21, 20, Theme::muted);
        label(t, "+", b.left + b.width - 14, b.top + 21, 20, Theme::muted);
    }

    void drawTopBar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "zicMod", 24, 30, 23, Theme::text, LEFT, true, 0.5f);
        label(t, "Kick32v4", 24 + 84, 30, 23, Theme::kick, LEFT, true, 0.5f);
        label(t, "BODY  >  MOTION  >  DRIVE  >  FILTER", 24, 54, 9, Theme::muted, LEFT, false, 2.2f);

        button(t, playBtn(), seq.playing ? STOP : PLAY, mouse, seq.playing, Theme::blockAccent[3]);
        button(t, trigBtn(), seq.playing ? MUTE : TRIG, mouse, kickMuted.load() || kickTrigger.load() != 0, Theme::kick);

        char buf[16];
        snprintf(buf, sizeof(buf), "%d", (int)std::round(seq.bpm));
        stepper(t, bpmBox(), buf, "BPM");
        snprintf(buf, sizeof(buf), "%+d", seq.transpose);
        stepper(t, tuneBox(), buf, "TUNE");

        // presets + dice
        float flash = flashClock.getElapsedTime().asSeconds();
        for (int i = 0; i < (int)presets().size(); ++i) {
            auto r = presetRect(i);
            bool hot = r.contains(mouse);
            bool lit = flashPreset == i && flash < 0.6f;
            sf::Color acc = Theme::blockAccent[i % 4];
            rrect(t, r, 12.0f, lit ? alpha(acc, 200) : (hot ? sf::Color(40, 43, 54) : Theme::card), lit ? acc : Theme::cardLine);
            label(t, presets()[i].name, r.left + r.width * 0.5f, r.top + 22, 9, lit ? sf::Color(14, 14, 18) : (hot ? Theme::text : Theme::muted), CENTER, true, 1.6f);
            label(t, std::to_string(i + 1), r.left + 8, r.top + 8, 7, Theme::faint, LEFT);
        }
        {
            auto r = diceBtn();
            bool lit = flashPreset == -2 && flash < 0.6f;
            button(t, r, DICE, mouse, lit, Theme::blockAccent[1]);
        }

        // output meter, right side
        sf::FloatRect m = meterBox();
        rrect(t, m, 12.0f, Theme::card, Theme::cardLine);
        label(t, "OUT", m.left + 10, m.top + 22, 8, Theme::muted, LEFT, false, 2.0f);
        float lvl = std::clamp(panel.engine.meterOut, 0.0f, 1.0f);
        sf::FloatRect bar { m.left + 36, m.top + 17, m.width - 46, 10 };
        rrect(t, bar, 5.0f, Theme::cell);
        if (lvl > 0.01f) rrect(t, { bar.left, bar.top, bar.width * lvl, bar.height }, 5.0f, lvl > 0.9f ? Theme::blockAccent[2] : Theme::kick);
    }

    // ------------------------------------------------------------------ HIT: scope + spectrogram + analyzer
    void updateSonogram()
    {
        int have = cap.count.load();
        if (cap.hitId != sonoHit) {
            sonoHit = cap.hitId;
            sonoDone = 0;
            for (int x = 0; x < SONO_COLS; ++x)
                for (int y = 0; y < SONO_ROWS; ++y) sonoImg.setPixel(x, y, sf::Color(0, 0, 0, 0));
        }
        if (have < 0) return;
        const int hop = Capture::SAMPLES / SONO_COLS;
        static std::vector<std::complex<float>> buf(SONO_FFT);
        static std::vector<float> win;
        if (win.empty()) {
            win.resize(SONO_FFT);
            for (int i = 0; i < SONO_FFT; ++i) win[i] = 0.5f - 0.5f * std::cos(2.0f * 3.14159265f * i / (SONO_FFT - 1));
        }
        bool changed = false;
        int budget = 24; // columns per frame
        while (sonoDone < SONO_COLS && budget-- > 0) {
            int center = sonoDone * hop + hop / 2;
            if (center + SONO_FFT / 4 > have && have < Capture::SAMPLES) break; // wait for the samples
            for (int i = 0; i < SONO_FFT; ++i) {
                int k = center - SONO_FFT / 2 + i;
                buf[i] = (k >= 0 && k < have) ? cap.samples[k] * win[i] : 0.0f;
            }
            fft(buf);
            for (int y = 0; y < SONO_ROWS; ++y) {
                // log frequency rows, top = high. Sum the bins covering the row (at least one)
                float f0 = fLo() * std::pow(fHi() / fLo(), (float)(SONO_ROWS - 1 - y) / SONO_ROWS);
                float f1 = fLo() * std::pow(fHi() / fLo(), (float)(SONO_ROWS - y) / SONO_ROWS);
                int k0 = std::max(1, (int)(f0 / sr() * SONO_FFT)), k1 = std::max(k0 + 1, (int)(f1 / sr() * SONO_FFT) + 1);
                float p = 0;
                for (int k = k0; k < k1; ++k) p = std::max(p, std::norm(buf[k]));
                float db = 10.0f * std::log10(p / (SONO_FFT * SONO_FFT * 0.0625f) + 1e-12f); // 0 dB = full scale sine
                float v = std::clamp((db + 60.0f) / 60.0f, 0.0f, 1.0f);
                sonoImg.setPixel(sonoDone, y, heat(v * v * (3.0f - 2.0f * v) * v)); // gamma: only the strong partials glow
            }
            sonoDone++;
            changed = true;
        }
        if (changed) sonoTex.update(sonoImg);
    }

    static sf::Color heat(float v)
    {
        // transparent -> deep violet -> orange -> near white
        const sf::Color c0(20, 10, 40, 0), c1(90, 30, 120, 255), c2(255, 120, 40, 255), c3(255, 240, 200, 255);
        if (v < 0.35f) return mix(c0, c1, v / 0.35f);
        if (v < 0.75f) return mix(c1, c2, (v - 0.35f) / 0.4f);
        return mix(c2, c3, (v - 0.75f) / 0.25f);
    }

    void drawHit(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 16.0f, Theme::card, Theme::cardLine);
        auto& e = panel.engine;
        const float totalMs = Capture::N * Capture::MS_PER_BUCKET;
        // left: scope over sonogram, right: analyzer
        const float anW = 330.0f;
        sf::FloatRect L { r.left + 16, r.top + 26, r.width - anW - 44, r.height - 42 };
        sf::FloatRect A { r.left + r.width - anW - 12, r.top + 26, anW, r.height - 42 };
        float waveH = 92.0f;
        sf::FloatRect S { L.left, L.top, L.width, waveH };
        sf::FloatRect G { L.left, L.top + waveH + 8, L.width, L.height - waveH - 8 };
        auto X = [&](float ms) { return L.left + L.width * ms / totalMs; };

        // time grid
        for (int ms = 100; ms < (int)totalMs; ms += 100) {
            vline(t, X((float)ms), S.top, L.height, sf::Color(255, 255, 255, 10));
            char buf[16];
            snprintf(buf, sizeof(buf), "%d ms", ms);
            label(t, buf, X((float)ms) - 4, G.top + G.height + 7, 8, Theme::faint, RIGHT, false, 0.8f);
        }

        // waveform: min / max per half millisecond
        float mid = S.top + S.height * 0.5f, amp = S.height * 0.48f;
        hline(t, S.left, mid, S.width, sf::Color(255, 255, 255, 18));
        {
            sf::VertexArray va(sf::TriangleStrip, 2 * Capture::N);
            sf::Color c = alpha(Theme::kick, 200);
            for (int i = 0; i < Capture::N; ++i) {
                float x = S.left + S.width * i / (Capture::N - 1);
                float hi = std::clamp(cap.hi[i], -1.0f, 1.0f), lo = std::clamp(cap.lo[i], -1.0f, 1.0f);
                if (hi - lo < 0.004f) { hi += 0.002f; lo -= 0.002f; }
                va[2 * i] = sf::Vertex({ x, mid - hi * amp }, c);
                va[2 * i + 1] = sf::Vertex({ x, mid - lo * amp }, c);
            }
            t.draw(va);
        }
        // amp shape over the waveform
        {
            std::vector<sf::Vector2f> ac;
            for (int i = 0; i <= 200; ++i) {
                float ms = totalMs * i / 200;
                ac.push_back({ X(ms), mid - e.ampAt(ms) * amp });
            }
            curve(t, ac, sf::Color(255, 255, 255, 45), 1.2f);
        }

        // spectrogram
        updateSonogram();
        rrect(t, G, 6.0f, sf::Color(0, 0, 0, 90));
        {
            sf::Sprite sp(sonoTex);
            sp.setPosition(G.left, G.top);
            sp.setScale(G.width / SONO_COLS, G.height / SONO_ROWS);
            t.draw(sp);
        }
        auto FY = [&](float hz) { return G.top + G.height * (1.0f - std::clamp(fNorm(hz), 0.0f, 1.0f)); };
        for (float hz : { 100.0f, 1000.0f, 10000.0f }) {
            hline(t, G.left, FY(hz), G.width, sf::Color(255, 255, 255, 14));
            label(t, hz >= 1000 ? (hz >= 10000 ? "10k" : "1k") : "100", G.left + G.width - 4, FY(hz) - 6, 7, Theme::faint, RIGHT);
        }
        // the pitch the body plays (drop + bend) on top of it, in Hz, for the captured note
        {
            float base = Kick32v4::ROOT_HZ * std::pow(2.0f, (cap.note - 60) / 12.0f);
            std::vector<sf::Vector2f> pc;
            for (int i = 0; i <= 240; ++i) {
                float ms = totalMs * i / 240;
                if (ms > e.decay.value) break;
                float hz = base * std::pow(2.0f, e.pitchOctAt(ms));
                pc.push_back({ X(ms), FY(hz) });
            }
            curve(t, pc, sf::Color(255, 255, 255, 170), 1.3f);
        }
        // markers
        auto marker = [&](float ms, sf::Color c, const char* name) {
            if (ms > totalMs) return;
            for (float y = S.top; y < G.top + G.height; y += 8) vline(t, X(ms), y, 4.0f, c, 1.5f);
            label(t, name, X(ms) + 5, S.top + 6, 8, c, LEFT, true, 1.2f);
        };
        marker(e.dropMs(), alpha(Theme::blockAccent[1], 220), "DROP");
        marker(e.decay.value, alpha(Theme::blockAccent[0], 200), "DECAY");

        label(t, "HIT", r.left + 16, r.top + 13, 10, Theme::text, LEFT, true, 2.4f);
        label(t, "out", r.left + 52, r.top + 13, 9, Theme::kick, LEFT, true, 1.0f);
        label(t, "amp", r.left + 84, r.top + 13, 9, Theme::muted, LEFT, true, 1.0f);
        label(t, "pitch", r.left + 120, r.top + 13, 9, Theme::text, LEFT, true, 1.0f);
        label(t, "spectrogram 20 Hz .. 16 kHz", r.left + 166, r.top + 13, 9, Theme::faint, LEFT, false, 1.0f);
        char buf[64];
        float hz = Kick32v4::ROOT_HZ * std::pow(2.0f, (cap.note - 60) / 12.0f);
        snprintf(buf, sizeof(buf), "note %d  ·  %.1f Hz  ·  600 ms", cap.note, hz);
        label(t, buf, L.left + L.width, r.top + 13, 9, Theme::muted, RIGHT, false, 1.0f);

        drawAnalyzer(t, A);
    }

    // live spectrum of the output (last 4096 samples, Hann), log frequency, with a slow peak hold
    void drawAnalyzer(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 8.0f, sf::Color(0, 0, 0, 90), Theme::cardLine);
        const int N = Capture::RING;
        static std::vector<std::complex<float>> buf(N);
        int pos = cap.ringPos;
        for (int i = 0; i < N; ++i) {
            float w = 0.5f - 0.5f * std::cos(2.0f * 3.14159265f * i / (N - 1));
            buf[i] = cap.ring[(pos + i) % N] * w;
        }
        fft(buf);
        const int COLS = 256;
        std::vector<sf::Vector2f> pts, hold;
        auto fy = [&](float db) { return r.top + r.height - 6 - std::clamp((db + 72.0f) / 72.0f, 0.0f, 1.0f) * (r.height - 24); };
        for (int c = 0; c < COLS; ++c) {
            float f0 = fLo() * std::pow(fHi() / fLo(), (float)c / COLS), f1 = fLo() * std::pow(fHi() / fLo(), (float)(c + 1) / COLS);
            int k0 = std::max(1, (int)(f0 / sr() * N)), k1 = std::max(k0 + 1, (int)(f1 / sr() * N) + 1);
            float p = 0;
            for (int k = k0; k < k1; ++k) p = std::max(p, std::norm(buf[k]));
            float db = 10.0f * std::log10(p / (N * N * 0.0625f) + 1e-12f);
            peakHold[c] = std::max(db, peakHold[c] - 0.35f);
            float x = r.left + 8 + (r.width - 16) * (c + 0.5f) / COLS;
            pts.push_back({ x, fy(db) });
            hold.push_back({ x, fy(peakHold[c]) });
        }
        for (float hz : { 100.0f, 1000.0f, 10000.0f }) {
            float x = r.left + 8 + (r.width - 16) * fNorm(hz);
            vline(t, x, r.top + 18, r.height - 26, sf::Color(255, 255, 255, 12));
            label(t, hz >= 1000 ? (hz >= 10000 ? "10k" : "1k") : "100", x + 3, r.top + r.height - 10, 7, Theme::faint, LEFT);
        }
        fill(t, pts, r.top + r.height - 6, alpha(Theme::blockAccent[3], 110), alpha(Theme::blockAccent[3], 10));
        curve(t, pts, alpha(Theme::blockAccent[3], 230), 1.5f);
        curve(t, hold, sf::Color(255, 255, 255, 70), 1.0f);
        label(t, "LIVE SPECTRUM", r.left + 10, r.top + 10, 8, Theme::faint, LEFT, false, 1.5f);
        label(t, "0 .. -72 dB", r.left + r.width - 10, r.top + 10, 8, Theme::faint, RIGHT, false, 1.0f);
    }

    // ------------------------------------------------------------------ cards
    void drawBlock(sf::RenderTarget& t, int b)
    {
        auto r = blockRect(b);
        auto& k = Kick32v4Panel::block(b);
        sf::Color accent = Theme::blockAccent[b];
        rrect(t, r, 18.0f, Theme::card, mix(Theme::cardLine, accent, glow[b] * 0.5f));
        // accent line on top, glowing with the block's activity
        rrect(t, { r.left + 18, r.top, r.width - 36, 3 }, 1.5f, alpha(accent, 120 + (int)(135 * glow[b])));
        if (glow[b] > 0.02f) rrect(t, { r.left + 18, r.top - 2, r.width - 36, 7 }, 3.5f, alpha(accent, (int)(60 * glow[b])));

        label(t, k.name, r.left + 18, r.top + 20, 13, accent, LEFT, true, 4.0f);
        label(t, k.subtitle, r.left + 18, r.top + 36, 9, Theme::muted, LEFT, false, 1.0f);

        sf::FloatRect pv = k.rows < 4 ? sf::FloatRect { r.left + r.width - 300, r.top + 8, 284, HEAD_S - 14 }
                                       : sf::FloatRect { r.left + 14, r.top + 48, r.width - 28, HEAD_T - 58 };
        switch (b) {
        case 0: drawWavePreview(t, pv, accent); break;
        case 1: drawMotionPreview(t, pv, accent); break;
        case 2: drawDrivePreview(t, pv, accent); break;
        case 3: drawFilterPreview(t, pv, accent); break;
        }

        // group pills with captions
        for (int g = 0; g < Kick32v4Panel::GROUPS; ++g) {
            auto& grp = Kick32v4Panel::group(g);
            if (Kick32v4Panel::blockOf(grp.firstPot) != b) continue;
            float x = colX(Kick32v4Panel::potCol(grp.firstPot)), y = rowY(grp.firstPot);
            sf::FloatRect pill { x + 6, y + 6, grp.count * CELL_W - 12, ROW_PITCH - 24 };
            rrect(t, pill, 14.0f, sf::Color(255, 255, 255, 4), alpha(accent, 40));
            float tw = textWidth(grp.name, 8, true, 2.0f);
            rrect(t, { pill.left + 14.0f, pill.top - 7.0f, tw + 14.0f, 14.0f }, 6.0f, Theme::card);
            label(t, grp.name, pill.left + 21.0f, pill.top, 8, alpha(accent, 235), LEFT, true, 2.0f);
        }
    }

    // chevrons in the gaps: body > drive > filter > out
    void drawFlow(sf::RenderTarget& t)
    {
        auto chevron = [&](float x, float y, sf::Color c) {
            sf::ConvexShape s(3);
            s.setPoint(0, { x - 6, y - 9 });
            s.setPoint(1, { x + 6, y });
            s.setPoint(2, { x - 6, y + 9 });
            s.setFillColor(c);
            t.draw(s);
        };
        float yMid = CARDS_Y + CARD_T_H * 0.5f;
        float g1 = colX(4) - GAP * 0.5f, g2 = colX(6) - GAP * 0.5f;
        for (int i = -1; i <= 1; ++i) {
            chevron(g1, yMid + i * 22, alpha(Theme::blockAccent[2], i == 0 ? 220 : 90));
            chevron(g2, yMid + i * 22, alpha(Theme::blockAccent[3], i == 0 ? 220 : 90));
        }
        label(t, "OUT", W - 10, yMid, 9, Theme::muted, RIGHT, true, 2.0f);
    }

    // ---- previews
    void previewBox(sf::RenderTarget& t, sf::FloatRect r) { rrect(t, r, 10.0f, sf::Color(0, 0, 0, 70), Theme::cardLine); }

    void drawWavePreview(sf::RenderTarget& t, sf::FloatRect r, sf::Color accent)
    {
        previewBox(t, r);
        const int n = 120;
        std::vector<sf::Vector2f> pts, ref;
        float cy = r.top + 6 + (r.height - 6) * 0.5f, sy = ((r.height - 6) * 0.5f - 5) / 1.3f;
        for (int i = 0; i <= n; ++i) {
            float x = r.left + 12 + (r.width - 24) * i / n;
            float v = std::clamp(panel.engine.waveSample((float)i / n), -1.3f, 1.3f);
            pts.push_back({ x, cy - v * sy });
            ref.push_back({ x, cy - std::sin(2.0f * 3.14159265f * i / n) * sy });
        }
        hline(t, r.left + 12, cy, r.width - 24, sf::Color(255, 255, 255, 14));
        curve(t, ref, sf::Color(255, 255, 255, 22), 1.0f);
        curve(t, pts, accent, 1.8f);
        label(t, "ONE CYCLE", r.left + 10, r.top + 8, 7, Theme::faint, LEFT, false, 1.5f);
    }

    // MOTION: what moves over the decay. Pitch (drop + bend, octaves), the FM index and the shift in Hz, over time.
    void drawMotionPreview(sf::RenderTarget& t, sf::FloatRect r, sf::Color accent)
    {
        previewBox(t, r);
        auto& e = panel.engine;
        float total = std::max(e.decay.value, 60.0f);
        float x0 = r.left + 12, w = r.width - 24, y0 = r.top + 15, h = r.height - 19;
        float mid = y0 + h * 0.55f;
        hline(t, x0, mid, w, sf::Color(255, 255, 255, 14));
        std::vector<sf::Vector2f> pc, fc, sc;
        float maxShift = std::max(50.0f, std::abs(e.shiftHzAt(0.0f)));
        for (int i = 0; i <= 160; ++i) {
            float ms = total * i / 160;
            float oct = e.pitchOctAt(ms);
            pc.push_back({ x0 + w * i / 160, std::clamp(mid - oct / 6.0f * h * 0.6f, y0, y0 + h) });
            fc.push_back({ x0 + w * i / 160, y0 + h - std::min(1.0f, e.fmIndexAt(ms) * 0.5f) * h });
            sc.push_back({ x0 + w * i / 160, mid - e.shiftHzAt(ms) / maxShift * h * 0.38f });
        }
        if (e.fmDepth.value > 0) curve(t, fc, alpha(Theme::blockAccent[2], 160), 1.3f);
        if (std::abs(e.shift.value) > 0) curve(t, sc, alpha(Theme::blockAccent[3], 180), 1.3f);
        curve(t, pc, accent, 1.8f);
        // live playhead
        if (e.playing() && e.envAmp() > 0.0f) {
            // ampEnv is not time; use the amp shape inverse is overkill: show the drop envelope position instead
            float x = x0 + w * std::clamp(1.0f - e.envAmp(), 0.0f, 1.0f);
            vline(t, x, y0, h, alpha(Theme::text, 40));
        }
        label(t, "OVER THE DECAY", r.left + 10, r.top + 8, 7, Theme::faint, LEFT, false, 1.5f);
        char buf[64];
        snprintf(buf, sizeof(buf), "pitch%s%s", e.fmDepth.value > 0 ? "  ·  FM index" : "", std::abs(e.shift.value) > 0 ? "  ·  shift Hz" : "");
        label(t, buf, r.left + r.width - 10, r.top + 8, 7, alpha(accent, 200), RIGHT, true, 1.0f);
    }

    // the drive transfer curve for an input -1..1 -> output, with the live signal riding on it
    void drawDrivePreview(sf::RenderTarget& t, sf::FloatRect r, sf::Color accent)
    {
        previewBox(t, r);
        auto& e = panel.engine;
        float cx = r.left + r.width * 0.5f, cy = r.top + r.height * 0.5f;
        float hw = r.width * 0.5f - 14, hh = r.height * 0.5f - 10;
        hline(t, cx - hw, cy, 2 * hw, sf::Color(255, 255, 255, 14));
        vline(t, cx, cy - hh, 2 * hh, sf::Color(255, 255, 255, 14));
        std::vector<sf::Vector2f> pts;
        const int n = 160;
        for (int i = 0; i <= n; ++i) {
            float x = -1.0f + 2.0f * i / n;
            float y = std::clamp(e.shaperPreview(x), -1.0f, 1.0f);
            pts.push_back({ cx + x * hw, cy - y * hh });
        }
        curve(t, pts, accent, 1.8f);
        // the signal: (pre / gain, post) pairs, the newest brightest. The x axis is the pre-gain input.
        float g = std::max(1.0f, e.driveGain());
        for (int i = 0; i < Capture::TRAIL; ++i) {
            int k = (cap.trailPos + i) % Capture::TRAIL;
            float x = std::clamp(cap.trailPre[k] / g, -1.0f, 1.0f), y = std::clamp(cap.trailPost[k], -1.0f, 1.0f);
            float age = (float)i / Capture::TRAIL;
            disc(t, cx + x * hw, cy - y * hh, 1.5f + 2.0f * age, alpha(Theme::text, (int)(10 + 150 * age * age)));
        }
        char buf[48];
        const char* ch = e.character.value < 33 ? "soft > hard" : (e.character.value < 66 ? "hard > fold" : "fold > fuzz");
        snprintf(buf, sizeof(buf), "x%.0f  ·  %s", e.driveGain(), ch);
        label(t, "TRANSFER", r.left + 10, r.top + 9, 8, Theme::faint, LEFT, false, 1.5f);
        label(t, buf, r.left + r.width - 10, r.top + 9, 8, alpha(accent, 220), RIGHT, true, 1.0f);
    }

    // frequency response of the filterbank, LP > HP in series, note 60. The bright curve follows the live cutoff
    // offset (envelope, FM, drop): it breathes with the hits. The faint one is the resting position.
    void drawFilterPreview(sf::RenderTarget& t, sf::FloatRect r, sf::Color accent)
    {
        previewBox(t, r);
        auto& e = panel.engine;
        const float s = e.sr();
        float gain = e.filterGain();
        auto fx = [&](float hz) { return r.left + 12 + (r.width - 24) * std::log(hz / 20.0f) / std::log(1000.0f); };
        auto fy = [&](float db) { return r.top + r.height - 8 - std::clamp((db + 36.0f) / 60.0f, 0.0f, 1.0f) * (r.height - 16); };
        for (float hz : { 100.0f, 1000.0f, 10000.0f }) vline(t, fx(hz), r.top + 6, r.height - 12, sf::Color(255, 255, 255, 10));
        hline(t, r.left + 12, fy(0.0f), r.width - 24, sf::Color(255, 255, 255, 16));
        auto response = [&](float oct, std::vector<sf::Vector2f>& pts) {
            float m = std::pow(2.0f, oct);
            float c1 = e.filterCutoff1Hz() * m, c2 = e.filterCutoff2Hz() * m, k = e.filterK();
            float g1 = std::tan((float)M_PI * std::clamp(c1, 15.0f, s * 0.42f) / s), g2 = std::tan((float)M_PI * std::clamp(c2, 15.0f, s * 0.42f) / s);
            float vm = e.vowelMixValue(), vk = e.vowelK();
            float gv1 = std::tan((float)M_PI * std::clamp(e.vowelHz1(), 15.0f, s * 0.42f) / s), gv2 = std::tan((float)M_PI * std::clamp(e.vowelHz2(), 15.0f, s * 0.42f) / s);
            for (int i = 0; i <= 120; ++i) {
                float f = 20.0f * std::pow(1000.0f, i / 120.0f);
                float wf = std::tan((float)M_PI * f / s);
                std::complex<float> s1(0.0f, wf / g1), s2(0.0f, wf / g2);
                std::complex<float> d1 = s1 * s1 + k * s1 + 1.0f, d2 = s2 * s2 + k * s2 + 1.0f;
                std::complex<float> H = (1.0f / d1) * (s2 * s2 / d2) * gain;
                if (vm > 0.0f) { // the vowel formants are in the same chain
                    std::complex<float> sv1(0.0f, wf / gv1), sv2(0.0f, wf / gv2);
                    std::complex<float> b1 = sv1 / (sv1 * sv1 + vk * sv1 + 1.0f), b2 = sv2 / (sv2 * sv2 + vk * sv2 + 1.0f);
                    H *= (1.0f - 0.3f * vm) + (b1 + b2) * vk * 4.0f * vm;
                }
                float db = 20.0f * std::log10(std::max(1e-5f, std::abs(H)));
                pts.push_back({ fx(f), fy(db) });
            }
        };
        std::vector<sf::Vector2f> rest, live;
        response(0.0f, rest);
        curve(t, rest, alpha(accent, 80), 1.4f);
        if (std::abs(liveOct) > 0.02f) {
            response(liveOct, live);
            curve(t, live, accent, 1.8f);
        } else {
            curve(t, rest, accent, 1.8f);
        }
        char buf[64];
        auto hz = [](char* b, size_t n, float v) { if (v >= 1000) snprintf(b, n, "%.1fk", v / 1000); else snprintf(b, n, "%.0f", v); };
        char a[16], bb[16];
        hz(a, 16, e.filterCutoff1Hz());
        hz(bb, 16, e.filterCutoff2Hz());
        if (e.vowelMixValue() > 0.0f) snprintf(buf, sizeof(buf), "LP %s  >  HP %s  ·  vowel %.0f / %.0f Hz", a, bb, e.vowelHz1(), e.vowelHz2());
        else snprintf(buf, sizeof(buf), "LP %s  >  HP %s  ·  drive x%.0f", a, bb, e.filterDriveGain());
        label(t, "RESPONSE", r.left + 10, r.top + 9, 8, Theme::faint, LEFT, false, 1.5f);
        label(t, buf, r.left + r.width - 10, r.top + 9, 8, alpha(accent, 220), RIGHT, true, 1.0f);
        label(t, "100", fx(100.0f) + 3, r.top + r.height - 10, 7, Theme::faint, LEFT);
        label(t, "1k", fx(1000.0f) + 3, r.top + r.height - 10, 7, Theme::faint, LEFT);
        label(t, "10k", fx(10000.0f) + 3, r.top + r.height - 10, 7, Theme::faint, LEFT);
    }

    void drawKnob(sf::RenderTarget& t, int pot, sf::Color accent)
    {
        auto ctr = knobCenter(pot);
        bool active = dragging == pot;
        bool hot = active || hoverPot == pot;
        float n = shown[pot];
        bool centered = panel.isCentered(pot);
        const float R = 27.0f;
        auto angle = [](float v) { return -135.0f + 270.0f * v; };

        disc(t, ctr.x, ctr.y, 36.0f, sf::Color(20, 22, 28));
        arc(t, ctr.x, ctr.y, R, 5.0f, -135.0f, 135.0f, Theme::track);
        // default position tick
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

    // a help line under the hovered knob
    void drawHelp(sf::RenderTarget& t, int pot)
    {
        auto ctr = knobCenter(pot);
        std::string text = Kick32v4Panel::help(pot);
        sf::Color accent = Theme::blockAccent[Kick32v4Panel::blockOf(pot)];
        // wrap on ~52 chars
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
        w += 28;
        float h = 16.0f * lines.size() + 14;
        float x = std::clamp(ctr.x - w * 0.5f, 10.0f, W - w - 10.0f), y = ctr.y + 70;
        if (y + h > H - 4) y = ctr.y - 48 - h;
        rrect(t, { x, y, w, h }, 8.0f, sf::Color(28, 30, 40, 245), alpha(accent, 140));
        for (size_t i = 0; i < lines.size(); ++i) label(t, lines[i], x + 14, y + 14 + 16 * i, 11, Theme::text, LEFT);
    }

    void drawSequencer(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "TEST SEQUENCER", CARD_X, SEQ_Y, 11, Theme::muted, LEFT, true, 2.4f);
        label(t, "click / drag: steps   wheel: note   shift+wheel: octave   right click: reset note   space: trigger / mute   enter: play   1-6: presets   D: dice", W - 24, SEQ_Y, 10, Theme::faint, RIGHT, false, 0.8f);

        int cur = seq.current.load();
        for (int i = 0; i < Sequencer::STEPS; ++i) {
            auto r = stepRect(i);
            const Step& s = seq.steps[i];
            bool isCur = i == cur;
            bool hot = r.contains(mouse);
            bool downbeat = (i % 4) == 0;

            sf::Color fill = s.on ? Theme::kick : (isCur ? sf::Color(46, 49, 62) : (hot ? sf::Color(36, 39, 49) : Theme::cell));
            if (s.on && s.note != 0) {
                float k = std::clamp(s.note / 12.0f, -1.0f, 1.0f);
                fill = k > 0 ? sf::Color(255, (sf::Uint8)(176 + 50 * k), (sf::Uint8)(64 + 90 * k)) : sf::Color((sf::Uint8)(255 + 100 * k), (sf::Uint8)(176 - 60 * -k), (sf::Uint8)(64 + 190 * -k));
            }
            if (s.on && isCur) rrect(t, { r.left - 3, r.top - 3, r.width + 6, r.height + 6 }, 12.0f, sf::Color(255, 190, 100, 60));
            rrect(t, r, 9.0f, fill, isCur ? Theme::text : sf::Color::Transparent);

            char buf[8];
            if (s.note != 0) {
                snprintf(buf, sizeof(buf), "%+d", s.note);
                label(t, buf, r.left + r.width * 0.5f, r.top + r.height * 0.5f, 14, s.on ? sf::Color(20, 14, 12) : Theme::muted, CENTER, true, 0.2f);
            } else if (s.on) {
                disc(t, r.left + r.width * 0.5f, r.top + r.height * 0.5f, 3.0f, sf::Color(20, 14, 12, 150));
            } else {
                snprintf(buf, sizeof(buf), "%d", i + 1);
                label(t, buf, r.left + r.width * 0.5f, r.top + r.height * 0.5f, 10, downbeat ? Theme::faint : sf::Color(48, 51, 62), CENTER);
            }
        }
    }
};
