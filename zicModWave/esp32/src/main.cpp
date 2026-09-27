#include <Arduino.h>
#include <cmath>
#include <algorithm>
#include "../../displayView.h"
#include "../../zicApp.h"
#include "displayESP32.h"

// Hardware Pin Definitions for 3x3 Control Grid
#define ENCODER_PIN_A  2
#define ENCODER_PIN_B  3
#define ENCODER_PIN_SW 43

struct PotInfo {
    const char* name;
    int pin;
    float filteredMv;
    int percentage;
};

// 8 Analog Potentiometers (A11 Speed, A10 Morph, A7, A8, A9, A4 Res, A5 Cutoff, A6)
PotInfo pots[8] = {
    { "A11 (Speed)", 11, 0.0f, -1 },
    { "A10 (Morph)", 10, 0.0f, -1 },
    { "A7",           7, 0.0f, -1 },
    { "A8",           8, 0.0f, -1 },
    { "A9",           9, 0.0f, -1 },
    { "A4 (Res)",     4, 0.0f, -1 },
    { "A5 (Cutoff)",  5, 0.0f, -1 },
    { "A6",           6, 0.0f, -1 }
};

int activePotIndex = -1;
int potOverlayTimer = 0;

struct Point3D {
    float x, y, z;
};

struct Point2D {
    int x, y;
};

// 12 Edges connecting 8 shape vertices
const int SHAPE_EDGES[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}, // Top face / edges
    {4, 5}, {5, 6}, {6, 7}, {7, 4}, // Bottom face
    {0, 4}, {1, 5}, {2, 6}, {3, 7}  // Vertical connecting edges
};

struct VertCorner {
    int u;
    int w1;
    int w2;
};

// 24 Corner angle pairs around the 8 3D vertices
const VertCorner VERT_CORNERS[24] = {
    {0, 1, 3}, {0, 1, 4}, {0, 3, 4},
    {1, 0, 2}, {1, 0, 5}, {1, 2, 5},
    {2, 1, 3}, {2, 1, 6}, {2, 3, 6},
    {3, 0, 2}, {3, 0, 7}, {3, 2, 7},
    {4, 0, 5}, {4, 0, 7}, {4, 5, 7},
    {5, 1, 4}, {5, 1, 6}, {5, 4, 6},
    {6, 2, 5}, {6, 2, 7}, {6, 5, 7},
    {7, 3, 4}, {7, 3, 6}, {7, 4, 6}
};

inline Point2D project3DPoint(Point3D p, float rotX, float rotY, float rotZ, float scale, int centerX, int centerY) {
    float y1 = p.y * cosf(rotX) - p.z * sinf(rotX);
    float z1 = p.y * sinf(rotX) + p.z * cosf(rotX);
    float x2 = p.x * cosf(rotY) + z1 * sinf(rotY);
    float z2 = -p.x * sinf(rotY) + z1 * cosf(rotY);
    float x3 = x2 * cosf(rotZ) - y1 * sinf(rotZ);
    float y3 = x2 * sinf(rotZ) + y1 * cosf(rotZ);
    float fov = 3.0f;
    float sz = z2 + 3.5f;
    return { centerX + (int)(x3 * scale * fov / sz), centerY + (int)(y3 * scale * fov / sz) };
}

float rotX = 0.0f;
float rotY = 0.0f;
float rotZ = 0.0f;
float animTime = 0.0f;

int centerX = 160;
int centerY = 92;
bool isRotating = true;

// 32-step simulated minimal sequencer pattern (1 = active note step, 0 = rest step)
const bool SEQ_STEPS[32] = {
    1,0,0,0, 1,0,1,0, 1,0,0,0, 1,1,0,0,
    1,0,0,1, 1,0,0,0, 1,0,1,0, 0,1,0,1
};

// 2-Bit Quadrature Gray Code State Table
static const int8_t KNOB_DIR[16] = {
    0, -1,  1,  0,
    1,  0,  0, -1,
   -1,  0,  0,  1,
    0,  1, -1,  0
};

static volatile uint8_t oldState = 0;
static volatile int encoderSubSteps = 0;
static volatile bool buttonPressed = false;
static volatile uint32_t lastButtonTime = 0;

