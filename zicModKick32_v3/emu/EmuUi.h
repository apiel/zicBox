#pragma once

/**
 * Emulator only: SFML front panel (32 knobs) + minimal 64 step test sequencer.
 * All the "what does a pot do" knowledge lives in core/Kick32v3Panel.h.
 *
 * The panel is drawn as the signal flow: BODY + TIMBRE (left) > DRIVE > FILTER. Every card has a live
 * preview in its header (body wave, formant curve, drive transfer curve, filterbank frequency response),
 * and the HIT scope on top shows the waveform of the last hit with its pitch and amp curves.
 */

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <complex>
#include <cstdio>
#include <mutex>

#include "Sequencer.h"
#include "core/Kick32v3Panel.h"

extern std::mutex audioMutex;
extern std::atomic<int> kickTrigger; // set to 1 to audition a kick
extern std::atomic<bool> kickMuted; // held: sequencer kicks are skipped

namespace Theme {
inline sf::Color rgb(int r, int g, int b, int a = 255) { return sf::Color(r, g, b, a); }
const sf::Color bg = rgb(11, 12, 16);
const sf::Color card = rgb(19, 21, 27);
const sf::Color cardLine = rgb(33, 36, 45);
const sf::Color cell = rgb(28, 30, 38);
const sf::Color track = rgb(46, 49, 60);
const sf::Color cap = rgb(30, 32, 41);
const sf::Color text = rgb(232, 234, 240);
const sf::Color muted = rgb(124, 129, 144);
const sf::Color faint = rgb(70, 74, 88);
const sf::Color blockAccent[4] = { rgb(255, 128, 64), rgb(92, 232, 140), rgb(255, 96, 128), rgb(56, 214, 255) }; // BODY, TAIL, DRIVE, FILTER
const sf::Color kick = rgb(255, 128, 64);
}

class EmuUi {
public:
    static constexpr float W = 1320.0f;
    static constexpr float H = 1150.0f;
    static constexpr float CARD_X = 24.0f, CELL_W = 150.0f, GAP = 36.0f;
    static constexpr float SCOPE_Y = 84.0f, SCOPE_H = 150.0f;
    static constexpr float CARDS_Y = 252.0f, ROW_PITCH = 134.0f, HEAD_S = 48.0f, STACK_GAP = 16.0f;
    static constexpr float CARD_S_H = HEAD_S + 2 * ROW_PITCH + 8.0f; // BODY / LAYERS
    static constexpr float CARD_T_H = 2 * CARD_S_H + STACK_GAP; // DRIVE / FILTER
    static constexpr float BODY_H = HEAD_S + 3 * ROW_PITCH + 8.0f; // BODY (3 rows), VOWEL takes the rest
    static constexpr float HEAD_T = CARD_T_H - 4 * ROW_PITCH - 8.0f;
    static constexpr float SEQ_Y = CARDS_Y + CARD_T_H + 22.0f;

    // Scope of the last hit, filled by the audio thread: min / max of the output per bucket
    struct Scope {
        static constexpr int N = 1200;
        static constexpr float MS_PER_BUCKET = 0.5f; // 600 ms
        float lo[N] = {}, hi[N] = {};
        int note = 60;
    };

