/** Description:
This C++ header defines a structured tool, called the `I2c` class, specifically designed for controlling external hardware components using the I2C (Inter-Integrated Circuit) protocol on a Linux-based system.

I2C is a standard communication method used by microcontrollers to communicate with peripherals like sensors or small screens. This class simplifies the complex system calls required for this interaction.

**How It Works:**

1.  **Connection Setup (`init`):** The program first calls the `init` function, which acts like plugging in the hardware. It opens the specific I2C digital channel (bus) on the system and assigns the unique address of the target device. This ensures the computer is ready to speak only to that specific peripheral.
2.  **Writing Data (`writeReg`):** To change a setting or send a command, the `writeReg` function is used. This sends a desired value to a specific internal memory location (called a register) within the external device.
3.  **Reading Data (`readReg`):** To check the status or retrieve sensor information, `readReg` first tells the device which register to look at, and then reads the value sent back from that location.
4.  **General Transfer (`send`/`pull`):** These functions handle sending or receiving larger chunks of data, often used for quickly updating displays.
5.  **Shutdown (`end`):** When communication is complete, the `end` function safely closes the communication channel, ensuring system resources are released.

Essentially, this class provides a reliable, high-level interface to manage low-level hardware communication on the I2C digital highway.

sha: 0824118cb813dbae589a75414b04c6ff6287d47dcecd7f46185e397b2225e1a6 
*/
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
