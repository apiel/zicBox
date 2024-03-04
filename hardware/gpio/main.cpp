#include "def.h"
#include "RotaryEncoder.h"
#include "Button.h"

int main(int argc, char** argv)
{
    printf("Start OSC encoder.\n");

#ifdef PIGPIO
    if (gpioInitialise() < 0) {
        printf("Failed to initialise GPIO\n");
        return 1;
    }
#endif

    if (argc == 2 && strcmp(argv[1], "--debug") == 0) {
        printf("Debug mode\n");
        debug = true;
    }

    if (!oscClient) {
        printf("Failed to create OSC client\n");
        return 1;
    }
    printf("Initialized OSC client on port 8888\n");

    RotaryEncoder encoders[ENCODER_COUNT] = {
        { 20, 19, 0 },
        { 13, 6, 1 },
        { 5, 7, 2 },
        { 8, 11, 3 },
        { 21, 26, 4 },
        { 9, 10, 5 },
        { 27, 17, 6 },
        { 22, 4, 7 },
        { 16, 12, 8 },
        { 25, 24, 9 },
        { 23, 18, 10 },
        { 15, 14, 11 },
    };

    Button buttons[BUTTON_COUNT] = {
        // {26, 0},
        // {19, 1},
        // {13, 2},
        // {6, 3},
        // {5, 4},
        // {22, 5},
        // {27, 6},
        // {17, 7},
        // {4, 8},
    };

    while (1) {
        usleep(1000);
    }

    return 0;
}

// g++ -std=c++11 main.cpp