    EmuUi(Kick32v3Panel::Panel& panel, Sequencer& seq, const Scope& scope)
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
    static float colX(int col) { return CARD_X + col * CELL_W + (col >= 4 ? GAP : 0.0f) + (col >= 6 ? GAP : 0.0f); }
    static sf::FloatRect blockRect(int b)
    {
        auto& k = Kick32v3Panel::block(b);
        float y = CARDS_Y + (b == 1 ? BODY_H + STACK_GAP : 0.0f);
        float h = b == 0 ? BODY_H : (b == 1 ? CARD_T_H - BODY_H - STACK_GAP : CARD_T_H);
        return { colX(k.col), y, k.cols * CELL_W, h };
    }
    static float headH(int b) { return Kick32v3Panel::block(b).rows < 4 ? HEAD_S : HEAD_T; }
    static float rowY(int pot)
    {
        int b = Kick32v3Panel::blockOf(pot);
        auto& k = Kick32v3Panel::block(b);
        return blockRect(b).top + headH(b) + (Kick32v3Panel::potRow(pot) - k.row) * ROW_PITCH;
    }
    static sf::Vector2f knobCenter(int pot) { return { colX(Kick32v3Panel::potCol(pot)) + CELL_W * 0.5f, rowY(pot) + 54.0f }; }
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
    static sf::FloatRect playBtn() { return { 250.0f, 18.0f, 64.0f, 44.0f }; }
    static sf::FloatRect trigBtn() { return { 324.0f, 18.0f, 64.0f, 44.0f }; }
    static sf::FloatRect bpmBox() { return { 400.0f, 18.0f, 140.0f, 44.0f }; }
    static sf::FloatRect tuneBox() { return { 550.0f, 18.0f, 140.0f, 44.0f }; }

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
        for (int i = 0; i < Kick32v3Panel::POTS; ++i) {
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
        for (int i = 0; i < Kick32v3Panel::POTS; ++i)
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
        for (int i = 0; i < Kick32v3Panel::POTS; ++i) {
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
        for (int b = 0; b < Kick32v3Panel::BLOCKS; ++b) drawBlock(t, b);
        drawFlow(t);
        for (int i = 0; i < Kick32v3Panel::POTS; ++i) drawKnob(t, i, Theme::blockAccent[Kick32v3Panel::blockOf(i)]);
        drawSequencer(t, mouse);
    }

private:
    Kick32v3Panel::Panel& panel;
    Sequencer& seq;
    const Scope& scope;

    sf::Font font;
    int dragging = -1, hoverPot = -1, lastClicked = -1;
    float dragY = 0.0f, dragStart = 0.0f;
    bool painting = false, paintValue = false, trigHeldByMouse = false;
    sf::Clock clock;

    // ---- primitives
    enum Align { LEFT, CENTER, RIGHT };

    static sf::Color alpha(sf::Color c, int a) { return sf::Color(c.r, c.g, c.b, (sf::Uint8)std::clamp(a, 0, 255)); }

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

    // a polyline, 2 px thick (two offset strips)
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
        label(t, value, b.left + 54, b.top + 22, 22, Theme::text, CENTER, true, 0.5f);
        label(t, caption, b.left + 84, b.top + 25, 9, Theme::muted, LEFT, false, 1.5f);
        label(t, "-", b.left + 16, b.top + 21, 20, Theme::muted);
        label(t, "+", b.left + b.width - 16, b.top + 21, 20, Theme::muted);
    }

    void drawTopBar(sf::RenderTarget& t, sf::Vector2f mouse)
    {
        label(t, "zicMod", 24, 34, 24, Theme::text, LEFT, true, 0.5f);
        label(t, "Kick32v3", 24 + 86, 34, 24, Theme::kick, LEFT, true, 0.5f);
        label(t, "BODY  >  TIMBRE  >  DRIVE  >  FILTER", 24, 58, 10, Theme::muted, LEFT, false, 2.2f);

        button(t, playBtn(), seq.playing ? STOP : PLAY, mouse, seq.playing, Theme::blockAccent[3]);
        button(t, trigBtn(), seq.playing ? MUTE : TRIG, mouse, kickMuted.load() || kickTrigger.load() != 0, Theme::kick);

        char buf[16];
        snprintf(buf, sizeof(buf), "%d", (int)std::round(seq.bpm));
        stepper(t, bpmBox(), buf, "BPM");
        snprintf(buf, sizeof(buf), "%+d", seq.transpose);
        stepper(t, tuneBox(), buf, "TUNE");

        // output meter, right side
        sf::FloatRect m { W - 24 - 260, 18, 260, 44 };
        rrect(t, m, 12.0f, Theme::card, Theme::cardLine);
        label(t, "OUT", m.left + 14, m.top + 22, 9, Theme::muted, LEFT, false, 2.0f);
        float lvl = std::clamp(panel.engine.meterOut, 0.0f, 1.0f);
        sf::FloatRect bar { m.left + 50, m.top + 17, m.width - 64, 10 };
        rrect(t, bar, 5.0f, Theme::cell);
        if (lvl > 0.01f) rrect(t, { bar.left, bar.top, bar.width * lvl, bar.height }, 5.0f, lvl > 0.9f ? Theme::blockAccent[2] : Theme::kick);
    }

