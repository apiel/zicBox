#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdio.h>

// ==== CH1115 Configuration ====
#define CH1115_I2C_ADDR 0x3C
#define CH1115_WIDTH 160
#define CH1115_HEIGHT 48

// ==== I2C Configuration ====
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

class CH1115 {
private:
    uint8_t buffer[CH1115_WIDTH * CH1115_HEIGHT / 8];

    void writeCommand(uint8_t cmd) {
        uint8_t data[2] = {0x00, cmd};
        i2c_write_blocking(I2C_PORT, CH1115_I2C_ADDR, data, 2, false);
    }

    void writeData(uint8_t *data, size_t len) {
        uint8_t chunk[len + 1];
        chunk[0] = 0x40; // Data mode
        memcpy(chunk + 1, data, len);
        i2c_write_blocking(I2C_PORT, CH1115_I2C_ADDR, chunk, len + 1, false);
    }

public:
    void init() {
        // Initialize I2C
        i2c_init(I2C_PORT, 400000); // 400kHz
        gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
        gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
        gpio_pull_up(I2C_SDA);
        gpio_pull_up(I2C_SCL);
        sleep_ms(100);

        // ==== CH1115 Init Sequence ====
        writeCommand(0xAE); // Display OFF
        writeCommand(0x02); // Lower column start
        writeCommand(0x10); // Higher column start
        writeCommand(0x40); // Start line
        writeCommand(0xB0); // Page 0
        writeCommand(0x81); writeCommand(0x80); // Contrast
        writeCommand(0xA1); // Segment remap
        writeCommand(0xC0); // COM scan direction
        writeCommand(0xA6); // Normal display
        writeCommand(0xA8); writeCommand(0x2F); // Multiplex 48
        writeCommand(0xD3); writeCommand(0x00); // Offset
        writeCommand(0xD5); writeCommand(0xF0); // Clock
        writeCommand(0xD9); writeCommand(0x22); // Pre-charge
        writeCommand(0xDB); writeCommand(0x40); // VCOM
        writeCommand(0xAD); writeCommand(0x8B); // DC-DC
        writeCommand(0x33); // Pump voltage
        writeCommand(0xA4); // Follow RAM
        writeCommand(0xAF); // Display ON

        clear();
        display();
    }

    void clear() {
        memset(buffer, 0x00, sizeof(buffer));
    }

    void setPixel(int x, int y, bool on) {
        if (x < 0 || x >= CH1115_WIDTH || y < 0 || y >= CH1115_HEIGHT) return;
        if (on)
            buffer[x + (y / 8) * CH1115_WIDTH] |= (1 << (y % 8));
        else
            buffer[x + (y / 8) * CH1115_WIDTH] &= ~(1 << (y % 8));
    }

    void drawRect(int x, int y, int w, int h, bool fill) {
        for (int i = x; i < x + w; i++) {
            for (int j = y; j < y + h; j++) {
                if (fill)
                    setPixel(i, j, true);
                else if (i == x || i == x + w - 1 || j == y || j == y + h - 1)
                    setPixel(i, j, true);
            }
        }
    }

    void display() {
        // Write each page (6 pages for 48px height)
        for (uint8_t page = 0; page < (CH1115_HEIGHT / 8); page++) {
            writeCommand(0xB0 + page); // Page address
            writeCommand(0x02);        // Column low start
            writeCommand(0x10);        // Column high start

            uint8_t chunk[CH1115_WIDTH + 1];
            chunk[0] = 0x40;
            memcpy(chunk + 1, &buffer[page * CH1115_WIDTH], CH1115_WIDTH);
            i2c_write_blocking(I2C_PORT, CH1115_I2C_ADDR, chunk, CH1115_WIDTH + 1, false);
        }
    }
};

// ==== MAIN ====
int main() {
    stdio_init_all();

    CH1115 display;
    display.init();

    display.clear();

    // Draw border around full screen
    display.drawRect(0, 0, CH1115_WIDTH, CH1115_HEIGHT, false);

    // Draw four squares safely in each corner
    display.drawRect(36, 6, 10, 10, true);   // top-left
    display.drawRect(118, 6, 10, 10, true);  // top-right
    // display.drawRect(36, 32, 10, 10, true);  // bottom-left
    // display.drawRect(118, 32, 10, 10, true); // bottom-right

    display.drawRect(36, 38, 10, 10, true);  // bottom-left
    display.drawRect(118, 38, 10, 10, true); // bottom-right

    display.display();

    while (true) {
        sleep_ms(1000);
    }

    return 0;
}
