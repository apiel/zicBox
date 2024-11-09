#ifndef _HELPER_I2C_H_
#define _HELPER_I2C_H_

#include <string>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdint>

// i2c can also be used with pigpio
//
// #include <pigpio.h>
// i2cOpen(1, address, 0);
// i2cWriteByteData(i2c, MPR121_SOFTRESET, 0x63);

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

    uint8_t send(uint8_t* ptr, int16_t len)
    {
        if (file == 0 || ptr == 0 || len <= 0)
            return 1;

        write(file, ptr, len);
        return 0;
    }

    uint8_t pull(uint8_t* ptr, int16_t len)
    {
        if (file == 0 || ptr == 0 || len <= 0)
            return 1;

        read(file, ptr, len);
        return 0;
    }
};


#endif