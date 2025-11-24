/** Description:
This specialized software component, known as a C++ class, acts as a **driver** for the MCP23017 chip. The MCP23017 is a hardware device commonly used to expand the number of input/output pins available to a microcontroller, allowing the main computer to interact with more external devices like buttons, lights, or sensors.

The core function of this code is to translate complex digital communications into simple instructions.

### How the Code Works

The entire system relies on a standard method of communication called I2C. The software handles all the low-level I2C messaging to configure and operate the external chip, which has two independent groups of pins (Channel A and Channel B).

**Key Capabilities:**

1.  **Initialization:** It establishes the connection with the MCP23017 chip using its network address.
2.  **Direction Control:** It allows the user to configure the pins on Channel A or B. For example, setting them as **inputs** (to read the state of a button) or as **outputs** (to control a light).
3.  **Input Configuration:** Functions are included to activate special internal circuits called pull-up resistors, which ensure reliable readings when connecting mechanical switches.
4.  **Reading Status:** It can quickly read the status of all eight pins on either channel simultaneously.
5.  **Pin Check:** Specific functions are available to check if a single individual pin is currently active or "High."

In essence, this software makes complex hardware control accessible through simple, reliable function calls.

sha: ed79f688bac310ffc0e74ee7f30b42a20c1410840b5183defa4d775fd8027dff 
*/
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
