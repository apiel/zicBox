#pragma once

#include <cstdint>
#include <cmath>

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

struct ReverbVoice {
    int delay; // Fixed delay offset
    float gain;
};

float applyShimmerReverb(float input, float amount, float* reverbBuffer, int& reverbIndex, int REVERB_BUFFER_SIZE, int ReverbVoiceCount, ReverbVoice* voices)
{
    if (amount == 0.0f) {
        return input;
    }

    // Store the input in the buffer
    reverbBuffer[reverbIndex] = input;

    float reverbOut = 0.0f;

    for (uint8_t i = 0; i < ReverbVoiceCount; i++) {
        int readIndex = (reverbIndex + REVERB_BUFFER_SIZE - voices[i].delay) % REVERB_BUFFER_SIZE;
        float delayedSample = reverbBuffer[readIndex];

        // Apply pitch shift (simple octave up using sine modulation)
        float pitchShifted = delayedSample * sin((float)readIndex * 0.01f); // crude pitch variation

        reverbOut += pitchShifted * voices[i].gain;
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