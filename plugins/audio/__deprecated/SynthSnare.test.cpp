/** Description:
This C++ program is a straightforward demonstration of audio synthesis and playback, essentially acting as a minimal, endless digital drum machine.

**The Goal:** The program's sole purpose is to continuously play a synthesized snare drum sound through the computer’s sound system (specifically using PulseAudio, a common Linux sound server).

**How It Works:**

1.  **Setup:** The program first defines crucial audio parameters, like the quality (sample rate, which dictates how many tiny slices of sound are processed per second) and the number of output channels. It also initializes a special mathematical reference table used by the synthesizer to efficiently calculate sound waves.
2.  **Components:** Two main digital "devices" are instantiated:
    *   The `SynthSnare`: This component is the sound generator. It mathematically creates the unique waveform of a snare drum hit.
    *   The `AudioOutputPulse`: This component acts as the connection to the computer's speakers, taking the digital sound data and piping it out in real time.
3.  **The Loop:** The program enters an infinite cycle that mimics a real-time clock. In this cycle:
    *   It operates based on small time slices (samples).
    *   Once every second, the program instructs the `SynthSnare` to "hit" the drum.
    *   For every tiny time slice, the Snare generator creates a piece of the resulting sound wave, and this piece is instantly handed off to the `AudioOutputPulse` device, which plays it immediately.

The result is a continuous, steady snare drum beat playing endlessly until the program is manually stopped.

sha: c070a1640faf9d7c9848cb5dca4733c21193d1acd2a9e572259badb212a9273e 
*/
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

