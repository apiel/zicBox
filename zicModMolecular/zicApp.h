#pragma once

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "audioEngine.h"
#include "types.h"

class ZicApp {
public:
    static constexpr int SCREEN_W = 480;  // Portrait Width
    static constexpr int SCREEN_H = 800;  // Portrait Height

    // UI Regions
    static constexpr int TOP_BAR_H = 44;
    static constexpr int BOTTOM_PANEL_H = 200;
    static constexpr int CANVAS_TOP = TOP_BAR_H;
    static constexpr int CANVAS_BOTTOM = SCREEN_H - BOTTOM_PANEL_H; // 600

    std::vector<SynthNode> nodes;
    std::vector<Connection> connections;
    std::vector<CatalystParticle> catalysts;

    std::string selectedNodeId = "";
    std::string selectedConnectionId = "";

    bool isConnecting = false;
    std::string connectSourceId = "";

    bool isDraggingNode = false;
    std::string dragNodeId = "";
    float dragOffsetX = 0.0f;
    float dragOffsetY = 0.0f;

    bool isDraggingSlider = false;
    int activeSliderIdx = -1; // 0 = Freq/ParamA, 1 = ParamB

    int themeIdx = 0;
    bool isPlaying = true;
    uint32_t nodeCounter = 1;

    AudioEngine audioEngine;

    ZicApp()
    {
        initDefaultPatch();
        seedCatalysts(30);
    }

    void initDefaultPatch()
    {
        nodes.clear();
        connections.clear();

        // Default OSC 1 (Sine Wave)
        SynthNode osc1;
        osc1.id = "osc_1";
        osc1.type = NodeType::OSC;
        osc1.subType = static_cast<int>(OscType::SAW);
        osc1.pos = { 150.0f, 220.0f };
        osc1.size = 76.0f;
        osc1.frequency = 220.0f; // A3
        osc1.paramB = 0.6f;
        osc1.isAudible = true;
        osc1.color = THEMES[themeIdx].colors.oscStart;
        nodes.push_back(osc1);

        // Default FX 1 (Lowpass Filter)
        SynthNode fx1;
        fx1.id = "fx_1";
        fx1.type = NodeType::FX;
        fx1.subType = static_cast<int>(FxType::FILTER_LP);
        fx1.pos = { 330.0f, 380.0f };
        fx1.size = 84.0f;
        fx1.paramA = 0.65f; // Cutoff
        fx1.paramB = 0.40f; // Resonance
        fx1.isAudible = true;
        fx1.color = THEMES[themeIdx].colors.fxStart;
        nodes.push_back(fx1);

        // Default Link: OSC 1 -> FX 1
        Connection conn;
        conn.id = "conn_1";
        conn.fromId = "osc_1";
        conn.toId = "fx_1";
        connections.push_back(conn);

        selectedNodeId = osc1.id;
    }

    void seedCatalysts(int count)
    {
        catalysts.clear();
        for (int i = 0; i < count; ++i) {
            CatalystParticle p;
            p.x = static_cast<float>(rand() % SCREEN_W);
            p.y = static_cast<float>(CANVAS_TOP + rand() % (CANVAS_BOTTOM - CANVAS_TOP));
            p.vx = ((rand() % 100) / 50.0f - 1.0f) * 1.5f;
            p.vy = ((rand() % 100) / 50.0f - 1.0f) * 1.5f;
            p.size = (rand() % 30) / 10.0f + 1.0f;
            p.color = makeColor(180 + rand() % 75, 200 + rand() % 55, 255, 180);
            catalysts.push_back(p);
        }
    }

    void addNode(NodeType type)
    {
        SynthNode n;
        n.id = (type == NodeType::OSC ? "osc_" : "fx_") + std::to_string(++nodeCounter);
        n.type = type;
        n.subType = 0;
        n.pos = { 100.0f + static_cast<float>(rand() % 280), 120.0f + static_cast<float>(rand() % 360) };
        n.size = 74.0f;
        n.frequency = 440.0f;
        n.paramA = 0.5f;
        n.paramB = 0.5f;
        n.isAudible = true;
        n.color = (type == NodeType::OSC) ? THEMES[themeIdx].colors.oscStart : THEMES[themeIdx].colors.fxStart;

        nodes.push_back(n);
        selectedNodeId = n.id;
        selectedConnectionId = "";
    }

