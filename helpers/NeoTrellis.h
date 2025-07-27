#pragma once

#include <cstring> // For strerror
#include <fcntl.h>
#include <functional>
#include <iostream>
#include <linux/i2c-dev.h>
#include <linux/i2c.h> // Required for i2c_msg and i2c_rdwr_ioctl_data structures
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define NEO_TRELLIS_NUM_KEYS 16
#define NEO_TRELLIS_KEY(x) (((x) / 4) * 8 + ((x) % 4))
#define NEO_TRELLIS_SEESAW_KEY(x) (((x) / 8) * 4 + ((x) % 8))

class NeoTrellis {
protected:
    std::thread loopThread;
    bool loopRunning = true;

public:
    enum {
        SEESAW_STATUS_BASE = 0x00,
        SEESAW_NEOPIXEL_BASE = 0x0E,
        SEESAW_KEYPAD_BASE = 0x10,
    };

    enum {
        SEESAW_STATUS_SWRST = 0x7F,
    };

    enum {
        SEESAW_NEOPIXEL_BUF = 0x04,
        SEESAW_NEOPIXEL_SHOW = 0x05,
    };

    enum {
        SEESAW_KEYPAD_EVENT = 0x01,
        SEESAW_KEYPAD_INTENSET = 0x02,
        SEESAW_KEYPAD_INTENCLR = 0x03,
        SEESAW_KEYPAD_COUNT = 0x04,
        SEESAW_KEYPAD_FIFO = 0x10,
    };

    enum {
        SEESAW_KEYPAD_EDGE_FALLING = 2,
        SEESAW_KEYPAD_EDGE_RISING = 3,
    };

    enum {
        SEESAW_NEOPIXEL_PIN = 0x01, // Pin connected to NeoPixels
        SEESAW_NEOPIXEL_SPEED = 0x02, // NeoPixel data rate
        SEESAW_NEOPIXEL_BUF_LENGTH = 0x03, // Number of NeoPixels * 3 uint8_ts (GRB)
    };

    NeoTrellis(std::function<void(uint8_t, bool)> callback)
        : _callback(callback)
    {
    }

    ~NeoTrellis() { };

    int i2c_fd; // File descriptor for the I2C bus
    uint8_t address; // I2C address of the NeoTrellis

    bool write8(uint8_t regHigh, uint8_t regLow, uint8_t value)
    {
        return this->writeReg(regHigh, regLow, &value, 1);
    }

    uint8_t read8(uint8_t regHigh, uint8_t regLow, uint16_t delay = 250)
    {
        uint8_t ret;
        this->read(regHigh, regLow, &ret, 1, delay);

        return ret;
    }

    bool read(uint8_t regHigh,
        uint8_t regLow,
        uint8_t* buf,
        uint8_t length,
        uint16_t delay_us = 250)
    {
        uint8_t prefix[2] = { regHigh, regLow };
        size_t pos = 0;
        constexpr size_t CHUNK_SIZE = 32;

        while (pos < length) {
            size_t to_read = std::min(CHUNK_SIZE, static_cast<size_t>(length - pos));
            if (::write(i2c_fd, prefix, sizeof(prefix)) != sizeof(prefix)) {
                std::cerr << "I2C register‑select write failed: "
                          << strerror(errno) << " (errno " << errno << ")\n";
                return false;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(delay_us));

            ssize_t got = ::read(i2c_fd, buf + pos, to_read);
            if (got != static_cast<ssize_t>(to_read)) {
                std::cerr << "I2C read failed: "
                          << strerror(errno) << " (errno " << errno << ")\n";
                return false;
            }

            pos += to_read;
        }

        return true;
    }

    bool writeReg(uint8_t regHigh, uint8_t regLow, uint8_t* buf, uint8_t num)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        std::vector<uint8_t> buffer;
        buffer.push_back(regHigh); // Seesaw module address
        buffer.push_back(regLow); // Seesaw function address
        buffer.insert(buffer.end(), buf, buf + num);

        ssize_t written = write(i2c_fd, buffer.data(), buffer.size());
        if (written != (ssize_t)buffer.size()) {
            std::cerr << "I2C write failed: wrote " << written << " uint8_ts instead of " << buffer.size() << std::endl;
            std::cerr << "Error: " << strerror(errno) << " (errno " << errno << ")" << std::endl;
            // throw std::runtime_error("I2C write failed");
            return false;
        }

