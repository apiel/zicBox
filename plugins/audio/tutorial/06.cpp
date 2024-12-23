// Compile and run:
// g++ 06.cpp -o 06.bin -I../../.. -lpulse-simple -lpulse && ./06.bin

#include "plugins/audio/AudioOutputPulse.h"
#include <cmath> // For sin()

// Define the size of the wavetable and the number of waveforms
const uint16_t WAVE_SIZE = 256; // Number of samples per waveform
const uint16_t NUM_WAVES = 3;   // Number of waveforms in the wavetable

// Generate a simple wavetable with multiple waveforms
void generateWavetable(float wavetable[WAVE_SIZE * NUM_WAVES]) {
    for (uint8_t wave = 0; wave < NUM_WAVES; ++wave) {
        for (int16_t i = 0; i < WAVE_SIZE; ++i) {
            float phase = static_cast<float>(i) / WAVE_SIZE;
            if (wave == 0) {
                wavetable[wave * WAVE_SIZE + i] = std::sin(2.0f * M_PI * phase); // Sine wave
            } else if (wave == 1) {
                wavetable[wave * WAVE_SIZE + i] = phase < 0.5f ? 1.0f : -1.0f;    // Square wave
            } else if (wave == 2) {
                wavetable[wave * WAVE_SIZE + i] = 2.0f * (phase - 0.5f);         // Sawtooth wave
            }
        }
    }
}

int main() {
    // Initialize audio properties with default settings
    AudioPlugin::Props props = defaultAudioProps;

    // Create an AudioOutputPulse instance to handle audio output
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    // Create an audio buffer to hold samples (one per track)
    float buffer[props.maxTracks] = { 0.0f };

    // Create a flat array to store the wavetable
    float wavetable[WAVE_SIZE * NUM_WAVES];

    // Generate waveforms
    generateWavetable(wavetable);

    uint8_t waveform = 0; // Sine wave
    int16_t index = 0;
    // Main loop to generate and play the sine wave
    while (1) {
        // Calculate the sine value for the current phase
        buffer[0] = wavetable[waveform * WAVE_SIZE + index];

        index++;
        while (index >= WAVE_SIZE) {
            index -= WAVE_SIZE;
        }

        // Send the generated sample to the audio output
        audioOutput.sample(buffer);
    }

    return 0;
}
