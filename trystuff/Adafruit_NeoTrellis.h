#pragma once

#include "Adafruit_neopixel.h"
#include "Adafruit_seesaw.h"

#define NEO_TRELLIS_ADDR 0x2E

#define NEO_TRELLIS_NEOPIX_PIN 3

#define NEO_TRELLIS_NUM_ROWS 4
#define NEO_TRELLIS_NUM_COLS 4
#define NEO_TRELLIS_NUM_KEYS (NEO_TRELLIS_NUM_ROWS * NEO_TRELLIS_NUM_COLS)

#define NEO_TRELLIS_MAX_CALLBACKS 32

#define NEO_TRELLIS_KEY(x) (((x) / 4) * 8 + ((x) % 4))
#define NEO_TRELLIS_SEESAW_KEY(x) (((x) / 8) * 4 + ((x) % 8))

#define NEO_TRELLIS_X(k) ((k) % 4)
#define NEO_TRELLIS_Y(k) ((k) / 4)

#define NEO_TRELLIS_XY(x, y) ((y) * NEO_TRELLIS_NUM_COLS + (x))

typedef void (*TrellisCallback)(keyEvent evt);

class Adafruit_NeoTrellis : public Adafruit_seesaw {
public:
    Adafruit_NeoTrellis()
        : Adafruit_seesaw()
    // , pixels(NEO_TRELLIS_NUM_KEYS, NEO_TRELLIS_NEOPIX_PIN, NEO_GRB + NEO_KHZ800)
    {
        for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++) {
            _callbacks[i] = NULL;
        }
    }

    ~Adafruit_NeoTrellis() { };

    void begin(const char* i2c_device = "/dev/i2c-1", uint8_t addr = 0x2E)
    {
        // pixels.begin(i2c_device, addr);
        Adafruit_seesaw::begin(i2c_device, addr);
        enableKeypadInterrupt();
    }

    void registerCallback(uint8_t key, TrellisCallback (*cb)(keyEvent))
    {
        _callbacks[key] = cb;
    }

    void unregisterCallback(uint8_t key)
    {
        _callbacks[key] = NULL;
    }

    void activateKey(uint8_t key, uint8_t edge, bool enable = true)
    {
        setKeypadEvent(NEO_TRELLIS_KEY(key), edge, enable);
    }

    void read(bool polling = true)
    {
        uint8_t count = getKeypadCount();
        // delayMicroseconds(500);
        std::this_thread::sleep_for(std::chrono::microseconds(500));
        if (count > 0) {
            // std::cout << "Keys count: " << (int)count << std::endl;
            if (polling)
                count = count + 2;
            keyEventRaw e[count];
            readKeypad(e, count);
            // std::cout << "Keys count: " << (int)count << std::endl;
            // std::cout << "Event: " << (int)e[0].bit.NUM << " " << (int)e[0].bit.EDGE << std::endl;
            for (int i = 0; i < count; i++) {
                // call any callbacks associated with the key
                e[i].bit.NUM = NEO_TRELLIS_SEESAW_KEY(e[i].bit.NUM);
                if (e[i].bit.NUM < NEO_TRELLIS_NUM_KEYS) {
                    // std::cout << "Event: " << (int)e[i].bit.NUM << " " << (int)e[i].bit.EDGE << std::endl;
                    if (_callbacks[e[i].bit.NUM] != NULL) {
                        keyEvent evt = { e[i].bit.EDGE, e[i].bit.NUM };
                        _callbacks[e[i].bit.NUM](evt);
                    }
                }
            }
        }
    }

    // Adafruit_neopixel pixels; ///< the onboard neopixel matrix

    friend class Adafruit_MultiTrellis; ///< for allowing use of protected methods
    ///< by aggregate class

protected:
    uint8_t _addr; ///< the I2C address of this board
    TrellisCallback (*_callbacks[NEO_TRELLIS_NUM_KEYS])(
        keyEvent); ///< the array of callback functions
};

// class Adafruit_MultiTrellis {
// public:
//     Adafruit_MultiTrellis(Adafruit_NeoTrellis* trelli, uint8_t rows,
//         uint8_t cols);
//     ~Adafruit_MultiTrellis() { };

//     bool begin();

//     void registerCallback(uint16_t num, TrellisCallback (*cb)(keyEvent));
//     void registerCallback(uint8_t x, uint8_t y, TrellisCallback (*cb)(keyEvent));
//     void unregisterCallback(uint16_t num);
//     void unregisterCallback(uint8_t x, uint8_t y);

//     void activateKey(uint16_t num, uint8_t edge, bool enable = true);
//     void activateKey(uint8_t x, uint8_t y, uint8_t edge, bool enable = true);

//     void setPixelColor(uint8_t x, uint8_t y, uint32_t color);
//     void setPixelColor(uint16_t num, uint32_t color);
//     void show();

//     void read();

// protected:
//     uint8_t _rows; ///< the number of trellis boards in the Y direction
//     uint8_t _cols; ///< the number of trellis boards in the X direction
//     Adafruit_NeoTrellis* _trelli; ///< a multidimensional array of neotrellis objects
// };
