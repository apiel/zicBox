/** Description:
This header file functions as a comprehensive toolkit for implementing real-time digital audio effects, primarily focusing on creating realistic Reverb and customizable Delay.

The system relies on temporary memory loops, known as buffers, which continuously record the incoming audio signal. For instance, the Reverb memory loop holds about one second of sound, while the Delay loop stores up to three seconds.

The core of the system is the process of generating echoes. This is achieved by reading the stored sound back from the memory loop at various points in time and mixing it with the current incoming sound. Some of the effect is also fed back into the memory loop to sustain the decay.

To create complex sounds, the code uses "voices" or "taps"—multiple distinct read points from the memory loop, each defined by a specific delay time and volume.

*   **Reverb:** Uses multiple taps occurring close together to simulate the dense, quick reflections found in a large hall.
*   **Delay:** Uses taps spaced farther apart to create distinct, rhythmic echoes.

The file provides several specialized functions, each pre-configured with different voice settings to produce unique sonic textures, from subtle echoes to deep, complex atmospheres.

A specialized effect, **Shimmer Reverb**, is also included. This effect takes the delayed echoes and slightly modulates their pitch, giving the resulting sound an ethereal, high-frequency, "angelic" quality beyond standard room simulation.

In summary, this toolkit offers a versatile and efficient way to digitally sculpt diverse audio effects by controlling how and when delayed copies of the original sound are reintroduced.

sha: 4ee19efbf871b8ad50cc31e9503ac92d09fd8f53a6687e7f411d4c5ffa28176f 
*/
#pragma once

#include "audio/utils/math.h"
#include "stm32/platform.h"
#include <cmath>
#include <cstdint>

#ifndef AUDIO_STORAGE
#define AUDIO_STORAGE
#endif

#ifdef IS_STM32
// 1. BUFFER SIZES (Using Power-of-Two for fast bitwise masking)
// Total internal RAM usage: ~288KB (Fits in H723 320KB AXI SRAM)
constexpr int REVERB_BUFFER_SIZE = 32768;
constexpr int REVERB_MASK = REVERB_BUFFER_SIZE - 1;

constexpr int DELAY_BUFFER_SIZE = 40960; // Set to fit AXI SRAM. Increase if using SDRAM.
constexpr int DELAY_MASK = DELAY_BUFFER_SIZE - 1;
#else
constexpr int REVERB_BUFFER_SIZE = 32768;
constexpr int REVERB_MASK = REVERB_BUFFER_SIZE - 1;

// 262144 is 2^18. Approx 5.4 seconds. Must be Power of Two!
constexpr int DELAY_BUFFER_SIZE = 262144;
constexpr int DELAY_MASK = DELAY_BUFFER_SIZE - 1;
#endif

#define REVERB_BUFFER AUDIO_STORAGE float buffer[REVERB_BUFFER_SIZE] = { 0.0f };
#define DELAY_BUFFER AUDIO_STORAGE float buffer[DELAY_BUFFER_SIZE] = { 0.0f };

struct BufferVoice {
    float delay;
    float gain;
};

// --- CORE UTILITY ---

float processDelayAndReverb(float signal, float amount, float* buffer, int& index, int bufferSize, int mask, int voiceCount, BufferVoice* voices)
{
    if (amount <= 0.0f) return signal;

    float reverbOut = 0.0f;

    // Store input
    buffer[index] = signal;

    for (uint8_t i = 0; i < voiceCount; i++) {
        int delaySamples = static_cast<int>(voices[i].delay * bufferSize);
        int readIndex = (index - delaySamples) & mask;
        reverbOut += buffer[readIndex] * voices[i].gain;
    }

    float globalFeedback = amount * 0.6f;
    buffer[index] = signal + (reverbOut * globalFeedback);

    index = (index + 1) & mask;

    float mix = amount * 0.8f;
    return signal + mix * (reverbOut - signal);
}

// --- REVERB FUNCTIONS ---

static constexpr int ReverbVoiceCount = 4;
static BufferVoice reverbVoices[ReverbVoiceCount] = {
    { 0.2f, 0.6f }, { 0.4f, 0.4f }, { 0.6f, 0.3f }, { 0.8f, 0.2f }
};

static BufferVoice reverb3Voices[4] = {
    { 0.1f, 0.4f }, { 0.2f, 0.3f }, { 0.3f, 0.2f }, { 0.4f, 0.1f }
};

