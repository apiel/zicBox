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
    opt.font = font;
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

        // 2. Render Canvas Grid (44..600)
        Color gridCol = makeColor(colors.bgGlow.r + 15, colors.bgGlow.g + 15, colors.bgGlow.b + 20, 100);
        for (int x = 0; x < SCREEN_W; x += 40) {
            d.line({ x, ZicApp::CANVAS_TOP }, { x, ZicApp::CANVAS_BOTTOM }, drawOpt(gridCol, 1));
        }
        for (int y = ZicApp::CANVAS_TOP; y < ZicApp::CANVAS_BOTTOM; y += 40) {
            d.line({ 0, y }, { SCREEN_W, y }, drawOpt(gridCol, 1));
        }

        // 3. Render Catalyst Particles
        app.updatePhysics();
        for (const auto& p : app.catalysts) {
            d.filledRect({ static_cast<int>(p.x), static_cast<int>(p.y) }, { static_cast<int>(p.size), static_cast<int>(p.size) }, drawOpt(p.color));
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
                Color connCol = colors.connStart;
                d.line({ static_cast<int>(from->pos.x), static_cast<int>(from->pos.y) },
                       { static_cast<int>(to->pos.x), static_cast<int>(to->pos.y) },
                       drawOpt(connCol, 3));

                // Moving pulse along link wire
                static float linkPulse = 0.0f;
                linkPulse += 0.02f;
                if (linkPulse > 1.0f) linkPulse -= 1.0f;

                float px = from->pos.x + (to->pos.x - from->pos.x) * linkPulse;
                float py = from->pos.y + (to->pos.y - from->pos.y) * linkPulse;
                d.circle({ static_cast<int>(px), static_cast<int>(py) }, 5, drawOpt(makeColor(255, 255, 255), 2));
            }
        }

        // 5. Render Node Molecules / Bubbles
        for (const auto& n : app.nodes) {
            bool isSelected = (n.id == app.selectedNodeId);
            int cx = static_cast<int>(n.pos.x);
            int cy = static_cast<int>(n.pos.y);
            int radius = static_cast<int>(n.size * 0.5f);

            // Selection Glow Halo
            if (isSelected) {
                d.circle({ cx, cy }, radius + 8, drawOpt(makeColor(255, 255, 255, 200), 2));
                d.circle({ cx, cy }, radius + 4, drawOpt(colors.accent, 2));
            }

            // Node Outer Ring & Pulse
            Color ringCol = n.color;
            if (n.pulsePhase > 0.0f) {
                ringCol = makeColor(255, 255, 255);
            }
            d.circle({ cx, cy }, radius, drawOpt(ringCol, 3));
            d.circle({ cx, cy }, radius - 6, drawOpt(makeColor(ringCol.r / 2, ringCol.g / 2, ringCol.b / 2, 180), 1));

            // Node Core Fill
            Color coreCol = n.isAudible ? ringCol : makeColor(40, 40, 50, 200);
            d.filledRect({ cx - radius + 12, cy - radius + 12 }, { (radius - 12) * 2, (radius - 12) * 2 }, drawOpt(makeColor(coreCol.r / 3, coreCol.g / 3, coreCol.b / 3, 160)));

            // Node Label / Subtype Text
            const char* typeLabel = "";
            if (n.type == NodeType::OSC) {
                static const char* oscNames[] = { "SINE", "SQR", "SAW", "TRI", "NOISE" };
                typeLabel = oscNames[std::clamp(n.subType, 0, 4)];
            } else {
                static const char* fxNames[] = { "LP-FLT", "HP-FLT", "DELAY", "DIST", "CRUSH" };
                typeLabel = fxNames[std::clamp(n.subType, 0, 4)];
            }

            d.text({ cx - 18, cy - 8 }, typeLabel, 10, textOpt(makeColor(255, 255, 255)));

            // Frequency HUD under bubble
            char hudBuf[32];
            if (n.type == NodeType::OSC) {
                std::snprintf(hudBuf, sizeof(hudBuf), "%.0fHz", n.frequency);
            } else {
                std::snprintf(hudBuf, sizeof(hudBuf), "P1:%.2f", n.paramA);
            }
            d.text({ cx - 20, cy + radius + 4 }, hudBuf, 10, textOpt(makeColor(180, 190, 210)));
        }

        // Connecting Link Line Preview
        if (app.isConnecting) {
            const SynthNode* src = nullptr;
            for (const auto& n : app.nodes) {
                if (n.id == app.connectSourceId) src = &n;
            }
            if (src) {
                d.line({ static_cast<int>(src->pos.x), static_cast<int>(src->pos.y) }, { SCREEN_W / 2, SCREEN_H / 2 }, drawOpt(makeColor(255, 255, 0), 2));
            }
        }

        // 6. Top Bar UI (0..44)
        Color topBg = makeColor(18, 22, 32);
        d.filledRect({ 0, 0 }, { SCREEN_W, ZicApp::TOP_BAR_H }, drawOpt(topBg));
        d.line({ 0, ZicApp::TOP_BAR_H }, { SCREEN_W, ZicApp::TOP_BAR_H }, drawOpt(colors.accent, 2));

        // Top Buttons
        drawButton(d, { 10, 8 }, { 65, 28 }, "+ OSC", colors.oscStart);
        drawButton(d, { 85, 8 }, { 60, 28 }, "+ FX", colors.fxStart);
        drawButton(d, { 155, 8 }, { 80, 28 }, app.isPlaying ? "PAUSE" : "PLAY", app.isPlaying ? makeColor(0, 200, 100) : makeColor(200, 80, 80));
        drawButton(d, { 245, 8 }, { 70, 28 }, "RESET", makeColor(80, 90, 110));
        drawButton(d, { 325, 8 }, { 145, 28 }, THEMES[app.themeIdx].name.c_str(), colors.accent);

        // 7. Bottom Configuration Panel UI (600..800)
        renderBottomPanel(d, app);
    }

