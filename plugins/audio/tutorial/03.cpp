#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

#define TWO_PI 6.283185307179586

// Compile and run:
// g++ 03.cpp -o 03.bin -I../../.. -pthread -D_REENTRANT -lpulse-simple -lpulse && ./03.bin

// Function to calculate the envelope value at a given sample index
float calculateEnvelope(int sampleIndex, int totalSamples, int rampSamples)
{
    if (sampleIndex < rampSamples) {
        // Ramp-up
        return (float)sampleIndex / rampSamples;
    } else if (sampleIndex >= totalSamples - rampSamples) {
        // Ramp-down
        return (float)(totalSamples - sampleIndex) / rampSamples;
    } else {
        // Sustain
        return 1.0f;
    }
}

int main(int argc, char* argv[])
{
    // Initialize audio properties with default settings
    AudioPlugin::Props props = defaultAudioProps;

    // Create an AudioOutputPulse instance to handle audio output
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    // Create an audio buffer to hold samples (one per track)
    float buffer[props.maxTracks] = { 0.0f };

    // Frequency of the sine wave (in Hz)
    float frequency = 440.0f; // Standard A4 note

    // Sampling rate (from audio properties)
    float sampleRate = props.sampleRate;

    // Phase accumulator for the sine wave
    float phase = 0.0f;

    // Phase increment per sample
    float phaseIncrement = TWO_PI * frequency / sampleRate;

    // Envelope configuration
    float rampDuration = 0.4f; // Ramp-up and ramp-down duration in seconds
    float beepDuration = 1.5f; // Total beep duration (including ramp-up and ramp-down) in seconds
    float breakDuration = 0.5f; // Duration of the silence between beeps in seconds

    // Calculate sample counts
    int rampSamples = rampDuration * sampleRate;
    int beepSamples = beepDuration * sampleRate;
    int breakSamples = breakDuration * sampleRate;

    // Main loop to generate and play the sine wave with envelope
    while (1) {
        // Generate the beep with ramp-up and ramp-down
        for (int i = 0; i < beepSamples; ++i) {
            // Calculate the current amplitude using the envelope function
            float amplitude = calculateEnvelope(i, beepSamples, rampSamples);

            // Generate the sine wave sample with envelope
            buffer[0] = amplitude * sin(phase);

            // Increment the phase
            phase += phaseIncrement;

            // Wrap the phase back into the range [0, TWO_PI]
            if (phase >= TWO_PI) {
                phase -= TWO_PI;
            }

            // Send the generated sample to the audio output
            audioOutput.sample(buffer);
        }

        // Generate silence for the break
        for (int i = 0; i < breakSamples; ++i) {
            buffer[0] = 0.0f; // Silence
            audioOutput.sample(buffer);
        }
    }

    return 0;
}
