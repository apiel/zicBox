#include "plugins/audio/AudioOutputPulse.h"

// g++ 01.cpp -o 01.bin -I../../.. -pthread -D_REENTRANT -lpulse-simple -lpulse -pthread && ./01.bin

int main(int argc, char* argv[])
{
    AudioPlugin::Props props = defaultAudioProps;
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    float buffer[props.maxTracks] = { 0.0f };

    while (1) {
        buffer[0] = rand() / (float)RAND_MAX;
        audioOutput.sample(buffer);
    }

    return 0;
}