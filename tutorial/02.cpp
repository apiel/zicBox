/** Description:
## C++ Sine Wave Audio Generator

This program is a straightforward digital audio application designed to generate and continuously play a single, pure tone. It utilizes specific tools designed to interface with the PulseAudio sound system, which is commonly used on Linux operating systems.

### What It Does

The core function of the code is to act as a simple synthesizer. It creates a mathematically perfect sound wave—a sine wave—and outputs it to the speakers indefinitely until the program is manually stopped.

### How It Works

1.  **Initialization:** The program first sets up the necessary technical rules for sound playback, such as the required sample rate (how many individual audio measurements are taken per second). It then establishes a connection with the computer’s sound system using a specialized audio output handler.

2.  **Defining the Tone:** A specific frequency is chosen for the tone, set here at 440 Hertz (Hz). This frequency corresponds to the standard musical note 'A' above middle C.

3.  **Continuous Generation Loop:** The program enters an endless cycle where it generates audio data one tiny piece ("sample") at a time.

4.  **Wave Calculation:** To create the smooth, pure tone, the program uses the sine mathematical function. It continuously tracks its position along the wave (known as the "phase"). By calculating the sine value at that specific point, it determines the exact volume level needed for that momentary sample.

5.  **Output:** This calculated volume level is immediately sent to the PulseAudio handler. By repeatedly calculating and sending these samples in rapid succession, the program creates a continuous digital data stream that the hardware translates into an audible, stable tone.

sha: 0f29c7aacf841f0b6d9fe470e360f1d0dfdf282ee94907434235229d1bd6cfd7 
*/
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
