#ifndef _MPR121_H
#define _MPR121_H

#include "def.h"

#define MPR121_TOUCH_THRESHOLD_DEFAULT 12
#define MPR121_RELEASE_THRESHOLD_DEFAULT 6

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

class Mpr121 {
protected:
    int i2c;

    void setThresholds(uint8_t touch, uint8_t release)
    {
#ifdef PIGPIO
        for (uint8_t i = 0; i < 12; i++) {
            i2cWriteByteData(MPR121_TOUCHTH_0 + 2 * i, touch);
            i2cWriteByteData(MPR121_RELEASETH_0 + 2 * i, release);
        }
#endif
    }

public:
    Mpr121(int address)
    {
        printf("[Mpr121] address: %x\n", address);

#ifdef PIGPIO
        i2c = i2cOpen(1, address, 0);

        i2cWriteByteData(MPR121_SOFTRESET, 0x63);
        // delay ?
        i2cWriteByteData(MPR121_ECR, 0x0);

        uint8_t c = i2cReadByteData(MPR121_CONFIG2);
        if (c != 0x24) {
            return;
        }

        setThresholds(MPR121_TOUCH_THRESHOLD_DEFAULT, MPR121_RELEASE_THRESHOLD_DEFAULT);
        i2cWriteByteData(MPR121_MHDR, 0x01);
        i2cWriteByteData(MPR121_NHDR, 0x01);
        i2cWriteByteData(MPR121_NCLR, 0x0E);
        i2cWriteByteData(MPR121_FDLR, 0x00);

        i2cWriteByteData(MPR121_MHDF, 0x01);
        i2cWriteByteData(MPR121_NHDF, 0x05);
        i2cWriteByteData(MPR121_NCLF, 0x01);
        i2cWriteByteData(MPR121_FDLF, 0x00);

        i2cWriteByteData(MPR121_NHDT, 0x00);
        i2cWriteByteData(MPR121_NCLT, 0x00);
        i2cWriteByteData(MPR121_FDLT, 0x00);

        i2cWriteByteData(MPR121_DEBOUNCE, 0);
        i2cWriteByteData(MPR121_CONFIG1, 0x10); // default, 16uA charge current
        i2cWriteByteData(MPR121_CONFIG2, 0x20); // 0.5uS encoding, 1ms period


        i2cWriteByteData(MPR121_ECR, 0x8F); // start with first 5 bits of baseline tracking
#endif

        printf("[Mpr121] %x initialized\n", address);
    }

    ~Mpr121()
    {
#ifdef PIGPIO
        i2cClose(i2c);
#endif
    }
};

#endif

// i2cWriteByteData(MPR121_MHDR, 0x5B);
// i2cWriteByteData(MPR121_NHDR, 0x5B);
// i2cWriteByteData(MPR121_NCLR, 0x5A);
// i2cWriteByteData(MPR121_FDLR, 0x00);
// i2cWriteByteData(MPR121_MHDF, 0x5B);
// i2cWriteByteData(MPR121_NHDF, 0x5B);
// i2cWriteByteData(MPR121_NCLF, 0x5A);
// i2cWriteByteData(MPR121_FDLF, 0x00);
// i2cWriteByteData(MPR121_NHDT, 0x5B);
// i2cWriteByteData(MPR121_NCLT, 0x5A);
// i2cWriteByteData(MPR121_FDLT, 0x00);