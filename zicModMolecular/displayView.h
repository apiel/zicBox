#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

#ifndef SCREEN_BUFFER_ROWS
#define SCREEN_BUFFER_ROWS 800
#endif
#ifndef SCREEN_BUFFER_COLS
#define SCREEN_BUFFER_COLS 480
#endif

#include "draw/draw.h"
#include "draw/fonts/fonts.h"
#include "types.h"
#include "zicApp.h"

inline DrawOptions drawOpt(Color color, int thickness = 1)
{
    DrawOptions opt;
    opt.color = color;
    opt.thickness = thickness;
    return opt;
}

inline DrawTextOptions textOpt(Color color, void* font = nullptr)
{
    DrawTextOptions opt;
    opt.color = color;
    opt.font = font ? font : &PoppinsLight_12;
    return opt;
}

class DisplayView {
public:
    static constexpr int SCREEN_W = ZicApp::SCREEN_W; // 480
    static constexpr int SCREEN_H = ZicApp::SCREEN_H; // 800

    void render(Draw& d, ZicApp& app)
    {
        const ThemeColors& colors = THEMES[app.themeIdx].colors;

        // 1. Background Fill
        d.filledRect({ 0, 0 }, { SCREEN_W, SCREEN_H }, drawOpt(colors.bgGlow));

        // 2. Render Canvas Grid (50..580)
        Color gridCol = makeColor(colors.bgGlow.r + 15, colors.bgGlow.g + 15, colors.bgGlow.b + 20, 90);
        for (int x = 0; x < SCREEN_W; x += 40) {
            d.line({ x, ZicApp::CANVAS_TOP }, { x, ZicApp::CANVAS_BOTTOM }, drawOpt(gridCol, 1));
        }
        for (int y = ZicApp::CANVAS_TOP; y < ZicApp::CANVAS_BOTTOM; y += 40) {
            d.line({ 0, y }, { SCREEN_W, y }, drawOpt(gridCol, 1));
        }

        // 3. Render Catalyst Particles (Flying Stars)
        app.updatePhysics();
        for (const auto& p : app.catalysts) {
            d.filledCircle({ static_cast<int>(p.x), static_cast<int>(p.y) }, static_cast<int>(p.size), drawOpt(p.color));
        }

        // 4. Render Connections / Flux Links
        for (const auto& conn : app.connections) {
            const SynthNode* from = nullptr;
            const SynthNode* to = nullptr;
            for (const auto& n : app.nodes) {
                if (n.id == conn.fromId) from = &n;
                if (n.id == conn.toId) to = &n;
            }

            if (from && to) {
                bool isConnSelected = (conn.id == app.selectedConnectionId);
                Color connCol = isConnSelected ? makeColor(255, 255, 100) : colors.connStart;
                int thickness = isConnSelected ? 4 : 2;

                d.line({ static_cast<int>(from->pos.x), static_cast<int>(from->pos.y) },
                       { static_cast<int>(to->pos.x), static_cast<int>(to->pos.y) },
                       drawOpt(connCol, thickness));

                // Moving pulse along link wire
                static float linkPulse = 0.0f;
                linkPulse += 0.025f;
                if (linkPulse > 1.0f) linkPulse -= 1.0f;

                float px = from->pos.x + (to->pos.x - from->pos.x) * linkPulse;
                float py = from->pos.y + (to->pos.y - from->pos.y) * linkPulse;
                d.filledCircle({ static_cast<int>(px), static_cast<int>(py) }, 6, drawOpt(makeColor(255, 255, 255, 230)));
            }
        }

        // 5. Render Node Molecules (FILLED LIQUID BUBBLES)
        for (const auto& n : app.nodes) {
            bool isSelected = (n.id == app.selectedNodeId);
            int cx = static_cast<int>(n.pos.x);
            int cy = static_cast<int>(n.pos.y);
            int radius = static_cast<int>(n.size * 0.5f);

            // Selection Glow Halo
            if (isSelected) {
                d.circle({ cx, cy }, radius + 9, drawOpt(makeColor(255, 255, 255, 220), 3));
                d.circle({ cx, cy }, radius + 4, drawOpt(colors.accent, 2));
            }

            // Disturbance / Catalyst Impact Expanding Ripple
            if (n.disturbance > 0.0f) {
                int distR = radius + static_cast<int>(n.disturbance * 18.0f);
                d.circle({ cx, cy }, distR, drawOpt(makeColor(255, 255, 255, static_cast<uint8_t>(n.disturbance * 200.0f)), 2));
            }

            // Outer Liquid Ring
            Color ringCol = n.color;
            if (n.pulsePhase > 0.0f) {
                ringCol = makeColor(255, 255, 255);
            }
            d.circle({ cx, cy }, radius, drawOpt(ringCol, 3));

            // FILLED CIRCLE CORE GRADIENT & BUBBLE SURFACE
            Color fillBg = n.isAudible ? makeColor(ringCol.r / 3, ringCol.g / 3, ringCol.b / 3, 220)
                                       : makeColor(25, 25, 32, 230);
            d.filledCircle({ cx, cy }, radius - 2, drawOpt(fillBg));

            // Inner Accent Ring
            d.circle({ cx, cy }, radius - 10, drawOpt(makeColor(ringCol.r, ringCol.g, ringCol.b, 120), 1));

            // Top Glass Highlight Curved Arc
            d.filledCircle({ cx - radius / 3, cy - radius / 3 }, radius / 4, drawOpt(makeColor(255, 255, 255, 70)));

            // Node Subtype Waveform Icon / Label
            const char* typeLabel = "";
            if (n.type == NodeType::OSC) {
                static const char* oscNames[] = { "SINE ~", "SQR |~|", "SAW /|", "TRI ^", "NOISE *" };
                typeLabel = oscNames[std::clamp(n.subType, 0, 4)];
            } else {
                static const char* fxNames[] = { "LOWPASS", "HIGHPASS", "DELAY", "DISTORT", "CRUSHER" };
                typeLabel = fxNames[std::clamp(n.subType, 0, 4)];
            }

            d.text({ cx - 22, cy - 10 }, typeLabel, 10, textOpt(makeColor(255, 255, 255), &PoppinsLight_12));

            // Frequency / Cutoff HUD below bubble
            char hudBuf[32];
            if (n.type == NodeType::OSC) {
                std::snprintf(hudBuf, sizeof(hudBuf), "%.0f Hz", n.frequency);
            } else {
                std::snprintf(hudBuf, sizeof(hudBuf), "Cut: %.2f", n.paramA);
            }
            d.text({ cx - 22, cy + radius + 4 }, hudBuf, 10, textOpt(makeColor(190, 200, 220), &PoppinsLight_8));
        }

        // Connecting Link Line Drag Preview
        if (app.isConnecting) {
            const SynthNode* src = nullptr;
            for (const auto& n : app.nodes) {
                if (n.id == app.connectSourceId) src = &n;
            }
            if (src) {
                d.line({ static_cast<int>(src->pos.x), static_cast<int>(src->pos.y) }, { SCREEN_W / 2, SCREEN_H / 2 }, drawOpt(makeColor(255, 255, 0), 3));
            }
        }

        // 6. Top Bar UI (0..50)
        Color topBg = makeColor(18, 22, 32);
        d.filledRect({ 0, 0 }, { SCREEN_W, ZicApp::TOP_BAR_H }, drawOpt(topBg));
        d.line({ 0, ZicApp::TOP_BAR_H }, { SCREEN_W, ZicApp::TOP_BAR_H }, drawOpt(colors.accent, 2));

        // Top Buttons (Finger friendly)
        drawButton(d, { 10, 8 }, { 70, 34 }, "+ OSC", colors.oscStart, &PoppinsLight_12);
        drawButton(d, { 90, 8 }, { 65, 34 }, "+ FX", colors.fxStart, &PoppinsLight_12);
        drawButton(d, { 165, 8 }, { 80, 34 }, app.isPlaying ? "PAUSE" : "PLAY", app.isPlaying ? makeColor(0, 200, 100) : makeColor(200, 80, 80), &PoppinsLight_12);
        drawButton(d, { 255, 8 }, { 70, 34 }, "RESET", makeColor(80, 90, 110), &PoppinsLight_12);
        drawButton(d, { 335, 8 }, { 135, 34 }, THEMES[app.themeIdx].name.c_str(), colors.accent, &PoppinsLight_12);

        // 7. Bottom Configuration Panel UI (580..800)
        renderBottomPanel(d, app);
    }

private:
    void drawButton(Draw& d, Position pos, Size sz, const char* label, Color bgCol, void* fontPtr = &PoppinsLight_12)
    {
        int x = static_cast<int>(pos.x);
        int y = static_cast<int>(pos.y);
        int w = sz.w;
        int h = sz.h;

        d.filledRect({ x, y }, { w, h }, drawOpt(makeColor(bgCol.r / 2, bgCol.g / 2, bgCol.b / 2, 230)));
        d.line({ x, y }, { x + w, y }, drawOpt(bgCol, 1));
        d.line({ x, y + h }, { x + w, y + h }, drawOpt(bgCol, 1));
        d.line({ x, y }, { x, y + h }, drawOpt(bgCol, 1));
        d.line({ x + w, y }, { x + w, y + h }, drawOpt(bgCol, 1));

        d.text({ x + 8, y + 8 }, label, 12, textOpt(makeColor(255, 255, 255), fontPtr));
    }

