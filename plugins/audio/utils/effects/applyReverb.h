#pragma once

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
