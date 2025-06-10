#include <iostream>
#include "helpers/i2c.h"

#define NEOTRELLIS_ADDR 0x2E

class NeoTrellis {
public:
    NeoTrellis(I2c& i2c) : i2c(i2c) {}

    bool begin() {
        if (!i2c.init(1, NEOTRELLIS_ADDR)) return false;

        uint8_t hw_id = 0;
        if (i2c.readReg(0x00 << 5 | 0x01, &hw_id) != 0) return false;
        // if (i2c.readReg(0x01, &hw_id) != 0) return false;
        if (hw_id != 0x55) {
            std::cerr << "Unexpected HW ID: " << std::hex << (int)hw_id << "\n";
            return false;
        }

        // Configure keypad scan
        i2c.writeReg(0x10 << 5 | 0x00, 0x01); // 1 row
        i2c.writeReg(0x10 << 5 | 0x01, 0x01); // 1 column
        i2c.writeReg(0x10 << 5 | 0x02, 0xFF); // enable all keys
        i2c.writeReg(0x10 << 5 | 0x03, 0xFF); // enable all interrupts

        return true;
    }

    void readKeypad() {
        uint8_t count = 0;
        if (i2c.readReg(0x10 << 5 | 0x05, &count) != 0) return;

        for (int i = 0; i < count; ++i) {
            uint8_t buf[2];
            if (i2c.pull(buf, 2) == 0) {
                uint8_t key = buf[0] & 0x7F;
                bool pressed = buf[0] & 0x80;
                std::cout << "Key " << (int)key << (pressed ? " pressed" : " released") << "\n";
            }
        }
    }

private:
    I2c& i2c;
};

int main() {
    I2c i2c;
    NeoTrellis trellis(i2c);

    if (!trellis.begin()) {
        std::cerr << "Failed to initialize NeoTrellis\n";
        return -1;
    }

    while (true) {
        trellis.readKeypad();
        usleep(100000); // 100ms
    }

    return 0;
}
