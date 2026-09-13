#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "audioESP32.h"
#include "displayESP32.h"
#include "neoTrellisESP32.h"
#include "touchESP32.h"
#include "../../displayView.h"
#include "../../zicApp.h"

ZicApp* app = nullptr;
DisplayView displayView;
DisplayESP32 displayESP32;
TouchESP32 touchESP32;
NeoTrellisESP32 neoTrellisESP32;

Styles appStyles = {
    { 320, 170 },
    2,
    { nullptr, nullptr, nullptr },
    {
        { 15, 18, 24, 255 },
        { 255, 255, 255, 255 },
        { 120, 120, 130, 255 },
        { 0, 200, 255, 255 },
        { 10, 10, 12, 255 },
        { 28, 28, 32, 255 },
        { 35, 35, 40, 255 }
    }
};
Draw* drawer = nullptr;

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting zicModSamples on LilyGO T-Display-S3...");

    app = new ZicApp(44100.0f);
    drawer = new Draw(appStyles);

    displayESP32.init();
    touchESP32.init();
    neoTrellisESP32.init(app);
    initAudioESP32();

    // Launch Audio Worker Task on Core 0 (Realtime Priority)
    xTaskCreatePinnedToCore(
        audioTaskESP32,
        "AudioTask",
        8192,
        app,
        5, // Audio Priority
        NULL,
        0   // Core 0
    );
}

void loop()
{
    touchESP32.update(displayView, *app);
    neoTrellisESP32.update(*app);
    displayESP32.render(*drawer, displayView, *app);

    vTaskDelay(pdMS_TO_TICKS(16)); // ~60 FPS
}
