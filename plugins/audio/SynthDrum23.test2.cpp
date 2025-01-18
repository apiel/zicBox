#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/SynthDrum23.h"
#include "plugins/audio/lookupTable.h"

#define MAX_TRACKS 16

// g++ plugins/audio/SynthDrum23.test2.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -I/usr/include/opus -I/usr/include/x86_64-linux-gnu -lsndfile  && ./test


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
    SynthDrum23 drum23(props, (char*)"zicSynthDrum23");
    hydate(drum23, R"(
WAVEFORM_TYPE 1.000000
SHAPE 0.000000
MACRO 0.000000
PITCH -21.000000
DURATION 1290.000000
GAIN_CLIPPING 19.000000
CLICK 44.000000
CLICK_DURATION 0.500002
CLICK_CUTOFF -44.000000
CLICK_RESONANCE 0.000000
ENV_AMP 0 0.000000:0.000000 1.000000:0.010000 0.000000:1.000000
ENV_FREQ 0 1.000000:0.000000 0.510000:0.030000 0.300000:0.070000 0.090000:0.190000 0.000000:1.000000)");

    float buffer[MAX_TRACKS] = { 0.0f };

    int count = 0;
    while (1) {
        if (count == props.sampleRate) { // every second
            count = 0;
            drum23.noteOn(60, 1.0f);
        }
        drum23.sample(buffer);
        audioOutput.sample(buffer);
        count++;
    }

    return 0;
}

