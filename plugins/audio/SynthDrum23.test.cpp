#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/SynthDrum23.h"
#include "plugins/audio/EffectDistortion2.h"
#include "audio/lookupTable.h"

#define MAX_TRACKS 16

// g++ plugins/audio/SynthDrum23.test.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -I/usr/include/opus -I/usr/include/x86_64-linux-gnu -lsndfile  && ./test


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
PITCH -29.000000
DURATION 1550.000000
GAIN_CLIPPING 20.000000
CLICK 25.000000
CLICK_DURATION 7.000000
CLICK_CUTOFF 74.000000
CLICK_RESONANCE 66.000000
ENV_AMP 0 0.000000:0.000000 1.000000:0.000000 0.990000:0.110000 0.290000:0.560000 0.000000:1.000000
ENV_FREQ 0 1.000000:0.000000 0.560000:0.020000 0.240000:0.130000 0.140000:0.410000 0.090000:1.000000)");

    EffectDistortion2 distortion2(props, (char*)"zicEffectDistortion2");
    hydate(distortion2, R"(
LEVEL 100.000000
DRIVE 70.000000
COMPRESS 55.000000
BASS 19.000000
WAVESHAPE 25.000000)");

    float buffer[MAX_TRACKS] = { 0.0f };

    int count = 0;
    while (1) {
        if (count == props.sampleRate) { // every second
            count = 0;
            drum23.noteOn(60, 1.0f);
        }
        drum23.sample(buffer);
        distortion2.sample(buffer);
        audioOutput.sample(buffer);
        count++;
    }

    return 0;
}

