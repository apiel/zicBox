#include "CppWrapper.h"
#include "kick.hpp"
#include "main.h"
#include "stm32/Encoder.hpp"
#include "stm32/ST7735.hpp"

extern "C" DAC_HandleTypeDef hdac1;
extern "C" SPI_HandleTypeDef hspi4;
extern "C" TIM_HandleTypeDef htim4;
extern "C" TIM_HandleTypeDef htim6;
extern "C" TIM_HandleTypeDef htim7;
extern "C" DMA_HandleTypeDef hdma_dac1_ch1;

#define BUFFER_SIZE 256

// Use RAM_D2 for better DMA performance
uint16_t audioBuffer[BUFFER_SIZE] __attribute__((section(".RAM_D2"))) __attribute__((aligned(32)));

// BPM Configuration
#define BPM 140.0f
#define SAMPLE_RATE 44104.0f

const uint32_t SAMPLES_PER_BEAT = (uint32_t)(SAMPLE_RATE * 60.0f / BPM);

// Display
ST7735 display(&hspi4, 80, 160, LCD_CS_Pin, LCD_DC_Pin, DISPLAY_BL_Pin);

// Encoder
Encoder encoder(&htim4);

// Kick state
volatile uint32_t sampleCounter = 0;
volatile bool kickActive = false;
volatile uint32_t kickSampleCounter = 0; // Count samples within a kick

// Diagnostics
volatile uint32_t half_complete_count = 0;
volatile uint32_t full_complete_count = 0;
volatile uint32_t kicks_triggered = 0;

int valueToEdit = 0;
// Button callback for interrupt
extern "C" void Encoder_ButtonCallback(uint16_t GPIO_Pin)
{
    valueToEdit++;
    if (valueToEdit > 6) valueToEdit = 0;
}

int x = 80;
int16_t lastY = -1; // Add as global
extern "C" void Display_TimerCallback(void)
{
    x = valueToEdit;
    if (x == lastY) return; // Skip if no change

    // Erase old rect
    display.filledRect({ 10, lastY }, { 20, 20 }, { { 0, 0, 0 } });
    // Draw new rect
    display.rect({ 10, x }, { 20, 20 }, { { 0, 255, 0 } });
    display.filledRect({ 15, x + 5 }, { 10, 10 }, { { 255, 255, 255 } });

    // display.filledRect({ 10, 10 }, { 80, 20 }, { { 0, 0, 0 } });
    // display.text({ 10, 10 }, "X: " + std::to_string(x), 12, { { 255, 255, 255 } });

    display.filledRect({ 10, 10 }, { 80, 20 }, { { 0, 0, 0 } });
    if (valueToEdit == 0) {
        display.text({ 10, 10 }, "Base Freq: ", 12, { { 255, 255, 255 } });
        // display.text({ 22, 10 }, std::to_string(kick.baseFrequency) + " Hz", 12, { { 255, 255, 255 } });
    } else if (valueToEdit == 1) {
        display.text({ 10, 10 }, "Sweep Depth: ", 12, { { 255, 255, 255 } });
        // display.text({ 22, 10 }, std::to_string(kick.sweepDepth) + " Hz", 12, { { 255, 255, 255 } });
    } else if (valueToEdit == 2) {
        display.text({ 10, 10 }, "Sweep Decay: ", 12, { { 255, 255, 255 } });
        // display.text({ 22, 10 }, std::to_string(kick.sweepDecay) + " Hz", 12, { { 255, 255, 255 } });
    } else if (valueToEdit == 3) {
        display.text({ 10, 10 }, "Amp Decay: ", 12, { { 255, 255, 255 } });
        // display.text({ 22, 10 }, std::to_string(kick.ampDecay) + " Hz", 12, { { 255, 255, 255 } });
    } else if (valueToEdit == 4) {
        display.text({ 10, 10 }, "Compression: ", 12, { { 255, 255, 255 } });
        // display.text({ 22, 10 }, std::to_string(kick.compressionAmount) + " Hz", 12, { { 255, 255, 255 } });
    } else if (valueToEdit == 5) {
        display.text({ 10, 10 }, "Drive: ", 12, { { 255, 255, 255 } });
        // display.text({ 22, 10 }, std::to_string(kick.driveAmount) + " Hz", 12, { { 255, 255, 255 } });
    } else if (valueToEdit == 6) {
        display.text({ 10, 10 }, "Click: ", 12, { { 255, 255, 255 } });
        // display.text({ 22, 10 }, std::to_string(kick.clickAmount) + " Hz", 12, { { 255, 255, 255 } });
    }

    display.render();

    lastY = x;
}