void IRAM_ATTR encoderISR()
{
    uint8_t currentState = (digitalRead(ENCODER_PIN_A) << 1) | digitalRead(ENCODER_PIN_B);
    uint8_t index = (oldState << 2) | currentState;
    encoderSubSteps += KNOB_DIR[index];
    oldState = currentState;
}

void IRAM_ATTR buttonISR()
{
    uint32_t now = millis();
    if (now - lastButtonTime > 200) { // 200ms debounce
        buttonPressed = true;
        lastButtonTime = now;
    }
}

void setup()
{
    setCpuFrequencyMhz(240); // Lock CPU at 240MHz max speed

    // Initialize display backlight pins (GPIO 46 & 48)
    pinMode(46, OUTPUT);
    digitalWrite(46, HIGH);
    pinMode(48, OUTPUT);
    digitalWrite(48, HIGH);

    // Initialize Encoder GPIOs with internal pullups
    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    pinMode(ENCODER_PIN_SW, INPUT_PULLUP);

    // Initialize Analog Potentiometer Pins
    for (int i = 0; i < 8; ++i) {
        pinMode(pots[i].pin, INPUT);
    }

    oldState = (digitalRead(ENCODER_PIN_A) << 1) | digitalRead(ENCODER_PIN_B);

    // Attach hardware interrupts on CHANGE for encoder pins
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_SW), buttonISR, FALLING);

    initDisplayESP32();
}

void processInputs()
{
    // Process accumulated encoder substeps
    noInterrupts();
    int substeps = encoderSubSteps;
    encoderSubSteps %= 4; // keep remainder
    bool pressed = buttonPressed;
    buttonPressed = false;
    interrupts();

    int detents = substeps / 4;
    if (detents != 0) {
        centerX += detents * 8; // Move 8 pixels per detent click
        centerX = std::clamp(centerX, 20, 300);
    }

    if (pressed) {
        isRotating = !isRotating;
    }
}

void computeMorphedVertices(float t, float timeAnim, Point3D outVerts[8])
{
    // Target Cube Vertices (Top 4, Bottom 4)
    Point3D cubeVerts[8] = {
        { -1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f },
        {  1.0f, -1.0f,  1.0f }, { -1.0f, -1.0f,  1.0f },
        { -1.0f,  1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f },
        {  1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f }
    };

    if (t < 0.33f) {
        // Stage 1: 3-Sided Pyramid (Tetrahedron) -> 4-Sided Square Pyramid
        float m = t / 0.33f;
        Point3D apex = { 0.0f, -1.3f, 0.0f };

        // Top 4 vertices merged at Apex
        outVerts[0] = apex;
        outVerts[1] = apex;
        outVerts[2] = apex;
        outVerts[3] = apex;

        // Bottom 4 vertices morph from 3-sided triangle base to 4-sided square base
        Point3D triBase[4] = {
            { -1.2f, 1.0f, -0.7f },
            {  1.2f, 1.0f, -0.7f },
            {  0.0f, 1.0f,  1.4f },
            {  0.0f, 1.0f,  1.4f }
        };

        for (int i = 0; i < 4; ++i) {
            outVerts[4 + i].x = triBase[i].x * (1.0f - m) + cubeVerts[4 + i].x * m;
            outVerts[4 + i].y = triBase[i].y * (1.0f - m) + cubeVerts[4 + i].y * m;
            outVerts[4 + i].z = triBase[i].z * (1.0f - m) + cubeVerts[4 + i].z * m;
        }
    } else if (t < 0.66f) {
        // Stage 2: 4-Sided Square Pyramid -> 3D Cube
        float m = (t - 0.33f) / 0.33f;
        Point3D apex = { 0.0f, -1.3f, 0.0f };

        // Top 4 vertices morph from Apex outwards to Cube top corners
        for (int i = 0; i < 4; ++i) {
            outVerts[i].x = apex.x * (1.0f - m) + cubeVerts[i].x * m;
            outVerts[i].y = apex.y * (1.0f - m) + cubeVerts[i].y * m;
            outVerts[i].z = apex.z * (1.0f - m) + cubeVerts[i].z * m;
        }

        // Bottom 4 vertices stay at full square base
        for (int i = 4; i < 8; ++i) {
            outVerts[i] = cubeVerts[i];
        }
    } else {
        // Stage 3: 3D Cube -> Swirling Noise / Flying Dot Cloud
        float m = (t - 0.66f) / 0.34f;
        for (int i = 0; i < 8; ++i) {
            float phase = i * 1.3f + timeAnim * 3.0f;
            float nx = sinf(phase * 1.7f) * 1.8f * m;
            float ny = cosf(phase * 2.3f) * 1.8f * m;
            float nz = sinf(phase * 3.1f) * 1.8f * m;

            outVerts[i].x = cubeVerts[i].x + nx;
            outVerts[i].y = cubeVerts[i].y + ny;
            outVerts[i].z = cubeVerts[i].z + nz;
        }
    }
}

