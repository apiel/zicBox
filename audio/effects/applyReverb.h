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
#include "audio/effects/fxBuffer.h"
#include <cmath>
#include <cstdint>

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
static BufferVoice reverb2Voices[ReverbVoiceCount] = {
    { 0.2f, 0.6f }, { 0.4f, 0.4f }, { 0.6f, 0.3f }, { 0.8f, 0.2f }
};

static BufferVoice reverb3Voices[4] = {
    { 0.1f, 0.4f }, { 0.2f, 0.3f }, { 0.3f, 0.2f }, { 0.4f, 0.1f }
};

float applyReverb2(float signal, float amount, float* reverbBuffer, int& reverbIndex)
{
    return processDelayAndReverb(signal, amount, reverbBuffer, reverbIndex, FX_BUFFER_SIZE, FX_BUFFER_MASK, ReverbVoiceCount, reverb2Voices);
}

float applyReverb3(float signal, float amount, float* reverbBuffer, int& reverbIndex)
{
    return processDelayAndReverb(signal, amount, reverbBuffer, reverbIndex, FX_BUFFER_SIZE, FX_BUFFER_MASK, 4, reverb3Voices);
}

float applyReverb(float signal, float reverbAmount, float* reverbBuffer, int& reverbIndex)
{
    if (reverbAmount <= 0.0f) return signal;

    float* state = reverbBuffer; 
    float* audioData = &reverbBuffer[12];

    const int combSizes[] = { 1117, 1301, 1571, 1787 };
    const int apSizes[]   = { 557, 443 };
    
    float feedback = reverbAmount * 0.82f; 
    if (feedback > 0.92f) feedback = 0.92f;
    
    float damping = 0.25f;
    float mix = reverbAmount * 0.4f;

    float combTotal = 0.0f;
    int internalOffset = 0;

    for (int i = 0; i < 4; ++i) {
        int idx = static_cast<int>(state[i]);
        float out = audioData[internalOffset + idx];
        
        state[6 + i] = (out * (1.0f - damping)) + (state[6 + i] * damping);
        
        audioData[internalOffset + idx] = signal + (state[6 + i] * feedback);
        
        combTotal += out;
        state[i] = static_cast<float>((idx + 1) % combSizes[i]);
        internalOffset += combSizes[i];
    }

    float output = combTotal * 0.25f;

    for (int i = 0; i < 2; ++i) {
        int idx = static_cast<int>(state[4 + i]);
        float bufOut = audioData[internalOffset + idx];
        
        float apIn = output;
        output = -apIn + bufOut;
        audioData[internalOffset + idx] = apIn + (bufOut * 0.5f);
        
        state[4 + i] = static_cast<float>((idx + 1) % apSizes[i]);
        internalOffset += apSizes[i];
    }

    return (signal * (1.0f - mix)) + (output * mix);
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
    return processDelayAndReverb(signal, amount, buffer, index, FX_BUFFER_SIZE, FX_BUFFER_MASK, 4, delay1Voices);
}

float applyDelay2(float signal, float amount, float* buffer, int& index)
{
    return processDelayAndReverb(signal, amount, buffer, index, FX_BUFFER_SIZE, FX_BUFFER_MASK, 4, delay2Voices);
}

float applyDelay3(float signal, float amount, float* buffer, int& index)
{
    return processDelayAndReverb(signal, amount, buffer, index, FX_BUFFER_SIZE, FX_BUFFER_MASK, 2, delay3Voices);
}

// --- SHIMMER FUNCTIONS ---

float applyShimmerReverb(float input, float amount, float* reverbBuffer, int& reverbIndex)
{
    if (amount <= 0.0f) return input;

    reverbBuffer[reverbIndex] = input;
    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
        int delayOffset = static_cast<int>(reverb2Voices[i].delay * FX_BUFFER_SIZE);
        int readIndex = (reverbIndex - delayOffset) & FX_BUFFER_MASK;
        float pitchShifted = reverbBuffer[readIndex] * Math::sin((float)readIndex * 0.01f);
        reverbOut += pitchShifted * reverb2Voices[i].gain;
    }

    reverbBuffer[reverbIndex] = input + (reverbOut * (amount * 0.5f));
    reverbIndex = (reverbIndex + 1) & FX_BUFFER_MASK;

    float mix = amount * 0.8f;
    return input + mix * (reverbOut - input);
}

float applyShimmer2Reverb(float input, float amount, float* reverbBuffer, int& reverbIndex, int& shimmerTime)
{
    if (amount <= 0.0f) return input;

    reverbBuffer[reverbIndex] = input;
    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
        int delayOffset = static_cast<int>(reverb2Voices[i].delay * FX_BUFFER_SIZE);
        int readIndex = (reverbIndex - delayOffset) & FX_BUFFER_MASK;
        float mod = Math::sin(0.05f * (float)shimmerTime + (float)i);
        reverbOut += (reverbBuffer[readIndex] * (0.7f + 0.3f * mod)) * reverb2Voices[i].gain;
    }

    reverbBuffer[reverbIndex] = input + (reverbOut * (amount * 0.5f));
    reverbIndex = (reverbIndex + 1) & FX_BUFFER_MASK;
    if (++shimmerTime > 100000) shimmerTime = 0;

    float mix = amount * 0.8f;
    return input + mix * (reverbOut - input);
}

float applyMiniReverb(float signal, float reverbAmount, float* reverbBuffer, int& reverbIndex)
{
    if (reverbAmount <= 0.0f) return signal;

    int reverbSamples = static_cast<int>(reverbAmount * FX_BUFFER_SIZE);
    if (reverbSamples < 1) reverbSamples = 1;

    int earlyTap = (reverbIndex - static_cast<int>(reverbSamples * 0.382f)) % reverbSamples;
    if (earlyTap < 0) earlyTap += reverbSamples;

    int lateTap = (reverbIndex - static_cast<int>(reverbSamples * 0.764f)) % reverbSamples;
    if (lateTap < 0) lateTap += reverbSamples;

    float reverbSignal = reverbBuffer[reverbIndex];

    float diffusedSignal = reverbSignal * 0.45f + reverbBuffer[earlyTap] * 0.35f + reverbBuffer[lateTap] * 0.20f;

    float feedback = reverbAmount * 0.65f;
    reverbBuffer[reverbIndex] = signal + diffusedSignal * feedback;

    reverbIndex = (reverbIndex + 1) % reverbSamples;

    float mix = reverbAmount * 0.55f;
    return signal * (1.0f - mix) + diffusedSignal * mix;
}

float applyFeedback(float input, float amount, float* fbBuffer, int& bufferIndex, float sampleRate)
{
    if (amount <= 0.001f) return input;
    float delayedSample = fbBuffer[bufferIndex];
    static float lowpassState = 0.0f;
    float cutoff = 400.0f + (2000.0f * amount); // Higher cutoff to actually hear it
    float alpha = (2.0f * 3.14159f * cutoff) / sampleRate;

    lowpassState += alpha * (delayedSample - lowpassState);
    float feedbackPath = lowpassState * amount;
    fbBuffer[bufferIndex] = input + (feedbackPath * 0.95f);

    float out = input + feedbackPath;

    bufferIndex++;
    if (bufferIndex >= FX_BUFFER_SIZE) bufferIndex = 0;

    return out;
}