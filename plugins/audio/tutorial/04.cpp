/** Description:
This C++ program functions as a simple digital sound synthesizer, continuously generating a rhythmic sequence of beeps. It utilizes specialized audio libraries (PulseAudio) to interface directly with the computer’s sound system, allowing it to output raw audio data.

The core tone generated is a precise 440 Hz sine wave, which corresponds to the musical note A4. To generate this waveform efficiently, the program employs a performance optimization: it pre-calculates all the required sine values and stores them in a fixed list called a lookup table. Instead of performing slow mathematical calculations for every single audio sample, the program simply looks up the next value, significantly speeding up the process.

A crucial feature is the volume control, managed by an "envelope" function. This ensures that the sound does not start or stop suddenly, which would cause an unpleasant click. Instead, the envelope creates a smooth volume increase (ramp-up), maintains the full volume, and then executes a smooth fade-out (ramp-down) for each individual beep.

The main structure of the program involves an infinite loop: it first generates the fully shaped beep (tone with smooth volume changes) and sends it to the speakers, followed by a short period of complete silence. This cycle repeats indefinitely, resulting in a continuous pulsing tone.

sha: 91423be06c1210cc28a10e033e563d4446b0991adf30a693e0b2980eba67ae7f 
*/
// Compile and run:
// g++ 04.cpp -o 04.bin -I../../.. -lpulse-simple -lpulse && ./04.bin

#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

#define TWO_PI 6.283185307179586

// Size of the lookup table
#define LUT_SIZE 1024

// Precompute the sine values and store them in a lookup table
float sineLUT[LUT_SIZE];

// Function to initialize the sine lookup table
void initializeSineLUT() {
    for (int i = 0; i < LUT_SIZE; ++i) {
        sineLUT[i] = sin(TWO_PI * i / LUT_SIZE);
    }
}

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

    // Phase increment per sample based on the desired frequency
    float phaseIncrement = frequency * LUT_SIZE / props.sampleRate;

    // Initialize the sine lookup table
    initializeSineLUT();

    // Envelope configuration
    float rampDuration = 0.1f; // Ramp-up and ramp-down duration in seconds
    float beepDuration = 0.5f; // Total beep duration (including ramp-up and ramp-down) in seconds
    float breakDuration = 0.2f; // Duration of the silence between beeps

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

            // Get the sine value from the lookup table
            int index = static_cast<int>(phase) % LUT_SIZE;
            buffer[0] = amplitude * sineLUT[index];

            // Increment the phase
            phase += phaseIncrement;

            // Wrap the phase to stay within the LUT size
            if (phase >= LUT_SIZE) {
                phase -= LUT_SIZE;
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
