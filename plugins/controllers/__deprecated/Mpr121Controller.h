/** Description:
This C++ header file defines the software framework necessary to control the **MPR121 capacitive touch sensor chip**. This controller is specialized for environments like the Raspberry Pi, relying on the `PIGPIO` library for direct hardware access.

### Core Function and Purpose

The primary goal of this controller is to act as a crucial link between the physical touch sensor (which detects proximity or contact on up to 12 inputs) and the main software application. It interprets physical contact events as digital key presses or releases, making the touch sensor behave like a standard keypad.

### How It Works

The system consists of two main parts:

1.  **The Sensor Handler (`Mpr121`):** This internal module handles all direct communication with a single touch chip using the I2C protocol—a common method for microchips to exchange data. It first initializes the chip by setting crucial parameters, such as the sensitivity thresholds required to register a touch or release. A dedicated, continuous monitoring process (running in its own thread) constantly checks the status of the chip's 12 inputs.
2.  **The System Controller (`Mpr121Controller`):** This is the main interface that the rest of the application interacts with. It allows multiple MPR121 chips to be configured by specifying their unique hardware addresses. When the sensor handler detects a change (e.g., a new touch or a finger lift), the System Controller receives this event and notifies the application exactly which key was affected.

In short, the code continuously monitors the hardware, compares the current touch status against the last known status, and translates any difference into actionable events for the program.

sha: 0510436849227c5c30f86c4df258b5092303b03222dc7f2ef57d150fdf91bb05 
*/
#ifndef _MPR121_CONTROLLER_H_
#define _MPR121_CONTROLLER_H_

#include "controllerInterface.h"

#include "keypadInterface.h"

#ifdef PIGPIO
// sudo apt-get install libpigpio-dev
#include <pigpio.h>
#endif

#include <cstring>
#include <stdio.h>
#include <thread>

#define MPR121_TOUCH_THRESHOLD_DEFAULT 12
#define MPR121_RELEASE_THRESHOLD_DEFAULT 6

/*md
## Mpr121Controller

Mpr121Controller is a controller for MPR121 capacitive touch sensor.

Those sensors used i2c bus. To be able to use it, you need to install pigpio library:

```sh
sudo apt-get install libpigpio-dev
```

You will need to activate the i2c interface in your rasberry pi using raspi-config.

> [!TIP]
> You can use i2cdetect to find the address of your i2c device.
> `sudo apt-get install i2c-tools`
> Use the command `i2cdetect -y 1` to list the addresses.
*/

enum {
    MPR121_TOUCHSTATUS_L = 0x00,
    MPR121_TOUCHSTATUS_H = 0x01,
    MPR121_FILTDATA_0L = 0x04,
    MPR121_FILTDATA_0H = 0x05,
    MPR121_BASELINE_0 = 0x1E,
    MPR121_MHDR = 0x2B,
    MPR121_NHDR = 0x2C,
    MPR121_NCLR = 0x2D,
    MPR121_FDLR = 0x2E,
    MPR121_MHDF = 0x2F,
    MPR121_NHDF = 0x30,
    MPR121_NCLF = 0x31,
    MPR121_FDLF = 0x32,
    MPR121_NHDT = 0x33,
    MPR121_NCLT = 0x34,
    MPR121_FDLT = 0x35,

    MPR121_TOUCHTH_0 = 0x41,
    MPR121_RELEASETH_0 = 0x42,
    MPR121_DEBOUNCE = 0x5B,
    MPR121_CONFIG1 = 0x5C,
    MPR121_CONFIG2 = 0x5D,
    MPR121_CHARGECURR_0 = 0x5F,
    MPR121_CHARGETIME_1 = 0x6C,
    MPR121_ECR = 0x5E,
    MPR121_AUTOCONFIG0 = 0x7B,
    MPR121_AUTOCONFIG1 = 0x7C,
    MPR121_UPLIMIT = 0x7D,
    MPR121_LOWLIMIT = 0x7E,
    MPR121_TARGETLIMIT = 0x7F,

    MPR121_GPIODIR = 0x76,
    MPR121_GPIOEN = 0x77,
    MPR121_GPIOSET = 0x78,
    MPR121_GPIOCLR = 0x79,
    MPR121_GPIOTOGGLE = 0x7A,

    MPR121_SOFTRESET = 0x80,
};

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

class Mpr121 {
protected:
    int i2c;
    int address;
    int start;
    ControllerInterface* controller;

    std::thread loopThread;
    bool loopRunning = true;

    void setThresholds(uint8_t touch, uint8_t release)
    {
#ifdef PIGPIO
        for (uint8_t i = 0; i < 12; i++) {
            i2cWriteByteData(i2c, MPR121_TOUCHTH_0 + 2 * i, touch);
            i2cWriteByteData(i2c, MPR121_RELEASETH_0 + 2 * i, release);
        }
#endif
    }

public:
    bool hasColor = true;