    void deleteSelectedNode()
    {
        if (selectedNodeId.empty()) return;

        // Remove connections associated with this node
        connections.erase(
            std::remove_if(connections.begin(), connections.end(), [&](const Connection& c) {
                return c.fromId == selectedNodeId || c.toId == selectedNodeId;
            }),
            connections.end());

        // Remove node
        nodes.erase(
            std::remove_if(nodes.begin(), nodes.end(), [&](const SynthNode& n) {
                return n.id == selectedNodeId;
            }),
            nodes.end());

        selectedNodeId = "";
    }

    void deleteSelectedConnection()
    {
        if (selectedConnectionId.empty()) return;

        connections.erase(
            std::remove_if(connections.begin(), connections.end(), [&](const Connection& c) {
                return c.id == selectedConnectionId;
            }),
            connections.end());

        selectedConnectionId = "";
    }

    void cycleTheme()
    {
        themeIdx = (themeIdx + 1) % 4;
        for (auto& n : nodes) {
            n.color = (n.type == NodeType::OSC) ? THEMES[themeIdx].colors.oscStart : THEMES[themeIdx].colors.fxStart;
        }
    }

    SynthNode* getSelectedNode()
    {
        for (auto& n : nodes) {
            if (n.id == selectedNodeId) return &n;
        }
        return nullptr;
    }

    SynthNode* findNodeAt(float x, float y)
    {
        for (auto& n : nodes) {
            float dx = x - n.pos.x;
            float dy = y - n.pos.y;
            float distSq = dx * dx + dy * dy;
            float r = n.size * 0.55f;
            if (distSq <= r * r) return &n;
        }
        return nullptr;
    }

    void updatePhysics()
    {
        // Update particles in canvas
        for (auto& p : catalysts) {
            p.x += p.vx;
            p.y += p.vy;

            // Bounce on canvas boundaries
            if (p.x < 10 || p.x > SCREEN_W - 10) p.vx = -p.vx;
            if (p.y < CANVAS_TOP + 10 || p.y > CANVAS_BOTTOM - 10) p.vy = -p.vy;

            // Check collision with nodes for visual pulse
            for (auto& n : nodes) {
                float dx = p.x - n.pos.x;
                float dy = p.y - n.pos.y;
                if (dx * dx + dy * dy < (n.size * 0.5f) * (n.size * 0.5f)) {
                    n.pulsePhase = 1.0f;
                }
            }
        }

        // Decay node pulse phase
        for (auto& n : nodes) {
            if (n.pulsePhase > 0.0f) {
                n.pulsePhase -= 0.05f;
                if (n.pulsePhase < 0.0f) n.pulsePhase = 0.0f;
            }
        }
    }

    void handleTouchDown(int x, int y)
    {
        // 1. Check Top Bar (Y: 0..44)
        if (y < TOP_BAR_H) {
            if (x >= 10 && x <= 75) { addNode(NodeType::OSC); return; }
            if (x >= 85 && x <= 145) { addNode(NodeType::FX); return; }
            if (x >= 155 && x <= 235) { isPlaying = !isPlaying; audioEngine.isMuted = !isPlaying; return; }
            if (x >= 245 && x <= 315) { initDefaultPatch(); return; }
            if (x >= 325 && x <= 470) { cycleTheme(); return; }
            return;
        }

        // 2. Check Bottom Panel (Y: 600..800)
        if (y >= CANVAS_BOTTOM) {
            handleBottomPanelTouch(x, y);
            return;
        }

        // 3. Canvas Region (Y: 44..600)
        SynthNode* touchedNode = findNodeAt(static_cast<float>(x), static_cast<float>(y));

        if (touchedNode) {
            selectedNodeId = touchedNode->id;
            selectedConnectionId = "";

            // Check if user is starting a link wire (Double touch / Long hold simulation or connect mode)
            if (isConnecting) {
                if (connectSourceId != touchedNode->id) {
                    Connection c;
                    c.id = "conn_" + std::to_string(++nodeCounter);
                    c.fromId = connectSourceId;
                    c.toId = touchedNode->id;
                    connections.push_back(c);
                }
                isConnecting = false;
                connectSourceId = "";
            } else {
                // Start dragging node
                isDraggingNode = true;
                dragNodeId = touchedNode->id;
                dragOffsetX = touchedNode->pos.x - static_cast<float>(x);
                dragOffsetY = touchedNode->pos.y - static_cast<float>(y);
            }
        } else {
            // Touch on empty canvas area -> deselect
            selectedNodeId = "";
            selectedConnectionId = "";
            isConnecting = false;
            connectSourceId = "";
        }
    }