void loop()
{
    // Process encoder and push button interrupts
    processInputs();

    // Read all 8 Analog Pots (A11, A10, A7, A8, A9, A4, A5, A6)
    for (int i = 0; i < 8; ++i) {
        int mv = analogReadMilliVolts(pots[i].pin);
        if (pots[i].percentage == -1) {
            pots[i].filteredMv = (float)mv;
        } else {
            pots[i].filteredMv += ((float)mv - pots[i].filteredMv) * 0.25f;
        }
        int newPct = std::clamp((int)(pots[i].filteredMv / 3100.0f * 100.0f), 0, 100);

        if (pots[i].percentage != -1 && abs(newPct - pots[i].percentage) >= 2) {
            activePotIndex = i;
            potOverlayTimer = 90; // Show bottom toast overlay for 1.5 seconds
        }
        pots[i].percentage = newPct;
    }

    if (potOverlayTimer > 0) {
        potOverlayTimer--;
    }

    // A11 (pots[0]) controls rotation speed
    float speedNorm = (pots[0].percentage >= 0) ? (pots[0].percentage / 100.0f) : 0.5f;
    float speedMult = 0.02f + speedNorm * 4.98f;

    // A10 (pots[1]) controls 3D Shape Morphing
    float morphVal = (pots[1].percentage >= 0) ? (pots[1].percentage / 100.0f) : 0.5f;

    // A5 (pots[6]) controls Filter Cutoff
    float cutoffVal = (pots[6].percentage >= 0) ? (pots[6].percentage / 100.0f) : 0.5f;

    // A4 (pots[5]) controls Filter Resonance
    float resVal = (pots[5].percentage >= 0) ? (pots[5].percentage / 100.0f) : 0.2f;

    Draw& d = getDrawer();
    d.clear();

    // --- Minimal 32-Step Top Sequencer (Equal Pitch & Soft Muted Palette) ---
    int currentStep = (int)(animTime * 12.0f) % 32;
    int seqStartX = 48; // 32 steps * 7px pitch = 224px span, centered
    int seqTopY = 10;
    int bw = 4;
    int bh = 3;

    for (int i = 0; i < 32; ++i) {
        int bx = seqStartX + i * 7;
        int by = seqTopY;

        if (i == currentStep) {
            // Playhead Step: Soft cool slate highlight
            d.filledRect({ bx, by - 1 }, { bw, bh + 2 }, waveDrawOpt(waveMakeColor(160, 195, 220, 255)));
        } else if (SEQ_STEPS[i]) {
            // Active Note Step: Soft muted blue-slate
            d.filledRect({ bx, by }, { bw, bh }, waveDrawOpt(waveMakeColor(60, 95, 125, 255)));
        } else {
            // Inactive Step: Very subtle dark slate dash
            d.filledRect({ bx, by }, { bw, bh }, waveDrawOpt(waveMakeColor(32, 38, 48, 255)));
        }
    }

    // Increment rotation angles proportional to pot speed if active
    if (isRotating) {
        float dt = 0.016f * speedMult;
        rotX += 0.02f * speedMult;
        rotY += 0.03f * speedMult;
        rotZ += 0.015f * speedMult;
        animTime += dt;
    }

    // Compute 8 morphed 3D vertices based on A10 pot value
    Point3D morphedVerts[8];
    computeMorphedVertices(morphVal, animTime, morphedVerts);

    Point2D projected[8];
    float scale = 38.0f;

    // Transform and project 3D vertices to 2D screen
    for (int i = 0; i < 8; ++i) {
        float x = morphedVerts[i].x;
        float y = morphedVerts[i].y;
        float z = morphedVerts[i].z;

        // Rotate X
        float y1 = y * cosf(rotX) - z * sinf(rotX);
        float z1 = y * sinf(rotX) + z * cosf(rotX);

        // Rotate Y
        float x2 = x * cosf(rotY) + z1 * sinf(rotY);
        float z2 = -x * sinf(rotY) + z1 * cosf(rotY);

        // Rotate Z
        float x3 = x2 * cosf(rotZ) - y1 * sinf(rotZ);
        float y3 = x2 * sinf(rotZ) + y1 * cosf(rotZ);

        // Perspective Projection
        float fov = 3.0f;
        float distance = 3.5f;
        float sz = z2 + distance;

        projected[i].x = centerX + (int)(x3 * scale * fov / sz);
        projected[i].y = centerY + (int)(y3 * scale * fov / sz);
    }

    // --- Clean 3D Corner Radius Filleting (A5 Cutoff) & Resonant Corner-Only Glow (A4 Resonance) ---
    // Straight body lines stay consistent soft blue-slate
    DrawOptions lineOpt = waveDrawOpt(waveMakeColor(70, 120, 160, 255), 2);

    // Resonant Corner Color: Fades smoothly from soft blue-slate to vibrant glowing cyan-white
    uint8_t cR = (uint8_t)(70 + resVal * 120);
    uint8_t cG = (uint8_t)(120 + resVal * 115);
    uint8_t cB = (uint8_t)(160 + resVal * 95);
    DrawOptions cornerOpt = waveDrawOpt(waveMakeColor(cR, cG, cB, 255), 2);

    float cornerRadius = cutoffVal * 0.35f;

    // 1. Draw Truncated Straight Body Edges
    for (int i = 0; i < 12; ++i) {
        Point3D u = morphedVerts[SHAPE_EDGES[i][0]];
        Point3D v = morphedVerts[SHAPE_EDGES[i][1]];

        Point3D diff = { v.x - u.x, v.y - u.y, v.z - u.z };
        float len = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

        // Skip degenerate 0-length edges (e.g. top merged face edges of pyramid)
        if (len < 0.01f) {
            continue;
        }

        if (cornerRadius > 0.005f) {
            float effR = std::min(cornerRadius, len * 0.40f);
            Point3D dir = { diff.x / len, diff.y / len, diff.z / len };

            Point3D pA3D = { u.x + dir.x * effR, u.y + dir.y * effR, u.z + dir.z * effR };
            Point3D pB3D = { v.x - dir.x * effR, v.y - dir.y * effR, v.z - dir.z * effR };

            Point2D pA = project3DPoint(pA3D, rotX, rotY, rotZ, scale, centerX, centerY);
            Point2D pB = project3DPoint(pB3D, rotX, rotY, rotZ, scale, centerX, centerY);
            d.line({ pA.x, pA.y }, { pB.x, pB.y }, lineOpt);
        } else {
            Point2D p1 = projected[SHAPE_EDGES[i][0]];
            Point2D p2 = projected[SHAPE_EDGES[i][1]];
            d.line({ p1.x, p1.y }, { p2.x, p2.y }, lineOpt);
        }
    }

    // 2. Draw Clean Rounded Corner Arcs (Glowing with Resonance)
    if (cornerRadius > 0.005f) {
        for (int c = 0; c < 24; ++c) {
            Point3D u = morphedVerts[VERT_CORNERS[c].u];
            Point3D w1 = morphedVerts[VERT_CORNERS[c].w1];
            Point3D w2 = morphedVerts[VERT_CORNERS[c].w2];

            Point3D d1 = { w1.x - u.x, w1.y - u.y, w1.z - u.z };
            Point3D d2 = { w2.x - u.x, w2.y - u.y, w2.z - u.z };
            float len1 = sqrtf(d1.x * d1.x + d1.y * d1.y + d1.z * d1.z);
            float len2 = sqrtf(d2.x * d2.x + d2.y * d2.y + d2.z * d2.z);

            // Skip degenerate corners where both edges are 0-length
            if (len1 < 0.01f && len2 < 0.01f) {
                continue;
            }

            float r1 = (len1 > 0.01f) ? std::min(cornerRadius, len1 * 0.40f) : 0.0f;
            float r2 = (len2 > 0.01f) ? std::min(cornerRadius, len2 * 0.40f) : 0.0f;

            Point3D p1_3D = (len1 > 0.01f) ? Point3D{ u.x + (d1.x / len1) * r1, u.y + (d1.y / len1) * r1, u.z + (d1.z / len1) * r1 } : u;
            Point3D p2_3D = (len2 > 0.01f) ? Point3D{ u.x + (d2.x / len2) * r2, u.y + (d2.y / len2) * r2, u.z + (d2.z / len2) * r2 } : u;

            // Clean 4-segment quadratic Bezier corner arc
            Point2D arcPts[4];
            for (int s = 0; s <= 3; ++s) {
                float t = (float)s / 3.0f;
                float omt = 1.0f - t;
                Point3D pt3D = {
                    omt * omt * p1_3D.x + 2.0f * omt * t * u.x + t * t * p2_3D.x,
                    omt * omt * p1_3D.y + 2.0f * omt * t * u.y + t * t * p2_3D.y,
                    omt * omt * p1_3D.z + 2.0f * omt * t * u.z + t * t * p2_3D.z
                };
                arcPts[s] = project3DPoint(pt3D, rotX, rotY, rotZ, scale, centerX, centerY);
            }

            for (int s = 0; s < 3; ++s) {
                d.line({ arcPts[s].x, arcPts[s].y }, { arcPts[s + 1].x, arcPts[s + 1].y }, cornerOpt);
            }
        }
    } else if (resVal > 0.05f) {
        // When Cutoff = 0, draw subtle resonant corner dots at sharp vertices
        for (int i = 0; i < 8; ++i) {
            d.filledCircle({ projected[i].x, projected[i].y }, 2, cornerOpt);
        }
    }

    // Soft modern bottom toast overlay when any pot is turned
    if (potOverlayTimer > 0 && activePotIndex >= 0 && activePotIndex < 8) {
        PotInfo& p = pots[activePotIndex];

        int barX = 20;
        int barY = 134;
        int barW = 280;
        int barH = 28;

        // Soft dark gray background with subtle gray border
        d.filledRect({ barX, barY }, { barW, barH }, waveDrawOpt(waveMakeColor(36, 38, 44, 230)));
        d.rect({ barX, barY }, { barW, barH }, waveDrawOpt(waveMakeColor(75, 80, 92, 255), 1));

        // Soft cool white-gray pot name label
        char titleBuf[32];
        snprintf(titleBuf, sizeof(titleBuf), "%s", p.name);
        d.text({ barX + 10, barY + 7 }, titleBuf, 10, waveTextOpt(waveMakeColor(220, 225, 235, 255)));

        // Soft dark gray track and Blue-Gray fill progress bar
        int trackX = barX + 105;
        int trackY = barY + 9;
        int trackW = 120;
        int trackH = 10;
        int fillW = (trackW * p.percentage) / 100;

        d.filledRect({ trackX, trackY }, { trackW, trackH }, waveDrawOpt(waveMakeColor(55, 58, 68, 255)));
        if (fillW > 0) {
            d.filledRect({ trackX, trackY }, { fillW, trackH }, waveDrawOpt(waveMakeColor(80, 130, 170, 255)));
        }

        // Soft cool white percentage text
        char pctBuf[16];
        snprintf(pctBuf, sizeof(pctBuf), "%d%%", p.percentage);
        d.text({ trackX + trackW + 10, barY + 7 }, pctBuf, 10, waveTextOpt(waveMakeColor(220, 225, 235, 255)));
    }

    // Push frame to LCD
    pushDisplayESP32();

    vTaskDelay(1); // Yield to FreeRTOS scheduler
}
