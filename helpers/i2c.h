#pragma once

#include <string>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdint>
#include <cstring>

class I2c {
public:
    int file = 0;

    bool init(int i2c, int dev_addr)
    {
        if (file == 0) {
            char filename[32];
            sprintf(filename, "/dev/i2c-%d", i2c); // I2C bus number passed
            file = open(filename, O_RDWR);
            if (file < 0) {
                file = 0;
                return false;
            }
            if (ioctl(file, I2C_SLAVE, dev_addr) < 0) { // set slave address
                close(file);
                file = 0;
                return false;
            }
            return true;
        }
        // assume done init already
        return true;
    }

    uint8_t end()
    {
        if (file != 0) {
            close(file);
            file = 0;
            return 0;
        }
        return 1;
    }

    uint8_t writeReg(uint8_t reg, uint8_t value)
    {
        if (file == 0)
            return 1;

        uint8_t buf[2] = {reg, value};
        if (write(file, buf, 2) != 2)
            return 1;

        return 0;
    }

    uint8_t readReg(uint8_t reg, uint8_t* value)
    {
        if (file == 0 || value == nullptr)
            return 1;

        if (write(file, &reg, 1) != 1)
            return 1;

        if (read(file, value, 1) != 1)
            return 1;

        return 0;
    }

    // For ssd1306, not used at the moment (might be deprecated at some point)
    uint8_t send(uint8_t* ptr, int16_t len)
    {
        if (file == 0 || ptr == 0 || len <= 0)
            return 1;

        write(file, ptr, len);
        return 0;
    }

    // For ssd1306 (might be deprecated at some point)
    uint8_t pull(uint8_t* ptr, int16_t len)
    {
        if (file == 0 || ptr == 0 || len <= 0)
            return 1;

        read(file, ptr, len);
        return 0;
    }
};
