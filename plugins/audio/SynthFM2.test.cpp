#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/SynthFM2.h"
#include "plugins/audio/EffectDistortion2.h"
#include "audio/lookupTable.h"

#include <string>
#include <sstream>

#define MAX_TRACKS 16

// g++ plugins/audio/SynthFM2.test.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -I/usr/include/opus -I/usr/include/x86_64-linux-gnu -lsndfile  && ./test


int noDebug(const char* format, ...) { return 0; }

void hydate(AudioPlugin& plugin, std::string configs) {
    std::istringstream stream(configs);
    std::string line;
    while (std::getline(stream, line)) {
        // skip line starting with #
        if (!line.empty() && line[0] != '#') {
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
    SynthFM2 synth(props, (char*)"zicSynth");
    hydate(synth, R"(
ATTACK_0 1.000000
DECAY_0 2460.000000
SUSTAIN_0 34.799999
RELEASE_0 2161.000000
RATIO_0 3.779998
FEEDBACK_0 28.000000
ATTACK_1 2001.000000
DECAY_1 5000.000000
SUSTAIN_1 32.800003
RELEASE_1 41.000000
RATIO_1 0.070000
FEEDBACK_1 37.000000
ATTACK_2 181.000000
DECAY_2 360.000000
SUSTAIN_2 40.800003
RELEASE_2 710.000000
RATIO_2 1.000000
FEEDBACK_2 31.000000
ATTACK_3 150.000000
DECAY_3 360.000000
SUSTAIN_3 29.799999
RELEASE_3 541.000000
RATIO_3 2.009999
FEEDBACK_3 53.000000
ALGO 6.000000
FREQUENCY 1340.000000)");

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
        if (count == props.sampleRate * 3) { // every 3 second
            count = 0;
            synth.noteOn(60, 1.0f);
        }
        synth.sample(buffer);
        // distortion2.sample(buffer);
        audioOutput.sample(buffer);
        count++;
    }

    return 0;
}

