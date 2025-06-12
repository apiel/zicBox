#include "helpers/i2c.h"
#include <iostream>

#define NEOTRELLIS_ADDR 0x2E

class NeoTrellis {
public:
    NeoTrellis(I2c& i2c)
        : i2c(i2c)
    {
    }

    bool begin()
    {
        if (!i2c.init(1, NEOTRELLIS_ADDR))
            return false;

        // uint8_t hw_id = 0;
        // if (i2c.readReg(0x00 << 5 | 0x01, &hw_id) != 0) return false;
        // // if (i2c.readReg(0x01, &hw_id) != 0) return false;
        // if (hw_id != 0x55) {
        //     std::cerr << "Unexpected HW ID: " << std::hex << (int)hw_id << "\n";
        //     return false;
        // }

        uint8_t hw_id = 0;
        if (i2c.readReg(0x00, 0x01, &hw_id) != 0) {
            std::cerr << "Failed to read HW_ID\n";
            return false;
        } else {
            std::cout << "HW_ID = 0x" << std::hex << (int)hw_id << std::endl;
        }

        // Configure keypad scan
        i2c.writeReg(0x10 << 5 | 0x00, 0x01); // 1 row
        i2c.writeReg(0x10 << 5 | 0x01, 0x01); // 1 column
        i2c.writeReg(0x10 << 5 | 0x02, 0xFF); // enable all keys
        i2c.writeReg(0x10 << 5 | 0x03, 0xFF); // enable all interrupts

        return true;
    }

    // void readKeypad()
    // {
    //     uint8_t count = 0;
    //     if (i2c.readReg(0x10 << 5 | 0x05, &count) != 0)
    //         return;

    //     for (int i = 0; i < count; ++i) {
    //         uint8_t buf[2];
    //         if (i2c.pull(buf, 2) == 0) {
    //             uint8_t key = buf[0] & 0x7F;
    //             bool pressed = buf[0] & 0x80;
    //             std::cout << "Key " << (int)key << (pressed ? " pressed" : " released") << "\n";
    //         }
    //     }
    // }

    bool readKeyEvent(uint8_t* key, uint8_t* edge)
    {
        if (i2c.file == 0 || key == nullptr || edge == nullptr)
            return false;

        // Point to KEYPAD module (0x10), EVENT register (0x10)
        uint8_t reg[2] = { 0x10, 0x10 };
        if (write(i2c.file, reg, 2) != 2) {
            perror("I2C write failed");
            return false;
        }

        uint8_t buf[3];
        if (read(i2c.file, buf, 3) != 3) {
            // No event available or read error
            return false;
        }

        *key = buf[0];
        *edge = buf[1]; // 0 = press, 1 = release

        return true;
    }

    bool getKeyEventCount(uint8_t* count)
    {
        return i2c.readReg(0x10, 0x04, count) == 0;
    }

private:
    I2c& i2c;
};

int main()
{
    I2c i2c;
    NeoTrellis trellis(i2c);

    if (!trellis.begin()) {
        std::cerr << "Failed to initialize NeoTrellis\n";
        return -1;
    }

    // while (true) {
    //     trellis.readKeypad();
    //     usleep(100000); // 100ms
    // }

    // uint8_t key, edge;
    // while (true) {
    //     if (trellis.readKeyEvent(&key, &edge)) {
    //         std::cout << "Key " << (int)key << (edge == 0 ? " pressed" : " released") << std::endl;
    //     }
    //     usleep(20000); // Sleep to avoid tight loop
    // }

    uint8_t keyCount = 0;
    uint8_t key = 0, edge = 0;

    while (true) {
        if (trellis.getKeyEventCount(&keyCount) && keyCount > 0) {
            while (keyCount--) {
                if (trellis.readKeyEvent(&key, &edge)) {
                    if (key <= 15) {
                        std::cout << "Key " << (int)key
                                  << (edge == 0 ? " pressed" : " released") << std::endl;
                    } else {
                        std::cout << "Invalid key: " << std::hex << (int)key << std::endl;
                    }
                }
            }
        }
        usleep(20000); // 20 ms delay between polls
    }

    return 0;
}
