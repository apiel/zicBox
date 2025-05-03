#pragma once

#include "controllerInterface.h"
#include "helpers/i2c.h"
#include "log.h"
// #include "helpers/getTicks.h"

#define MCP23017_IODIRA 0x00 // I/O direction register for port A
#define MCP23017_IODIRB 0x01 // I/O direction register for port B
#define MCP23017_GPPUA 0x0C // Pull-up resistor register for port A
#define MCP23017_GPPUB 0x0D // Pull-up resistor register for port B
#define MCP23017_GPIOA 0x12 // GPIO register for port A
#define MCP23017_GPIOB 0x13 // GPIO register for port B

class Mcp23017Controller : public ControllerInterface {
protected:
    I2c i2c;
    int address = 0x20; // Default I2C address of MCP23017

public:
    Mcp23017Controller(Props& props, uint16_t id)
        : ControllerInterface(props, id)
    {
    }

    void config(nlohmann::json& config) override
    {
        address = config.value("address", address);

        if (!i2c.init(1, address)) {
            logError("Failed to init i2c");
            return;
        }

        // Set all pins as inputs
        if (i2c.writeReg(MCP23017_IODIRA, 0xFF) != 0 || i2c.writeReg(MCP23017_IODIRB, 0xFF) != 0) {
            logError("Failed to set I/O direction");
            i2c.end();
            return;
        }

        // Enable pull-up resistors on all pins
        if (i2c.writeReg(MCP23017_GPPUA, 0xFF) != 0 || i2c.writeReg(MCP23017_GPPUB, 0xFF) != 0) {
            logError("Failed to enable pull-up resistors");
            i2c.end();
            return;
        }
    }
};
