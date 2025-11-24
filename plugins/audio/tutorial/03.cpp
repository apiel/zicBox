/** Description:
This C++ program is designed to function as a continuous audio tone generator. Its primary goal is to play a repeating pattern of beeps followed by periods of silence through the computer's audio system, specifically utilizing the PulseAudio framework common in Linux environments.

The program creates sound purely mathematically. It defines a specific frequency (440 Hz, corresponding to the standard A note) and calculates the exact volume value for each tiny moment in time using a sine wave formula.

A crucial component is the **envelope function**. This is the sound's volume control. Instead of abruptly starting and stopping the tone, which would cause an unpleasant "click," the envelope ensures a smooth listening experience. It dictates a gentle ramp-up (fade-in) to full volume, sustains that volume for a period, and then executes a smooth ramp-down (fade-out) before silence begins.

The overall mechanism runs in a continuous loop:

1.  **Generate Beep:** It generates 1.5 seconds of the 440 Hz tone, applying the volume envelope to ensure smooth transitions.
2.  **Generate Break:** It generates 0.5 seconds of absolute silence.

This cycle repeats indefinitely, resulting in a continuous pattern of fading tones separated by short breaks.

sha: d6ae4b4561741bba5435839938de0e33bac5d43c506d4457d654b7a7172a437d 
*/
// Compile and run:
// g++ 03.cpp -o 03.bin -I../../.. -lpulse-simple -lpulse && ./03.bin

#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

#define TWO_PI 6.283185307179586

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

    // Phase accumulator for the sine wave
    float phase = 0.0f;

    // Phase increment per sample
    float phaseIncrement = TWO_PI * frequency / props.sampleRate;

    // Envelope configuration
    float rampDuration = 0.4f; // Ramp-up and ramp-down duration in seconds
    float beepDuration = 1.5f; // Total beep duration (including ramp-up and ramp-down) in seconds
    float breakDuration = 0.5f; // Duration of the silence between beeps in seconds

    // Calculate sample counts
    int rampSamples = rampDuration * props.sampleRate;
    int beepSamples = beepDuration * props.sampleRate;
    int breakSamples = breakDuration * props.sampleRate;

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
