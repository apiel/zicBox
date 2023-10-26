#ifdef PIGPIO
// sudo apt-get install libpigpio-dev
#include <pigpio.h>
#endif

#include <lo/lo.h>

#include <stdint.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <cstring>

#define ENCODER_COUNT 8
#define BUTTON_COUNT 9

lo_address oscClient = lo_address_new(NULL, "8888");
bool debug = false;

class RotaryEncoder
{
protected:
    int levelA = 0;
    int levelB = 0;
    int lastGpio = -1;

    static void pulseEx(int gpio, int level, uint32_t tick, void *user)
    {
        RotaryEncoder *self = (RotaryEncoder *)user;
        self->pulse(gpio, level, tick);
    }

    void send(uint8_t direction)
    {
        if (debug)
        {
            printf("[encoderId %d] send: %d\n", encoderId, direction);
        }
        lo_send(oscClient, "/encoder", "ii", encoderId, direction);
    }

public:
    // void (*midiHandler)(std::vector<unsigned char>* message) = NULL;
    int gpioA;
    int gpioB;
    uint8_t encoderId;

    RotaryEncoder(int _gpioA, int _gpioB, uint8_t _encoderId)
        : gpioA(_gpioA), gpioB(_gpioB), encoderId(_encoderId)
    {
        printf("[encoderId: %d] gpioA: %d, gpioB: %d\n", encoderId, gpioA, gpioB);

#ifdef PIGPIO
        gpioSetMode(gpioA, PI_INPUT);
        gpioSetMode(gpioB, PI_INPUT);
        gpioSetPullUpDown(gpioA, PI_PUD_UP);
        gpioSetPullUpDown(gpioB, PI_PUD_UP);
        gpioSetAlertFuncEx(gpioA, pulseEx, this);
        gpioSetAlertFuncEx(gpioB, pulseEx, this);
#endif
    }

    void pulse(int gpio, int level, uint32_t tick)
    {
        if (gpio == gpioA)
        {
            levelA = level;
        }
        else
        {
            levelB = level;
        }

        if (gpio != lastGpio) /* debounce */
        {
            lastGpio = gpio;

            if ((gpio == gpioA) && (level == 1))
            {
                if (levelB)
                {
                    send(-1);
                }
            }
            else if ((gpio == gpioB) && (level == 1))
            {
                if (levelA)
                {
                    send(1);
                }
            }
        }
    }
};

class Button
{
protected:
    static void pulseEx(int gpio, int level, uint32_t tick, void *user)
    {
        Button *self = (Button *)user;
        self->pulse(gpio, level, tick);
    }

    void send(uint8_t direction, uint32_t tick)
    {
        if (debug)
        {
            printf("[buttomId %d] send: %d\n", buttomId, direction);
        }
        lo_send(oscClient, "/button", "iii", buttomId, direction, tick);
    }

public:
    int gpio;
    uint8_t buttomId;

    Button(int _gpio, uint8_t _buttomId)
        : gpio(_gpio), buttomId(_buttomId)
    {
        printf("[buttomId %d] gpio: %d\n", buttomId, gpio);

#ifdef PIGPIO
        gpioSetMode(gpio, PI_INPUT);
        gpioSetPullUpDown(gpio, PI_PUD_UP);
        gpioSetAlertFuncEx(gpio, pulseEx, this);
#endif
    }

    void pulse(int gpio, int level, uint32_t tick)
    {
        send(level, tick);
    }
};

int main(int argc, char **argv)
{
    printf("Start OSC encoder.\n");

#ifdef PIGPIO
    if (gpioInitialise() < 0)
    {
        printf("Failed to initialise GPIO\n");
        return 1;
    }
#endif

    if (argc == 2 && strcmp(argv[1], "--debug") == 0)
    {
        printf("Debug mode\n");
        debug = true;
    }

    if (!oscClient)
    {
        printf("Failed to create OSC client\n");
        return 1;
    }
    printf("Initialized OSC client on port 8888\n");

    RotaryEncoder encoders[ENCODER_COUNT] = {
        {1, 7, 0},
        {11, 8, 1},
        {25, 9, 2},
        {10, 24, 3},
        {21, 20, 4},
        {16, 12, 5},
        {23, 18, 6},
        {15, 14, 7},
    };

    Button buttons[BUTTON_COUNT] = {
        {26, 0},
        {19, 1},
        {13, 2},
        {6, 3},
        {5, 4},
        {22, 5},
        {27, 6},
        {17, 7},
        {4, 8},
    };

    while (1)
    {
        usleep(1000);
    }

    return 0;
}

// g++ -std=c++11 main.cpp
