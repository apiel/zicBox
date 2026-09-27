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

// 8 Analog Potentiometers (A11 Speed, A10 Morph, A7, A8, A9, A4, A5, A6)
PotInfo pots[8] = {
    { "A11 (Speed)", 11, 0.0f, -1 },
    { "A10 (Morph)", 10, 0.0f, -1 },
    { "A7",           7, 0.0f, -1 },
    { "A8",           8, 0.0f, -1 },
    { "A9",           9, 0.0f, -1 },
    { "A4",           4, 0.0f, -1 },
    { "A5",           5, 0.0f, -1 },
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

float rotX = 0.0f;
float rotY = 0.0f;
float rotZ = 0.0f;
float animTime = 0.0f;

int centerX = 160;
int centerY = 86;
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

    Draw& d = getDrawer();
    d.clear();

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
    float scale = 48.0f;

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

    // Soft Blue-Gray wireframe line color: RGB(80, 130, 170) to RGB(100, 160, 200)
    uint8_t lineR = (uint8_t)(80 + morphVal * 30);
    uint8_t lineG = (uint8_t)(130 + morphVal * 30);
    uint8_t lineB = (uint8_t)(170 + morphVal * 30);
    DrawOptions lineOpt = waveDrawOpt(waveMakeColor(lineR, lineG, lineB, 255), 2);

    // Draw 12 Edges connecting morphed vertices
    for (int i = 0; i < 12; ++i) {
        Point2D p1 = projected[SHAPE_EDGES[i][0]];
        Point2D p2 = projected[SHAPE_EDGES[i][1]];
        d.line({ p1.x, p1.y }, { p2.x, p2.y }, lineOpt);
    }

    // Swirling particle cloud for Stage 3 (morphVal > 0.66)
    if (morphVal > 0.66f) {
        float m3 = (morphVal - 0.66f) / 0.34f;
        int numDots = (int)(m3 * 24);

        for (int j = 0; j < numDots; ++j) {
            float speed = 1.0f + (j % 5) * 0.4f;
            float rad = (0.4f + (j % 7) * 0.25f) * (1.0f + m3 * 1.5f);
            float angle = animTime * speed + j * 0.523f;

            float px = cosf(angle) * rad;
            float py = sinf(angle * 1.4f + j) * rad * 0.7f;
            float pz = sinf(angle) * rad;

            // Rotate particle 3D position
            float py1 = py * cosf(rotX) - pz * sinf(rotX);
            float pz1 = py * sinf(rotX) + pz * cosf(rotX);
            float px2 = px * cosf(rotY) + pz1 * sinf(rotY);
            float pz2 = -px * sinf(rotY) + pz1 * cosf(rotY);
            float px3 = px2 * cosf(rotZ) - py1 * sinf(rotZ);
            float py3 = px2 * sinf(rotZ) + py1 * cosf(rotZ);

            float psz = pz2 + 3.5f;
            int screenPx = centerX + (int)(px3 * scale * 3.0f / psz);
            int screenPy = centerY + (int)(py3 * scale * 3.0f / psz);

            // Draw soft blue-gray particle dots
            d.filledCircle({ screenPx, screenPy }, 2, waveDrawOpt(waveMakeColor(90, 150, 190, 255)));
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
