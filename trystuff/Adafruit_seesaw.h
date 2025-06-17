#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <linux/i2c-dev.h>
#include <linux/i2c.h> // Required for i2c_msg and i2c_rdwr_ioctl_data structures
#include <thread>

#include <cstring> // For strerror
#include <fcntl.h>
#include <functional>
#include <iomanip> // For std::hex, std::setw, std::setfill
#include <iostream>
#include <linux/i2c-dev.h>
#include <linux/i2c.h> // Required for i2c_msg and i2c_rdwr_ioctl_data structures
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <vector>

typedef uint8_t byte;

// #define SEESAW_ADDRESS (0x49) ///< Default Seesaw I2C address

enum {
    SEESAW_STATUS_BASE = 0x00,
    SEESAW_GPIO_BASE = 0x01,
    SEESAW_SERCOM0_BASE = 0x02,

    SEESAW_TIMER_BASE = 0x08,
    SEESAW_ADC_BASE = 0x09,
    SEESAW_DAC_BASE = 0x0A,
    SEESAW_INTERRUPT_BASE = 0x0B,
    SEESAW_DAP_BASE = 0x0C,
    SEESAW_EEPROM_BASE = 0x0D,
    SEESAW_NEOPIXEL_BASE = 0x0E,
    SEESAW_TOUCH_BASE = 0x0F,
    SEESAW_KEYPAD_BASE = 0x10,
    SEESAW_ENCODER_BASE = 0x11,
    SEESAW_SPECTRUM_BASE = 0x12,
};

enum {
    SEESAW_GPIO_DIRSET_BULK = 0x02,
    SEESAW_GPIO_DIRCLR_BULK = 0x03,
    SEESAW_GPIO_BULK = 0x04,
    SEESAW_GPIO_BULK_SET = 0x05,
    SEESAW_GPIO_BULK_CLR = 0x06,
    SEESAW_GPIO_BULK_TOGGLE = 0x07,
    SEESAW_GPIO_INTENSET = 0x08,
    SEESAW_GPIO_INTENCLR = 0x09,
    SEESAW_GPIO_INTFLAG = 0x0A,
    SEESAW_GPIO_PULLENSET = 0x0B,
    SEESAW_GPIO_PULLENCLR = 0x0C,
};

enum {
    SEESAW_STATUS_HW_ID = 0x01,
    SEESAW_STATUS_VERSION = 0x02,
    SEESAW_STATUS_OPTIONS = 0x03,
    SEESAW_STATUS_TEMP = 0x04,
    SEESAW_STATUS_SWRST = 0x7F,
};

enum {
    SEESAW_TIMER_STATUS = 0x00,
    SEESAW_TIMER_PWM = 0x01,
    SEESAW_TIMER_FREQ = 0x02,
};

enum {
    SEESAW_ADC_STATUS = 0x00,
    SEESAW_ADC_INTEN = 0x02,
    SEESAW_ADC_INTENCLR = 0x03,
    SEESAW_ADC_WINMODE = 0x04,
    SEESAW_ADC_WINTHRESH = 0x05,
    SEESAW_ADC_CHANNEL_OFFSET = 0x07,
};

enum {
    SEESAW_SERCOM_STATUS = 0x00,
    SEESAW_SERCOM_INTEN = 0x02,
    SEESAW_SERCOM_INTENCLR = 0x03,
    SEESAW_SERCOM_BAUD = 0x04,
    SEESAW_SERCOM_DATA = 0x05,
};

enum {
    SEESAW_NEOPIXEL_STATUS = 0x00,
    SEESAW_NEOPIXEL_PIN = 0x01,
    SEESAW_NEOPIXEL_SPEED = 0x02,
    SEESAW_NEOPIXEL_BUF_LENGTH = 0x03,
    SEESAW_NEOPIXEL_BUF = 0x04,
    SEESAW_NEOPIXEL_SHOW = 0x05,
};

enum {
    SEESAW_TOUCH_CHANNEL_OFFSET = 0x10,
};

enum {
    SEESAW_KEYPAD_STATUS = 0x00,
    SEESAW_KEYPAD_EVENT = 0x01,
    SEESAW_KEYPAD_INTENSET = 0x02,
    SEESAW_KEYPAD_INTENCLR = 0x03,
    SEESAW_KEYPAD_COUNT = 0x04,
    SEESAW_KEYPAD_FIFO = 0x10,
};

