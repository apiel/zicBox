#include "plugins/audio/SampleSequencer.h"
#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/lookupTable.h"

#define MAX_TRACKS 16

// g++ plugins/audio/SampleSequencer.test.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -I/usr/include/opus -I/usr/include/x86_64-linux-gnu -lsndfile  && ./test

int noDebug(const char* format, ...) { return 0; }

void hydate(AudioPlugin& plugin, std::string configs)
{
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
    SampleSequencer plugin(props, (char*)"zicSampleSequencer");
    hydate(plugin, R"(
STATUS 1.000000)");

    float buffer[MAX_TRACKS] = { 0.0f };

    int bpm = 160;

    uint64_t clockCounter = 0;
    uint32_t sampleCounter = 0;
    uint32_t sampleCountTarget = (uint32_t)(((float)props.sampleRate * 60.0f / bpm) / 12.0f);
    while (1) {
        sampleCounter++;
        if (sampleCounter >= sampleCountTarget) {
            sampleCounter = 0;
            clockCounter++;
            plugin.onClockTick(&clockCounter);
        }
        plugin.sample(buffer);
        audioOutput.sample(buffer);
    }

    return 0;
}
