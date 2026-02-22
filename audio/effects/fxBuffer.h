#pragma once

// #include "stm32/platform.h"

// Must be a power of 2
#ifndef FX_BUFFER_SIZE
// #define FX_BUFFER_SIZE 262144
#define FX_BUFFER_SIZE 131072
#endif
constexpr int FX_BUFFER_MASK = FX_BUFFER_SIZE - 1;

// #ifndef AUDIO_STORAGE
// #define AUDIO_STORAGE
// #endif
// #define FX_BUFFER AUDIO_STORAGE float buffer[FX_BUFFER_SIZE] = { 0.0f };

#define FX_BUFFER float buffer[FX_BUFFER_SIZE] = { 0.0f };
