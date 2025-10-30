#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/SynthSnare.h"
#include "audio/lookupTable.h"

#include <sstream>

#define MAX_TRACKS 16

// g++ plugins/audio/SynthSnare.test.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -I/usr/include/opus -I/usr/include/x86_64-linux-gnu -lsndfile  && ./test


int noDebug(const char* format, ...) { return 0; }

void hydate(AudioPlugin& plugin, std::string configs) {
    std::istringstream stream(configs);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty()) {
        plugin.hydrate(line);
        }
    }
}

int main(int argc, char* argv[])
{
    LookupTable lookupTable;
    
    AudioPlugin::Props props = {
        .sampleRate = 44100,
        .channels = 2,
        .audioPluginHandler = nullptr,
        .maxTracks = MAX_TRACKS,
        .lookupTable = &lookupTable,
    };

    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");
    SynthSnare drum(props, (char*)"zicSynthSnare");
//     hydate(drum, R"(
// )");

    float buffer[MAX_TRACKS] = { 0.0f };

    int count = 0;
    while (1) {
        if (count == props.sampleRate) { // every second
            count = 0;
            drum.noteOn(60, 1.0f);
        }
        drum.sample(buffer);
        audioOutput.sample(buffer);
        count++;
    }

    return 0;
}

