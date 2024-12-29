#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/SynthDrum23.h"
#include "plugins/audio/lookupTable.h"

#define MAX_TRACKS 16
#define TWO_PI 6.283185307179586

// g++ test.cpp -o test -I../../.. -pthread -D_REENTRANT -lpulse-simple -lpulse -pthread && ./test
// g++ plugins/audio/test.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -pthread && ./test

// g++ plugins/audio/test.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -I/usr/include/opus -I/usr/include/x86_64-linux-gnu -lsndfile  && ./test


int noDebug(const char* format, ...) { return 0; }

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

    drum23.hydrate("WAVEFORM_TYPE 1.000000");
    drum23.hydrate("SHAPE 0.000000");
    drum23.hydrate("MACRO 0.000000");
    drum23.hydrate("PITCH -29.000000");
    drum23.hydrate("DURATION 1550.000000");
    drum23.hydrate("GAIN_CLIPPING 20.000000");
    drum23.hydrate("CLICK 25.000000");
    drum23.hydrate("CLICK_DURATION 7.000000");
    drum23.hydrate("CLICK_CUTOFF 74.000000");
    drum23.hydrate("CLICK_RESONANCE 66.000000");
    drum23.hydrate("ENV_AMP 0.000000:0.000000 1.000000:0.000000 0.990000:0.110000 0.290000:0.560000 0.000000:1.000000");
    drum23.hydrate("ENV_FREQ 1.000000:0.000000 0.560000:0.020000 0.240000:0.130000 0.140000:0.410000 0.090000:1.000000");

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

