#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>
#include <stdio.h>

// SSD1306 Configuration
#define SSD1306_I2C_ADDR 0x3C

#define SSD1306_WIDTH 128
#define SSD1306_HEIGHT 64

// #define SSD1306_WIDTH 64
// #define SSD1306_HEIGHT 48

// I2C Configuration
#define I2C_PORT i2c0
#define I2C_SDA 0
#define I2C_SCL 1

class SSD1306 {
private:
    uint8_t buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
    
    void writeCommand(uint8_t cmd) {
        uint8_t data[2] = {0x00, cmd};
        i2c_write_blocking(I2C_PORT, SSD1306_I2C_ADDR, data, 2, false);
    }
    
    void writeData(uint8_t* data, size_t len) {
        uint8_t buffer[len + 1];
        buffer[0] = 0x40; // Data mode
        memcpy(buffer + 1, data, len);
        i2c_write_blocking(I2C_PORT, SSD1306_I2C_ADDR, buffer, len + 1, false);
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
        
        // Initialization sequence
        writeCommand(0xAE); // Display off
        writeCommand(0xD5); // Set display clock divide
        writeCommand(0x80);
        writeCommand(0xA8); // Set multiplex ratio
        writeCommand(0x3F);
        writeCommand(0xD3); // Set display offset
        writeCommand(0x00);
        writeCommand(0x40); // Set start line
        writeCommand(0x8D); // Charge pump
        writeCommand(0x14);
        writeCommand(0x20); // Memory mode
        writeCommand(0x00);
        writeCommand(0xA1); // Set segment remap
        writeCommand(0xC8); // Set COM scan direction
        writeCommand(0xDA); // Set COM pins
        writeCommand(0x12);
        writeCommand(0x81); // Set contrast
        writeCommand(0xCF);
        writeCommand(0xD9); // Set precharge
        writeCommand(0xF1);
        writeCommand(0xDB); // Set VCOMH
        writeCommand(0x40);
        writeCommand(0xA4); // Display all on resume
        writeCommand(0xA6); // Normal display (not inverted)
        writeCommand(0xAF); // Display on
        
        clear();
        display();
    }
    
    void clear() {
        memset(buffer, 0, sizeof(buffer));
    }
    
    void setPixel(int x, int y, bool on) {
        if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT)
            return;
        
        if (on)
            buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
        else
            buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
    
    void drawChar(int x, int y, char c) {
        // Simple 5x7 font (you'd need a full font array)
        // This is just a demonstration for a few characters
        static const uint8_t font[][5] = {
            {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
            {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
            {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
        };
        
        if (c >= 'A' && c <= 'C') {
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < 8; j++) {
                    if (font[c - 'A'][i] & (1 << j)) {
                        setPixel(x + i, y + j, true);
                    }
                }
            }
        }
    }
    
    void drawText(int x, int y, const char* text) {
        while (*text) {
            drawChar(x, y, *text);
            x += 6;
            text++;
        }
    }
    
    void drawRect(int x, int y, int w, int h, bool fill) {
        if (fill) {
            for (int i = x; i < x + w; i++) {
                for (int j = y; j < y + h; j++) {
                    setPixel(i, j, true);
                }
            }
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
        writeCommand(0x21); // Column address
        writeCommand(0);
        writeCommand(SSD1306_WIDTH - 1);
        writeCommand(0x22); // Page address
        writeCommand(0);
        writeCommand((SSD1306_HEIGHT / 8) - 1);
        
        // Send buffer in chunks
        for (int i = 0; i < sizeof(buffer); i += 32) {
            writeData(&buffer[i], 32);
        }
    }
};

int main() {
    stdio_init_all();
    
    SSD1306 display;
    display.init();
    
    // Draw some test patterns
    display.clear();
    
    // Draw a border
    display.drawRect(0, 0, SSD1306_WIDTH, SSD1306_HEIGHT, false);
    
    // Draw some filled rectangles
    display.drawRect(0, 12, SSD1306_WIDTH / 2, SSD1306_HEIGHT / 2, true);
    display.drawRect(SSD1306_WIDTH / 2, SSD1306_HEIGHT / 2, 16, 16, true);
    display.drawRect(SSD1306_WIDTH / 2 + 16, SSD1306_HEIGHT / 2 + 16, 16, 16, true);
    
    // // Draw some pixels
    // for (int i = 0; i < 50; i++) {
    //     display.setPixel(70 + i, 20, true);
    // }

    display.drawText(64, 44, "ABCABCABC");
    
    display.display();
    
    while (true) {
        sleep_ms(1000);
    }
    
    return 0;
}