enum {
    SEESAW_KEYPAD_EDGE_HIGH = 0,
    SEESAW_KEYPAD_EDGE_LOW,
    SEESAW_KEYPAD_EDGE_FALLING,
    SEESAW_KEYPAD_EDGE_RISING,
};

enum {
    SEESAW_ENCODER_STATUS = 0x00,
    SEESAW_ENCODER_INTENSET = 0x10,
    SEESAW_ENCODER_INTENCLR = 0x20,
    SEESAW_ENCODER_POSITION = 0x30,
    SEESAW_ENCODER_DELTA = 0x40,
};

enum {
    SEESAW_SPECTRUM_RESULTS_LOWER = 0x00, // Audio spectrum bins 0-31
    SEESAW_SPECTRUM_RESULTS_UPPER = 0x01, // Audio spectrum bins 32-63
    SEESAW_SPECTRUM_CHANNEL = 0xFD,
    SEESAW_SPECTRUM_RATE = 0xFE,
    SEESAW_SPECTRUM_STATUS = 0xFF,
};

#define ADC_INPUT_0_PIN 2 ///< default ADC input pin
#define ADC_INPUT_1_PIN 3 ///< default ADC input pin
#define ADC_INPUT_2_PIN 4 ///< default ADC input pin
#define ADC_INPUT_3_PIN 5 ///< default ADC input pin

#define PWM_0_PIN 4 ///< default PWM output pin
#define PWM_1_PIN 5 ///< default PWM output pin
#define PWM_2_PIN 6 ///< default PWM output pin
#define PWM_3_PIN 7 ///< default PWM output pin

#ifndef INPUT_PULLDOWN
#define INPUT_PULLDOWN \
    #endif

// clang-format off
#define SEESAW_HW_ID_CODE_SAMD09 0x55 ///< seesaw HW ID code for SAMD09
#define SEESAW_HW_ID_CODE_TINY806 0x84 ///< seesaw HW ID code for ATtiny806
#define SEESAW_HW_ID_CODE_TINY807 0x85 ///< seesaw HW ID code for ATtiny807
#define SEESAW_HW_ID_CODE_TINY816 0x86 ///< seesaw HW ID code for ATtiny816
#define SEESAW_HW_ID_CODE_TINY817 0x87 ///< seesaw HW ID code for ATtiny817
#define SEESAW_HW_ID_CODE_TINY1616 0x88 ///< seesaw HW ID code for ATtiny1616
#define SEESAW_HW_ID_CODE_TINY1617 0x89 ///< seesaw HW ID code for ATtiny1617
// clang-format on

union keyEventRaw {
    struct {
        uint8_t EDGE : 2; ///< the edge that was triggered
        uint8_t NUM : 6; ///< the event number
    } bit; ///< bitfield format
    uint8_t reg; ///< register format
};

union keyEvent {
    struct {
        uint8_t EDGE : 2; ///< the edge that was triggered
        uint16_t NUM : 14; ///< the event number
    } bit; ///< bitfield format
    uint16_t reg; ///< register format
};

union keyState {
    struct {
        uint8_t STATE : 1; ///< the current state of the key
        uint8_t ACTIVE : 4; ///< the registered events for that key
    } bit; ///< bitfield format
    uint8_t reg; ///< register format
};

class Adafruit_seesaw {
public:
    // constructors
    Adafruit_seesaw()
    {
        // if (i2c_bus == NULL) {
        //     _i2cbus = &Wire;
        // } else {
        //     _i2cbus = i2c_bus;
        // }
    }

    ~Adafruit_seesaw(void) { };

    int i2c_fd; // File descriptor for the I2C bus
    uint8_t address; // I2C address of the NeoTrellis

    void begin(const char* i2c_device = "/dev/i2c-1", uint8_t addr = 0x2E)
    {
        address = addr;

        i2c_fd = open(i2c_device, O_RDWR);
        if (i2c_fd < 0) {
            std::cerr << "Failed to open I2C device '" << i2c_device << "': " << strerror(errno) << std::endl;
            throw std::runtime_error("Failed to open I2C device");
        }
        std::cout << "Opened I2C device '" << i2c_device << "'" << std::endl;

        if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) {
            std::cerr << "Failed to set I2C slave address to 0x" << std::hex << (int)address << ": " << strerror(errno) << std::endl;
            close(i2c_fd);
            throw std::runtime_error("Failed to set I2C slave address");
        }

