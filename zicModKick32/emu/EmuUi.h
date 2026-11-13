#pragma once

/**
 * Emulator only: SFML front panel (32 knobs) + minimal 64 step sequencer.
 * All the "what does a pot do" knowledge lives in core/Kick32Panel.h.
 */

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <mutex>

#include "Sequencer.h"
#include "core/Kick32Panel.h"

extern std::mutex audioMutex;
extern std::atomic<int> kickTrigger; // set to 1 to audition a kick
extern std::atomic<bool> kickMuted; // held: sequencer kicks are skipped

namespace Theme {
inline sf::Color rgb(int r, int g, int b, int a = 255) { return sf::Color(r, g, b, a); }
const sf::Color bg = rgb(12, 13, 17);
const sf::Color card = rgb(20, 22, 28);
const sf::Color cardLine = rgb(34, 37, 46);
const sf::Color cell = rgb(28, 30, 38);
const sf::Color track = rgb(46, 49, 60);
const sf::Color cap = rgb(30, 32, 41);
const sf::Color text = rgb(232, 234, 240);
const sf::Color muted = rgb(124, 129, 144);
const sf::Color faint = rgb(70, 74, 88);
const sf::Color rowAccent[4] = { rgb(255, 128, 64), rgb(56, 214, 255), rgb(176, 124, 255), rgb(92, 232, 140) };
const sf::Color kick = rgb(255, 128, 64);
}

class EmuUi {
public:
    static constexpr float W = 1320.0f;
    static constexpr float H = 1008.0f;
    static constexpr float CARD_X = 48.0f, CARD_W = 1208.0f, CARD_H = 148.0f; // the 4 x 8 panel, the traces run in the gutters and under the knobs
    static constexpr float ROW_Y0 = 84.0f, ROW_PITCH = 180.0f;                // 32 px between rows

