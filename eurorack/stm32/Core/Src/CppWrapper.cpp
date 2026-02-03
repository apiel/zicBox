#include "CppWrapper.h"
#include "RotaryEncoder.hpp"
#include "ST7735.hpp"
#include "kick.hpp"
#include "main.h"

extern "C" DAC_HandleTypeDef hdac1;
extern "C" TIM_HandleTypeDef htim1;
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
ST7735 display(&hspi4, &htim1);

// Encoder
RotaryEncoder encoder(&htim4);

// Kick state
volatile uint32_t sampleCounter = 0;
volatile bool kickActive = false;
volatile uint32_t kickSampleCounter = 0; // Count samples within a kick

// Diagnostics
volatile uint32_t half_complete_count = 0;
volatile uint32_t full_complete_count = 0;
volatile uint32_t kicks_triggered = 0;

// Button callback for interrupt
extern "C" void Encoder_ButtonCallback(uint16_t GPIO_Pin)
{
    encoder.handleButtonInterrupt();
}

// TEST MODE: Set to 1 for simple square wave, 0 for kick
// #define TEST_MODE 1

int x = 80;
int16_t lastY = -1; // Add as global
extern "C" void Display_TimerCallback(void)
{
    if (x == lastY) return; // Skip if no change

    // Erase old circle
    if (lastY >= 0) {
        display.fillCircle(40, lastY, 16, ST7735::BLACK);
    }

    // Draw new circle
    display.fillCircle(40, x, 10, x % 2 == 0 ? ST7735::GREEN : ST7735::RED);
    display.drawCircle(40, x, 15, ST7735::WHITE);

    lastY = x;
}

void Display_Init()
{
    display.init();

    display.fillScreen(ST7735::BLACK);
    Display_TimerCallback();

    HAL_TIM_Base_Start_IT(&htim7);

    // Display is on when gpio is low (default value set in .ioc)
    // HAL_GPIO_WritePin(GPIOE, DISPLAY_BL_Pin, GPIO_PIN_RESET); // display on
    // Display is off when gpio is high
    // HAL_GPIO_WritePin(GPIOE, DISPLAY_BL_Pin, GPIO_PIN_SET); // display off
}

void Encoder_Init(void)
{
    // Initialize encoder
    encoder.init();

    // Set up callbacks
    encoder.setRotateCallback([](int32_t value, RotaryEncoder::Direction dir) {
        // Called on rotation
        if (dir == RotaryEncoder::Direction::CW) {
            // Clockwise
            x++;
            // Render_Display();
        } else if (dir == RotaryEncoder::Direction::CCW) {
            // Counter-clockwise
            x--;
            // Render_Display();
        }
    });

    encoder.setButtonCallback([]() {
        // Called on button press
        HAL_GPIO_TogglePin(GPIOE, DISPLAY_BL_Pin);
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
#if TEST_MODE
    // SIMPLE TEST: Just play a square wave at 140 BPM
    static uint32_t test_phase = 0;

    for (int i = 0; i < size; i++) {
        // Toggle LED and trigger "kick" every beat
        if (sampleCounter >= SAMPLES_PER_BEAT) {
            HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3);
            kicks_triggered++;
            sampleCounter = 0;
        }

        // Simple 440Hz square wave
        test_phase++;
        audioBuffer[start_index + i] = ((test_phase / 50) % 2) ? 3000 : 1000;

        sampleCounter++;
    }
#else
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
#endif

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