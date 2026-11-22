#include "audio/engines/Acid32.h"
#include <iostream>
#include <cassert>
#include <cmath>

int main()
{
    Acid32 acid(44100.0f);
    std::cout << "Testing Acid32 initial state...\n";

    // Play a note
    acid.noteOn(36, 0.9f, false, true); // Accent C2
    float maxSample = 0.0f;
    for (int i = 0; i < 44100; ++i) {
        float s = acid.sample();
        if (std::abs(s) > maxSample) {
            maxSample = std::abs(s);
        }
    }
    std::cout << "Max output sample rendered: " << maxSample << "\n";
    assert(maxSample > 0.05f && maxSample <= 1.5f);

    // Test slide legato
    std::cout << "Testing legato slide...\n";
    acid.noteOn(36, 0.8f, false, false);
    for (int i = 0; i < 500; ++i) acid.sample();
    acid.noteOn(48, 0.8f, true, true); // Slide to C3
    for (int i = 0; i < 1000; ++i) acid.sample();

    acid.noteOff();
    for (int i = 0; i < 20000; ++i) acid.sample();

    std::cout << "Acid32 DSP test passed successfully!\n";
    return 0;
}