void Display_Init()
{
    display.init();

    Display_TimerCallback();

    HAL_TIM_Base_Start_IT(&htim7);
}

void Encoder_Init(void)
{
    // float baseFrequency = 55.0f;      // Starting frequency (Hz)
    // float sweepDepth = 800.0f;        // How much the pitch sweeps down
    // float sweepDecay = 0.9994f;       // How fast pitch envelope decays (0.9990-0.9998)
    // float ampDecay = 0.9992f;         // How fast amplitude decays (0.9985-0.9995)
    // float compressionAmount = 0.5f;   // Compression (0.0-1.0)
    // float driveAmount = 0.6f;         // Distortion/saturation (0.0-2.0)
    // float clickAmount = 0.3f;         // Click/attack transient (0.0-1.0)

    encoder.init();
    encoder.setRotateCallback([](int32_t value, Encoder::Direction dir) {
        // x += encoder.getClicks();
        if (valueToEdit == 0) kick.baseFrequency += encoder.getClicks();
        if (valueToEdit == 1) kick.sweepDepth += encoder.getClicks();
        if (valueToEdit == 2) kick.sweepDecay += encoder.getClicks() * 0.0001f;
        if (valueToEdit == 3) kick.ampDecay += encoder.getClicks() * 0.0001f;
        if (valueToEdit == 4) kick.compressionAmount += encoder.getClicks() * 0.01f;
        if (valueToEdit == 5) kick.driveAmount += encoder.getClicks() * 0.01f;
        if (valueToEdit == 6) kick.clickAmount += encoder.getClicks() * 0.01f;
    });
}

void Cpp_Init(void)
{
    // LED Test
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
    HAL_Delay(500);

    // Initialize display FIRST (before audio)
    // If display hangs, we'll know before audio starts
    Display_Init();

    Encoder_Init();

    // Initialize buffer with silence
    for (int i = 0; i < BUFFER_SIZE; i++) {
        audioBuffer[i] = 2048;
    }

    // Clean D-Cache
    SCB_CleanDCache_by_Addr((uint32_t*)audioBuffer, BUFFER_SIZE * sizeof(uint16_t));

    // Start Timer
    if (HAL_TIM_Base_Start(&htim6) != HAL_OK) {
        for (int i = 0; i < 6; i++) {
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
            HAL_Delay(200);
        }
        while (1)
            ;
    }

    // Start DAC DMA
    HAL_StatusTypeDef status = HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1,
        (uint32_t*)audioBuffer,
        BUFFER_SIZE,
        DAC_ALIGN_12B_R);

    if (status != HAL_OK) {
        while (1) {
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
            HAL_Delay(50);
        }
    }

    // Trigger first kick
    kick.trigger();
    kickActive = true;
    sampleCounter = 0;
    kickSampleCounter = 0;

    // SUCCESS
    HAL_Delay(200);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
}

void Cpp_Loop(void)
{
    encoder.update();
}

void Fill_Buffer(int start_index, int size)
{
    // REAL KICK MODE
    for (int i = 0; i < size; i++) {
        // Check if it's time for a new kick
        if (sampleCounter >= SAMPLES_PER_BEAT) {
            kick.trigger();
            kickActive = true;
            kickSampleCounter = 0;
            sampleCounter = 0;
            kicks_triggered++;

            // LED ON at kick start
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET);
        }

        // Generate audio sample
        if (kickActive) {
            // Process kick engine
            uint16_t sample = kick.process(1.0f);
            audioBuffer[start_index + i] = sample;

            kickSampleCounter++;

            // Check if kick finished (envelope died)
            // Also add safety timeout: max 8000 samples (about 180ms)
            if (kick.pitchEnv < 0.0001f || kickSampleCounter > 8000) {
                kickActive = false;
                HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);
            }
        } else {
            // Silence between kicks
            audioBuffer[start_index + i] = 2048;
        }

        sampleCounter++;
    }

    // CRITICAL: Clean cache so DMA reads new data
    SCB_CleanDCache_by_Addr((uint32_t*)&audioBuffer[start_index],
        size * sizeof(uint16_t));
}

extern "C" void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    half_complete_count++;
    Fill_Buffer(0, BUFFER_SIZE / 2);
}

extern "C" void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    full_complete_count++;
    Fill_Buffer(BUFFER_SIZE / 2, BUFFER_SIZE / 2);
}

extern "C" void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef* hdac)
{
    for (int i = 0; i < 20; i++) {
        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
        HAL_Delay(50);
    }
    while (1)
        ;
}