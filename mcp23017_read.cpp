#include <iostream>
#include "helpers/i2c.h" // Include your I2C library header

// g++ -o mcp23017_read mcp23017_read.cpp

#define MCP23017_ADDRESS 0x20  // Default I2C address of MCP23017
#define MCP23017_IODIRA 0x00   // I/O direction register for port A
#define MCP23017_IODIRB 0x01   // I/O direction register for port B
#define MCP23017_GPPUA 0x0C    // Pull-up resistor register for port A
#define MCP23017_GPPUB 0x0D    // Pull-up resistor register for port B
#define MCP23017_GPIOA 0x12    // GPIO register for port A
#define MCP23017_GPIOB 0x13    // GPIO register for port B

int main() {
    I2c i2c;
    if (!i2c.init(1, MCP23017_ADDRESS)) {
        std::cerr << "Failed to initialize I2C" << std::endl;
        return 1;
    }

    // Set all pins as inputs
    if (i2c.writeReg(MCP23017_IODIRA, 0xFF) != 0 || i2c.writeReg(MCP23017_IODIRB, 0xFF) != 0) {
        std::cerr << "Failed to set I/O direction" << std::endl;
        i2c.end();
        return 1;
    }

    // Enable pull-up resistors on all pins
    if (i2c.writeReg(MCP23017_GPPUA, 0xFF) != 0 || i2c.writeReg(MCP23017_GPPUB, 0xFF) != 0) {
        std::cerr << "Failed to enable pull-up resistors" << std::endl;
        i2c.end();
        return 1;
    }

    // Read the values of each channel
    uint8_t gpioA, gpioB;
    if (i2c.readReg(MCP23017_GPIOA, &gpioA) != 0 || i2c.readReg(MCP23017_GPIOB, &gpioB) != 0) {
        std::cerr << "Failed to read GPIO values" << std::endl;
        i2c.end();
        return 1;
    }

    std::cout << "GPIOA: " << std::hex << static_cast<int>(gpioA) << std::endl;
    std::cout << "GPIOB: " << std::hex << static_cast<int>(gpioB) << std::endl;

    // Print the value of each pin
    for (int i = 0; i < 8; ++i) {
        std::cout << "GPIOA" << i << ": " << ((gpioA >> i) & 0x01) << std::endl;
    }
    for (int i = 0; i < 8; ++i) {
        std::cout << "GPIOB" << i << ": " << ((gpioB >> i) & 0x01) << std::endl;
    }

    i2c.end();
    return 0;
}