float applyReverb(float signal, float reverbAmount, float* reverbBuffer, int& reverbIndex)
{
    if (reverbAmount <= 0.0f) return signal;

    float reverbSignal = reverbBuffer[reverbIndex];
    float feedback = reverbAmount * 0.7f;
    reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;

    // This specific function scales size by amount, so we cannot use bitwise masking here
    int reverbSamples = static_cast<int>(reverbAmount * REVERB_BUFFER_SIZE);
    if (reverbSamples < 1) reverbSamples = 1;
    reverbIndex = (reverbIndex + 1) % reverbSamples;

    float mix = reverbAmount * 0.5f;
    return signal * (1.0f - mix) + reverbSignal * mix;
}

float applyReverb2(float signal, float amount, float* reverbBuffer, int& reverbIndex)
{
    return processDelayAndReverb(signal, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, REVERB_MASK, ReverbVoiceCount, reverbVoices);
}

float applyReverb3(float signal, float amount, float* reverbBuffer, int& reverbIndex)
{
    return processDelayAndReverb(signal, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, REVERB_MASK, 4, reverb3Voices);
}

// --- DELAY FUNCTIONS ---

static BufferVoice delay1Voices[4] = {
    { 0.20f, 0.6f }, { 0.40f, 0.4f }, { 0.60f, 0.3f }, { 0.80f, 0.2f }
};

static BufferVoice delay2Voices[4] = {
    { 0.05f, 0.5f }, { 0.10f, 0.4f }, { 0.70f, 0.35f }, { 0.80f, 0.25f }
};

static BufferVoice delay3Voices[2] = {
    { 0.30f, 0.6f }, { 0.60f, 0.4f }
};

float applyDelay(float signal, float amount, float* buffer, int& index)
{
    return processDelayAndReverb(signal, amount, buffer, index, DELAY_BUFFER_SIZE, DELAY_MASK, 4, delay1Voices);
}

float applyDelay2(float signal, float amount, float* buffer, int& index)
{
    return processDelayAndReverb(signal, amount, buffer, index, DELAY_BUFFER_SIZE, DELAY_MASK, 4, delay2Voices);
}

float applyDelay3(float signal, float amount, float* buffer, int& index)
{
    return processDelayAndReverb(signal, amount, buffer, index, DELAY_BUFFER_SIZE, DELAY_MASK, 2, delay3Voices);
}

// --- SHIMMER FUNCTIONS ---

float applyShimmerReverb(float input, float amount, float* reverbBuffer, int& reverbIndex)
{
    if (amount <= 0.0f) return input;

    reverbBuffer[reverbIndex] = input;
    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
        int delayOffset = static_cast<int>(reverbVoices[i].delay * REVERB_BUFFER_SIZE);
        int readIndex = (reverbIndex - delayOffset) & REVERB_MASK;
        float pitchShifted = reverbBuffer[readIndex] * Math::sin((float)readIndex * 0.01f);
        reverbOut += pitchShifted * reverbVoices[i].gain;
    }

    reverbBuffer[reverbIndex] = input + (reverbOut * (amount * 0.5f));
    reverbIndex = (reverbIndex + 1) & REVERB_MASK;

    float mix = amount * 0.8f;
    return input + mix * (reverbOut - input);
}

float applyShimmer2Reverb(float input, float amount, float* reverbBuffer, int& reverbIndex, int& shimmerTime)
{
    if (amount <= 0.0f) return input;

    reverbBuffer[reverbIndex] = input;
    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
        int delayOffset = static_cast<int>(reverbVoices[i].delay * REVERB_BUFFER_SIZE);
        int readIndex = (reverbIndex - delayOffset) & REVERB_MASK;
        float mod = Math::sin(0.05f * (float)shimmerTime + (float)i);
        reverbOut += (reverbBuffer[readIndex] * (0.7f + 0.3f * mod)) * reverbVoices[i].gain;
    }

    reverbBuffer[reverbIndex] = input + (reverbOut * (amount * 0.5f));
    reverbIndex = (reverbIndex + 1) & REVERB_MASK;
    if (++shimmerTime > 100000) shimmerTime = 0;

    float mix = amount * 0.8f;
    return input + mix * (reverbOut - input);
}