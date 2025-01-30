#ifndef _HELPER_GPIO_ENCODER_H_
#define _HELPER_GPIO_ENCODER_H_

#include "helpers/gpio.h"

#include <functional>
#include <stdio.h>
#include <thread>
#include <vector>

using namespace std;

#ifdef DEBUG_GPIO_ENCODER
#define LOG_GPIO_ENCODER(...) printf(__VA_ARGS__)
#else
#define LOG_GPIO_ENCODER(...)
#endif

class GpioEncoder {
public:
    struct Encoder {
        int id;
        uint8_t gpioA;
        uint8_t gpioB;
        int levelA = 0;
        int levelB = 0;
        int lastGpio = -1;
    };
    std::vector<Encoder> encoders;

protected:
    std::thread loopThread;
    bool loopRunning = true;
    std::function<void(Encoder, int8_t)> onEncoder;

public:
    GpioEncoder(std::vector<Encoder> encoders, std::function<void(Encoder, int8_t)> onEncoder)
        : encoders(encoders)
        , onEncoder(onEncoder)
    {
    }

    void startThread()
    {
        loopThread = std::thread(&GpioEncoder::loop, this);
        pthread_setname_np(loopThread.native_handle(), "gpioEncoder");
    }

    ~GpioEncoder()
    {
        loopRunning = false;
    }

    void initEncoders()
    {
        for (auto& encoder : encoders) {
            gpioSetMode(encoder.gpioA, GPIO_INPUT);
            gpioSetPullUp(encoder.gpioA);
            encoder.levelA = gpioRead(encoder.gpioA);
            gpioSetMode(encoder.gpioB, GPIO_INPUT);
            gpioSetPullUp(encoder.gpioB);
            encoder.levelB = gpioRead(encoder.gpioB);
        }
    }

    int init()
    {
        if (initGpio() == -1) {
            return -1;
        }
        initEncoders();
        return 0;
    }

    void loop()
    {
        while (loopRunning) {
            handler();
            std::this_thread::sleep_for(1ms);
        }
    }

    void handler()
    {
        for (auto& encoder : encoders) {
            int levelA = gpioRead(encoder.gpioA);
            if (levelA != encoder.levelA) {
                encoder.levelA = levelA;
                if (encoder.lastGpio != encoder.gpioA) {
                    encoder.lastGpio = encoder.gpioA;
                    if (levelA && encoder.levelB) {
                        onEncoder(encoder, -1);
                    }
                }
            }
            int levelB = gpioRead(encoder.gpioB);
            if (levelB != encoder.levelB) {
                encoder.levelB = levelB;
                if (encoder.lastGpio != encoder.gpioB) {
                    encoder.lastGpio = encoder.gpioB;
                    if (levelB && encoder.levelA) {
                        onEncoder(encoder, 1);
                    }
                }
            }
            LOG_GPIO_ENCODER("[%d %d %d %d] ", encoder.id, encoder.gpioA, encoder.gpioB, encoder.lastGpio);
        }
    }
};

#endif