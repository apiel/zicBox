#pragma once

/**
 * Emulator only: SFML front panel (32 knobs) + minimal 64 step sequencer.
 * All the "what does a pot do" knowledge lives in core/Kick32v2Panel.h.
 */

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <mutex>

#include "Sequencer.h"
#include "core/Kick32v2Panel.h"

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
const sf::Color blockAccent[3] = { rgb(255, 128, 64), rgb(56, 214, 255), rgb(92, 232, 140) }; // BODY, LAYER, MASTER
const sf::Color kick = rgb(255, 128, 64);
}

class EmuUi {
public:
    static constexpr float W = 1320.0f;
    static constexpr float H = 1090.0f;
    static constexpr float CARD_X = 24.0f, CELL_W = 150.0f, BLOCK_GAP = 24.0f, SPINE = 30.0f; // spine: the vertical title of a module
    static constexpr float ROW_Y0 = 262.0f, ROW_PITCH = 146.0f;
    static constexpr float SCOPE_Y = 84.0f, SCOPE_H = 160.0f;
    static constexpr float SEQ_Y = 880.0f;

    // Scope of the last hit, filled by the audio thread: per bucket the peak of the output and of the layer
    struct Scope {
        static constexpr int N = 640;  // buckets
        float out[N] = {}, layer[N] = {};
        float msPerBucket = 1.0f;
    };