    EmuUi(Kick32Panel::Panel& panel, Sequencer& seq, const float* peaks, int peakCount, std::atomic<int>& peakHead)
        : panel(panel)
        , seq(seq)
        , peaks(peaks)
        , peakCount(peakCount)
        , peakHead(peakHead)
    {
        const char* paths[] = {
            "../../draw/fonts/Roboto-Regular.ttf", "../draw/fonts/Roboto-Regular.ttf", "draw/fonts/Roboto-Regular.ttf",
            "/usr/share/fonts/truetype/roboto/unhinted/RobotoTTF/Roboto-Regular.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        };
        for (auto p : paths)
            if (font.loadFromFile(p)) break;
    }

    // ------------------------------------------------------------------ layout
    static sf::FloatRect cardRect(int row) { return { CARD_X, ROW_Y0 + row * ROW_PITCH, CARD_W, CARD_H }; }
    static sf::Vector2f knobCenter(int pot)
    {
        auto c = cardRect(pot / 8);
        return { c.left + CARD_W / 8.0f * (pot % 8 + 0.5f), c.top + 72.0f };
    }
    static sf::FloatRect knobCell(int pot)
    {
        auto ctr = knobCenter(pot);
        float w = CARD_W / 8.0f;
        return { ctr.x - w * 0.5f, ctr.y - 40.0f, w, 112.0f };
    }
    static sf::FloatRect stepRect(int i)
    {
        int row = i / 16, col = i % 16;
        float gap = 6.0f, beatGap = 14.0f;
        float w = (W - CARD_X - 24.0f - 12 * gap - 3 * beatGap) / 16.0f;
        float x = CARD_X + col * (w + gap) + (col / 4) * (beatGap - gap);
        return { x, 818.0f + row * 40.0f, w, 34.0f };
    }
    static sf::FloatRect playBtn() { return { 230.0f, 18.0f, 64.0f, 44.0f }; }
    static sf::FloatRect trigBtn() { return { 304.0f, 18.0f, 64.0f, 44.0f }; }
    static sf::FloatRect bpmBox() { return { 380.0f, 18.0f, 140.0f, 44.0f }; }
    static sf::FloatRect tuneBox() { return { 530.0f, 18.0f, 140.0f, 44.0f }; }

    // ------------------------------------------------------------------ input (logical coordinates)
    void mousePressed(sf::Vector2f p, sf::Mouse::Button button, bool shift)
    {
        std::lock_guard<std::mutex> lock(audioMutex);
        if (button == sf::Mouse::Left) {
            if (playBtn().contains(p)) { seq.toggle(); return; }
            if (trigBtn().contains(p)) { trigDown(); trigHeldByMouse = true; return; }
            if (bpmBox().contains(p)) { stepBpm(p.x > bpmBox().left + 70 ? 1.0f : -1.0f, shift); return; }
            if (tuneBox().contains(p)) { stepTune(p.x > tuneBox().left + 70 ? 1 : -1, shift); return; }
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
        for (int i = 0; i < Kick32Panel::POTS; ++i) {
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

    void mouseReleased()
    {
        if (trigHeldByMouse) { trigUp(); trigHeldByMouse = false; }
        dragging = -1;
        painting = false;
    }

    void mouseMoved(sf::Vector2f p, bool shift)
    {
        hoverPot = -1;
        for (int i = 0; i < Kick32Panel::POTS; ++i)
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
        for (int i = 0; i < Kick32Panel::POTS; ++i) {
            if (!knobCell(i).contains(p)) continue;
            panel.set(i, panel.norm(i) + (delta > 0 ? 1 : -1) * (shift ? 0.002f : 0.01f));
            return;
        }
        if (bpmBox().contains(p)) stepBpm(delta > 0 ? 1.0f : -1.0f, shift);
        else if (tuneBox().contains(p)) stepTune(delta > 0 ? 1 : -1, shift);
    }

    // ------------------------------------------------------------------ drawing
    void draw(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        t.clear(Theme::bg);
        drawTopBar(t, mouse);
        for (int r = 0; r < Kick32Panel::ROWS; ++r) drawRowBase(t, r);
        drawTraces(t);
        for (int i = 0; i < Kick32Panel::POTS; ++i) drawKnob(t, i, Theme::rowAccent[i / 8]); // on top: the traces run under the knobs
        drawSequencer(t, mouse);
    }

private:
    Kick32Panel::Panel& panel;
    Sequencer& seq;
    const float* peaks;
    int peakCount;
    std::atomic<int>& peakHead;

    sf::Font font;
    int dragging = -1, hoverPot = -1, lastClicked = -1;
    float dragY = 0.0f, dragStart = 0.0f;
    bool painting = false, paintValue = false, trigHeldByMouse = false;
    sf::Clock clock, animClock;

    // ---- primitives
    enum Align { LEFT, CENTER, RIGHT };

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
        const int seg = 6;
        s.setPointCount(4 * (seg + 1));
        // clockwise from the top right corner
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

    // ---- widgets
    enum Icon { PLAY, STOP, TRIG, MUTE };

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
        case TRIG: // a pad being hit: dot + ring
            disc(t, x, y, 4.0f, c);
            arc(t, x, y, 10.0f, 2.5f, 0.0f, 360.0f, c, false);
            break;
        case MUTE: // speaker + cross
            poly({ { -11, -4 }, { -6, -4 }, { -6, 4 }, { -11, 4 } });
            poly({ { -6, -4 }, { 0, -9 }, { 0, 9 }, { -6, 4 } });
            line({ 4, -5 }, { 12, 5 }, 2.5f);
            line({ 4, 5 }, { 12, -5 }, 2.5f);
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

    void drawTopBar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "zicMod", 24, 34, 24, Theme::text, LEFT, true, 0.5f);
        label(t, "Kick32", 24 + 86, 34, 24, Theme::kick, LEFT, true, 0.5f);
        label(t, "WAVETABLE KICK  /  32 POTS", 24, 58, 10, Theme::muted, LEFT, false, 2.2f);

        button(t, playBtn(), seq.playing ? STOP : PLAY, mouse, seq.playing, Theme::rowAccent[3]);
        button(t, trigBtn(), seq.playing ? MUTE : TRIG, mouse, kickMuted.load() || kickTrigger.load() != 0, Theme::kick);

        char buf[16];
        snprintf(buf, sizeof(buf), "%d", (int)std::round(seq.bpm));
        stepper(t, bpmBox(), buf, "BPM");
        snprintf(buf, sizeof(buf), "%+d", seq.transpose);
        stepper(t, tuneBox(), buf, "TUNE");

        drawWavePreview(t, { 690.0f, 18.0f, 140.0f, 44.0f });
        drawPitchPreview(t, { 842.0f, 18.0f, 140.0f, 44.0f });
        drawPeaks(t, { 994.0f, 18.0f, W - 24.0f - 994.0f, 44.0f });
    }

    void stepper(sf::RenderTarget& t, sf::FloatRect b, const char* value, const char* caption)
    {
        rrect(t, b, 12.0f, Theme::card, Theme::cardLine);
        label(t, value, b.left + 54, b.top + 22, 22, Theme::text, CENTER, true, 0.5f);
        label(t, caption, b.left + 84, b.top + 25, 9, Theme::muted, LEFT, false, 1.5f);
        label(t, "-", b.left + 16, b.top + 21, 20, Theme::muted);
        label(t, "+", b.left + b.width - 16, b.top + 21, 20, Theme::muted);
    }

    void drawWavePreview(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 12.0f, Theme::card, Theme::cardLine);
        const int n = 96;
        sf::VertexArray va(sf::LineStrip, n + 1);
        for (int i = 0; i <= n; ++i) {
            float v = panel.engine.waveSample((float)i / n);
            va[i] = sf::Vertex({ r.left + 14 + (r.width - 28) * i / n, r.top + r.height * 0.5f - v * (r.height * 0.5f - 9) }, Theme::rowAccent[1]);
        }
        t.draw(va);
        label(t, "WAVE", r.left + 10, r.top + 8, 8, Theme::faint, LEFT, false, 1.5f);
    }

    // Pitch over time (log scale, 0..5 octaves above the root): shows Sweep and Bend
    void drawPitchPreview(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 12.0f, Theme::card, Theme::cardLine);
        const int n = 80;
        sf::VertexArray va(sf::LineStrip, n + 1);
        float base = r.top + r.height - 9.0f, top = r.top + 9.0f;
        for (int i = 0; i <= n; ++i) {
            float oct = std::log2(std::max(0.05f, panel.engine.pitchRatio((float)i / n)));
            float y = std::clamp(base - (oct + 1.0f) / 6.0f * (base - top), top, base);
            va[i] = sf::Vertex({ r.left + 14 + (r.width - 28) * i / n, y }, Theme::kick);
        }
        t.draw(va);
        label(t, "PITCH", r.left + 10, r.top + 8, 8, Theme::faint, LEFT, false, 1.5f);
    }