    // ------------------------------------------------------------------ hit scope
    void drawScope(sf::RenderTarget& t, sf::FloatRect r)
    {
        rrect(t, r, 16.0f, Theme::card, Theme::cardLine);
        auto& e = panel.engine;
        const int N = Scope::N;
        float totalMs = N * Scope::MS_PER_BUCKET;
        float x0 = r.left + 16, w = r.width - 32, mid = r.top + r.height * 0.55f, amp = r.height * 0.40f;
        auto X = [&](float ms) { return x0 + w * ms / totalMs; };

        for (int ms = 100; ms < (int)totalMs; ms += 100) {
            vline(t, X((float)ms), r.top + 22, r.height - 36, sf::Color(255, 255, 255, 10));
            char buf[16];
            snprintf(buf, sizeof(buf), "%d ms", ms);
            label(t, buf, X((float)ms) - 4, r.top + r.height - 10, 8, Theme::faint, RIGHT, false, 0.8f);
        }
        hline(t, x0, mid, w, sf::Color(255, 255, 255, 18));

        // the waveform: min / max per half millisecond
        {
            sf::VertexArray va(sf::TriangleStrip, 2 * N);
            sf::Color c = alpha(Theme::kick, 190);
            for (int i = 0; i < N; ++i) {
                float x = x0 + w * i / (N - 1);
                float hi = std::clamp(scope.hi[i], -1.0f, 1.0f), lo = std::clamp(scope.lo[i], -1.0f, 1.0f);
                if (hi - lo < 0.004f) { hi += 0.002f; lo -= 0.002f; }
                va[2 * i] = sf::Vertex({ x, mid - hi * amp }, c);
                va[2 * i + 1] = sf::Vertex({ x, mid - lo * amp }, c);
            }
            t.draw(va);
        }

        // markers: end of the drop, end of the decay, the rebounds
        auto marker = [&](float ms, sf::Color c, const char* name) {
            if (ms > totalMs) return;
            for (float y = r.top + 22; y < r.top + r.height - 16; y += 8) vline(t, X(ms), y, 4.0f, c, 1.5f);
            label(t, name, X(ms) + 5, r.top + 26, 8, c, LEFT, true, 1.2f);
        };
        marker(e.dropMs(), alpha(Theme::blockAccent[1], 220), "DROP");
        marker(e.decay.value, alpha(Theme::blockAccent[0], 200), "DECAY");

        // pitch (octaves above the note, white) and amp shape (faint) curves
        const int n = 240;
        std::vector<sf::Vector2f> pc, ac;
        for (int i = 0; i <= n; ++i) {
            float ms = totalMs * i / n;
            float oct = e.pitchOctAt(ms);
            pc.push_back({ X(ms), std::clamp(mid - (oct / 6.0f) * amp * 1.1f, r.top + 20, r.top + r.height - 14) });
            ac.push_back({ X(ms), mid - e.ampAt(ms) * amp });
        }
        curve(t, ac, sf::Color(255, 255, 255, 40), 1.5f);
        curve(t, pc, sf::Color(240, 242, 248, 210), 1.5f);

        label(t, "HIT", r.left + 16, r.top + 13, 10, Theme::text, LEFT, true, 2.4f);
        label(t, "out", r.left + 56, r.top + 13, 9, Theme::kick, LEFT, true, 1.0f);
        label(t, "pitch", r.left + 126, r.top + 13, 9, Theme::text, LEFT, true, 1.0f);
        label(t, "amp", r.left + 168, r.top + 13, 9, Theme::muted, LEFT, true, 1.0f);
        char buf[48];
        float hz = Kick32v3::ROOT_HZ * std::pow(2.0f, (scope.note - 60) / 12.0f);
        snprintf(buf, sizeof(buf), "note %d  ·  %.1f Hz  ·  600 ms", scope.note, hz);
        label(t, buf, r.left + r.width - 16, r.top + 13, 9, Theme::muted, RIGHT, false, 1.0f);
    }

