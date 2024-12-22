// Compile and run:
// g++ 02.cpp -o 02.bin -I../../.. -lpulse-simple -lpulse && ./02.bin

#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

#define TWO_PI 6.283185307179586

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

    // Main loop to generate and play the sine wave
    while (1) {
        // Calculate the sine value for the current phase
        buffer[0] = sin(phase);

        // Increment the phase
        phase += phaseIncrement;

        // Wrap the phase back into the range [0, TWO_PI]
        if (phase >= TWO_PI) {
            phase -= TWO_PI;
        }

        // Send the generated sample to the audio output
        audioOutput.sample(buffer);
    }

    return 0;
}
