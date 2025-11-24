/** Description:
This C++ program serves as a real-time test harness designed to confirm that different software components of an audio system can function correctly together.

The code first establishes the fundamental rules for sound handling, such as setting the quality (sample rate) and ensuring the output is stereo (two channels). It then initializes three main modules: a Synthesizer (`SynthSample`) responsible for generating the sound waves, a specific Sound Effect (`EffectDistortion2`), and an Audio Output component (`AudioOutputPulse`) that sends the generated sound to the computer's speakers using the PulseAudio system.

Before playback starts, configuration details—like the synthesizer's rhythmic timing or the desired intensity of the distortion effect—are loaded into these modules.

The core functionality operates within an endless loop that simulates continuous audio playback. On a precise schedule (every three seconds), the program instructs the Synthesizer to play a specific musical note. The sound data generated is instantly collected and passed directly to the Audio Output module, ensuring immediate, real-time playback. The overall goal is to demonstrate successful, continuous audio synthesis and output.

sha: 42467626fba05d8b14f7f6566a29996c419ce5868919efe0433c20b0bfa4901b 
*/
#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/SynthSample.h"
#include "plugins/audio/EffectDistortion2.h"
#include "audio/lookupTable.h"

#include <string>
#include <sstream>

#define MAX_TRACKS 16

// g++ plugins/audio/SynthSample.test.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -I/usr/include/opus -I/usr/include/x86_64-linux-gnu -lsndfile  && ./test


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
    SynthSample synth(props, (char*)"zicSynth");
    hydate(synth, R"(
START 0.000000
END 100.000000
DENSITY 16.000000
#DENSITY_RANDOMIZE 25.000000
#DENSITY_DELAY 500.000000
BROWSER 1.000000)");

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

