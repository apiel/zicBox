/** Description:
This C++ program serves a very specific function: it generates and continuously streams random noise through the computer's audio system.

**How It Works:**

1.  **Tool Selection:** The program relies on a specialized component designed to communicate with **PulseAudio**, the system software that manages sound output on many operating systems. It initiates an audio connection, essentially opening a dedicated channel on your computer's virtual sound mixer labeled "zicAudioOutputPulse."

2.  **The Audio Generator:** The core of the program is an endless loop. In each very brief iteration of this loop, the program performs two critical steps:
    *   **Random Data Creation:** It calculates a completely random numerical value between zero and one. This single number represents the momentary volume or amplitude—a small "sample" of audio data.
    *   **Immediate Output:** This random audio sample is instantly packaged and sent to the PulseAudio system.

3.  **The Result:** Because the program is rapidly feeding the sound card a stream of uncorrelated, random volume levels, the audible result is continuous static, hiss, or white noise that plays indefinitely until the program is manually terminated. It functions purely as a simple, random sound generator.

sha: 2a99a46eb8d2d3ded34068a145f9c26b6480375b0045a2798cb516dc46e67f34 
*/
// g++ 01.cpp -o 01.bin -I../../.. -lpulse-simple -lpulse && ./01.bin

#include "plugins/audio/AudioOutputPulse.h"

int main(int argc, char* argv[])
{
    AudioPlugin::Props props = defaultAudioProps;
    AudioOutputPulse audioOutput(props, (char*)"zicAudioOutputPulse");

    float buffer[props.maxTracks] = { 0.0f };

    while (1) {
        buffer[0] = rand() / (float)RAND_MAX;
        audioOutput.sample(buffer);
    }

    return 0;
}