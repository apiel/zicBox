#pragma once

#include "helpers/i2c.h"
#include "log.h"

#define MCP23017_IODIRA 0x00 // I/O direction register for channel A
#define MCP23017_IODIRB 0x01 // I/O direction register for channel B
#define MCP23017_GPPUA 0x0C // Pull-up resistor register for channel A
#define MCP23017_GPPUB 0x0D // Pull-up resistor register for channel B
#define MCP23017_GPIOA 0x12 // GPIO register for channel A
#define MCP23017_GPIOB 0x13 // GPIO register for channel B
#define MCP23017_OLATA 0x14    // Output latch register for channel A
#define MCP23017_OLATB 0x15    // Output latch register for channel B

class Mcp23017 {
protected:
    I2c i2c;

public:
    bool init(int address = 0x20) // Default I2C address of MCP23017
    {
        if (!i2c.init(1, address)) {
            logError("Failed to init i2c");
            return false;
        }
        return true;
    }

    bool setAsInputs(uint8_t channel) {
        if (i2c.writeReg(channel, 0xFF) != 0) {
            logError("Failed to set I/O direction for channel " + std::to_string(channel));
            i2c.end();
            return false;
        }
        return true;
    }

    bool setInputModeChannelA() {
        return setAsInputs(MCP23017_IODIRA);
    }

    bool setInputModeChannelB() {
        return setAsInputs(MCP23017_IODIRB);
    }

    bool setAsOutputs(uint8_t channel) {
        if (i2c.writeReg(channel, 0x00) != 0) { // Don't if this work, haven't tested
            logError("Failed to set I/O direction for channel " + std::to_string(channel));
            i2c.end();
            return false;
        }
        return true;
    }

    bool setOutputModeChannelA() {
        return setAsOutputs(MCP23017_OLATA);
    }

    bool setOutputModeChannelB() {
        return setAsOutputs(MCP23017_OLATB);
    }

    bool enablePullUp(uint8_t channel) {
        if (i2c.writeReg(channel, 0xFF) != 0) {
            logError("Failed to enable pull-up resistors for channel " + std::to_string(channel));
            i2c.end();
            return false;
        }
        return true;
    }

    bool enablePullUpChannelA() {
        return enablePullUp(MCP23017_GPPUA);
    }

    bool enablePullUpChannelB() {
        return enablePullUp(MCP23017_GPPUB);
    }

    uint8_t channelValue;
    uint8_t readChannel(uint8_t channel) {
        if (i2c.readReg(channel, &channelValue) != 0) {
            logError("Failed to read channel " + std::to_string(channel));
            i2c.end();
            return 0;
        }
        return channelValue;
    }

    uint8_t readChannelA() {
        return readChannel(MCP23017_GPIOA);
    }

    uint8_t readChannelB() {
        return readChannel(MCP23017_GPIOB);
    }

    bool isChannelPinHigh(uint8_t channelValue, uint8_t pin) {
        return (channelValue >> pin) & 0x01;
    }
};