    Mpr121(ControllerInterface* _controller, int address, int index)
        : address(address)
        , controller(_controller)
        , start(index * 12)
    {
        printf("[Mpr121] address: %x start: %d\n", address, start);

#ifdef PIGPIO
        i2c = i2cOpen(1, address, 0);

        i2cWriteByteData(i2c, MPR121_SOFTRESET, 0x63);
        // delay ?
        i2cWriteByteData(i2c, MPR121_ECR, 0x0);

        uint8_t c = i2cReadByteData(i2c, MPR121_CONFIG2);
        if (c != 0x24) {
            return;
        }

        setThresholds(MPR121_TOUCH_THRESHOLD_DEFAULT, MPR121_RELEASE_THRESHOLD_DEFAULT);
        i2cWriteByteData(i2c, MPR121_MHDR, 0x01);
        i2cWriteByteData(i2c, MPR121_NHDR, 0x01);
        i2cWriteByteData(i2c, MPR121_NCLR, 0x0E);
        i2cWriteByteData(i2c, MPR121_FDLR, 0x00);

        i2cWriteByteData(i2c, MPR121_MHDF, 0x01);
        i2cWriteByteData(i2c, MPR121_NHDF, 0x05);
        i2cWriteByteData(i2c, MPR121_NCLF, 0x01);
        i2cWriteByteData(i2c, MPR121_FDLF, 0x00);

        i2cWriteByteData(i2c, MPR121_NHDT, 0x00);
        i2cWriteByteData(i2c, MPR121_NCLT, 0x00);
        i2cWriteByteData(i2c, MPR121_FDLT, 0x00);

        i2cWriteByteData(i2c, MPR121_DEBOUNCE, 0);
        i2cWriteByteData(i2c, MPR121_CONFIG1, 0x10); // default, 16uA charge current
        i2cWriteByteData(i2c, MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period

        i2cWriteByteData(i2c, MPR121_ECR, 0x8F); // start with first 5 bits of baseline tracking
#endif

        printf("[Mpr121] %x initialized\n", address);
        loopThread = std::thread(&Mpr121::loop, this);
        pthread_setname_np(loopThread.native_handle(), "mpr121");
    }

    ~Mpr121()
    {
        printf("[Mpr121] %x destroy\n", address);
        loopRunning = false;
#ifdef PIGPIO
        i2cClose(i2c);
#endif
    }

    uint16_t touched(void)
    {
#ifdef PIGPIO
        uint16_t t = i2cReadWordData(i2c, MPR121_TOUCHSTATUS_L);
        return t & 0x0FFF;
#else
        return 0;
#endif
    }

    uint16_t filteredData(uint8_t t)
    {
#ifdef PIGPIO
        if (t > 12)
            return 0;
        return i2cReadWordData(i2c, MPR121_FILTDATA_0L + t * 2);
#else
        return 0;
#endif
    }

    uint16_t lasttouched = 0;
    uint16_t currtouched = 0;
    void loop()
    {
        printf("[Mpr121] %x start loop\n", address);
        while (loopRunning) {
            currtouched = touched();
            for (uint8_t i = 0; i < 12; i++) {
                // it if *is* touched and *wasnt* touched before, alert!
                if ((currtouched & _BV(i)) && !(lasttouched & _BV(i))) {
                    // uint16_t value = filteredData(i);
                    // printf("[%x] %d touched: %d\n", address, i, value);
                    controller->onKey(controller->id, start + i, 1);
                    // printf("[%d] %d touched\n", controller->id, start + i);
                }
                // if it *was* touched and now *isnt*, alert!
                if (!(currtouched & _BV(i)) && (lasttouched & _BV(i))) {
                    // printf("[%x] %d released\n", address, i);
                    controller->onKey(controller->id, start + i, 0);
                    // printf("[%d] %d released\n", controller->id, start + i);
                }
            }

            // reset our state
            lasttouched = currtouched;
        }
    }
};

class Mpr121Controller : public KeypadInterface {
protected:
    int count = 0;

public:
    Mpr121Controller(Props& props, uint16_t id)
        : KeypadInterface(props, id)
    {
#ifdef PIGPIO
        gpioCfgSetInternals(PI_CFG_NOSIGHANDLER);
        if (gpioInitialise() < 0) {
            printf(".......................Failed to initialise GPIO\n");
        } else {
            printf("[Mpr121] GPIO initialised\n");
        }
#endif
    }

    /*md **Config:** */
    bool config(char* key, char* params)
    {
        /*md - `ADDRESS: 0x5c` will connect to i2c device at address `0x5c` and watch all touch inputs. This can be called multiple times to instantiate multiple i2c devices */
        if (strcmp(key, "ADDRESS") == 0) {
            char* p;
            int address = strtoul(strtok(params, " "), &p, 16);
            new Mpr121(this, address, count++);
        }
        return false;
    }

    void setButton(int id, uint8_t color)
    {
        // Do nothing has there is no color...
    }
};

#endif