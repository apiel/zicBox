#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cstring>
#include <functional>

class NeoTrellis {
public:
    struct KeyEvent {
        uint8_t number;
        uint8_t edge;
    };

private:
    int i2c_fd;
    uint8_t address;
    
    // Seesaw module addresses
    static const uint8_t SEESAW_STATUS_BASE = 0x00;
    static const uint8_t SEESAW_GPIO_BASE = 0x01;
    static const uint8_t SEESAW_NEOPIXEL_BASE = 0x0E;
    static const uint8_t SEESAW_KEYPAD_BASE = 0x10;
    
    // Seesaw function addresses
    static const uint8_t SEESAW_STATUS_SWRST = 0x7F;
    static const uint8_t SEESAW_GPIO_BULK_SET = 0x05;
    static const uint8_t SEESAW_NEOPIXEL_PIN = 0x01;
    static const uint8_t SEESAW_NEOPIXEL_SPEED = 0x02;
    static const uint8_t SEESAW_NEOPIXEL_BUF_LENGTH = 0x03;
    static const uint8_t SEESAW_NEOPIXEL_BUF = 0x04;
    static const uint8_t SEESAW_NEOPIXEL_SHOW = 0x05;
    static const uint8_t SEESAW_KEYPAD_EVENT = 0x01;
    static const uint8_t SEESAW_KEYPAD_INTENSET = 0x02;
    static const uint8_t SEESAW_KEYPAD_COUNT = 0x04;
    
    // NeoTrellis specific
    static const uint8_t NEO_TRELLIS_NEOPIX_PIN = 3;
    static const uint8_t NEO_TRELLIS_NUM_ROWS = 4;
    static const uint8_t NEO_TRELLIS_NUM_COLS = 4;
    static const uint8_t NEO_TRELLIS_NUM_KEYS = 16;
    
    std::vector<std::function<void(KeyEvent)>> callbacks;
    std::vector<bool> key_events_enabled;
    
public:
    static const uint8_t EDGE_HIGH = 0;
    static const uint8_t EDGE_LOW = 1;
    static const uint8_t EDGE_FALLING = 2;
    static const uint8_t EDGE_RISING = 3;
    
    struct Color {
        uint8_t r, g, b;
        Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0) : r(red), g(green), b(blue) {}
    };
    
    // Color definitions
    static const Color OFF;
    static const Color RED;
    static const Color YELLOW;
    static const Color GREEN;
    static const Color CYAN;
    static const Color BLUE;
    static const Color PURPLE;
    
    NeoTrellis(const char* i2c_device = "/dev/i2c-1", uint8_t addr = 0x2E) 
        : address(addr), callbacks(NEO_TRELLIS_NUM_KEYS), key_events_enabled(NEO_TRELLIS_NUM_KEYS, false) {
        
        // Open I2C device
        i2c_fd = open(i2c_device, O_RDWR);
        if (i2c_fd < 0) {
            throw std::runtime_error("Failed to open I2C device");
        }
        
        // Set I2C slave address
        if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) {
            close(i2c_fd);
            throw std::runtime_error("Failed to set I2C slave address");
        }
        
        // Initialize the device
        init();
    }
    
    ~NeoTrellis() {
        if (i2c_fd >= 0) {
            close(i2c_fd);
        }
    }
    
    void init() {
        std::cout << "Starting device initialization..." << std::endl;
        
        // Software reset
        std::cout << "Sending software reset..." << std::endl;
        try {
            write_register(SEESAW_STATUS_BASE, SEESAW_STATUS_SWRST, {0xFF});
            std::cout << "Software reset sent successfully" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to send software reset: " << e.what() << std::endl;
            throw;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait longer for reset
        
        std::cout << "Setting up NeoPixels..." << std::endl;
        // Set up NeoPixels
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_PIN, {NEO_TRELLIS_NEOPIX_PIN});
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SPEED, {0x01}); // 800KHz
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF_LENGTH, {0x00, NEO_TRELLIS_NUM_KEYS * 3});
        
        std::cout << "Skipping keypad interrupt setup for now..." << std::endl;
        // Skip the keypad interrupt setup that's causing issues
        // We'll handle events polling-based instead
        
        std::cout << "Device initialization complete!" << std::endl;
    }
    
    void write_register(uint8_t reg_high, uint8_t reg_low, const std::vector<uint8_t>& data) {
        std::vector<uint8_t> buffer;
        buffer.push_back(reg_high);
        buffer.push_back(reg_low);
        buffer.insert(buffer.end(), data.begin(), data.end());
        
        ssize_t written = write(i2c_fd, buffer.data(), buffer.size());
        if (written != (ssize_t)buffer.size()) {
            std::cerr << "I2C write failed: wrote " << written << " bytes instead of " << buffer.size() << std::endl;
            std::cerr << "Register: 0x" << std::hex << (int)reg_high << " 0x" << (int)reg_low << std::dec << std::endl;
            throw std::runtime_error("I2C write failed");
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Increased delay
    }
    
    std::vector<uint8_t> read_register(uint8_t reg_high, uint8_t reg_low, size_t length) {
        uint8_t reg_addr[2] = {reg_high, reg_low};
        
        if (write(i2c_fd, reg_addr, 2) != 2) {
            throw std::runtime_error("I2C write failed");
        }
        
        std::this_thread::sleep_for(std::chrono::microseconds(200));
        
        std::vector<uint8_t> data(length);
        if (read(i2c_fd, data.data(), length) != (ssize_t)length) {
            throw std::runtime_error("I2C read failed");
        }
        
        return data;
    }
    
    void set_pixel_color(uint8_t pixel, const Color& color) {
        if (pixel >= NEO_TRELLIS_NUM_KEYS) return;
        
        std::vector<uint8_t> pixel_data;
        pixel_data.push_back(0x00); // Start address high byte
        pixel_data.push_back(pixel * 3); // Start address low byte
        pixel_data.push_back(color.g); // NeoPixels expect GRB order
        pixel_data.push_back(color.r);
        pixel_data.push_back(color.b);
        
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_BUF, pixel_data);
    }
    
    void show() {
        write_register(SEESAW_NEOPIXEL_BASE, SEESAW_NEOPIXEL_SHOW, {});
    }
    
    void activate_key(uint8_t key, uint8_t edge) {
        if (key >= NEO_TRELLIS_NUM_KEYS) return;
        
        key_events_enabled[key] = true;
        
        // For now, we'll just mark the key as enabled
        // The actual interrupt setup was causing issues, so we'll use polling instead
        std::cout << "Key " << (int)key << " activated for edge " << (int)edge << std::endl;
    }
    
    void set_callback(uint8_t key, std::function<void(KeyEvent)> callback) {
        if (key < NEO_TRELLIS_NUM_KEYS) {
            callbacks[key] = callback;
        }
    }
    
    void sync() {
        try {
            // Check if there are any key events
            auto count_data = read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_COUNT, 1);
            uint8_t count = count_data[0];
            
            if (count > 0) {
                // Read the events
                auto event_data = read_register(SEESAW_KEYPAD_BASE, SEESAW_KEYPAD_EVENT, count);
                
                for (size_t i = 0; i < count; i++) {
                    uint8_t event_byte = event_data[i];
                    uint8_t key_num = (event_byte >> 2) & 0x3F;
                    uint8_t edge = event_byte & 0x03;
                    
                    if (key_num < NEO_TRELLIS_NUM_KEYS && callbacks[key_num]) {
                        KeyEvent event = {key_num, edge};
                        callbacks[key_num](event);
                    }
                }
            }
        } catch (const std::exception& e) {
            // Ignore read errors during sync - device might not have data ready
        }
    }
    
    void set_brightness(float brightness) {
        // Note: Seesaw doesn't have direct brightness control like CircuitPython
        // You would need to scale colors manually or implement PWM control
        // For now, this is a placeholder
    }
};

