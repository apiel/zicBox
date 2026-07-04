/** Description:
This C++ program serves as a digital reader for an external electronic component called an I/O expander (specifically, the MCP23017 chip). This chip provides 16 additional connections, allowing the central computer to monitor many external devices, like buttons or sensors.

The core purpose of the code is to check the current electrical state of all 16 pins.

The process involves several steps:
1.  **Initialization:** The program establishes communication with the chip using specialized functions (likely utilizing the I2C protocol).
2.  **Configuration:** It sends commands to the chip, setting all 16 external connections (divided into Channel A and Channel B) to operate in "input mode." This means the pins are listening for signals rather than sending them out.
3.  **Setup:** The code enables internal "pull-up resistors." This is a crucial electrical feature that ensures a stable, reliable reading when no device is actively connected to the pin.
4.  **Reading:** The program queries the chip and retrieves the status of all 16 inputs simultaneously.
5.  **Reporting:** Finally, the results are displayed. The program lists each of the 16 pins individually (A0 through A7, and B0 through B7), clearly reporting whether the detected electrical state is "HIGH" (active or default) or "LOW" (usually meaning a connection to ground).

sha: 55cb55b80df127d209e9865d54a91a0d8811dc26f78aae060ad5b76b76f6ade7 
*/
// g++ -o mcp23017_read mcp23017_read.cpp

// #include <iostream>
// #include "helpers/i2c.h" // Include your I2C library header


// #define MCP23017_ADDRESS 0x20  // Default I2C address of MCP23017
// #define MCP23017_IODIRA 0x00   // I/O direction register for port A
// #define MCP23017_IODIRB 0x01   // I/O direction register for port B
// #define MCP23017_GPPUA 0x0C    // Pull-up resistor register for port A
// #define MCP23017_GPPUB 0x0D    // Pull-up resistor register for port B
// #define MCP23017_GPIOA 0x12    // GPIO register for port A
// #define MCP23017_GPIOB 0x13    // GPIO register for port B

// int main() {
//     I2c i2c;
//     if (!i2c.init(1, MCP23017_ADDRESS)) {
//         std::cerr << "Failed to initialize I2C" << std::endl;
//         return 1;
//     }

//     // Set all pins as inputs
//     if (i2c.writeReg(MCP23017_IODIRA, 0xFF) != 0 || i2c.writeReg(MCP23017_IODIRB, 0xFF) != 0) {
//         std::cerr << "Failed to set I/O direction" << std::endl;
//         i2c.end();
//         return 1;
//     }

//     // Enable pull-up resistors on all pins
//     if (i2c.writeReg(MCP23017_GPPUA, 0xFF) != 0 || i2c.writeReg(MCP23017_GPPUB, 0xFF) != 0) {
//         std::cerr << "Failed to enable pull-up resistors" << std::endl;
//         i2c.end();
//         return 1;
//     }

//     // Read the values of each channel
//     uint8_t gpioA, gpioB;
//     if (i2c.readReg(MCP23017_GPIOA, &gpioA) != 0 || i2c.readReg(MCP23017_GPIOB, &gpioB) != 0) {
//         std::cerr << "Failed to read GPIO values" << std::endl;
//         i2c.end();
//         return 1;
//     }

//     std::cout << "GPIOA: " << std::hex << static_cast<int>(gpioA) << std::endl;
//     std::cout << "GPIOB: " << std::hex << static_cast<int>(gpioB) << std::endl;

//     // Print the value of each pin
//     for (int i = 0; i < 8; ++i) {
//         std::cout << "GPIOA" << i << ": " << ((gpioA >> i) & 0x01) << std::endl;
//     }
//     for (int i = 0; i < 8; ++i) {
//         std::cout << "GPIOB" << i << ": " << ((gpioB >> i) & 0x01) << std::endl;
//     }

//     i2c.end();
//     return 0;
// }



// g++ -o mcp23017_read mcp23017_read.cpp -I.
#include "helpers/Mcp23017.h"
#include <iostream>

int main() {
    Mcp23017 mcp;

    mcp.init();
    mcp.setInputModeChannelA();
    mcp.setInputModeChannelB();
    mcp.enablePullUpChannelA();
    mcp.enablePullUpChannelB();

    uint8_t gpioA = mcp.readChannelA();
    uint8_t gpioB = mcp.readChannelB();

    std::cout << "GPIOA: " << std::hex << static_cast<int>(gpioA) << std::endl;
    std::cout << "GPIOB: " << std::hex << static_cast<int>(gpioB) << std::endl;

    for (int i = 0; i < 8; ++i) {
        std::cout << "GPIOA" << i << ": " << (mcp.isChannelPinHigh(gpioA, i) ? "HIGH" : "LOW" ) << std::endl;
    }
    for (int i = 0; i < 8; ++i) {
        std::cout << "GPIOB" << i << ": " << (mcp.isChannelPinHigh(gpioB, i) ? "HIGH" : "LOW" ) << std::endl;
    }

    // // Print the value of each pin
    // for (int i = 0; i < 8; ++i) {
    //     std::cout << "GPIOA" << i << ": " << ((gpioA >> i) & 0x01) << std::endl;
    // }
    // for (int i = 0; i < 8; ++i) {
    //     std::cout << "GPIOB" << i << ": " << ((gpioB >> i) & 0x01) << std::endl;
    // }

    return 0;
}

