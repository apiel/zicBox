#include <Arduino.h>
#include <TFT_eSPI.h>

#include "../../audioEngine.h"
#include "../../displayView.h"
#include "../../zicApp.h"
#include "audioESP32.h"
#include "displayESP32.h"

// Pin Definitions
#define PIN_MIDI_RX       44
#define PIN_ENCODER_PUSH  43  // TX Pin reused as Push Encoder SW input!
#define PIN_ENCODER_A     2
#define PIN_ENCODER_B     3

// 8 Analog Potentiometer Pins
const uint8_t POT_PINS[8] = { 4, 5, 6, 7, 8, 9, 10, 11 };

// Global Objects
ZicApp app(44100.0f);
DisplayView displayView;
TFT_eSPI tft = TFT_eSPI();

// Encoder State Tracking
volatile int encoderPos = 0;
int lastEncoderPos = 0;
uint32_t lastPushPressMs = 0;
bool lastPushState = HIGH;

// Smooth ADC Pot Readings
float lastPotValues[8] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };

void IRAM_ATTR handleEncoderISR()
{
    static uint8_t old_AB = 0;
    old_AB <<= 2;
    old_AB |= ((digitalRead(PIN_ENCODER_A) << 1) | digitalRead(PIN_ENCODER_B)) & 0x03;
    static const int8_t enc_states[] = { 0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0 };
    encoderPos += enc_states[(old_AB & 0x0F)];
}

void setup()
{
    Serial.begin(115200);

    // Initialize Hardware Serial 1 for MIDI Clock input on RX pin (GPIO 44)
    Serial1.begin(31250, SERIAL_8N1, PIN_MIDI_RX, -1);

    // Initialize Push Encoder Click Switch on TX pin (GPIO 43)
    pinMode(PIN_ENCODER_PUSH, INPUT_PULLUP);

    // Initialize Encoder Phase A & B pins
    pinMode(PIN_ENCODER_A, INPUT_PULLUP);
    pinMode(PIN_ENCODER_B, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_A), handleEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_B), handleEncoderISR, CHANGE);

    // Initialize 8 Potentiometer ADC pins
    for (int i = 0; i < 8; ++i) {
        pinMode(POT_PINS[i], INPUT);
    }

    // Initialize Display & PDM Audio
    initDisplayESP32(tft);
    initAudioESP32();

    // Launch Audio Thread on Core 0 (Realtime Audio Loop)
    xTaskCreatePinnedToCore(
        audioTaskESP32,
        "AudioTask",
        4096,
        &app,
        3, // High Priority
        NULL,
        0  // Core 0
    );

    Serial.println("zicModGrain Firmware Initialized.");
}

void loop()
{
    // 1. Process Quadrature Encoder Rotation
    if (encoderPos != lastEncoderPos) {
        int diff = (encoderPos - lastEncoderPos) / 4;
        if (diff != 0) {
            app.handleEncoderTurn(diff);
            lastEncoderPos += diff * 4;
        }
    }

    // 2. Process Encoder Push Click on TX Pin (GPIO 43)
    bool pushState = digitalRead(PIN_ENCODER_PUSH);
    if (pushState == LOW && lastPushState == HIGH) {
        uint32_t now = millis();
        if (now - lastPushPressMs > 200) { // Debounce
            app.handleEncoderClick();
            lastPushPressMs = now;
        }
    }
    lastPushState = pushState;

    // 3. Sample 8 Analog Potentiometers
    for (int i = 0; i < 8; ++i) {
        int raw = analogRead(POT_PINS[i]); // 0..4095
        float normVal = (float)raw / 4095.0f;
        if (lastPotValues[i] < 0.0f || std::abs(normVal - lastPotValues[i]) > 0.015f) { // Noise threshold
            app.applyPotValue((PotIndex)i, normVal);
            lastPotValues[i] = normVal;
        }
    }

    // 4. Render LCD Screen
    renderDisplayESP32(tft, displayView, app);

    vTaskDelay(pdMS_TO_TICKS(16)); // ~60 FPS UI refresh
}