    // ------------------------------------------------------------------ cards
    void drawBlock(sf::RenderTarget& t, int b)
    {
        auto r = blockRect(b);
        auto& k = Kick32v3Panel::block(b);
        sf::Color accent = Theme::blockAccent[b];
        rrect(t, r, 18.0f, Theme::card, Theme::cardLine);
        // accent line on top
        rrect(t, { r.left + 18, r.top, r.width - 36, 3 }, 1.5f, alpha(accent, 200));

        label(t, k.name, r.left + 18, r.top + 20, 13, accent, LEFT, true, 4.0f);
        label(t, k.subtitle, r.left + 18, r.top + 36, 9, Theme::muted, LEFT, false, 1.0f);

        sf::FloatRect pv = k.rows < 4 ? sf::FloatRect { r.left + r.width - 300, r.top + 8, 284, HEAD_S - 14 }
                                       : sf::FloatRect { r.left + 14, r.top + 48, r.width - 28, HEAD_T - 58 };
        switch (b) {
        case 0: drawWavePreview(t, pv, accent); break;
        case 1: drawVowelPreview(t, pv, accent); break;
        case 2: drawDrivePreview(t, pv, accent); break;
        case 3: drawFilterPreview(t, pv, accent); break;
        }

        // group pills with captions
        for (int g = 0; g < Kick32v3Panel::GROUPS; ++g) {
            auto& grp = Kick32v3Panel::group(g);
            if (Kick32v3Panel::blockOf(grp.firstPot) != b) continue;
            float x = colX(Kick32v3Panel::potCol(grp.firstPot)), y = rowY(grp.firstPot);
            sf::FloatRect pill { x + 6, y + 6, grp.count * CELL_W - 12, ROW_PITCH - 24 };
            rrect(t, pill, 14.0f, sf::Color(255, 255, 255, 4), alpha(accent, 40));
            sf::Text cap(grp.name, font, 8);
            cap.setLetterSpacing(2.0f);
            cap.setStyle(sf::Text::Bold);
            float tw = cap.getLocalBounds().width;
            rrect(t, { pill.left + 14.0f, pill.top - 7.0f, tw + 14.0f, 14.0f }, 6.0f, Theme::card);
            label(t, grp.name, pill.left + 21.0f, pill.top, 8, alpha(accent, 235), LEFT, true, 2.0f);
        }
    }

