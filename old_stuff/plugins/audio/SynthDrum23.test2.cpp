/** Description:
This C++ program functions as a dedicated test bench for a specific digital audio component: the `SynthDrum23`, which is a software-based drum sound generator.

The process establishes a complete virtual audio pipeline. First, the program defines the required characteristics for the sound, such as the sample rate (determining the sound quality) and the number of channels (like stereo).

It then initializes two essential virtual components: the `AudioOutputPulse` object, responsible for sending the generated sound waves to the computer's speakers (using the standard Linux PulseAudio system), and the drum synthesizer itself.

A critical phase is the detailed configuration of the synthesizer. Using a set of parameters, the code "customizes" the drum sound by defining its pitch, overall duration, and intricate envelope curves. These envelopes dictate precisely how the volume and frequency of the sound change and decay over time, shaping the final acoustic character of the drum hit.

Once configured, the program enters an infinite loop. In this loop, it runs the core audio processing: every second, it instructs the synthesizer to trigger a new drum note. The synthesizer calculates the sound data in small chunks, and that resulting digital wave information is immediately streamed to the audio output system, producing a steady, continuous beat for demonstration purposes.

sha: 9d3b4d8f9c71714de39251c7486c361260adae2c9e524e7ec27fdaba1c1e4c65 
*/
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

