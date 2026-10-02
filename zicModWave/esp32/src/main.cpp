#include <Arduino.h>
#include <cmath>
#include <algorithm>
#include "../../displayView.h"
#include "../../zicApp.h"
#include "displayESP32.h"
#include "audioESP32.h"

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

// 8 Analog Potentiometers mapped to 3 rows
PotInfo pots[8] = {
    { "Wave (A11)",     11, 0.0f, -1 }, // Row 1 Left
    { "ModFX (A10)",    10, 0.0f, -1 }, // Row 1 Right
    { "Cutoff (A7)",     7, 0.0f, -1 }, // Row 2 Left
    { "Reso (A8)",       8, 0.0f, -1 }, // Row 2 Mid
    { "RingMod (A9)",    9, 0.0f, -1 }, // Row 2 Right
    { "ModDepth (A4)",   4, 0.0f, -1 }, // Row 3 Left
    { "ModSpeed (A5)",   5, 0.0f, -1 }, // Row 3 Mid
    { "DlySend (A6)",    6, 0.0f, -1 }  // Row 3 Right
};

ZicApp app;

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

// Background Task for Audio Processing
void audioFreeRTOSTask(void* parameter)
{
    audioTaskESP32(parameter);
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

    // Initialize Analog Potentiometer Pins & Read Physical Positions on Startup
    for (int i = 0; i < 8; ++i) {
        pinMode(pots[i].pin, INPUT);
        int sumRaw = 0;
        for (int s = 0; s < 32; ++s) {
            sumRaw += analogRead(pots[i].pin);
        }
        float raw = sumRaw / 32.0f;
        pots[i].filteredMv = raw;
        float normVal = std::clamp(raw / 4095.0f, 0.0f, 1.0f);
        pots[i].percentage = (int)(normVal * 100.0f);
        app.potValues[i] = normVal;
    }
    app.syncPotsToEngine();
    app.potOverlayTimer = 0; // Keep screen clean without toast popups on boot

    oldState = (digitalRead(ENCODER_PIN_A) << 1) | digitalRead(ENCODER_PIN_B);

    // Attach hardware interrupts on CHANGE for encoder pins
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), encoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_SW), buttonISR, FALLING);

    initDisplayESP32();
    initAudioESP32();

    // Start Audio Thread on Core 0 (keeping Core 1 100% dedicated to UI rendering)
    xTaskCreatePinnedToCore(
        audioFreeRTOSTask,
        "AudioTask",
        4096,
        &app,
        3, // Realtime Audio Priority
        NULL,
        0  // Core 0
    );
}

void processInputs()
{
    noInterrupts();
    int substeps = encoderSubSteps;
    encoderSubSteps %= 4;
    bool pressed = buttonPressed;
    buttonPressed = false;
    interrupts();

    int detents = substeps / 4;
    if (detents != 0) {
        app.handleEncoderTurn(detents);
    }

    if (pressed) {
        app.handleEncoderClick();
    }
}