    // chevrons in the gaps: sources > drive > filter > out
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
        std::vector<sf::Vector2f> pts;
        for (int i = 0; i <= n; ++i) {
            float v = std::clamp(panel.engine.waveSample((float)i / n), -1.3f, 1.3f);
            pts.push_back({ r.left + 12 + (r.width - 24) * i / n, r.top + r.height * 0.5f - v * (r.height * 0.5f - 6) / 1.3f });
        }
        hline(t, r.left + 12, r.top + r.height * 0.5f, r.width - 24, sf::Color(255, 255, 255, 14));
        curve(t, pts, accent, 1.8f);
        label(t, "ONE CYCLE", r.left + 10, r.top + 9, 8, Theme::faint, LEFT, false, 1.5f);
    }

    // the two formants over the spectrum (20 Hz .. 20 kHz): the vowel as a filter curve
    void drawVowelPreview(sf::RenderTarget& t, sf::FloatRect r, sf::Color accent)
    {
        previewBox(t, r);
        auto& e = panel.engine;
        const float sr = e.sr(), k = e.vowelK(), m = e.vowelMixValue();
        auto fx = [&](float hz) { return r.left + 12 + (r.width - 24) * std::log(hz / 20.0f) / std::log(1000.0f); };
        auto fy = [&](float db) { return r.top + r.height - 8 - std::clamp((db + 30.0f) / 42.0f, 0.0f, 1.0f) * (r.height - 16); };
        float g1 = std::tan((float)M_PI * std::clamp(e.vowelHz1(), 15.0f, sr * 0.42f) / sr);
        float g2 = std::tan((float)M_PI * std::clamp(e.vowelHz2(), 15.0f, sr * 0.42f) / sr);
        std::vector<sf::Vector2f> pts;
        for (int i = 0; i <= 120; ++i) {
            float f = 20.0f * std::pow(1000.0f, i / 120.0f);
            float wf = std::tan((float)M_PI * f / sr);
            std::complex<float> s1(0.0f, wf / g1), s2(0.0f, wf / g2);
            std::complex<float> b1 = s1 / (s1 * s1 + k * s1 + 1.0f), b2 = s2 / (s2 * s2 + k * s2 + 1.0f);
            std::complex<float> H = (1.0f - 0.5f * m) + (b1 + b2) * k * 2.2f * m; // dry + formants, as in the engine
            float db = 20.0f * std::log10(std::max(1e-5f, std::abs(H)));
            pts.push_back({ fx(f), fy(db) });
        }
        for (float hz : { 100.0f, 1000.0f, 10000.0f }) vline(t, fx(hz), r.top + 6, r.height - 12, sf::Color(255, 255, 255, 10));
        curve(t, pts, alpha(accent, m > 0.0f ? 230 : 70), 1.8f);
        char buf[48];
        snprintf(buf, sizeof(buf), "F1 %.0f  F2 %.0f Hz", e.vowelHz1(), e.vowelHz2());
        label(t, "FORMANTS", r.left + 10, r.top + 9, 8, Theme::faint, LEFT, false, 1.5f);
        if (m > 0.0f) label(t, buf, r.left + r.width - 10, r.top + 9, 8, alpha(accent, 220), RIGHT, true, 1.0f);
    }

    // the drive transfer curve for an input -1..1 -> output
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
        char buf[48];
        float gain = std::pow(2.0f, e.drive.value * 0.07f);
        const char* ch = e.character.value < 33 ? "soft > hard" : (e.character.value < 66 ? "hard > fold" : "fold > fuzz");
        snprintf(buf, sizeof(buf), "x%.0f  ·  %s", gain, ch);
        label(t, "TRANSFER", r.left + 10, r.top + 9, 8, Theme::faint, LEFT, false, 1.5f);
        label(t, buf, r.left + r.width - 10, r.top + 9, 8, alpha(accent, 220), RIGHT, true, 1.0f);
    }

    // frequency response of the filterbank (static: no env, note 60, no FM). LP > HP in series.
    void drawFilterPreview(sf::RenderTarget& t, sf::FloatRect r, sf::Color accent)
    {
        previewBox(t, r);
        auto& e = panel.engine;
        const float sr = e.sr();
        float c1 = e.filterCutoff1Hz(), c2 = e.filterCutoff2Hz(), k = e.filterK();
        float g1 = std::tan((float)M_PI * std::clamp(c1, 15.0f, sr * 0.42f) / sr), g2 = std::tan((float)M_PI * std::clamp(c2, 15.0f, sr * 0.42f) / sr);
        float gain = e.filterGain();
        auto fx = [&](float hz) { return r.left + 12 + (r.width - 24) * std::log(hz / 20.0f) / std::log(1000.0f); };
        auto fy = [&](float db) { return r.top + r.height - 8 - std::clamp((db + 36.0f) / 60.0f, 0.0f, 1.0f) * (r.height - 16); };
        for (float hz : { 100.0f, 1000.0f, 10000.0f }) vline(t, fx(hz), r.top + 6, r.height - 12, sf::Color(255, 255, 255, 10));
        hline(t, r.left + 12, fy(0.0f), r.width - 24, sf::Color(255, 255, 255, 16));
        std::vector<sf::Vector2f> pts;
        for (int i = 0; i <= 120; ++i) {
            float f = 20.0f * std::pow(1000.0f, i / 120.0f);
            float wf = std::tan((float)M_PI * f / sr);
            std::complex<float> s1(0.0f, wf / g1), s2(0.0f, wf / g2);
            std::complex<float> d1 = s1 * s1 + k * s1 + 1.0f, d2 = s2 * s2 + k * s2 + 1.0f;
            std::complex<float> H = (1.0f / d1) * (s2 * s2 / d2) * gain;
            float db = 20.0f * std::log10(std::max(1e-5f, std::abs(H)));
            pts.push_back({ fx(f), fy(db) });
        }
        curve(t, pts, accent, 1.8f);
        char buf[64];
        auto hz = [](char* b, size_t n, float v) { if (v >= 1000) snprintf(b, n, "%.1fk", v / 1000); else snprintf(b, n, "%.0f", v); };
        char a[16], bb[16];
        hz(a, 16, c1);
        hz(bb, 16, c2);
        snprintf(buf, sizeof(buf), "LP %s  >  HP %s  ·  drive x%.0f", a, bb, e.filterDriveGain());
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
        float n = panel.norm(pot);
        bool centered = panel.isCentered(pot);
        const float R = 27.0f;
        auto angle = [](float v) { return -135.0f + 270.0f * v; };

        disc(t, ctr.x, ctr.y, 36.0f, sf::Color(21, 23, 29));
        arc(t, ctr.x, ctr.y, R, 5.0f, -135.0f, 135.0f, Theme::track);
        float from = centered ? 0.0f : -135.0f;
        float to = angle(n);
        if (std::abs(to - from) > 0.5f) {
            arc(t, ctr.x, ctr.y, R, active ? 13.0f : 10.0f, from, to, sf::Color(accent.r, accent.g, accent.b, active ? 70 : 34), false);
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
                float k = std::clamp(s.note / 12.0f, -1.0f, 1.0f);
                fill = k > 0 ? sf::Color(255, (sf::Uint8)(128 + 70 * k), (sf::Uint8)(64 + 60 * k)) : sf::Color((sf::Uint8)(255 + 120 * k), (sf::Uint8)(128 + 60 * -k), (sf::Uint8)(64 + 190 * -k));
            }
            if (s.on && isCur) rrect(t, { r.left - 3, r.top - 3, r.width + 6, r.height + 6 }, 12.0f, sf::Color(255, 160, 100, 60));
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
