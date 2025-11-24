/** Description:
This C++ program functions as a focused test environment for essential digital audio components. It simulates a tiny, self-contained music production chain designed to continuously output sound.

The setup begins by defining crucial properties for the audio system, such as the sound quality (sample rate) and the number of audio tracks available.

It then initializes three main components:
1.  **The Output Mixer:** This module is responsible for connecting the generated digital sound directly to your computer's speaker system (using the standardized PulseAudio system).
2.  **The Digital Instrument (Synth):** This acts as a sound generator, specifically configured here to play a sampled drum sound. It is preset with specific start and end points of the sample to control the sound's length.
3.  **The Effect Pedal (Distortion):** This module is designed to modify the audio, configured with specific settings for drive, compression, and tone control, though it is bypassed in the final running test sequence.

The core of the program is an endless loop. In this cycle, the system continually asks the Digital Instrument to generate tiny segments of sound data. Crucially, once every second, the program triggers a "note" event, causing the drum sound to play again. This sound data is immediately sent to the Output Mixer, resulting in a continuous, rhythmic output of the drum sample through your speakers.

sha: 96baf0dab6319bfd4fe0023f0ac4ebb75a8a663f0752f0c6ccbb6f8352c951a2 
*/
#include "plugins/audio/AudioOutputPulse.h"
#include "plugins/audio/SynthDrumSample.h"
#include "plugins/audio/EffectDistortion2.h"
#include "audio/lookupTable.h"

#include <string>
#include <sstream>

#define MAX_TRACKS 16

// g++ plugins/audio/SynthDrumSample.test.cpp -o test -I./ -pthread -D_REENTRANT -lpulse-simple -lpulse -I/usr/include/opus -I/usr/include/x86_64-linux-gnu -lsndfile  && ./test


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
    SynthDrumSample synth(props, (char*)"zicSynth");
    hydate(synth, R"(
START 0.000000
END 93.000000
BROWSER 16.000000)");

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
            synth.noteOn(60, 1.0f);
        }
        synth.sample(buffer);
        // distortion2.sample(buffer);
        audioOutput.sample(buffer);
        count++;
    }

    return 0;
}

