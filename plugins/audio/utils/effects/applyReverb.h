#pragma once

#include <cmath>
#include <cstdint>

float applyReverb(float signal, float reverbAmount, float* reverbBuffer, int& reverbIndex, uint64_t sampleRate, int REVERB_BUFFER_SIZE)
{
    if (reverbAmount == 0.0f) {
        return signal;
    }
    int reverbSamples = static_cast<int>((reverbAmount * 0.5f) * sampleRate); // Reverb duration scaled
    float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
    float mix = reverbAmount * 0.5f; // Mix scaled proportionally

    if (reverbSamples > REVERB_BUFFER_SIZE) {
        reverbSamples = REVERB_BUFFER_SIZE; // Cap the reverb duration to buffer size
    }

    float reverbSignal = reverbBuffer[reverbIndex];
    reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;
    reverbIndex = (reverbIndex + 1) % reverbSamples;

    return signal * (1.0f - mix) + reverbSignal * mix;
}

float applyReverb(float signal, float reverbAmount, float* reverbBuffer, int& reverbIndex, int REVERB_BUFFER_SIZE)
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

struct BufferVoice {
    int delay; // Fixed delay offset
    float gain;
};
static constexpr int ReverbVoiceCount = 4; // Reduced from 8 to 4 for efficiency
BufferVoice reverbVoices[ReverbVoiceCount] = {
    { 1800 * 2, 0.6f }, // First early reflection
    { 4200 * 2, 0.4f }, // Mid reflection
    { 6900 * 2, 0.3f }, // Late reflection
    { 9600 * 2, 0.2f }, // Very late tail
};

float applyShimmerReverb(float input, float amount, float* reverbBuffer, int& reverbIndex, int REVERB_BUFFER_SIZE)
{
    if (amount == 0.0f) {
        return input;
    }

    // Store the input in the buffer
    reverbBuffer[reverbIndex] = input;

    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
        int readIndex = (reverbIndex + REVERB_BUFFER_SIZE - reverbVoices[i].delay) % REVERB_BUFFER_SIZE;
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

float applyShimmer2Reverb(float input, float amount, float* reverbBuffer, int& reverbIndex, int REVERB_BUFFER_SIZE, int& shimmerTime)
{
    if (amount == 0.0f)
        return input;

    reverbBuffer[reverbIndex] = input;

    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
        int readIndex = (reverbIndex + REVERB_BUFFER_SIZE - reverbVoices[i].delay) % REVERB_BUFFER_SIZE;
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

float applyDelay(float signal, float amount, float* buffer, int& index, int BUFFER_SIZE, int voiceCount, BufferVoice* voices)
{
    if (amount == 0.0f) {
        return signal;
    }

    // Store input signal in buffer
    buffer[index] = signal;

    float reverbOut = 0.0f;

    // Process only 4 key delay taps
    for (uint8_t i = 0; i < voiceCount; i++) {
        int readIndex = (index + BUFFER_SIZE - voices[i].delay) % BUFFER_SIZE;
        reverbOut += buffer[readIndex] * voices[i].gain;
    }

    // Apply global feedback (reduces CPU by avoiding per-voice feedback writes)
    float globalFeedback = amount * 0.6f;
    buffer[index] = signal + reverbOut * globalFeedback;

    // Advance buffer index
    index = (index + 1) % BUFFER_SIZE;

    // Final wet/dry mix
    float mix = amount * 0.8f;
    return signal * (1.0f - mix) + reverbOut * mix;
}

float applyReverb2(float signal, float amount, float* reverbBuffer, int& reverbIndex, int REVERB_BUFFER_SIZE)
{
    return applyDelay(signal, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, ReverbVoiceCount, reverbVoices);
}

BufferVoice reverb3Voices[4] = {
    { 2000, 0.4f }, // First early reflection
    { 4000, 0.3f }, // Mid reflection
    { 6000, 0.2f }, // Late reflection
    { 8000, 0.1f }, // Very late tail
};
float applyReverb3(float signal, float amount, float* reverbBuffer, int& reverbIndex, int REVERB_BUFFER_SIZE)
{
    return applyDelay(signal, amount, reverbBuffer, reverbIndex, REVERB_BUFFER_SIZE, 4, reverb3Voices);
}

static constexpr int DelayVoiceCount = 4; // Reduced from 8 to 4 for efficiency
BufferVoice delay1Voices[DelayVoiceCount] = {
    { (int)(48000 * 3 * 0.20f), 0.6f }, // First early reflection
    { (int)(48000 * 3 * 0.40f), 0.4f }, // Mid reflection
    { (int)(48000 * 3 * 0.60f), 0.3f }, // Late reflection
    { (int)(48000 * 3 * 0.80f), 0.2f }, // Very late tail
};
float applyDelay(float signal, float amount, float* buffer, int& index, int BUFFER_SIZE)
{
    return applyDelay(signal, amount, buffer, index, BUFFER_SIZE, DelayVoiceCount, delay1Voices);
}

BufferVoice delay2Voices[DelayVoiceCount] = {
    { (int)(48000 * 3 * 0.05f), 0.5f },
    { (int)(48000 * 3 * 0.10f), 0.4f },
    { (int)(48000 * 3 * 0.70f), 0.35f }, // very close to previous one = comb-like flutter
    { (int)(48000 * 3 * 0.80f), 0.25f },
};
float applyDelay2(float signal, float amount, float* buffer, int& index, int BUFFER_SIZE)
{
    return applyDelay(signal, amount, buffer, index, BUFFER_SIZE, DelayVoiceCount, delay2Voices);
}

BufferVoice delay3Voices[2] = {
    { (int)(48000 * 3 * 0.3f), 0.6f },
    { (int)(48000 * 3 * 0.6f), 0.4f },
};
float applyDelay3(float signal, float amount, float* buffer, int& index, int BUFFER_SIZE)
{
    return applyDelay(signal, amount, buffer, index, BUFFER_SIZE, 2, delay3Voices);
}