        std::cout << "Set I2C slave address to 0x" << std::hex << (int)address << std::endl;

        // Perform a software reset on the Seesaw chip
        std::cout << "Sending software reset..." << std::endl;
        try {
            // Software reset command: write 0xFF to STATUS_SWRST register
            // write_register(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, { 0xFF });
            this->write8(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, 0xFF);
            std::cout << "Software reset sent successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to send software reset: " << e.what() << std::endl;
            throw; // Re-throw to indicate critical failure
        }
    }

    uint32_t getOptions()
    {
        uint8_t buf[4];
        this->read(SEESAW_STATUS_BASE, SEESAW_STATUS_OPTIONS, buf, 4);
        uint32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
        return ret;
    }

    uint32_t getVersion()
    {
        uint8_t buf[4];
        this->read(SEESAW_STATUS_BASE, SEESAW_STATUS_VERSION, buf, 4);
        uint32_t ret = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | (uint32_t)buf[3];
        return ret;
    }

    bool getProdDatecode(uint16_t* pid, uint8_t* year, uint8_t* mon, uint8_t* day)
    {
        uint32_t vers = getVersion();
        *pid = vers >> 16;

        *year = vers & 0x3F;
        *mon = (vers >> 7) & 0xF;
        *day = (vers >> 11) & 0x1F;
        return true;
    }

    void setKeypadEvent(uint8_t key, uint8_t edge, bool enable = true)
    {
        keyState ks;
        ks.bit.STATE = enable;
        ks.bit.ACTIVE = (1 << edge);
        uint8_t cmd[] = { key, ks.reg };
        this->writeReg(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_EVENT, cmd, 2);
    }

    void enableKeypadInterrupt()
    {
        this->write8(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_INTENSET, 0x01);
    }

    void disableKeypadInterrupt()
    {
        this->write8(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_INTENCLR, 0x01);
    }

    uint8_t getKeypadCount()
    {
        return this->read8(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_COUNT, 500);
    }

    bool readKeypad(keyEventRaw* buf, uint8_t count)
    {
        return this->read(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_FIFO, (uint8_t*)buf, count, 1000);
    }

    // virtual size_t write(uint8_t character)
    // {
    //     this->write8(SEESAW_SERCOM0_BASE, SEESAW_SERCOM_DATA, character);
    //     return 1;
    // }

    // virtual size_t write(const char* str)address
    // {
    //     uint8_t buf[32];
    //     uint8_t len = 0;
    //     while (*str) {
    //         buf[len] = *str;
    //         str++;
    //         len++;
    //     }
    //     this->write(SEESAW_SERCOM0_BASE, SEESAW_SERCOM_DATA, buf, len);
    //     return len;
    // }

