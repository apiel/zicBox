#pragma once

#include <thread>

#include "controllerInterface.h"
#include "helpers/Mcp23017.h"
#include "helpers/controller.h"
#include "log.h"
// #include "helpers/getTicks.h"

using namespace std;

class Mcp23017Controller : public ControllerInterface {
protected:
    Mcp23017 mcp;
    int address = 0x20; // Default I2C address of MCP23017

    std::thread loopThread;
    bool loopRunning = true;

    struct Key {
        int key;
        uint8_t lastState = 0;
    };

    Key A0 = { -1, 0 };
    Key A1 = { -1, 0 };
    Key A2 = { -1, 0 };
    Key A3 = { -1, 0 };
    Key A4 = { -1, 0 };
    Key A5 = { -1, 0 };
    Key A6 = { -1, 0 };
    Key A7 = { -1, 0 };
    Key B0 = { -1, 0 };
    Key B1 = { -1, 0 };
    Key B2 = { -1, 0 };
    Key B3 = { -1, 0 };
    Key B4 = { -1, 0 };
    Key B5 = { -1, 0 };
    Key B6 = { -1, 0 };
    Key B7 = { -1, 0 };

    void setMapping(nlohmann::json& config, Key& key)
    {
        if (config.is_number()) {
            key.key = config;
        } else if (config.is_string()) {
            key.key = getKeyCode(config.get<std::string>().c_str());
        }
    }

public:
    Mcp23017Controller(Props& props, uint16_t id)
        : ControllerInterface(props, id)
    {
    }

    void config(nlohmann::json& config) override
    {
        address = config.value("address", address);

        if (!mcp.init(address)) {
            logError("Failed to init MCP23017");
            return;
        }

        // Set all pins as inputs
        if (!mcp.setInputModeChannelA() || !mcp.setInputModeChannelB()) {
            logError("Failed to set I/O direction");
            return;
        }

        // Enable pull-up resistors on all pins
        if (!mcp.enablePullUpChannelA() || !mcp.enablePullUpChannelB()) {
            logError("Failed to enable pull-up resistors");
            return;
        }

        setMapping(config["A0"], A0);
        setMapping(config["A1"], A1);
        setMapping(config["A2"], A2);
        setMapping(config["A3"], A3);
        setMapping(config["A4"], A4);
        setMapping(config["A5"], A5);
        setMapping(config["A6"], A6);
        setMapping(config["A7"], A7);
        setMapping(config["B0"], B0);
        setMapping(config["B1"], B1);
        setMapping(config["B2"], B2);
        setMapping(config["B3"], B3);
        setMapping(config["B4"], B4);
        setMapping(config["B5"], B5);
        setMapping(config["B6"], B6);
        setMapping(config["B7"], B7);

        startThread();
    }

    void startThread()
    {
        loopThread = std::thread(&Mcp23017Controller::loop, this);
        pthread_setname_np(loopThread.native_handle(), "mcp23017Controller");
    }

    void loop()
    {
        while (loopRunning) {
            handler();
            std::this_thread::sleep_for(5ms);
        }
    }

    void checkState(Key& key, uint8_t channel, int8_t gpio)
    {
        int8_t state = mcp.isChannelPinHigh(channel, gpio) ? 0 : 1; // Low is pressed
        if (key.lastState != state) {
            onKey(id, key.key, state);
            key.lastState = state;
        }
    }

    void handler()
    {
        uint8_t channelA = mcp.readChannelA();
        uint8_t channelB = mcp.readChannelB();

        checkState(A0, channelA, 0);
        checkState(A1, channelA, 1);
        checkState(A2, channelA, 2);
        checkState(A3, channelA, 3);
        checkState(A4, channelA, 4);
        checkState(A5, channelA, 5);
        checkState(A6, channelA, 6);
        checkState(A7, channelA, 7);
        checkState(B0, channelB, 0);
        checkState(B1, channelB, 1);
        checkState(B2, channelB, 2);
        checkState(B3, channelB, 3);
        checkState(B4, channelB, 4);
        checkState(B5, channelB, 5);
        checkState(B6, channelB, 6);
        checkState(B7, channelB, 7);
    }
};
