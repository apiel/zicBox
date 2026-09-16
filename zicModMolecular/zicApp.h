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
    static constexpr int TOP_BAR_H = 50;  // Expanded for finger touch
    static constexpr int BOTTOM_PANEL_H = 220; // Expanded bottom panel
    static constexpr int CANVAS_TOP = TOP_BAR_H;
    static constexpr int CANVAS_BOTTOM = SCREEN_H - BOTTOM_PANEL_H; // 580

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
    int activeSliderIdx = -1; // 0 = Freq/ParamA, 1 = ParamB/Gain, 2 = ModDepth

    int themeIdx = 0;
    bool isPlaying = true;
    uint32_t nodeCounter = 1;

    AudioEngine audioEngine;

    ZicApp()
    {
        initDefaultPatch();
        seedCatalysts(35);
    }

    void initDefaultPatch()
    {
        nodes.clear();
        connections.clear();

        // Default OSC 1 (Sawtooth)
        SynthNode osc1;
        osc1.id = "osc_1";
        osc1.type = NodeType::OSC;
        osc1.subType = static_cast<int>(OscType::SAW);
        osc1.pos = { 160.0f, 220.0f };
        osc1.size = 86.0f; // Large touch bubble
        osc1.frequency = 220.0f;
        osc1.paramB = 0.65f;
        osc1.isAudible = true;
        osc1.color = THEMES[themeIdx].colors.oscStart;
        nodes.push_back(osc1);

        // Default FX 1 (Lowpass Filter)
        SynthNode fx1;
        fx1.id = "fx_1";
        fx1.type = NodeType::FX;
        fx1.subType = static_cast<int>(FxType::FILTER_LP);
        fx1.pos = { 320.0f, 380.0f };
        fx1.size = 90.0f;
        fx1.paramA = 0.65f; // Cutoff
        fx1.paramB = 0.40f; // Resonance
        fx1.isAudible = true;
        fx1.color = THEMES[themeIdx].colors.fxStart;
        nodes.push_back(fx1);

        // Default Link: OSC 1 -> FX 1 (Modulate Cutoff)
        Connection conn;
        conn.id = "conn_1";
        conn.fromId = "osc_1";
        conn.toId = "fx_1";
        conn.target = ModTarget::CUTOFF;
        conn.depth = 0.60f;
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
            p.vx = ((rand() % 100) / 50.0f - 1.0f) * 1.6f;
            p.vy = ((rand() % 100) / 50.0f - 1.0f) * 1.6f;
            p.size = (rand() % 30) / 10.0f + 1.5f;
            p.color = makeColor(180 + rand() % 75, 200 + rand() % 55, 255, 190);
            catalysts.push_back(p);
        }
    }

    void addNode(NodeType type)
    {
        SynthNode n;
        n.id = (type == NodeType::OSC ? "osc_" : "fx_") + std::to_string(++nodeCounter);
        n.type = type;
        n.subType = 0;
        n.pos = { 100.0f + static_cast<float>(rand() % 280), 120.0f + static_cast<float>(rand() % 340) };
        n.size = 86.0f;
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

        connections.erase(
            std::remove_if(connections.begin(), connections.end(), [&](const Connection& c) {
                return c.fromId == selectedNodeId || c.toId == selectedNodeId;
            }),
            connections.end());

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

    Connection* getSelectedConnection()
    {
        for (auto& c : connections) {
            if (c.id == selectedConnectionId) return &c;
        }
        return nullptr;
    }

    SynthNode* findNodeAt(float x, float y)
    {
        for (auto& n : nodes) {
            float dx = x - n.pos.x;
            float dy = y - n.pos.y;
            float distSq = dx * dx + dy * dy;
            float r = n.size * 0.65f; // Generous hit radius for fingers
            if (distSq <= r * r) return &n;
        }
        return nullptr;
    }

    Connection* findConnectionAt(float x, float y)
    {
        for (auto& c : connections) {
            const SynthNode* from = nullptr;
            const SynthNode* to = nullptr;
            for (const auto& n : nodes) {
                if (n.id == c.fromId) from = &n;
                if (n.id == c.toId) to = &n;
            }

            if (from && to) {
                // Distance from point to line segment
                float dx = to->pos.x - from->pos.x;
                float dy = to->pos.y - from->pos.y;
                float lengthSq = dx * dx + dy * dy;
                if (lengthSq < 1e-4) continue;

                float t = ((x - from->pos.x) * dx + (y - from->pos.y) * dy) / lengthSq;
                t = std::clamp(t, 0.0f, 1.0f);

                float projX = from->pos.x + t * dx;
                float projY = from->pos.y + t * dy;
                float distSq = (x - projX) * (x - projX) + (y - projY) * (y - projY);

                if (distSq <= 18.0f * 18.0f) { // 18px tap tolerance for fingers
                    return &c;
                }
            }
        }
        return nullptr;
    }

    void updatePhysics()
    {
        // Update particles in canvas
        for (auto& p : catalysts) {
            p.x += p.vx;
            p.y += p.vy;

            if (p.x < 10 || p.x > SCREEN_W - 10) p.vx = -p.vx;
            if (p.y < CANVAS_TOP + 10 || p.y > CANVAS_BOTTOM - 10) p.vy = -p.vy;

            // Check collision with nodes for visual & audio disturbance!
            for (auto& n : nodes) {
                float dx = p.x - n.pos.x;
                float dy = p.y - n.pos.y;
                if (dx * dx + dy * dy < (n.size * 0.5f) * (n.size * 0.5f)) {
                    n.pulsePhase = 1.0f;
                    n.disturbance = 0.8f; // Trigger acoustic impact sound transient
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
        // 1. Check Top Bar (Y: 0..50)
        if (y < TOP_BAR_H) {
            if (x >= 10 && x <= 80) { addNode(NodeType::OSC); return; }
            if (x >= 90 && x <= 155) { addNode(NodeType::FX); return; }
            if (x >= 165 && x <= 245) { isPlaying = !isPlaying; audioEngine.isMuted = !isPlaying; return; }
            if (x >= 255 && x <= 325) { initDefaultPatch(); return; }
            if (x >= 335 && x <= 470) { cycleTheme(); return; }
            return;
        }

        // 2. Check Bottom Panel (Y: 580..800)
        if (y >= CANVAS_BOTTOM) {
            handleBottomPanelTouch(x, y);
            return;
        }

        // 3. Canvas Region (Y: 50..580)
        SynthNode* touchedNode = findNodeAt(static_cast<float>(x), static_cast<float>(y));

        if (touchedNode) {
            selectedNodeId = touchedNode->id;
            selectedConnectionId = "";

            if (isConnecting) {
                if (connectSourceId != touchedNode->id) {
                    Connection c;
                    c.id = "conn_" + std::to_string(++nodeCounter);
                    c.fromId = connectSourceId;
                    c.toId = touchedNode->id;
                    c.target = (touchedNode->type == NodeType::OSC) ? ModTarget::FREQUENCY : ModTarget::CUTOFF;
                    c.depth = 0.60f;
                    connections.push_back(c);
                    selectedConnectionId = c.id;
                    selectedNodeId = "";
                }
                isConnecting = false;
                connectSourceId = "";
            } else {
                isDraggingNode = true;
                dragNodeId = touchedNode->id;
                dragOffsetX = touchedNode->pos.x - static_cast<float>(x);
                dragOffsetY = touchedNode->pos.y - static_cast<float>(y);
            }
            return;
        }

        // Check if user touched a connection wire
        Connection* touchedConn = findConnectionAt(static_cast<float>(x), static_cast<float>(y));
        if (touchedConn) {
            selectedConnectionId = touchedConn->id;
            selectedNodeId = "";
            isConnecting = false;
            connectSourceId = "";
            return;
        }

        // Touch on empty canvas area -> deselect
        selectedNodeId = "";
        selectedConnectionId = "";
        isConnecting = false;
        connectSourceId = "";
    }

    void handleTouchMove(int x, int y)
    {
        if (isDraggingNode) {
            SynthNode* n = getSelectedNode();
            if (n) {
                n->pos.x = std::clamp(static_cast<float>(x) + dragOffsetX, 45.0f, static_cast<float>(SCREEN_W - 45));
                n->pos.y = std::clamp(static_cast<float>(y) + dragOffsetY, static_cast<float>(CANVAS_TOP + 45), static_cast<float>(CANVAS_BOTTOM - 45));

                if (n->type == NodeType::OSC) {
                    float normX = (n->pos.x - 45.0f) / static_cast<float>(SCREEN_W - 90);
                    n->frequency = 55.0f * std::pow(2.0f, normX * 5.0f);
                } else {
                    n->paramA = (n->pos.x - 45.0f) / static_cast<float>(SCREEN_W - 90);
                    n->paramB = 1.0f - (n->pos.y - CANVAS_TOP - 45.0f) / static_cast<float>(CANVAS_BOTTOM - CANVAS_TOP - 90);
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
        int relY = y - CANVAS_BOTTOM;
        SynthNode* n = getSelectedNode();
        Connection* c = getSelectedConnection();

        if (c) {
            // Flux Link Configuration Panel
            // Target selector buttons (Y: 55..95)
            if (relY >= 55 && relY <= 95) {
                int btnW = 90;
                int startX = 16;
                static const ModTarget targets[] = { ModTarget::FREQUENCY, ModTarget::GAIN, ModTarget::CUTOFF, ModTarget::RESONANCE };
                for (int i = 0; i < 4; ++i) {
                    int bx = startX + i * (btnW + 8);
                    if (x >= bx && x <= bx + btnW) {
                        c->target = targets[i];
                        return;
                    }
                }
            }

            // Slider: Modulation Depth (Y: 110..145)
            if (relY >= 110 && relY <= 145 && x >= 100 && x <= 450) {
                isDraggingSlider = true;
                float norm = static_cast<float>(x - 100) / 350.0f;
                c->depth = std::clamp(norm, 0.0f, 1.0f);
                return;
            }

            // Action Button: Sever Link (Y: 165..205)
            if (relY >= 165 && relY <= 205 && x >= 16 && x <= 460) {
                deleteSelectedConnection();
                return;
            }

            return;
        }

        if (n) {
            // Node Configuration Panel
            // Subtype selector buttons (Y: 50..90 in panel)
            if (relY >= 50 && relY <= 90) {
                int buttonWidth = 76;
                int startX = 16;
                for (int i = 0; i < 5; ++i) {
                    int bx = startX + i * (buttonWidth + 8);
                    if (x >= bx && x <= bx + buttonWidth) {
                        n->subType = i;
                        return;
                    }
                }
            }

            // Slider 1: Frequency / Param A (Y: 100..130)
            if (relY >= 100 && relY <= 130 && x >= 100 && x <= 450) {
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

            // Slider 2: Gain / Param B (Y: 135..165)
            if (relY >= 135 && relY <= 165 && x >= 100 && x <= 450) {
                isDraggingSlider = true;
                float norm = static_cast<float>(x - 100) / 350.0f;
                norm = std::clamp(norm, 0.0f, 1.0f);
                n->paramB = norm;
                return;
            }

            // Action Buttons (Y: 172..210)
            if (relY >= 172 && relY <= 210) {
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