protected:
    bool write8(byte regHigh, byte regLow, byte value)
    {
        return this->writeReg(regHigh, regLow, &value, 1);
    }

    uint8_t read8(byte regHigh, byte regLow, uint16_t delay = 250)
    {
        uint8_t ret;
        this->read(regHigh, regLow, &ret, 1, delay);

        return ret;
    }

    // bool read(uint8_t regHigh, uint8_t regLow, uint8_t* buf, uint8_t length, uint16_t delay = 250)
    // {
    //     // uint8_t pos = 0;
    //     // uint8_t prefix[2];
    //     // prefix[0] = (uint8_t)regHigh;
    //     // prefix[1] = (uint8_t)regLow;

    //     // // on arduino we need to read in 32 byte chunks
    //     // while (pos < num) {
    //     //     uint8_t read_now = std::min(32, num - pos);

    //     //     if (!_i2c_dev->write(prefix, 2)) {
    //     //         return false;
    //     //     }

    //     //     std::this_thread::sleep_for(std::chrono::microseconds(delay));

    //     //     if (!_i2c_dev->read(buf + pos, read_now)) {
    //     //         return false;
    //     //     }
    //     //     pos += read_now;
    //     // }
    //     // return true;

    //     uint8_t reg_addr[2] = { regHigh, regLow };
    //     std::vector<uint8_t> data(length);
    //     struct i2c_msg msgs[2];
    //     msgs[0].addr = address;
    //     msgs[0].flags = 0; // 0 for write
    //     msgs[0].len = 2; // Command is 2 bytes (reg_high, reg_low)
    //     msgs[0].buf = reg_addr;

    //     // Message 1: Read the specified number of bytes
    //     msgs[1].addr = address;
    //     msgs[1].flags = I2C_M_RD; // I2C_M_RD for read
    //     msgs[1].len = length;
    //     msgs[1].buf = data.data();

    //     // Combine the messages into an i2c_rdwr_ioctl_data structure
    //     struct i2c_rdwr_ioctl_data ioctl_data;
    //     ioctl_data.msgs = msgs;
    //     ioctl_data.nmsgs = 2; // We are sending two messages in one atomic transaction

    //     // Perform the combined write-then-read I2C transaction
    //     if (ioctl(i2c_fd, I2C_RDWR, &ioctl_data) < 0) {
    //         std::cerr << "Failed to perform I2C_RDWR (read transaction): " << strerror(errno) << " (errno " << errno << ")" << std::endl;
    //         throw std::runtime_error("I2C read transaction failed");
    //         return false;
    //     }

    //     // Copy the read data into the buffer
    //     std::memcpy(buf, data.data(), length);
    //     return true;
    // }

    bool read(uint8_t regHigh,
        uint8_t regLow,
        uint8_t* buf,
        uint8_t length,
        uint16_t delay_us = 250)
    {
        uint8_t prefix[2] = { regHigh, regLow };
        size_t pos = 0;

        // Linux’ i2c‑dev normally allows up to 8192 bytes in one go,
        // but we imitate Arduino’s 32‑byte Wire buffer limit so the
        // logic stays identical and you won’t overflow small MCUs.
        constexpr size_t CHUNK_SIZE = 32;

        while (pos < length) {
            size_t to_read = std::min(CHUNK_SIZE, static_cast<size_t>(length - pos));

            /* -------- First: write the 2‑byte register address -------- */
            // This puts a STOP condition on the bus; that is *okay*
            // for most sensors because they only need the register
            // pointer set.  If yours *requires* a repeated‑start,
            // stick with I2C_RDWR at the end of this answer.
            if (::write(i2c_fd, prefix, sizeof(prefix)) != sizeof(prefix)) {
                std::cerr << "I2C register‑select write failed: "
                          << strerror(errno) << " (errno " << errno << ")\n";
                return false;
            }

            /* Optional tiny settling delay, mirroring Arduino code */
            std::this_thread::sleep_for(std::chrono::microseconds(delay_us));

            /* -------- Second: read the data -------- */
            ssize_t got = ::read(i2c_fd, buf + pos, to_read);
            if (got != static_cast<ssize_t>(to_read)) {
                std::cerr << "I2C read failed: "
                          << strerror(errno) << " (errno " << errno << ")\n";
                return false;
            }

            pos += to_read;

            /* For the *next* chunk we do NOT advance the register
               pointer again – most sensors auto‑increment it after
               each read.  Therefore we leave `prefix` untouched and
               simply issue the next read immediately. */
        }

        return true;
    }

    bool writeReg(uint8_t regHigh, uint8_t regLow, uint8_t* buf, uint8_t num)
    {
        // uint8_t prefix[2];
        // prefix[0] = (uint8_t)regHigh;
        // prefix[1] = (uint8_t)regLow;
        // if (!_i2c_dev->write(buf, num, true, prefix, 2)) {
        //     return false;
        // }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        std::vector<uint8_t> buffer;
        buffer.push_back(regHigh); // Seesaw module address
        buffer.push_back(regLow); // Seesaw function address
        buffer.insert(buffer.end(), buf, buf + num);

        ssize_t written = write(i2c_fd, buffer.data(), buffer.size());
        if (written != (ssize_t)buffer.size()) {
            std::cerr << "I2C write failed: wrote " << written << " bytes instead of " << buffer.size() << std::endl;
            std::cerr << "Error: " << strerror(errno) << " (errno " << errno << ")" << std::endl;
            // throw std::runtime_error("I2C write failed");
            return false;
        }

        return true;
    }

    union sercom_inten {
        struct {
            uint8_t DATA_RDY : 1; ///< this bit is set when data becomes available
        } bit; ///< bitfields
        uint8_t reg; ///< full register
    };
    sercom_inten _sercom_inten; ///< sercom interrupt enable register instance
};

#endif