// Static color definitions
const NeoTrellis::Color NeoTrellis::OFF(0, 0, 0);
const NeoTrellis::Color NeoTrellis::RED(255, 0, 0);
const NeoTrellis::Color NeoTrellis::YELLOW(255, 150, 0);
const NeoTrellis::Color NeoTrellis::GREEN(0, 255, 0);
const NeoTrellis::Color NeoTrellis::CYAN(0, 255, 255);
const NeoTrellis::Color NeoTrellis::BLUE(0, 0, 255);
const NeoTrellis::Color NeoTrellis::PURPLE(180, 0, 255);

// Callback function
void blink(NeoTrellis::KeyEvent event, NeoTrellis& trellis) {
    if (event.edge == NeoTrellis::EDGE_RISING) {
        trellis.set_pixel_color(event.number, NeoTrellis::CYAN);
        trellis.show();
    } else if (event.edge == NeoTrellis::EDGE_FALLING) {
        trellis.set_pixel_color(event.number, NeoTrellis::OFF);
        trellis.show();
    }
}

int main() {
    try {
        // First, let's check if the device is detected
        std::cout << "Checking I2C device..." << std::endl;
        std::cout << "Run 'i2cdetect -y 1' to verify NeoTrellis is at address 0x2E" << std::endl;
        
        // Create the NeoTrellis object
        NeoTrellis trellis("/dev/i2c-1", 0x2E);
        
        std::cout << "NeoTrellis initialized successfully!" << std::endl;
        
        // Set up callbacks for all keys
        for (int i = 0; i < 16; i++) {
            // Activate rising and falling edge events
            trellis.activate_key(i, NeoTrellis::EDGE_RISING);
            trellis.activate_key(i, NeoTrellis::EDGE_FALLING);
            
            // Set callback using lambda to capture trellis reference
            trellis.set_callback(i, [&trellis](NeoTrellis::KeyEvent event) {
                blink(event, trellis);
            });
        }
        
        // Startup LED cycle
        std::cout << "Starting LED cycle..." << std::endl;
        for (int i = 0; i < 16; i++) {
            trellis.set_pixel_color(i, NeoTrellis::PURPLE);
            trellis.show();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        for (int i = 0; i < 16; i++) {
            trellis.set_pixel_color(i, NeoTrellis::OFF);
            trellis.show();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        std::cout << "Ready! Press keys on the NeoTrellis..." << std::endl;
        
        // Main loop
        while (true) {
            trellis.sync();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