    void drawPeaks(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 12.0f, Theme::card, Theme::cardLine);
        int head = peakHead.load();
        float bw = (r.width - 28) / peakCount;
        for (int i = 0; i < peakCount; ++i) {
            float v = std::min(1.0f, peaks[(head + i) % peakCount]);
            float h = std::max(1.5f, v * (r.height - 14));
            sf::RectangleShape bar({ std::max(1.0f, bw - 1.0f), h });
            bar.setPosition(r.left + 14 + i * bw, r.top + r.height * 0.5f - h * 0.5f);
            bar.setFillColor(sf::Color(255, 128, 64, (sf::Uint8)(70 + 185 * i / peakCount)));
            t.draw(bar);
        }
        label(t, "OUT", r.left + 10, r.top + 8, 8, Theme::faint, LEFT, false, 1.5f);
    }

    // ------------------------------------------------------------------ panel art
    // Each row is split in small groups (a pill with a caption tab and a faint hatching), the way
    // you would print them on the panel. The traces below show where the signal goes.
    struct Group { int row, col, count; const char* name; };
    static const Group* groups(int& n)
    {
        static const Group g[] = {
            { 0, 0, 2, "VOICE" }, { 0, 2, 2, "PITCH" }, { 0, 4, 2, "FM" }, { 0, 6, 2, "OSC" },
            { 1, 0, 2, "SHAPE" }, { 1, 2, 2, "HARMONICS" }, { 1, 4, 2, "FOLD" }, { 1, 6, 2, "TWIST" },
            { 2, 0, 2, "SOURCE" }, { 2, 2, 2, "FILTER" }, { 2, 4, 2, "MOTION" }, { 2, 6, 2, "SEND" },
            { 3, 0, 2, "SATURATE" }, { 3, 2, 2, "DAMAGE" }, { 3, 4, 3, "EQ" }, { 3, 7, 1, "OUT" }
        };
        n = (int)(sizeof(g) / sizeof(g[0]));
        return g;
    }

    void drawGroup(sf::RenderTarget& t, const Group& g)
    {
        auto c = cardRect(g.row);
        sf::Color accent = Theme::rowAccent[g.row];
        float cell = CARD_W / 8.0f;
        sf::FloatRect r(c.left + g.col * cell + 8.0f, c.top + 30.0f, g.count * cell - 16.0f, CARD_H - 36.0f);
        rrect(t, r, 14.0f, sf::Color(255, 255, 255, 5), sf::Color(accent.r, accent.g, accent.b, 62));

        // 45 degree hatching, clipped to the pill
        sf::VertexArray hatch(sf::Lines);
        const float inset = 7.0f, step = 9.0f;
        float w = r.width - 2 * inset, h = r.height - 2 * inset;
        for (float k = -h; k < w; k += step) {
            float lo = std::max(0.0f, k), hi = std::min(w, h + k);
            if (lo >= hi) continue;
            sf::Color hc(accent.r, accent.g, accent.b, 11);
            hatch.append(sf::Vertex({ r.left + inset + lo, r.top + inset + lo - k }, hc));
            hatch.append(sf::Vertex({ r.left + inset + hi, r.top + inset + hi - k }, hc));
        }
        t.draw(hatch);

        // caption tab sitting on the top edge
        sf::Text txt(g.name, font, 8);
        txt.setLetterSpacing(2.0f);
        txt.setStyle(sf::Text::Bold);
        float tw = txt.getLocalBounds().width;
        rrect(t, { r.left + 14.0f, r.top - 7.0f, tw + 14.0f, 14.0f }, 6.0f, Theme::card);
        label(t, g.name, r.left + 21.0f, r.top, 8, sf::Color(accent.r, accent.g, accent.b, 235), LEFT, true, 2.0f);
    }

    void drawRowBase(sf::RenderTarget& t, int row)
    {
        auto c = cardRect(row);
        sf::Color accent = Theme::rowAccent[row];
        rrect(t, c, 18.0f, Theme::card, Theme::cardLine);
        disc(t, c.left + 20, c.top + 15, 3.0f, accent);
        label(t, Kick32Panel::sectionName(row), c.left + 32, c.top + 15, 11, accent, LEFT, true, 2.4f);
        int n;
        const Group* g = groups(n);
        for (int i = 0; i < n; ++i)
            if (g[i].row == row) drawGroup(t, g[i]);
    }

    // ---- traces: rounded PCB style polylines
    struct Path {
        std::vector<sf::Vector2f> p;
        std::vector<float> cum;
        float len = 0.0f;
    };

    static Path makePath(std::initializer_list<sf::Vector2f> pts, float radius = 16.0f)
    {
        std::vector<sf::Vector2f> v(pts);
        Path o;
        auto norm = [](sf::Vector2f d, float& l) { l = std::sqrt(d.x * d.x + d.y * d.y); return l > 0 ? d / l : d; };
        o.p.push_back(v.front());
        for (size_t i = 1; i + 1 < v.size(); ++i) {
            float l1, l2;
            sf::Vector2f d1 = norm(v[i - 1] - v[i], l1), d2 = norm(v[i + 1] - v[i], l2);
            float r = std::min({ radius, l1 * 0.5f, l2 * 0.5f });
            sf::Vector2f a = v[i] + d1 * r, b = v[i] + d2 * r;
            for (int k = 0; k <= 8; ++k) {
                float u = k / 8.0f;
                o.p.push_back(a * ((1 - u) * (1 - u)) + v[i] * (2 * (1 - u) * u) + b * (u * u));
            }
        }
        o.p.push_back(v.back());
        o.cum.assign(o.p.size(), 0.0f);
        for (size_t i = 1; i < o.p.size(); ++i) {
            sf::Vector2f d = o.p[i] - o.p[i - 1];
            o.cum[i] = o.cum[i - 1] + std::sqrt(d.x * d.x + d.y * d.y);
        }
        o.len = o.cum.back();
        return o;
    }

    static sf::Vector2f pointAt(const Path& o, float s, sf::Vector2f* tangent = nullptr)
    {
        size_t i = std::upper_bound(o.cum.begin(), o.cum.end(), s) - o.cum.begin();
        i = std::clamp<size_t>(i, 1, o.p.size() - 1);
        sf::Vector2f d = o.p[i] - o.p[i - 1];
        float l = std::sqrt(d.x * d.x + d.y * d.y);
        if (tangent) *tangent = l > 0 ? d / l : sf::Vector2f(1, 0);
        float u = l > 0 ? (s - o.cum[i - 1]) / l : 0.0f;
        return o.p[i - 1] + d * std::clamp(u, 0.0f, 1.0f);
    }

    template <typename ColorFn>
    void strokePath(sf::RenderTarget& t, const Path& o, float width, ColorFn colorAt)
    {
        size_t n = o.p.size();
        sf::VertexArray va(sf::TriangleStrip, 2 * n);
        for (size_t i = 0; i < n; ++i) {
            sf::Vector2f d = o.p[std::min(i + 1, n - 1)] - o.p[i > 0 ? i - 1 : 0];
            float l = std::sqrt(d.x * d.x + d.y * d.y);
            sf::Vector2f nrm = l > 0 ? sf::Vector2f(-d.y / l, d.x / l) : sf::Vector2f(0, 1);
            sf::Color c = colorAt(o.p[i]);
            va[2 * i] = sf::Vertex(o.p[i] + nrm * (width * 0.5f), c);
            va[2 * i + 1] = sf::Vertex(o.p[i] - nrm * (width * 0.5f), c);
        }
        t.draw(va);
    }

    // glow + core and direction chevrons. The brightness follows the live level, nothing moves.
    template <typename ColorFn>
    void drawTrace(sf::RenderTarget& t, const Path& o, ColorFn colorAt, float strength, float live, const std::vector<sf::Vector2f>& marks)
    {
        auto fade = [&](float a) { return (sf::Uint8)std::clamp(a * strength, 0.0f, 255.0f); };
        strokePath(t, o, 7.0f, [&](sf::Vector2f p) { auto c = colorAt(p); c.a = fade(22.0f + 40.0f * live); return c; });
        strokePath(t, o, 2.2f, [&](sf::Vector2f p) { auto c = colorAt(p); c.a = fade(120.0f + 80.0f * live); return c; });

        // Direction chevrons only at chosen places: centred in the gap between two groups, or in the
        // middle of a long vertical run. `marks` are points near the trace, each snaps to it.
        for (const auto& m : marks) {
            sf::Vector2f p, dir;
            float bd = 1e9f;
            for (size_t i = 1; i < o.p.size(); ++i) { // closest point on every segment
                sf::Vector2f d = o.p[i] - o.p[i - 1];
                float l2 = d.x * d.x + d.y * d.y;
                if (l2 <= 0.0f) continue;
                float u = std::clamp(((m.x - o.p[i - 1].x) * d.x + (m.y - o.p[i - 1].y) * d.y) / l2, 0.0f, 1.0f);
                sf::Vector2f q = o.p[i - 1] + d * u, e = q - m;
                float dd = e.x * e.x + e.y * e.y;
                if (dd < bd) { bd = dd; p = q; dir = d / std::sqrt(l2); }
            }
            if (bd > 1e8f) continue;
            sf::Vector2f nrm(-dir.y, dir.x);
            sf::ConvexShape a(3);
            a.setPoint(0, p + dir * 5.5f);
            a.setPoint(1, p - dir * 4.5f + nrm * 5.0f);
            a.setPoint(2, p - dir * 4.5f - nrm * 5.0f);
            auto c = colorAt(p);
            c.a = fade(215.0f);
            a.setFillColor(c);
            t.draw(a);
        }
    }

    sf::Color rowGradient(float y, int alpha = 255) const // orange -> cyan -> violet -> green following the rows
    {
        float f = std::clamp((y - (ROW_Y0 + 72.0f)) / ROW_PITCH, 0.0f, 3.0f);
        int i = std::min(2, (int)f);
        float u = std::clamp((f - i - 0.42f) / 0.16f, 0.0f, 1.0f); // hold each row's colour, change quickly in the gap
        const sf::Color& a = Theme::rowAccent[i];
        const sf::Color& b = Theme::rowAccent[i + 1];
        return sf::Color((sf::Uint8)(a.r + (b.r - a.r) * u), (sf::Uint8)(a.g + (b.g - a.g) * u), (sf::Uint8)(a.b + (b.b - a.b) * u), (sf::Uint8)alpha);
    }

    void ring(sf::RenderTarget& t, float x, float y, float r, sf::Color c)
    {
        arc(t, x, y, r, 2.0f, 0.0f, 360.0f, c, false);
        disc(t, x, y, r * 0.35f, c);
    }

    void drawTraces(sf::RenderTarget& t)
    {
        auto& e = panel.engine;
        auto n = [](Param& p) { return (p.value - p.min) / (p.max - p.min); };
        auto lv = [](float) { return 0.0f; }; // the traces do not react to the live level: they only show the amounts set
        float seconds = animClock.getElapsedTime().asSeconds();

        const float cell = CARD_W / 8.0f, left = CARD_X, right = CARD_X + CARD_W;
        const float XL = 24.0f, XR = right + 13.0f, XP = right + 27.0f;
        auto cy = [](int r) { return cardRect(r).top + 72.0f; };
        auto gap = [](int r) { auto c = cardRect(r); return c.top + c.height + (ROW_PITCH - CARD_H) * 0.5f; }; // middle of the gap below row r
        auto K = [](int pot) { return knobCenter(pot); };

        auto bx = [&](float k) { return left + k * cell; }; // x of the gap between groups (in cell units)

        // MAIN: the oscillator. Trigger in -> BODY -> (carriage return) -> WAVE -> down the right side -> MASTER -> OUT
        Path main = makePath({ { XL, cy(0) }, { XR, cy(0) }, { XR, gap(0) }, { XL, gap(0) }, { XL, cy(1) }, { XR, cy(1) },
            { XR, gap(2) }, { XL, gap(2) }, { XL, cy(3) }, { right + 30.0f, cy(3) } });
        std::vector<sf::Vector2f> mainMarks;
        for (int r : { 0, 1 })                                  // along the groups of BODY and WAVE
            for (float k : { 2.0f, 4.0f, 6.0f }) mainMarks.push_back({ bx(k), cy(r) });
        for (float k : { 7.0f, 5.0f, 3.0f, 1.0f }) { mainMarks.push_back({ bx(k), gap(0) }); mainMarks.push_back({ bx(k), gap(2) }); } // carriage returns
        mainMarks.push_back({ XR, cy(1) + (gap(2) - cy(1)) * 0.5f }); // down the right side
        for (float k : { 2.0f, 7.0f }) mainMarks.push_back({ bx(k), cy(3) }); // MASTER
        float live = lv(std::max({ e.meterBody, e.meterPre, e.meterOut }));
        drawTrace(t, main, [&](sf::Vector2f p) { return rowGradient(p.y); }, 1.0f, live, mainMarks);

        // PITCH: leaves the PITCH group, goes over the top and comes back into Noise Track
        float track = n(e.noiseTrack);
        Path pitch = makePath({ { K(3).x, K(3).y - 29.0f }, { K(3).x, 73.0f }, { XP, 73.0f }, { XP, cy(2) }, { K(23).x + 30.0f, cy(2) } });
        drawTrace(t, pitch, [&](sf::Vector2f) { return Theme::rowAccent[0]; }, 0.25f + 0.75f * track, lv(e.meterBody) * track,
            { { (K(3).x + XP) * 0.5f, 73.0f }, { XP, (73.0f + cy(2)) * 0.5f } });

        // NOISE: out of the SOURCE group, a tap after the filter that is mixed in after the crush ...
        float tapX = bx(4.0f);
        float noiseAmt = n(e.noiseAmt), crunch = n(e.crunch);
        Path noiseOut = makePath({ { left + 14.0f, cy(2) }, { tapX, cy(2) }, { tapX, cy(3) } });
        drawTrace(t, noiseOut, [&](sf::Vector2f) { return Theme::rowAccent[2]; }, 0.25f + 0.75f * noiseAmt, lv(e.meterNoise) * noiseAmt,
            { { bx(2.0f), cy(2) }, { tapX, cy(2) + 52.0f } });

        // ... and the SEND to Crunch: dropped into the main trace, before the distortion
        Path send = makePath({ { tapX, cy(2) }, { K(22).x, cy(2) }, { K(22).x, gap(2) } });
        drawTrace(t, send, [&](sf::Vector2f) { return Theme::rowAccent[2]; }, 0.25f + 0.75f * crunch, lv(e.meterNoise) * crunch,
            { { bx(6.0f), cy(2) } });

        // terminals and junctions
        ring(t, XL, cy(0), 6.5f, withAlpha(rowGradient(cy(0)), 230));
        label(t, "TRIG", XL, cy(0) - 18.0f, 8, Theme::muted, CENTER, true, 1.2f);
        ring(t, right + 30.0f, cy(3), 7.5f, withAlpha(Theme::rowAccent[3], 120 + 135 * lv(e.meterOut)));
        label(t, "OUT", right + 30.0f, cy(3) + 20.0f, 8, Theme::muted, CENTER, true, 1.2f);
        ring(t, left + 14.0f, cy(2), 5.0f, withAlpha(Theme::rowAccent[2], 90 + 150 * std::max(noiseAmt, crunch)));
        disc(t, K(22).x, gap(2), 4.0f, withAlpha(Theme::rowAccent[2], 90 + 165 * crunch));
        disc(t, tapX, cy(3), 4.0f, withAlpha(Theme::rowAccent[2], 90 + 165 * noiseAmt));
        disc(t, K(3).x, K(3).y - 29.0f, 3.5f, withAlpha(Theme::rowAccent[0], 90 + 165 * track));
    }

    static sf::Color withAlpha(sf::Color c, float a) { c.a = (sf::Uint8)std::clamp(a, 0.0f, 255.0f); return c; }

    void drawKnob(sf::RenderTarget& t, int pot, sf::Color accent)
    {
        auto ctr = knobCenter(pot);
        bool active = dragging == pot;
        bool hot = active || hoverPot == pot;
        float n = panel.norm(pot);
        bool centered = panel.isCentered(pot);
        const float R = 27.0f;
        auto angle = [](float v) { return -135.0f + 270.0f * v; };

        // the traces and the hatching pass UNDER the knob: a plain disc keeps the knob itself clean
        disc(t, ctr.x, ctr.y, 36.0f, sf::Color(21, 23, 29));

        // track + glow + value arc
        arc(t, ctr.x, ctr.y, R, 5.0f, -135.0f, 135.0f, Theme::track);
        float from = centered ? 0.0f : -135.0f;
        float to = angle(n);
        if (std::abs(to - from) > 0.5f) {
            arc(t, ctr.x, ctr.y, R, active ? 13.0f : 10.0f, from, to, sf::Color(accent.r, accent.g, accent.b, active ? 70 : 34), false);
            arc(t, ctr.x, ctr.y, R, 5.0f, from, to, accent);
        }

        // cap + indicator
        disc(t, ctr.x, ctr.y, 19.0f, hot ? sf::Color(52, 56, 70) : Theme::cardLine);
        disc(t, ctr.x, ctr.y, 18.0f, Theme::cap);
        float a = angle(n) * 3.14159265f / 180.0f;
        sf::VertexArray ind(sf::Lines, 2);
        ind[0] = sf::Vertex({ ctr.x + 7.0f * std::sin(a), ctr.y - 7.0f * std::cos(a) }, Theme::text);
        ind[1] = sf::Vertex({ ctr.x + 16.0f * std::sin(a), ctr.y - 16.0f * std::cos(a) }, Theme::text);
        t.draw(ind);
        disc(t, ctr.x + 16.0f * std::sin(a), ctr.y - 16.0f * std::cos(a), 1.6f, hot ? accent : Theme::text);

        // texts
        char buf[32];
        panel.formatValue(pot, buf, sizeof(buf));
        label(t, panel.param(pot).label, ctr.x, ctr.y + 43.0f, 12, hot ? Theme::text : sf::Color(176, 181, 196), CENTER, false, 0.6f);
        label(t, buf, ctr.x, ctr.y + 57.0f, 11, hot ? accent : Theme::muted, CENTER, hot, 0.4f);
    }

    void drawSequencer(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "SEQUENCER", CARD_X, 794, 11, Theme::muted, LEFT, true, 2.4f);
        label(t, "click / drag: steps    wheel: note    shift+wheel: octave    right click: reset note    space: trigger / mute    enter: play", W - 24, 794, 10, Theme::faint, RIGHT, false, 0.8f);

        int cur = seq.current.load();
        for (int i = 0; i < Sequencer::STEPS; ++i) {
            auto r = stepRect(i);
            const Step& s = seq.steps[i];
            bool isCur = i == cur;
            bool hot = r.contains(mouse);
            bool downbeat = (i % 4) == 0;

            sf::Color fill = s.on ? Theme::kick : (isCur ? sf::Color(46, 49, 62) : (hot ? sf::Color(36, 39, 49) : Theme::cell));
            if (s.on && s.note != 0) {
                // colour drifts with pitch so melodic movement is readable
                float k = std::clamp(s.note / 12.0f, -1.0f, 1.0f);
                fill = k > 0 ? sf::Color(255, (sf::Uint8)(128 + 70 * k), (sf::Uint8)(64 + 60 * k)) : sf::Color((sf::Uint8)(255 + 120 * k), (sf::Uint8)(128 + 60 * -k), (sf::Uint8)(64 + 190 * -k));
            }
            if (s.on && isCur) {
                rrect(t, { r.left - 3, r.top - 3, r.width + 6, r.height + 6 }, 12.0f, sf::Color(255, 160, 100, 60));
            }
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
