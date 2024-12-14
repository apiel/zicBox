#include "AudioOutputPulse.h"

#define MAX_TRACKS 16
#define TWO_PI 6.283185307179586

// g++ test.cpp -o test -I../.. -pthread -D_REENTRANT -lpulse-simple -lpulse -pthread && ./test

int noDebug(const char* format, ...) { return 0; }

int main(int argc, char* argv[])
{
    AudioPlugin::Props props = {
        .debug = noDebug,
        .sampleRate = 44100,
        .channels = 2,
        .audioPluginHandler = nullptr,
        .maxTracks = MAX_TRACKS,
        .lookupTable = nullptr,
    };

    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    float buffer[MAX_TRACKS] = { 0.0f };

    // while (1) {
    //     buffer[0] = rand() / (float)RAND_MAX;
    //     audioOutput.sample(buffer);
    // }

    // Main sine wave parameters
    float frequency = 220.0f; // Base frequency in Hz
    float phase = 0.0f;       // Phase accumulator
    float phaseIncrement = (TWO_PI * frequency) / props.sampleRate; // Increment per sample

    // LFO parameters for amplitude modulation
    float lfoFrequency = 0.5f; // Low Frequency Oscillator (LFO) in Hz
    float lfoPhase = 0.0f;
    float lfoPhaseIncrement = (TWO_PI * lfoFrequency) / props.sampleRate;

    while (1) {
        // Amplitude modulation using LFO
        float lfoValue = (std::sin(lfoPhase) + 1.0f) / 2.0f; // LFO oscillates between 0 and 1

        // Generate modulated sine wave for track 0
        buffer[0] = lfoValue * std::sin(phase);

        // Advance main phase and wrap around
        phase += phaseIncrement;
        if (phase >= TWO_PI) {
            phase -= TWO_PI;
        }

        // Advance LFO phase and wrap around
        lfoPhase += lfoPhaseIncrement;
        if (lfoPhase >= TWO_PI) {
            lfoPhase -= TWO_PI;
        }

        audioOutput.sample(buffer);
    }
}
