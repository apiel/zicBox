#pragma once

#include "draw/draw.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

struct DriftParticle {
    float x, y, z;
    float size;
    float speed;
    float hue;
    uint8_t trackOwner;
};

class DriftVisualizer {
private:
    std::vector<DriftParticle> particles;
    float kickPulseFlash = 0.0f;
    float bassPulse = 0.0f;
    int numParticles = 40;

public:
    DriftVisualizer()
    {
        initParticles();
    }

    void initParticles()
    {
        particles.clear();
        for (int i = 0; i < numParticles; ++i) {
            float z = 10.0f + (rand() % 300);
            float x = (rand() % 320) - 160.0f;
            float y = (rand() % 240) - 120.0f;
            float size = 1.5f + (rand() % 30) * 0.1f;
            float speed = 1.2f + (rand() % 25) * 0.1f;
            particles.push_back({ x, y, z, size, speed, (float)(rand() % 360), (uint8_t)(i % 8) });
        }
    }

    void triggerKickPulse()
    {
        kickPulseFlash = 1.0f;
    }

    void triggerBassPulse(float amount)
    {
        bassPulse = std::clamp(amount, 0.0f, 1.0f);
    }

    void updateAndDraw(Draw& d, float bpm, int screenW, int screenH, int startX = 0, int startY = 0, int viewW = 0, int viewH = 0)
    {
        if (viewW == 0) viewW = screenW - startX;
        if (viewH == 0) viewH = screenH - startY;

        // Decay pulse effects
        if (kickPulseFlash > 0.0f) {
            kickPulseFlash -= 0.06f;
            if (kickPulseFlash < 0.0f) kickPulseFlash = 0.0f;
        }
        if (bassPulse > 0.0f) {
            bassPulse -= 0.04f;
            if (bassPulse < 0.0f) bassPulse = 0.0f;
        }

        float speedMult = (bpm / 120.0f) * 3.5f * (1.0f + kickPulseFlash * 0.8f);
        int centerX = startX + viewW / 2;
        int centerY = startY + viewH / 2;

        // Deep cosmic / synthwave gradient background base
        uint8_t bgR = (uint8_t)(8 + kickPulseFlash * 35.0f);
        uint8_t bgG = (uint8_t)(8 + bassPulse * 20.0f);
        uint8_t bgB = (uint8_t)(16 + kickPulseFlash * 45.0f + bassPulse * 25.0f);
        d.filledRect({ startX, startY }, { viewW, viewH }, { .color = { bgR, bgG, bgB, 255 } });

        // Draw dynamic audio grid warp lines
        int numLines = 6;
        for (int i = 0; i < numLines; ++i) {
            int yPos = startY + (viewH * (i + 1)) / (numLines + 1);
            float wave = std::sin(yPos * 0.05f + kickPulseFlash * 5.0f) * (4.0f + kickPulseFlash * 12.0f);
            Color lineCol = { (uint8_t)(30 + kickPulseFlash * 60), (uint8_t)(40 + bassPulse * 80), (uint8_t)(80 + kickPulseFlash * 100), 120 };
            d.line({ startX, (int)(yPos + wave) }, { startX + viewW, (int)(yPos - wave) }, { .color = lineCol, .thickness = 1 });
        }

        // Render Drift Particles
        for (auto& p : particles) {
            p.z -= p.speed * speedMult;
            if (p.z <= 1.0f) {
                p.z = 300.0f;
                p.x = (rand() % viewW) - (viewW / 2.0f);
                p.y = (rand() % viewH) - (viewH / 2.0f);
            }

            // 3D Perspective Projection
            float projScale = 140.0f / p.z;
            int px = centerX + (int)(p.x * projScale);
            int py = centerY + (int)(p.y * projScale);
            int pSize = std::max(1, (int)(p.size * projScale));

            if (px >= startX + 2 && px < startX + viewW - 2 && py >= startY + 2 && py < startY + viewH - 2) {
                uint8_t r = (uint8_t)std::clamp(100.0f + kickPulseFlash * 155.0f, 0.0f, 255.0f);
                uint8_t g = (uint8_t)std::clamp(120.0f + bassPulse * 135.0f, 0.0f, 255.0f);
                uint8_t b = (uint8_t)std::clamp(200.0f + kickPulseFlash * 55.0f, 0.0f, 255.0f);

                Color particleColor = { r, g, b, (uint8_t)std::clamp(180.0f + kickPulseFlash * 75.0f, 0.0f, 255.0f) };

                if (pSize <= 2) {
                    d.pixel({ px, py }, { .color = particleColor });
                } else {
                    d.filledRect({ px - pSize / 2, py - pSize / 2 }, { pSize, pSize }, { .color = particleColor });
                }
            }
        }
    }
};

inline DriftVisualizer driftVisualizer;