    EmuUi(Kick32v2Panel::Panel& panel, Sequencer& seq, const Scope& scope)
        : panel(panel)
        , seq(seq)
        , scope(scope)
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
    static float colX(int col) { return CARD_X + SPINE + col * CELL_W + (col >= 4 ? BLOCK_GAP + SPINE : 0.0f); }
    static float rowY(int row) { return ROW_Y0 + row * ROW_PITCH; }
    static sf::FloatRect blockRect(int b)
    {
        auto& k = Kick32v2Panel::block(b);
        float x = colX(k.col) - SPINE, y = rowY(k.row) - 10.0f;
        return { x, y, SPINE + k.cols * CELL_W, k.rows * ROW_PITCH - 10.0f };
    }
    static sf::Vector2f knobCenter(int pot)
    {
        return { colX(Kick32v2Panel::potCol(pot)) + CELL_W * 0.5f, rowY(Kick32v2Panel::potRow(pot)) + 50.0f };
    }
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
        for (int i = 0; i < Kick32v2Panel::POTS; ++i) {
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
        for (int i = 0; i < Kick32v2Panel::POTS; ++i)
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
        for (int i = 0; i < Kick32v2Panel::POTS; ++i) {
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
        drawScope(t, { CARD_X, SCOPE_Y, W - 2 * CARD_X, SCOPE_H });
        for (int b = 0; b < Kick32v2Panel::BLOCKS; ++b) drawBlock(t, b);
        for (int i = 0; i < Kick32v2Panel::POTS; ++i) drawKnob(t, i, Theme::blockAccent[Kick32v2Panel::blockOf(i)]);
        drawSequencer(t, mouse);
    }

private:
    Kick32v2Panel::Panel& panel;
    Sequencer& seq;
    const Scope& scope;

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
        label(t, "Kick32v2", 24 + 86, 34, 24, Theme::kick, LEFT, true, 0.5f);
        label(t, "PUNCH + TAIL KICK FORGE  /  32 POTS", 24, 58, 10, Theme::muted, LEFT, false, 2.2f);

        button(t, playBtn(), seq.playing ? STOP : PLAY, mouse, seq.playing, Theme::blockAccent[2]);
        button(t, trigBtn(), seq.playing ? MUTE : TRIG, mouse, kickMuted.load() || kickTrigger.load() != 0, Theme::kick);

        char buf[16];
        snprintf(buf, sizeof(buf), "%d", (int)std::round(seq.bpm));
        stepper(t, bpmBox(), buf, "BPM");
        snprintf(buf, sizeof(buf), "%+d", seq.transpose);
        stepper(t, tuneBox(), buf, "TUNE");

        drawWavePreview(t, { 690.0f, 18.0f, 200.0f, 44.0f });
        drawSourcePreview(t, { 902.0f, 18.0f, 200.0f, 44.0f });
        drawBlendPreview(t, { 1114.0f, 18.0f, W - 24.0f - 1114.0f, 44.0f });
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
            va[i] = sf::Vertex({ r.left + 14 + (r.width - 28) * i / n, r.top + r.height * 0.5f - v * (r.height * 0.5f - 9) }, Theme::blockAccent[0]);
        }
        t.draw(va);
        label(t, "BODY WAVE", r.left + 10, r.top + 8, 8, Theme::faint, LEFT, false, 1.5f);
    }

    // ------------------------------------------------------------------ previews (top bar)
    static sf::Color alpha(sf::Color c, int a) { return sf::Color(c.r, c.g, c.b, (sf::Uint8)std::clamp(a, 0, 255)); }

    // One cycle of the tail wave (with the stack as a smear of detuned copies)
    void drawSourcePreview(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 12.0f, Theme::card, Theme::cardLine);
        auto& e = panel.engine;
        float st = e.stack.value / 100.0f;
        const int n = 100;
        for (int v = st > 0.0f ? 2 : 0; v >= 0; --v) {
            float det = v == 0 ? 0.0f : (v == 1 ? 1.0f : -1.0f) * st * st * 0.06f;
            sf::VertexArray va(sf::LineStrip, n + 1);
            for (int k = 0; k <= n; ++k) {
                float p = (float)k / n * (1.0f + det) + det;
                float val = e.tailWaveSample(p);
                va[k] = sf::Vertex({ r.left + 14 + (r.width - 28) * k / n, r.top + r.height * 0.5f + 4 - std::clamp(val, -1.0f, 1.0f) * (r.height * 0.5f - 11) },
                    alpha(Theme::blockAccent[1], v == 0 ? 255 : 90));
            }
            t.draw(va);
        }
        label(t, "TAIL WAVE", r.left + 10, r.top + 8, 8, Theme::faint, LEFT, false, 1.5f);
    }

    // Handover: the body level (orange) giving way to the tail (cyan) at Start
    void drawBlendPreview(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 12.0f, Theme::card, Theme::cardLine);
        auto& e = panel.engine;
        float total = std::max(e.decay.value, e.tailStartMs() + 40.0f);
        float tm = e.tail.value / 100.0f;
        float x0 = r.left + 12, w = r.width - 24, base = r.top + r.height - 7, h = r.height - 20;
        sf::VertexArray bodyL(sf::LineStrip, 61), tailL(sf::LineStrip, 61);
        for (int i = 0; i <= 60; ++i) {
            float ms = total * i / 60;
            float a = std::clamp((ms - e.tailStartMs()) / 20.0f, 0.0f, 1.0f) * tm;
            float x = x0 + w * i / 60;
            bodyL[i] = sf::Vertex({ x, base - std::sqrt(1.0f - a) * h }, Theme::blockAccent[0]);
            tailL[i] = sf::Vertex({ x, base - std::sqrt(a) * h }, Theme::blockAccent[1]);
        }
        t.draw(bodyL);
        t.draw(tailL);
        char buf[32];
        snprintf(buf, sizeof(buf), "%d ms  %+d st", (int)e.tailStartMs(), (int)e.tailPitch.value);
        label(t, "HANDOVER", r.left + 10, r.top + 8, 8, Theme::faint, LEFT, false, 1.5f);
        label(t, buf, r.left + r.width - 10, r.top + 8, 8, Theme::blockAccent[1], RIGHT, true, 1.0f);
    }

    // ------------------------------------------------------------------ hit scope
    // The last hit: output (orange) and the tail part of it (cyan) amplitude over time, the pitch curve on top,
    // and where the layer comes in. Redrawn on every hit.
    void drawScope(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 16.0f, Theme::card, Theme::cardLine);
        auto& e = panel.engine;
        const int N = Scope::N;
        float totalMs = N * scope.msPerBucket;
        float x0 = r.left + 16, w = r.width - 32, mid = r.top + r.height * 0.55f, amp = r.height * 0.38f;
        auto X = [&](float ms) { return x0 + w * ms / totalMs; };

        // grid every 100 ms
        for (int ms = 0; ms <= (int)totalMs; ms += 100) {
            sf::RectangleShape g({ 1.0f, r.height - 34 });
            g.setPosition(X((float)ms), r.top + 22);
            g.setFillColor(sf::Color(255, 255, 255, ms == 0 ? 0 : 10));
            t.draw(g);
            char buf[16];
            snprintf(buf, sizeof(buf), "%d ms", ms);
            if (ms > 0) label(t, buf, X((float)ms) - 4, r.top + r.height - 10, 8, Theme::faint, RIGHT, false, 0.8f);
        }
        sf::RectangleShape axis({ w, 1.0f });
        axis.setPosition(x0, mid);
        axis.setFillColor(sf::Color(255, 255, 255, 18));
        t.draw(axis);

        // body + layer as mirrored filled envelopes
        auto band = [&](const float* v, sf::Color c, float scale) {
            sf::VertexArray va(sf::TriangleStrip, 2 * N);
            for (int i = 0; i < N; ++i) {
                float a = std::min(1.0f, v[i] * scale) * amp;
                float x = x0 + w * i / (N - 1);
                va[2 * i] = sf::Vertex({ x, mid - a }, c);
                va[2 * i + 1] = sf::Vertex({ x, mid + a }, c);
            }
            t.draw(va);
        };
        band(scope.out, alpha(Theme::blockAccent[0], 150), 1.0f);
        band(scope.layer, alpha(Theme::blockAccent[1], 140), 0.8f);

        // decay end + layer start markers
        auto marker = [&](float ms, sf::Color c, const char* name) {
            if (ms > totalMs) return;
            for (float y = r.top + 22; y < r.top + r.height - 16; y += 8) {
                sf::RectangleShape d({ 1.5f, 4.0f });
                d.setPosition(X(ms), y);
                d.setFillColor(c);
                t.draw(d);
            }
            label(t, name, X(ms) + 5, r.top + 26, 8, c, LEFT, true, 1.2f);
        };
        marker(e.decay.value, alpha(Theme::blockAccent[0], 200), "DECAY");
        if (e.tail.value > 0.0f) marker(e.tailStartMs(), alpha(Theme::blockAccent[1], 220), "TAIL");

        // pitch curve (0..6 oct above the note), white
        const int n = 200;
        sf::VertexArray pc(sf::LineStrip, n + 1);
        float decayMs = e.decay.value;
        for (int i = 0; i <= n; ++i) {
            float ms = totalMs * i / n;
            float oct = std::log2(std::max(0.01f, e.pitchRatio(std::sqrt(std::min(1.0f, ms / decayMs)))));
            float y = std::clamp(mid - (oct / 6.0f) * amp * 1.15f, r.top + 20, r.top + r.height - 14);
            pc[i] = sf::Vertex({ X(ms), y }, sf::Color(240, 242, 248, ms <= decayMs ? 200 : 50));
        }
        t.draw(pc);

        label(t, "HIT", r.left + 16, r.top + 13, 10, Theme::text, LEFT, true, 2.4f);
        label(t, "body", r.left + 58, r.top + 13, 9, Theme::blockAccent[0], LEFT, true, 1.0f);
        label(t, "tail", r.left + 98, r.top + 13, 9, Theme::blockAccent[1], LEFT, true, 1.0f);
        label(t, "pitch", r.left + 142, r.top + 13, 9, Theme::text, LEFT, true, 1.0f);
    }

    // ------------------------------------------------------------------ modules
    // A module = a card with its name written up its spine, one pill per line of 4 pots.
    void drawBlock(sf::RenderTarget& t, int b)
    {
        auto r = blockRect(b);
        sf::Color accent = Theme::blockAccent[b];
        rrect(t, r, 18.0f, Theme::card, Theme::cardLine);
        rrect(t, { r.left, r.top, SPINE - 4, r.height }, 14.0f, alpha(accent, 28));

        sf::Text txt(Kick32v2Panel::block(b).name, font, 13);
        txt.setStyle(sf::Text::Bold);
        txt.setLetterSpacing(4.0f);
        txt.setFillColor(accent);
        auto bb = txt.getLocalBounds();
        txt.setOrigin(bb.left + bb.width * 0.5f, bb.top + bb.height * 0.5f);
        txt.setRotation(-90.0f);
        txt.setPosition(r.left + (SPINE - 4) * 0.5f, r.top + r.height * 0.5f);
        t.draw(txt);

        auto& k = Kick32v2Panel::block(b);
        for (int line = 0; line < k.rows; ++line) {
            int first = k.firstPot + line * 4;
            sf::FloatRect pill { colX(k.col) + 6, rowY(k.row + line) + 2, k.cols * CELL_W - 12, ROW_PITCH - 24 };
            rrect(t, pill, 14.0f, sf::Color(255, 255, 255, 4), alpha(accent, 40));
            const char* g = Kick32v2Panel::groupName(first);
            sf::Text cap(g, font, 8);
            cap.setLetterSpacing(2.0f);
            cap.setStyle(sf::Text::Bold);
            float tw = cap.getLocalBounds().width;
            rrect(t, { pill.left + 14.0f, pill.top - 7.0f, tw + 14.0f, 14.0f }, 6.0f, Theme::card);
            label(t, g, pill.left + 21.0f, pill.top, 8, alpha(accent, 235), LEFT, true, 2.0f);
        }
    }

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
        label(t, "TEST SEQUENCER", CARD_X, SEQ_Y, 11, Theme::muted, LEFT, true, 2.4f);
        label(t, "click / drag: steps    wheel: note    shift+wheel: octave    right click: reset note    space: trigger / mute    enter: play", W - 24, SEQ_Y, 10, Theme::faint, RIGHT, false, 0.8f);

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
