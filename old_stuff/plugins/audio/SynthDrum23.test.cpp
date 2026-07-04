/** Description:
This C++ program functions as a minimalist, self-contained audio workstation designed to continuously generate and process sound.

The core idea is to simulate a signal flow found in professional audio production, using specialized software modules referred to here as "plugins."

### The Audio Pipeline

1.  **Setup and Configuration:** The program first establishes standard audio settings, such as playing 44,100 sound slices (samples) per second (CD quality) across two channels (stereo).
2.  **Sound Generation:** A specialized component, the **SynthDrum23** plugin, is initialized. This component acts as the sound source, configured with detailed parameters defining its waveform, pitch, duration, and loudness decay (envelope).
3.  **Sound Processing:** The generated sound immediately flows into the **EffectDistortion2** plugin. This module is configured to modify the sound, adding attributes like drive, compression, and wave shaping, resulting in a distorted or crunchy texture.
4.  **Output:** Finally, the processed sound is handed off to the **AudioOutputPulse** component, which connects the data stream to the computer's speakers or headphones.

### Operation

The application runs in an infinite loop, constantly cycling through the audio chain one sample at a time. A built-in counter ensures that the drum sound is triggered automatically and reliably exactly once every second. Thus, the program continuously plays a repeating, distorted drum hit.

sha: e597a3bfe837a73f6119e027a8263a124ca522af76a58775d365da1cfde064d7 
*/
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