void computeMorphedVertices(float t, float timeAnim, Point3D outVerts[8])
{
    Point3D cubeVerts[8] = {
        { -1.0f, -1.0f, -1.0f }, {  1.0f, -1.0f, -1.0f },
        {  1.0f, -1.0f,  1.0f }, { -1.0f, -1.0f,  1.0f },
        { -1.0f,  1.0f, -1.0f }, {  1.0f,  1.0f, -1.0f },
        {  1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f }
    };

    if (t < 0.33f) {
        float m = t / 0.33f;
        Point3D apex = { 0.0f, -1.3f, 0.0f };
        outVerts[0] = apex; outVerts[1] = apex; outVerts[2] = apex; outVerts[3] = apex;
        Point3D triBase[4] = { { -1.2f, 1.0f, -0.7f }, { 1.2f, 1.0f, -0.7f }, { 0.0f, 1.0f, 1.4f }, { 0.0f, 1.0f, 1.4f } };
        for (int i = 0; i < 4; ++i) {
            outVerts[4 + i].x = triBase[i].x * (1.0f - m) + cubeVerts[4 + i].x * m;
            outVerts[4 + i].y = triBase[i].y * (1.0f - m) + cubeVerts[4 + i].y * m;
            outVerts[4 + i].z = triBase[i].z * (1.0f - m) + cubeVerts[4 + i].z * m;
        }
    } else if (t < 0.66f) {
        float m = (t - 0.33f) / 0.33f;
        Point3D apex = { 0.0f, -1.3f, 0.0f };
        for (int i = 0; i < 4; ++i) {
            outVerts[i].x = apex.x * (1.0f - m) + cubeVerts[i].x * m;
            outVerts[i].y = apex.y * (1.0f - m) + cubeVerts[i].y * m;
            outVerts[i].z = apex.z * (1.0f - m) + cubeVerts[i].z * m;
        }
        for (int i = 4; i < 8; ++i) outVerts[i] = cubeVerts[i];
    } else {
        float m = (t - 0.66f) / 0.34f;
        for (int i = 0; i < 8; ++i) {
            float phase = i * 1.3f + timeAnim * 3.0f;
            outVerts[i].x = cubeVerts[i].x + sinf(phase * 1.7f) * 1.8f * m;
            outVerts[i].y = cubeVerts[i].y + cosf(phase * 2.3f) * 1.8f * m;
            outVerts[i].z = cubeVerts[i].z + sinf(phase * 3.1f) * 1.8f * m;
        }
    }
}

