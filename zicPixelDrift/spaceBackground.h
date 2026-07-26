#pragma once

#include "draw/draw.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

struct Particle {
    float x, y, z;
    float size;
    float speed;
    float hue; // Color spectrum phase
};

class SpaceBackground {
private:
    std::vector<Particle> particles;
    float kickPulseFlash = 0.0f;
    int numParticles = 28;

public:
    SpaceBackground()
    {
        initParticles();
    }

    void initParticles()
    {
        particles.clear();
        for (int i = 0; i < numParticles; ++i) {
            float z = 10.0f + (rand() % 300);
            float x = (rand() % 320) - 160.0f;
            float y = (rand() % 176) - 88.0f;
            float size = 1.5f + (rand() % 40) * 0.1f;
            float speed = 1.0f + (rand() % 20) * 0.1f;
            particles.push_back({ x, y, z, size, speed, (float)(rand() % 360) });
        }
    }

    void triggerKickPulse()
    {
        kickPulseFlash = 1.0f;
    }

    void updateAndDraw(Draw& d, float bpm, float drive, int screenW, int screenH, int startX = 0, int startY = 0, int viewW = 0, int viewH = 0)
    {
        if (viewW == 0) viewW = screenW - startX;
        if (viewH == 0) viewH = screenH - startY;

        // Decay kick pulse glow effect
        if (kickPulseFlash > 0.0f) {
            kickPulseFlash -= 0.08f;
            if (kickPulseFlash < 0.0f) kickPulseFlash = 0.0f;
        }

        float speedMult = (bpm / 120.0f) * 4.0f;
        int centerX = startX + viewW / 2;
        int centerY = startY + viewH / 2;

        // Draw deep cosmic space background base
        uint8_t bgR = (uint8_t)(10 + kickPulseFlash * 25.0f);
        uint8_t bgG = (uint8_t)(10 + drive * 15.0f);
        uint8_t bgB = (uint8_t)(18 + kickPulseFlash * 35.0f);
        d.filledRect({ startX, startY }, { viewW, viewH }, { .color = { bgR, bgG, bgB, 255 } });

        // Update & Render Space Particles / Asteroids
        for (auto& p : particles) {
            p.z -= p.speed * speedMult;
            if (p.z <= 1.0f) {
                p.z = 300.0f;
                p.x = (rand() % viewW) - (viewW / 2.0f);
                p.y = (rand() % viewH) - (viewH / 2.0f);
            }

            // 3D Perspective Projection
            float projScale = 120.0f / p.z;
            int px = centerX + (int)(p.x * projScale);
            int py = centerY + (int)(p.y * projScale);
            int pSize = std::max(1, (int)(p.size * projScale));

            if (px >= startX + 2 && px < startX + viewW - 2 && py >= startY + 2 && py < startY + viewH - 2) {
                // Color calculation based on drive and kick pulse
                uint8_t r = (uint8_t)std::clamp(100.0f + drive * 155.0f + kickPulseFlash * 100.0f, 0.0f, 255.0f);
                uint8_t g = (uint8_t)std::clamp(120.0f - drive * 50.0f + kickPulseFlash * 80.0f, 0.0f, 255.0f);
                uint8_t b = (uint8_t)std::clamp(200.0f - drive * 100.0f + kickPulseFlash * 55.0f, 0.0f, 255.0f);

                Color particleColor = { r, g, b, 255 };

                if (pSize <= 2) {
                    d.pixel({ px, py }, { .color = particleColor });
                } else {
                    d.filledRect({ px - pSize / 2, py - pSize / 2 }, { pSize, pSize }, { .color = particleColor });
                }
            }
        }
    }
};