    void handleTouchMove(int x, int y)
    {
        if (isDraggingNode) {
            SynthNode* n = getSelectedNode();
            if (n) {
                n->pos.x = std::clamp(static_cast<float>(x) + dragOffsetX, 40.0f, static_cast<float>(SCREEN_W - 40));
                n->pos.y = std::clamp(static_cast<float>(y) + dragOffsetY, static_cast<float>(CANVAS_TOP + 40), static_cast<float>(CANVAS_BOTTOM - 40));

                // Position X dynamically adjusts frequency or cutoff!
                if (n->type == NodeType::OSC) {
                    float normX = (n->pos.x - 40.0f) / static_cast<float>(SCREEN_W - 80);
                    n->frequency = 55.0f * std::pow(2.0f, normX * 5.0f);
                } else {
                    n->paramA = (n->pos.x - 40.0f) / static_cast<float>(SCREEN_W - 80);
                    n->paramB = 1.0f - (n->pos.y - CANVAS_TOP - 40.0f) / static_cast<float>(CANVAS_BOTTOM - CANVAS_TOP - 80);
                }
            }
        } else if (isDraggingSlider) {
            handleBottomPanelTouch(x, y);
        }
    }

    void handleTouchUp()
    {
        isDraggingNode = false;
        isDraggingSlider = false;
    }

private:
    void handleBottomPanelTouch(int x, int y)
    {
        SynthNode* n = getSelectedNode();

        if (n) {
            // Node Configuration Panel
            int relY = y - CANVAS_BOTTOM;

            // Subtype selector buttons (Y: 45..85 in panel)
            if (relY >= 45 && relY <= 85) {
                int buttonWidth = 70;
                int startX = 16;
                int count = (n->type == NodeType::OSC) ? 5 : 5;
                for (int i = 0; i < count; ++i) {
                    int bx = startX + i * (buttonWidth + 8);
                    if (x >= bx && x <= bx + buttonWidth) {
                        n->subType = i;
                        return;
                    }
                }
            }

            // Slider 1: Frequency / Param A (Y: 95..125)
            if (relY >= 95 && relY <= 125 && x >= 100 && x <= 450) {
                isDraggingSlider = true;
                float norm = static_cast<float>(x - 100) / 350.0f;
                norm = std::clamp(norm, 0.0f, 1.0f);
                if (n->type == NodeType::OSC) {
                    n->frequency = 55.0f * std::pow(2.0f, norm * 5.0f);
                } else {
                    n->paramA = norm;
                }
                return;
            }

            // Slider 2: Gain / Param B (Y: 130..160)
            if (relY >= 130 && relY <= 160 && x >= 100 && x <= 450) {
                isDraggingSlider = true;
                float norm = static_cast<float>(x - 100) / 350.0f;
                norm = std::clamp(norm, 0.0f, 1.0f);
                n->paramB = norm;
                return;
            }

            // Action Buttons (Y: 165..195)
            if (relY >= 165 && relY <= 195) {
                if (x >= 16 && x <= 140) {
                    n->isAudible = !n->isAudible;
                    return;
                }
                if (x >= 150 && x <= 270) {
                    isConnecting = true;
                    connectSourceId = n->id;
                    return;
                }
                if (x >= 280 && x <= 460) {
                    deleteSelectedNode();
                    return;
                }
            }
        }
    }
};