    void renderBottomPanel(Draw& d, ZicApp& app)
    {
        const ThemeColors& colors = THEMES[app.themeIdx].colors;
        int panelY = ZicApp::CANVAS_BOTTOM;

        // Panel Background Fill
        d.filledRect({ 0, panelY }, { SCREEN_W, ZicApp::BOTTOM_PANEL_H }, drawOpt(colors.sidebarBg));
        d.line({ 0, panelY }, { SCREEN_W, panelY }, drawOpt(colors.accent, 2));

        SynthNode* node = app.getSelectedNode();
        Connection* conn = app.getSelectedConnection();

        if (conn) {
            // FLUX LINK CONFIGURATION PANEL
            d.text({ 16, panelY + 14 }, "FLUX LINK CONFIGURATION", 16, textOpt(makeColor(255, 255, 255), &PoppinsLight_16));

            char linkDesc[64];
            std::snprintf(linkDesc, sizeof(linkDesc), "%s  --->  %s", conn->fromId.c_str(), conn->toId.c_str());
            d.text({ 16, panelY + 36 }, linkDesc, 12, textOpt(makeColor(0, 210, 255), &PoppinsLight_12));

            // Target Modulation Selector (Y: panelY + 60)
            d.text({ 16, panelY + 66 }, "Target:", 12, textOpt(makeColor(180, 190, 210), &PoppinsLight_12));
            int btnW = 90;
            int startX = 75;
            int btnY = panelY + 58;

            static const char* targetNames[] = { "FREQ (FM)", "GAIN (AM)", "CUTOFF", "RESONANCE" };
            static const ModTarget targets[] = { ModTarget::FREQUENCY, ModTarget::GAIN, ModTarget::CUTOFF, ModTarget::RESONANCE };

            for (int i = 0; i < 4; ++i) {
                int bx = startX + i * (btnW + 6);
                Color bCol = (conn->target == targets[i]) ? colors.accent : makeColor(40, 45, 60);
                drawButton(d, { static_cast<float>(bx), static_cast<float>(btnY) }, { btnW, 34 }, targetNames[i], bCol, &PoppinsLight_8);
            }

            // Modulation Depth Slider (Y: panelY + 115)
            int sY = panelY + 115;
            char depthLabel[32];
            std::snprintf(depthLabel, sizeof(depthLabel), "Depth: %.0f%%", conn->depth * 100.0f);
            d.text({ 16, sY + 6 }, depthLabel, 12, textOpt(makeColor(180, 190, 210), &PoppinsLight_12));

            d.filledRect({ 110, sY }, { 340, 24 }, drawOpt(makeColor(30, 36, 48)));
            int fillW = static_cast<int>(conn->depth * 340.0f);
            d.filledRect({ 110, sY }, { fillW, 24 }, drawOpt(colors.accent));

            // Action Button: Sever Link (Y: panelY + 168)
            drawButton(d, { 16, static_cast<float>(panelY + 168) }, { 448, 38 }, "SEVER FLUX LINK", makeColor(200, 50, 50), &PoppinsLight_16);
            return;
        }

        if (node) {
            // MOLECULE CONFIGURATION PANEL
            char titleBuf[64];
            const char* typeName = (node->type == NodeType::OSC) ? "OSCILLATOR" : "EFFECT";
            std::snprintf(titleBuf, sizeof(titleBuf), "%s MOLECULE [%s]", typeName, node->id.c_str());
            d.text({ 16, panelY + 14 }, titleBuf, 16, textOpt(makeColor(255, 255, 255), &PoppinsLight_16));

            // Subtype Pill Buttons (Y: panelY + 50)
            int btnW = 76;
            int startX = 16;
            int btnY = panelY + 48;

            if (node->type == NodeType::OSC) {
                static const char* oscNames[] = { "Sine", "Square", "Saw", "Tri", "Noise" };
                for (int i = 0; i < 5; ++i) {
                    int bx = startX + i * (btnW + 8);
                    Color bCol = (node->subType == i) ? colors.oscStart : makeColor(40, 45, 60);
                    drawButton(d, { static_cast<float>(bx), static_cast<float>(btnY) }, { btnW, 36 }, oscNames[i], bCol, &PoppinsLight_12);
                }
            } else {
                static const char* fxNames[] = { "Lowpass", "Highpass", "Delay", "Distort", "Crush" };
                for (int i = 0; i < 5; ++i) {
                    int bx = startX + i * (btnW + 8);
                    Color bCol = (node->subType == i) ? colors.fxStart : makeColor(40, 45, 60);
                    drawButton(d, { static_cast<float>(bx), static_cast<float>(btnY) }, { btnW, 36 }, fxNames[i], bCol, &PoppinsLight_12);
                }
            }

            // Slider 1: Pitch / Cutoff (Y: panelY + 100)
            int s1Y = panelY + 100;
            const char* s1Label = (node->type == NodeType::OSC) ? "Freq (Hz)" : "Param A";
            d.text({ 16, s1Y + 4 }, s1Label, 12, textOpt(makeColor(180, 190, 210), &PoppinsLight_12));

            d.filledRect({ 100, s1Y }, { 350, 22 }, drawOpt(makeColor(30, 36, 48)));
            float val1Norm = 0.5f;
            if (node->type == NodeType::OSC) {
                val1Norm = std::log2(node->frequency / 55.0f) / 5.0f;
                val1Norm = std::clamp(val1Norm, 0.0f, 1.0f);
            } else {
                val1Norm = std::clamp(node->paramA, 0.0f, 1.0f);
            }
            int fillW1 = static_cast<int>(val1Norm * 350.0f);
            d.filledRect({ 100, s1Y }, { fillW1, 22 }, drawOpt(colors.accent));

            // Slider 2: Gain / Param B (Y: panelY + 135)
            int s2Y = panelY + 135;
            const char* s2Label = (node->type == NodeType::OSC) ? "Gain" : "Param B";
            d.text({ 16, s2Y + 4 }, s2Label, 12, textOpt(makeColor(180, 190, 210), &PoppinsLight_12));

            d.filledRect({ 100, s2Y }, { 350, 22 }, drawOpt(makeColor(30, 36, 48)));
            float val2Norm = std::clamp(node->paramB, 0.0f, 1.0f);
            int fillW2 = static_cast<int>(val2Norm * 350.0f);
            d.filledRect({ 100, s2Y }, { fillW2, 22 }, drawOpt(colors.accent));

            // Action Buttons (Y: panelY + 172)
            int actY = panelY + 172;
            drawButton(d, { 16, static_cast<float>(actY) }, { 124, 36 }, node->isAudible ? "FEED: ON" : "FEED: LFO", node->isAudible ? makeColor(0, 180, 100) : makeColor(140, 140, 160), &PoppinsLight_12);
            drawButton(d, { 150, static_cast<float>(actY) }, { 120, 36 }, app.isConnecting ? "CANCEL LINK" : "LINK WIRE", app.isConnecting ? makeColor(220, 180, 0) : makeColor(70, 80, 110), &PoppinsLight_12);
            drawButton(d, { 280, static_cast<float>(actY) }, { 184, 36 }, "DELETE MOLECULE", makeColor(200, 50, 50), &PoppinsLight_12);
        } else {
            // Empty Selection Overview State
            d.text({ 16, panelY + 18 }, "MOLECULAR SYNTH UNIVERSE", 16, textOpt(makeColor(255, 255, 255), &PoppinsLight_16));

            char statsBuf[128];
            std::snprintf(statsBuf, sizeof(statsBuf), "Molecules: %d  |  Flux Links: %d  |  Catalysts: %d", static_cast<int>(app.nodes.size()), static_cast<int>(app.connections.size()), static_cast<int>(app.catalysts.size()));
            d.text({ 16, panelY + 50 }, statsBuf, 12, textOpt(makeColor(0, 210, 255), &PoppinsLight_12));

            d.text({ 16, panelY + 90 }, "Touch Interface Guide:", 12, textOpt(makeColor(200, 205, 220), &PoppinsLight_12));
            d.text({ 24, panelY + 115 }, "- Tap '+ OSC' or '+ FX' to spawn synth molecules", 12, textOpt(makeColor(150, 160, 180), &PoppinsLight_12));
            d.text({ 24, panelY + 140 }, "- Touch and drag molecules with finger to position them", 12, textOpt(makeColor(150, 160, 180), &PoppinsLight_12));
            d.text({ 24, panelY + 165 }, "- Flying stars trigger acoustic impact sounds on collision", 12, textOpt(makeColor(150, 160, 180), &PoppinsLight_12));
        }
    }
};