        return true;
    }

    void begin(uint8_t addr = 0x2E, const char* i2c_device = "/dev/i2c-1")
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

        try {
            this->write8(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, 0xFF);
            std::cout << "Software reset sent successfully." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to send software reset: " << e.what() << std::endl;
            throw; // Re-throw to indicate critical failure
        }

        // Enable the keypad interruptNEO_TRELLIS_SEESAW_KEY
        this->write8(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_INTENSET, 0x01);

        this->write8(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_PIN, 0x03); // The GPIO pin on ATSAMD09 connected to NeoPixels
        this->write8(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SPEED, 0x01);
        uint8_t writeBuf[2] = { 0x00, (uint8_t)(NEO_TRELLIS_NUM_KEYS * 3) };
        this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF_LENGTH, writeBuf, 2);
    }

    void startThread(std::string threadName = "neotrellis")
    {
        //activate all keys and set callbacks
        for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++) {
            activateKey(i, NeoTrellis::SEESAW_KEYPAD_EDGE_RISING);
            activateKey(i, NeoTrellis::SEESAW_KEYPAD_EDGE_FALLING);
            // trellis.registerCallback(i, blink);
        }

        loopThread = std::thread(&NeoTrellis::loop, this);
        pthread_setname_np(loopThread.native_handle(), threadName.c_str());
    }

    void loop()
    {
        while (loopRunning) {
            read();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }

    union keyState {
        struct {
            uint8_t STATE : 1; ///< the current state of the key
            uint8_t ACTIVE : 4; ///< the registered events for that key
        } bit; ///< bitfield format
        uint8_t reg; ///< register format
    };

    void activateKey(uint8_t key, uint8_t edge, bool enable = true)
    {
        keyState ks;
        ks.bit.STATE = enable;
        ks.bit.ACTIVE = (1 << edge);
        uint8_t cmd[] = { (uint8_t)NEO_TRELLIS_KEY(key), ks.reg };
        this->writeReg(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_EVENT, cmd, 2);
    }

    union keyEventRaw {
        struct {
            uint8_t EDGE : 2; ///< the edge that was triggered
            uint8_t NUM : 6; ///< the event number
        } bit; ///< bitfield format
        uint8_t reg; ///< register format
    };

    void read(bool polling = true)
    {
        uint8_t count = this->read8(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_COUNT, 500);
        ;
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        if (count > 0) {
            if (polling)
                count = count + 2;
            keyEventRaw e[count];
            this->read(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_FIFO, (uint8_t*)e, count, 1000);
            for (int i = 0; i < count; i++) {
                e[i].bit.NUM = NEO_TRELLIS_SEESAW_KEY(e[i].bit.NUM);
                if (e[i].bit.NUM < NEO_TRELLIS_NUM_KEYS) {
                    if (_callback) {
                        _callback(e[i].bit.NUM, e[i].bit.EDGE == SEESAW_KEYPAD_EDGE_RISING);
                    }
                }
            }
        }
    }

    struct Color {
        uint8_t r, g, b;
        Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0)
            : r(red)
            , g(green)
            , b(blue)
        {
        }
    };

    void setPixelColor(uint8_t pixel, const Color& color)
    {
        if (pixel >= NEO_TRELLIS_NUM_KEYS)
            return;

        uint8_t writeBuf[6] = { 0x00, (uint8_t)(pixel * 3), color.g, color.r, color.b, 0x00 };
        this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, 5);
    }

    // void setPixelColor(uint8_t pixel, const Color& color)
    // {
    //     if (pixel >= NEO_TRELLIS_NUM_KEYS)
    //         return;

    //     uint8_t p[3];
    //     p[1] = color.r;
    //     p[0] = color.g;
    //     p[2] = color.b;

    //     uint8_t writeBuf[6] = {
    //         0x00,
    //         (uint8_t)(pixel * 3),
    //         p[0], p[1], p[2],
    //         0x00
    //     };

    //     this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, writeBuf, 5);
    // }

    void show(void)
    {
        this->writeReg(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SHOW, NULL, 0);
    }

protected:
    uint8_t _addr;
    std::function<void(uint8_t, bool)> _callback;
};
