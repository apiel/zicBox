#include "CppWrapper.h"
#include "eurorack/stm32/Core/Inc/Core.h"
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

// Display
ST7735 display(&hspi4, 160, 80, LCD_CS_Pin, LCD_DC_Pin, DISPLAY_BL_Pin);

// Encoder
Encoder encoder(&htim4);

Core core(display);

// Button callback for interrupt
extern "C" void Encoder_ButtonCallback(uint16_t GPIO_Pin)
{
    core.encBtn();
}

extern "C" void Display_TimerCallback(void)
{
    if (core.render()) display.render();
}

void Display_Init()
{
    display.init();

    Display_TimerCallback();

    HAL_TIM_Base_Start_IT(&htim7);
}

void Encoder_Init(void)
{
    encoder.init();
    encoder.setRotateCallback([](int32_t value, Encoder::Direction dir) {
        core.onEncoder(encoder.getClicks());
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
        // Scale to 12-bit range
        audioBuffer[start_index + i] = static_cast<int32_t>(core.sample() * 4095.0f);
    }

    // CRITICAL: Clean cache so DMA reads new data
    SCB_CleanDCache_by_Addr((uint32_t*)&audioBuffer[start_index],
        size * sizeof(uint16_t));
}

extern "C" void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
    Fill_Buffer(0, BUFFER_SIZE / 2);
}

extern "C" void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
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