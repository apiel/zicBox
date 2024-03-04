#include "Button.h"
#include "RotaryEncoder.h"
#include "Mpr121.h"
#include "def.h"

#include "../../dustscript/dustscript.h"

void configCallback(char* key, char* value, const char* filename)
{
    if (strcmp(key, "print") == 0) {
        printf(">> LOG: %s\n", value);
    } else if (strcmp(key, "debug") == 0) {
        debug = (strcmp(value, "true") == 0);
    } else if (strcmp(key, "ROTARY_ENCODER") == 0) {
        int gpioA = atoi(strtok(value, " "));
        int gpioB = atoi(strtok(NULL, " "));
        int encoderId = atoi(strtok(NULL, " "));
        new RotaryEncoder(gpioA, gpioB, encoderId);
    } else if (strcmp(key, "BUTTON") == 0) {
        int gpio = atoi(strtok(value, " "));
        int buttonId = atoi(strtok(NULL, " "));
        new Button(gpio, buttonId);
    } else if (strcmp(key, "MPR121") == 0) {
        char * p;
        int address = strtoul(strtok(value, " "), &p, 16);
        new Mpr121(address);
    }
}

int main(int argc, char** argv)
{
    printf("Start OSC encoder.\n");

#ifdef PIGPIO
    if (gpioInitialise() < 0) {
        printf("Failed to initialise GPIO\n");
        return 1;
    }
#endif

    if (!oscClient) {
        printf("Failed to create OSC client\n");
        return 1;
    }
    printf("Initialized OSC client on port 8888\n");

    DustScript::load(
        argc >= 2 ? argv[1] : "config.cfg",
        [&](char* key, char* value, const char* filename, uint8_t indentation, DustScript& instance) {
            configCallback(key, value, filename);
        });

    while (1) {
        usleep(1000);
    }

    return 0;
}

// g++ -std=c++11 main.cpp
