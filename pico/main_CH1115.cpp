#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdio.h>

// ==== CH1115 Configuration ====
#define CH1115_I2C_ADDR 0x3C   // Try 0x3D if nothing detected
#define CH1115_WIDTH 88
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

        // === CH1115 Init Sequence ===
        writeCommand(0xAE); // Display OFF
        writeCommand(0x02); // Set lower column address
        writeCommand(0x10); // Set higher column address
        writeCommand(0x40); // Set display start line 0
        writeCommand(0xB0); // Set page address to 0
        writeCommand(0x81); // Contrast control
        writeCommand(0x80);
        writeCommand(0xA1); // Segment remap (mirror horizontally)
        writeCommand(0xC0); // COM scan direction normal
        writeCommand(0xA6); // Normal display (not inverted)
        writeCommand(0xA8); // Multiplex ratio
        writeCommand(0x2F); // 1/48 duty
        writeCommand(0xD3); // Display offset
        writeCommand(0x00);
        writeCommand(0xD5); // Display clock divide ratio
        writeCommand(0xF0);
        writeCommand(0xD9); // Discharge / precharge period
        writeCommand(0x22);
        writeCommand(0xDB); // VCOM deselect level
        writeCommand(0x40);
        writeCommand(0xAD); // DC-DC control
        writeCommand(0x8B); // Enable internal DC-DC
        writeCommand(0x33); // Pump voltage (optional)
        writeCommand(0xA4); // Display follows RAM
        writeCommand(0xAF); // Display ON

        clear();
        display();
    }

    void clear() {
        memset(buffer, 0x00, sizeof(buffer));
    }

    void setPixel(int x, int y, bool on) {
        if (x < 0 || x >= CH1115_WIDTH || y < 0 || y >= CH1115_HEIGHT)
            return;
        if (on)
            buffer[x + (y / 8) * CH1115_WIDTH] |= (1 << (y % 8));
        else
            buffer[x + (y / 8) * CH1115_WIDTH] &= ~(1 << (y % 8));
    }

    void drawRect(int x, int y, int w, int h, bool fill) {
        if (fill) {
            for (int i = x; i < x + w; i++)
                for (int j = y; j < y + h; j++)
                    setPixel(i, j, true);
        } else {
            for (int i = x; i < x + w; i++) {
                setPixel(i, y, true);
                setPixel(i, y + h - 1, true);
            }
            for (int j = y; j < y + h; j++) {
                setPixel(x, j, true);
                setPixel(x + w - 1, j, true);
            }
        }
    }

    void display() {
        // CH1115: 6 pages (48px / 8)
        for (uint8_t page = 0; page < (CH1115_HEIGHT / 8); page++) {
            writeCommand(0xB0 + page); // Page address
            writeCommand(0x02);        // Lower column
            writeCommand(0x10);        // Higher column

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

    // Test pattern
    display.clear();

    // Draw border
    display.drawRect(0, 0, CH1115_WIDTH, CH1115_HEIGHT, false);

    // Draw filled boxes
    display.drawRect(5, 5, 20, 20, true);
    display.drawRect(30, 5, 20, 20, true);
    display.drawRect(55, 5, 20, 20, true);

    // Draw a horizontal line
    for (int i = 0; i < 60; i++) {
        display.setPixel(i, 35, true);
    }

    display.display();

    while (true) {
        sleep_ms(1000);
    }

    return 0;
}
