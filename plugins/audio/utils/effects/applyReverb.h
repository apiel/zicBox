#pragma once

#include <cmath>
#include <cstdint>

constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
constexpr int DELAY_BUFFER_SIZE = REVERB_BUFFER_SIZE * 3; // 3 second

#define DELAY_BUFFER float buffer[DELAY_BUFFER_SIZE] = { 0.0f };

float applyReverb(float signal, float reverbAmount, float* reverbBuffer, int& reverbIndex, uint64_t sampleRate, int bufferSize)
{
    if (reverbAmount == 0.0f) {
        return signal;
    }
    int reverbSamples = static_cast<int>((reverbAmount * 0.5f) * sampleRate); // Reverb duration scaled
    float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
    float mix = reverbAmount * 0.5f; // Mix scaled proportionally

    if (reverbSamples > bufferSize) {
        reverbSamples = bufferSize; // Cap the reverb duration to buffer size
    }

    float reverbSignal = reverbBuffer[reverbIndex];
    reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;
    reverbIndex = (reverbIndex + 1) % reverbSamples;

    return signal * (1.0f - mix) + reverbSignal * mix;
}

float applyReverb(float signal, float reverbAmount, float* reverbBuffer, int& reverbIndex)
{
    if (reverbAmount == 0.0f) {
        return signal;
    }

    float reverbSignal = reverbBuffer[reverbIndex];

    float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
    reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;

    int reverbSamples = reverbAmount * REVERB_BUFFER_SIZE; // Reverb duration scaled
    reverbIndex = (reverbIndex + 1) % reverbSamples;

    float mix = reverbAmount * 0.5f; // Mix scaled proportionally
    return signal * (1.0f - mix) + reverbSignal * mix;
}

struct BufferVoice
{
    float delay;
    float gain;
};

float processDelayAndReverb(float signal, float amount, float *buffer, int &index, int bufferSize, int voiceCount, BufferVoice *voices)
{
    if (amount == 0.0f)
    {
        return signal;
    }

    // Store input signal in buffer
    buffer[index] = signal;

    float reverbOut = 0.0f;

    // Process only 4 key delay taps
    for (uint8_t i = 0; i < voiceCount; i++)
    {
        int delay = voices[i].delay * bufferSize;
        int readIndex = (index + bufferSize - delay) % bufferSize;
        reverbOut += buffer[readIndex] * voices[i].gain;
    }

    // Apply global feedback (reduces CPU by avoiding per-voice feedback writes)
    float globalFeedback = amount * 0.6f;
    buffer[index] = signal + reverbOut * globalFeedback;

    // Advance buffer index
    index = (index + 1) % bufferSize;

    // Final wet/dry mix
    float mix = amount * 0.8f;
    // float mix = powf(amount, 2.0f) * 0.9f; // smoother response
    return signal * (1.0f - mix) + reverbOut * mix;
}

static constexpr int ReverbVoiceCount = 4; // Reduced from 8 to 4 for efficiency
BufferVoice reverbVoices[ReverbVoiceCount] = {
    {0.2, 0.6f}, // First early reflection
    {0.4, 0.4f}, // Mid reflection
    {0.6, 0.3f}, // Late reflection
    {0.8, 0.2f}, // Very late tail
};
float applyReverb2(float signal, float amount, float *reverbBuffer, int &reverbIndex)
{
    return processDelayAndReverb(signal, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, ReverbVoiceCount, reverbVoices);
}

BufferVoice reverb3Voices[4] = {
    {0.1, 0.4f}, // First early reflection
    {0.2, 0.3f}, // Mid reflection
    {0.3, 0.2f}, // Late reflection
    {0.4, 0.1f}, // Very late tail
};
float applyReverb3(float signal, float amount, float *reverbBuffer, int &reverbIndex)
{
    return processDelayAndReverb(signal, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, 4, reverb3Voices);
}

BufferVoice delay1Voices[4] = {
    {0.20f, 0.6f}, // First early reflection
    {0.40f, 0.4f}, // Mid reflection
    {0.60f, 0.3f}, // Late reflection
    {0.80f, 0.2f}, // Very late tail
};
float applyDelay(float signal, float amount, float *buffer, int &index)
{
    return processDelayAndReverb(signal, amount, buffer, index, DELAY_BUFFER_SIZE, 4, delay1Voices);
}

BufferVoice delay2Voices[4] = {
    {0.05f, 0.5f},
    {0.10f, 0.4f},
    {0.70f, 0.35f}, // very close to previous one = comb-like flutter
    {0.80f, 0.25f},
};
float applyDelay2(float signal, float amount, float *buffer, int &index)
{
    return processDelayAndReverb(signal, amount, buffer, index, DELAY_BUFFER_SIZE, 4, delay2Voices);
}

BufferVoice delay3Voices[2] = {
    {0.30f, 0.6f},
    {0.60f, 0.4f},
};
float applyDelay3(float signal, float amount, float *buffer, int &index)
{
    return processDelayAndReverb(signal, amount, buffer, index, DELAY_BUFFER_SIZE, 2, delay3Voices);
}

float applyShimmerReverb(float input, float amount, float *reverbBuffer, int &reverbIndex)
{
    if (amount == 0.0f)
    {
        return input;
    }

    // Store the input in the buffer
    reverbBuffer[reverbIndex] = input;

    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++)
    {
        int delay = reverbVoices[i].delay * REVERB_BUFFER_SIZE;
        int readIndex = (reverbIndex + REVERB_BUFFER_SIZE - delay) % REVERB_BUFFER_SIZE;
        float delayedSample = reverbBuffer[readIndex];

        // Apply pitch shift (simple octave up using sine modulation)
        float pitchShifted = delayedSample * sin((float)readIndex * 0.01f); // crude pitch variation

        reverbOut += pitchShifted * reverbVoices[i].gain;
    }

    // Global feedback
    float feedback = amount * 0.5f;
    reverbBuffer[reverbIndex] = input + reverbOut * feedback;

    // Advance index
    reverbIndex = (reverbIndex + 1) % REVERB_BUFFER_SIZE;

    // Wet/dry mix
    float mix = amount * 0.8f;
    return input * (1.0f - mix) + reverbOut * mix;
}

float applyShimmer2Reverb(float input, float amount, float *reverbBuffer, int &reverbIndex, int &shimmerTime)
{
    if (amount == 0.0f)
        return input;

    reverbBuffer[reverbIndex] = input;

    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++)
    {
        int delay = reverbVoices[i].delay * REVERB_BUFFER_SIZE;
        int readIndex = (reverbIndex + REVERB_BUFFER_SIZE - delay) % REVERB_BUFFER_SIZE;
        float delayedSample = reverbBuffer[readIndex];

        // Use shimmerTime for modulation — phase moves smoothly
        float mod = sinf(0.05f * shimmerTime + i); // speed and offset per voice

        // Simple pitch modulation by amplitude mod
        float modulatedSample = delayedSample * (0.7f + 0.3f * mod);

        reverbOut += modulatedSample * reverbVoices[i].gain;
    }

    // Feedback to buffer
    float feedback = amount * 0.5f;
    reverbBuffer[reverbIndex] = input + reverbOut * feedback;

    reverbIndex = (reverbIndex + 1) % REVERB_BUFFER_SIZE;
    shimmerTime = (shimmerTime + 1) % 100000;

    // Wet/dry mix
    float mix = amount * 0.8f;
    return input * (1.0f - mix) + reverbOut * mix;
}
