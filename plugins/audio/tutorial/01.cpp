#include "plugins/audio/AudioOutputPulse.h"

#define MAX_TRACKS 16
#define TWO_PI 6.283185307179586

// g++ 01.cpp -o 01.bin -I../../.. -pthread -D_REENTRANT -lpulse-simple -lpulse -pthread && ./01.bin

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

    while (1) {
        buffer[0] = rand() / (float)RAND_MAX;
        audioOutput.sample(buffer);
    }

    return 0;
}