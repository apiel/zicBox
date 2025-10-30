#include "plugins/audio/SampleSequencer.h"
#include "plugins/audio/AudioOutputPulse.h"
#include "audio/lookupTable.h"

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
STEP 0 0 0.80 0.0 0.0 ---
STEP 1 1 0.80 0.0 100.0 samples/269720__ianstargem__electronic-closed-high-hat-1.wav
STEP 2 0 0.80 0.0 0.0 ---
STEP 3 0 0.80 0.0 0.0 ---
STEP 4 0 0.80 0.0 0.0 ---
STEP 5 1 0.80 0.0 100.0 samples/269720__ianstargem__electronic-closed-high-hat-1.wav
STEP 6 0 0.80 0.0 0.0 ---
STEP 7 0 0.80 0.0 0.0 ---
STEP 8 0 0.80 0.0 0.0 ---
STEP 9 1 0.80 0.0 100.0 samples/269720__ianstargem__electronic-closed-high-hat-1.wav
STEP 10 0 0.80 0.0 0.0 ---
STEP 11 0 0.80 0.0 0.0 ---
STEP 12 0 0.80 0.0 0.0 ---
STEP 13 1 0.80 0.0 100.0 samples/269720__ianstargem__electronic-closed-high-hat-1.wav
STEP 14 0 0.80 0.0 0.0 ---
STEP 15 0 0.80 0.0 0.0 ---
STEP 16 0 0.80 0.0 0.0 ---
STEP 17 1 0.80 0.0 100.0 samples/269720__ianstargem__electronic-closed-high-hat-1.wav
STEP 18 0 0.80 0.0 0.0 ---
STEP 19 0 0.80 0.0 0.0 ---
STEP 20 0 0.80 0.0 0.0 ---
STEP 21 1 0.80 0.0 100.0 samples/269720__ianstargem__electronic-closed-high-hat-1.wav
STEP 22 0 0.80 0.0 0.0 ---
STEP 23 0 0.80 0.0 0.0 ---
STEP 24 0 0.80 0.0 0.0 ---
STEP 25 1 0.80 0.0 100.0 samples/269720__ianstargem__electronic-closed-high-hat-1.wav
STEP 26 0 0.80 0.0 0.0 ---
STEP 27 0 0.80 0.0 0.0 ---
STEP 28 0 0.80 0.0 0.0 ---
STEP 29 1 0.80 0.0 100.0 samples/269720__ianstargem__electronic-closed-high-hat-1.wav
STEP 30 0 0.80 0.0 0.0 ---
STEP 31 0 0.80 0.0 0.0 ---
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
            // FIXME
            // plugin.onClockTick(&clockCounter);
        }
        plugin.sample(buffer);
        audioOutput.sample(buffer);
    }

    return 0;
}
