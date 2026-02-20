#pragma once

#if defined(STM32H723xx) || defined(STM32H7xx) || defined(ARM_MATH_CM7)
#define IS_STM32
#define AUDIO_STORAGE __attribute__((section(".RAM_D1"))) alignas(32)
#endif