private:
    void drawButton(Draw& d, Position pos, Size sz, const char* label, Color bgCol)
    {
        int x = static_cast<int>(pos.x);
        int y = static_cast<int>(pos.y);
        int w = sz.w;
        int h = sz.h;

        d.filledRect({ x, y }, { w, h }, drawOpt(makeColor(bgCol.r / 2, bgCol.g / 2, bgCol.b / 2, 220)));
        d.line({ x, y }, { x + w, y }, drawOpt(bgCol, 1));
        d.line({ x, y + h }, { x + w, y + h }, drawOpt(bgCol, 1));
        d.line({ x, y }, { x, y + h }, drawOpt(bgCol, 1));
        d.line({ x + w, y }, { x + w, y + h }, drawOpt(bgCol, 1));

        d.text({ x + 8, y + 6 }, label, 10, textOpt(makeColor(255, 255, 255)));
    }

    void renderBottomPanel(Draw& d, ZicApp& app)
    {
        const ThemeColors& colors = THEMES[app.themeIdx].colors;
        int panelY = ZicApp::CANVAS_BOTTOM;

        // Panel Background Fill
        d.filledRect({ 0, panelY }, { SCREEN_W, ZicApp::BOTTOM_PANEL_H }, drawOpt(colors.sidebarBg));
        d.line({ 0, panelY }, { SCREEN_W, panelY }, drawOpt(colors.accent, 2));

        SynthNode* node = app.getSelectedNode();

        if (node) {
            // Header Title
            char titleBuf[64];
            const char* typeName = (node->type == NodeType::OSC) ? "OSCILLATOR" : "EFFECT";
            std::snprintf(titleBuf, sizeof(titleBuf), "%s [%s]", typeName, node->id.c_str());
            d.text({ 16, panelY + 12 }, titleBuf, 10, textOpt(makeColor(255, 255, 255)));

            // Subtype Pill Buttons (Y: panelY + 45)
            int btnW = 70;
            int startX = 16;
            int btnY = panelY + 45;

            if (node->type == NodeType::OSC) {
                static const char* oscNames[] = { "Sine", "Square", "Saw", "Tri", "Noise" };
                for (int i = 0; i < 5; ++i) {
                    int bx = startX + i * (btnW + 8);
                    Color bCol = (node->subType == i) ? colors.oscStart : makeColor(40, 45, 60);
                    drawButton(d, { static_cast<float>(bx), static_cast<float>(btnY) }, { btnW, 32 }, oscNames[i], bCol);
                }
            } else {
                static const char* fxNames[] = { "Lowpass", "Highpass", "Delay", "Distort", "Crush" };
                for (int i = 0; i < 5; ++i) {
                    int bx = startX + i * (btnW + 8);
                    Color bCol = (node->subType == i) ? colors.fxStart : makeColor(40, 45, 60);
                    drawButton(d, { static_cast<float>(bx), static_cast<float>(btnY) }, { btnW, 32 }, fxNames[i], bCol);
                }
            }

            // Slider 1: Pitch / Cutoff (Y: panelY + 95)
            int s1Y = panelY + 95;
            const char* s1Label = (node->type == NodeType::OSC) ? "Freq (Hz)" : "Param A";
            d.text({ 16, s1Y + 4 }, s1Label, 10, textOpt(makeColor(180, 190, 210)));

            d.filledRect({ 100, s1Y }, { 350, 20 }, drawOpt(makeColor(30, 36, 48)));
            float val1Norm = 0.5f;
            if (node->type == NodeType::OSC) {
                val1Norm = std::log2(node->frequency / 55.0f) / 5.0f;
                val1Norm = std::clamp(val1Norm, 0.0f, 1.0f);
            } else {
                val1Norm = std::clamp(node->paramA, 0.0f, 1.0f);
            }
            int fillW1 = static_cast<int>(val1Norm * 350.0f);
            d.filledRect({ 100, s1Y }, { fillW1, 20 }, drawOpt(colors.accent));

            // Slider 2: Gain / Param B (Y: panelY + 130)
            int s2Y = panelY + 130;
            const char* s2Label = (node->type == NodeType::OSC) ? "Gain" : "Param B";
            d.text({ 16, s2Y + 4 }, s2Label, 10, textOpt(makeColor(180, 190, 210)));

            d.filledRect({ 100, s2Y }, { 350, 20 }, drawOpt(makeColor(30, 36, 48)));
            float val2Norm = std::clamp(node->paramB, 0.0f, 1.0f);
            int fillW2 = static_cast<int>(val2Norm * 350.0f);
            d.filledRect({ 100, s2Y }, { fillW2, 20 }, drawOpt(colors.accent));

            // Action Buttons (Y: panelY + 165)
            int actY = panelY + 165;
            drawButton(d, { 16, static_cast<float>(actY) }, { 124, 28 }, node->isAudible ? "FEED: ON" : "FEED: LFO", node->isAudible ? makeColor(0, 180, 100) : makeColor(140, 140, 160));
            drawButton(d, { 150, static_cast<float>(actY) }, { 120, 28 }, app.isConnecting ? "CANCEL LINK" : "LINK WIRE", app.isConnecting ? makeColor(220, 180, 0) : makeColor(70, 80, 110));
            drawButton(d, { 280, static_cast<float>(actY) }, { 180, 28 }, "DELETE MOLECULE", makeColor(200, 50, 50));
        } else {
            // Empty Selection State
            d.text({ 16, panelY + 20 }, "MOLECULAR SYNTH UNIVERSE", 10, textOpt(makeColor(255, 255, 255)));

            char statsBuf[128];
            std::snprintf(statsBuf, sizeof(statsBuf), "Molecules: %d  |  Flux Links: %d  |  Catalysts: %d", static_cast<int>(app.nodes.size()), static_cast<int>(app.connections.size()), static_cast<int>(app.catalysts.size()));
            d.text({ 16, panelY + 50 }, statsBuf, 10, textOpt(makeColor(0, 210, 255)));

            d.text({ 16, panelY + 90 }, "Instructions:", 10, textOpt(makeColor(200, 205, 220)));
            d.text({ 24, panelY + 115 }, "- Tap '+ OSC' or '+ FX' to spawn synth molecules", 10, textOpt(makeColor(150, 160, 180)));
            d.text({ 24, panelY + 138 }, "- Touch and drag molecules to position them", 10, textOpt(makeColor(150, 160, 180)));
            d.text({ 24, panelY + 161 }, "- Tap a molecule to tweak params at the bottom", 10, textOpt(makeColor(150, 160, 180)));
        }
    }
};