void loop()
{
    processInputs();

    // Read 8 Potentiometers with 16x Fast Raw ADC Oversampling & Adaptive Slew Rate Filter
    for (int i = 0; i < 8; ++i) {
        int sumRaw = 0;
        for (int s = 0; s < 16; ++s) {
            sumRaw += analogRead(pots[i].pin);
        }
        float raw = sumRaw / 16.0f;

        if (pots[i].filteredMv == 0.0f) {
            pots[i].filteredMv = raw;
        } else {
            float diff = std::abs(raw - pots[i].filteredMv);
            float alpha = 0.05f; // Strong noise suppression when idle
            if (diff > 120.0f) {
                alpha = 0.85f; // Instant response (0ms lag) when turned fast
            } else if (diff > 35.0f) {
                alpha = 0.35f; // Moderate smoothing during normal motion
            }
            pots[i].filteredMv += (raw - pots[i].filteredMv) * alpha;
        }
        float normVal = std::clamp(pots[i].filteredMv / 4095.0f, 0.0f, 1.0f);
        int newPct = (int)(normVal * 100.0f);

        if (pots[i].percentage == -1) {
            pots[i].percentage = newPct;
        } else if (abs(newPct - pots[i].percentage) >= 2) {
            pots[i].percentage = newPct;
            app.applyPotValue((PotIndex)i, normVal);
        }
    }

    if (app.potOverlayTimer > 0) app.potOverlayTimer--;

    // Keep menu overlay active indefinitely while editing parameter
    if (app.isEditing) {
        app.menuOverlayTimer = 90;
    } else if (app.menuOverlayTimer > 0) {
        app.menuOverlayTimer--;
    }

    // Parameter assignments from 8 pots
    float waveVal = app.potValues[POT_WAVE];
    float cutoffVal = app.potValues[POT_CUTOFF];
    float resVal = app.potValues[POT_RESONANCE];
    float pitchVal = app.engine.pitch.value; // MIDI note 24..72
    float delaySendVal = app.potValues[POT_DLY_SEND];

    // Pot A10 smooth crossfade: FM for synth waves (<=50%), Bitcrush for noise waves (>=80%)
    float fxVal = app.potValues[POT_CRUSH_FM]; // 0.0 .. 1.0
    float noiseFade = std::clamp((waveVal - 0.50f) / 0.30f, 0.0f, 1.0f);
    float fmVal = fxVal * (1.0f - noiseFade);
    float crushVal = fxVal * noiseFade;

    float pitchNorm = std::clamp((pitchVal - 24.0f) / 48.0f, 0.0f, 1.0f);
    float speedMult = 0.1f + pitchNorm * 4.9f;

    Draw& d = getDrawer();
    d.clear();

    // --- 3D SYNTH VISUALIZER VIEW ---

        // Minimal 16-Step Top Sequencer Bar
        int currentStep = app.engine.stepIndex % 16;
        int seqStartX = 48;
        int seqTopY = 10;
        int bw = 8, bh = 3;

        int delayStepTap1 = (delaySendVal > 0.02f) ? ((currentStep - 4 + 16) % 16) : -1;
        int delayStepTap2 = (delaySendVal > 0.45f) ? ((currentStep - 8 + 16) % 16) : -1;

        for (int i = 0; i < 16; ++i) {
            int bx = seqStartX + i * 14;
            int by = seqTopY;

            if (i == currentStep) {
                d.filledRect({ bx, by - 1 }, { bw, bh + 2 }, waveDrawOpt(waveMakeColor(160, 195, 220, 255)));
            } else if (i == delayStepTap1) {
                d.filledRect({ bx, by - 1 }, { bw, bh + 2 }, waveDrawOpt(waveMakeColor(100, 145, 180, 180)));
            } else if (i == delayStepTap2) {
                d.filledRect({ bx, by - 1 }, { bw, bh + 2 }, waveDrawOpt(waveMakeColor(70, 110, 145, 120)));
            } else if (app.engine.rhythmMask[i]) {
                d.filledRect({ bx, by }, { bw, bh }, waveDrawOpt(waveMakeColor(60, 95, 125, 255)));
            } else {
                d.filledRect({ bx, by }, { bw, bh }, waveDrawOpt(waveMakeColor(32, 38, 48, 255)));
            }
        }

        if (isRotating) {
            float dt = 0.016f * speedMult;
            rotX += 0.02f * speedMult;
            rotY += 0.03f * speedMult;
            rotZ += 0.015f * speedMult;
            animTime += dt;
        }

        Point3D morphedVerts[8];
        computeMorphedVertices(waveVal, animTime, morphedVerts);

        Point2D projected[8];
        float scale = 38.0f;

        for (int i = 0; i < 8; ++i) {
            projected[i] = project3DPoint(morphedVerts[i], rotX, rotY, rotZ, scale, centerX, centerY);
        }

        DrawOptions lineOpt = waveDrawOpt(waveMakeColor(70, 120, 160, 255), 2);
        uint8_t cR = (uint8_t)(70 + resVal * 120);
        uint8_t cG = (uint8_t)(120 + resVal * 115);
        uint8_t cB = (uint8_t)(160 + resVal * 95);
        DrawOptions cornerOpt = waveDrawOpt(waveMakeColor(cR, cG, cB, 255), 2);

        float cornerRadius = cutoffVal * 0.35f;

        // 3D Ghost Echoes (Dly Send)
        if (delaySendVal > 0.02f) {
            int maxGhosts = 1 + (int)(delaySendVal * 2.99f);
            for (int e = maxGhosts; e >= 1; --e) {
                float lag = e * (0.15f + delaySendVal * 0.25f);
                float gRotX = rotX - lag * 0.7f;
                float gRotY = rotY - lag * 1.0f;
                float gRotZ = rotZ - lag * 0.5f;
                float gAnimTime = animTime - lag * 0.15f;
                float gScale = scale * (1.0f - e * (0.05f + delaySendVal * 0.05f));

                float fade = powf(0.55f - delaySendVal * 0.10f, (float)e) * (0.35f + delaySendVal * 0.65f);
                DrawOptions gLineOpt = waveDrawOpt(waveMakeColor((uint8_t)(45 * fade), (uint8_t)(80 * fade), (uint8_t)(115 * fade), (uint8_t)(200 * fade)), 1);

                Point3D gMorphedVerts[8];
                computeMorphedVertices(waveVal, gAnimTime, gMorphedVerts);

                Point2D gProjected[8];
                for (int i = 0; i < 8; ++i) {
                    gProjected[i] = project3DPoint(gMorphedVerts[i], gRotX, gRotY, gRotZ, gScale, centerX, centerY);
                }

                for (int i = 0; i < 12; ++i) {
                    Point3D gu = gMorphedVerts[SHAPE_EDGES[i][0]];
                    Point3D gv = gMorphedVerts[SHAPE_EDGES[i][1]];
                    Point3D gDiff = { gv.x - gu.x, gv.y - gu.y, gv.z - gu.z };
                    float gLen = sqrtf(gDiff.x * gDiff.x + gDiff.y * gDiff.y + gDiff.z * gDiff.z);
                    if (gLen < 0.01f) continue;

                    if (cornerRadius > 0.005f) {
                        float effR = std::min(cornerRadius, gLen * 0.40f);
                        Point3D gDir = { gDiff.x / gLen, gDiff.y / gLen, gDiff.z / gLen };
                        Point3D gpA3D = { gu.x + gDir.x * effR, gu.y + gDir.y * effR, gu.z + gDir.z * effR };
                        Point3D gpB3D = { gv.x - gDir.x * effR, gv.y - gDir.y * effR, gv.z - gDir.z * effR };
                        Point2D gpA = project3DPoint(gpA3D, gRotX, gRotY, gRotZ, gScale, centerX, centerY);
                        Point2D gpB = project3DPoint(gpB3D, gRotX, gRotY, gRotZ, gScale, centerX, centerY);
                        d.line({ gpA.x, gpA.y }, { gpB.x, gpB.y }, gLineOpt);
                    } else {
                        d.line({ gProjected[SHAPE_EDGES[i][0]].x, gProjected[SHAPE_EDGES[i][0]].y },
                               { gProjected[SHAPE_EDGES[i][1]].x, gProjected[SHAPE_EDGES[i][1]].y }, gLineOpt);
                    }
                }
            }
        }

        // Draw 3D Edges
        for (int i = 0; i < 12; ++i) {
            Point3D u = morphedVerts[SHAPE_EDGES[i][0]];
            Point3D v = morphedVerts[SHAPE_EDGES[i][1]];
            Point3D diff = { v.x - u.x, v.y - u.y, v.z - u.z };
            float len = sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
            if (len < 0.01f) continue;

            if (cornerRadius > 0.005f) {
                float effR = std::min(cornerRadius, len * 0.40f);
                Point3D dir = { diff.x / len, diff.y / len, diff.z / len };
                Point3D pA3D = { u.x + dir.x * effR, u.y + dir.y * effR, u.z + dir.z * effR };
                Point3D pB3D = { v.x - dir.x * effR, v.y - dir.y * effR, v.z - dir.z * effR };

                if (fmVal > 0.02f) {
                    const int FMSegs = 6;
                    Point2D fmPts[FMSegs + 1];
                    Point3D eDir = { pB3D.x - pA3D.x, pB3D.y - pA3D.y, pB3D.z - pA3D.z };
                    float eLen = sqrtf(eDir.x * eDir.x + eDir.y * eDir.y + eDir.z * eDir.z);

                    if (eLen > 0.001f) {
                        Point3D uDir = { eDir.x / eLen, eDir.y / eLen, eDir.z / eLen };
                        Point3D nVec = (fabsf(uDir.y) < 0.9f) ? Point3D{ -uDir.z, 0.0f, uDir.x } : Point3D{ 1.0f, 0.0f, 0.0f };
                        for (int s = 0; s <= FMSegs; ++s) {
                            float t = (float)s / (float)FMSegs;
                            Point3D bPt = { pA3D.x + t * eDir.x, pA3D.y + t * eDir.y, pA3D.z + t * eDir.z };
                            float ripple = sinf(t * 18.0f + animTime * 16.0f + i * 0.7f) * (fmVal * 0.12f);
                            Point3D rPt = { bPt.x + nVec.x * ripple, bPt.y + nVec.y * ripple, bPt.z + nVec.z * ripple };
                            fmPts[s] = project3DPoint(rPt, rotX, rotY, rotZ, scale, centerX, centerY);
                        }
                        for (int s = 0; s < FMSegs; ++s) {
                            d.line({ fmPts[s].x, fmPts[s].y }, { fmPts[s + 1].x, fmPts[s + 1].y }, lineOpt);
                        }
                    }
                } else {
                    Point2D pA = project3DPoint(pA3D, rotX, rotY, rotZ, scale, centerX, centerY);
                    Point2D pB = project3DPoint(pB3D, rotX, rotY, rotZ, scale, centerX, centerY);
                    d.line({ pA.x, pA.y }, { pB.x, pB.y }, lineOpt);
                }
            } else {
                Point2D p1 = projected[SHAPE_EDGES[i][0]];
                Point2D p2 = projected[SHAPE_EDGES[i][1]];
                d.line({ p1.x, p1.y }, { p2.x, p2.y }, lineOpt);
            }
        }

        // Draw 3D Rounded Corner Arcs
        if (cornerRadius > 0.005f) {
            for (int c = 0; c < 24; ++c) {
                Point3D u = morphedVerts[VERT_CORNERS[c].u];
                Point3D w1 = morphedVerts[VERT_CORNERS[c].w1];
                Point3D w2 = morphedVerts[VERT_CORNERS[c].w2];
                Point3D d1 = { w1.x - u.x, w1.y - u.y, w1.z - u.z };
                Point3D d2 = { w2.x - u.x, w2.y - u.y, w2.z - u.z };
                float len1 = sqrtf(d1.x * d1.x + d1.y * d1.y + d1.z * d1.z);
                float len2 = sqrtf(d2.x * d2.x + d2.y * d2.y + d2.z * d2.z);
                if (len1 < 0.01f && len2 < 0.01f) continue;

                float r1 = (len1 > 0.01f) ? std::min(cornerRadius, len1 * 0.40f) : 0.0f;
                float r2 = (len2 > 0.01f) ? std::min(cornerRadius, len2 * 0.40f) : 0.0f;
                Point3D p1_3D = (len1 > 0.01f) ? Point3D{ u.x + (d1.x / len1) * r1, u.y + (d1.y / len1) * r1, u.z + (d1.z / len1) * r1 } : u;
                Point3D p2_3D = (len2 > 0.01f) ? Point3D{ u.x + (d2.x / len2) * r2, u.y + (d2.y / len2) * r2, u.z + (d2.z / len2) * r2 } : u;

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
        }

        // Flying single pixel dust cloud (Bitcrush)
        if (crushVal > 0.02f) {
            int numDots = (int)(crushVal * 28.0f);
            if (numDots < 4) numDots = 4;
            uint8_t alpha = (uint8_t)(60 + crushVal * 100);

            for (int k = 0; k < numDots; ++k) {
                float phase = k * 1.17f + animTime * (1.8f + (k % 5) * 0.25f);
                float rad = 1.1f + sinf(animTime * 1.5f + k * 0.7f) * 0.30f + (k % 4) * 0.20f;
                Point3D dot3D = { cosf(phase) * rad, sinf(phase * 1.3f + k * 0.8f) * rad * 0.8f, sinf(phase * 0.9f + k * 1.4f) * rad };
                Point2D dot2D = project3DPoint(dot3D, rotX, rotY, rotZ, scale, centerX, centerY);
                d.filledRect({ dot2D.x, dot2D.y }, { 1, 1 }, waveDrawOpt(waveMakeColor(85, 135, 175, alpha)));
            }
        }

        // Bottom Toast HUD overlay when any pot is turned
        if (app.potOverlayTimer > 0 && app.lastMovedPotIndex >= 0 && app.lastMovedPotIndex < 8) {
            PotInfo& p = pots[app.lastMovedPotIndex];

            int barX = 20, barY = 134, barW = 280, barH = 28;
            d.filledRect({ barX, barY }, { barW, barH }, waveDrawOpt(waveMakeColor(36, 38, 44, 230)));
            d.rect({ barX, barY }, { barW, barH }, waveDrawOpt(waveMakeColor(75, 80, 92, 255), 1));

            int trackX = barX + 110, trackY = barY + 9, trackW = 110, trackH = 10;
            d.filledRect({ trackX, trackY }, { trackW, trackH }, waveDrawOpt(waveMakeColor(55, 58, 68, 255)));

            char titleBuf[32], pctBuf[16];

            if (app.lastMovedPotIndex == 1) {
                float noiseFade = std::clamp((app.potValues[POT_WAVE] - 0.50f) / 0.30f, 0.0f, 1.0f);
                const char* fxLabel = (noiseFade < 0.2f) ? "FM Depth" : ((noiseFade > 0.8f) ? "Bitcrush" : "FM + Crush");
                snprintf(titleBuf, sizeof(titleBuf), "%s (A10)", fxLabel);
                snprintf(pctBuf, sizeof(pctBuf), "%d%%", p.percentage);
                int fillW = (trackW * p.percentage) / 100;
                if (fillW > 0) d.filledRect({ trackX, trackY }, { fillW, trackH }, waveDrawOpt(waveMakeColor(80, 130, 170, 255)));
            } else {
                snprintf(titleBuf, sizeof(titleBuf), "%s", p.name);
                snprintf(pctBuf, sizeof(pctBuf), "%d%%", p.percentage);
                int fillW = (trackW * p.percentage) / 100;
                if (fillW > 0) d.filledRect({ trackX, trackY }, { fillW, trackH }, waveDrawOpt(waveMakeColor(80, 130, 170, 255)));
            }

            d.text({ barX + 10, barY + 6 }, titleBuf, 12, waveTextOpt(waveMakeColor(220, 225, 235, 255)));
            d.text({ trackX + trackW + 10, barY + 6 }, pctBuf, 12, waveTextOpt(waveMakeColor(220, 225, 235, 255)));
        }

        // Encoder Menu Overlay (Top Bar / Pinned when editing)
        if (app.menuOverlayTimer > 0 || app.isEditing) {
            int mX = 35, mY = 24, mW = 250, mH = 26;
            d.filledRect({ mX, mY }, { mW, mH }, waveDrawOpt(waveMakeColor(28, 32, 40, 240)));

            if (app.isEditing) {
                d.rect({ mX, mY }, { mW, mH }, waveDrawOpt(waveMakeColor(80, 180, 240, 255), 2)); // Glowing active edit border
            } else {
                d.rect({ mX, mY }, { mW, mH }, waveDrawOpt(waveMakeColor(65, 75, 90, 255), 1));
            }

            char mName[32], mVal[32];
            snprintf(mName, sizeof(mName), "%s", app.getMenuItemName(app.currentMenuItem));
            app.getMenuItemFormattedValue(app.currentMenuItem, mVal, sizeof(mVal));

            d.text({ mX + 10, mY + 5 }, mName, 12, waveTextOpt(waveMakeColor(170, 200, 230, 255)));
            d.text({ mX + 130, mY + 5 }, mVal, 12, waveTextOpt(app.isEditing ? waveMakeColor(100, 220, 255, 255) : waveMakeColor(230, 235, 245, 255)));
        }

    // Push frame to LCD
    pushDisplayESP32();

    taskYIELD(); // Yield without 10ms delay penalty
}
