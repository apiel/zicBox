#pragma once

#include "Mcp23017Controller.h"
#include "controllerInterface.h"
#include "helpers/GpioEncoder.h"
#include "helpers/GpioKey.h"
#include "helpers/NeoTrellis.h"
#include "helpers/getTicks.h"
#include "log.h"

class PixelController : public ControllerInterface {
protected:
    GpioKey gpioKey = GpioKey({},
        [this](GpioKey::Key key, uint8_t state) {
            //   printf("gpio%d key [%d] state changed %d\n", key.gpio, key.key, state);
            onKey(id, key.key, state);
        });

    GpioEncoder gpioEncoder = GpioEncoder({},
        [this](GpioEncoder::Encoder enc, int8_t direction) {
            // printf("encoder%d gpioA%d gpioB%d direction %d\n", encoder.id, encoder.gpioA, encoder.gpioB, direction);
            encoder(enc.id, direction, getTicks());
        });

    NeoTrellis trellis1;
    NeoTrellis trellis2;
    NeoTrellis trellis3;
    std::vector<int> trellisKeys1;
    std::vector<int> trellisKeys2;
    std::vector<int> trellisKeys3;
    bool trellisUpdated1 = false;
    bool trellisUpdated2 = false;
    bool trellisUpdated3 = false;

public:
    PixelController(Props& props, uint16_t id)
        : ControllerInterface(props, id)
        , mcp23017Controller(props, id)
        , trellis1([this](uint8_t num, bool pressed) {
            // logDebug("Trellis1: %d %s", num, pressed ? "pressed" : "released");
            onKey(this->id, this->trellisKeys1.at(num), pressed);
        })
        , trellis2([this](uint8_t num, bool pressed) {
            // logDebug("Trellis2: %d %s", num, pressed ? "pressed" : "released");
            onKey(this->id, this->trellisKeys2.at(num), pressed);
        })
        , trellis3([this](uint8_t num, bool pressed) {
            // logDebug("Trellis3: %d %s", num, pressed ? "pressed" : "released");
            onKey(this->id, this->trellisKeys3.at(num), pressed);
        })
    {
    }

    void setLayout(std::string layout)
    {
        if (layout == "rpi0_4enc_6btn") {
            gpioKey.keys = {
                { 12, getKeyCode("'q'") }, // pin 32 = gpio 12
                { 24, getKeyCode("'w'") }, // pin 18 = gpio 24
                { 27, getKeyCode("'e'") }, // pin 13 = gpio 27
                { 8, getKeyCode("'a'") }, // pin 24 = gpio 8
                { 25, getKeyCode("'s'") }, // pin 22 = gpio 25
                { 23, getKeyCode("'d'") }, // pin 16 = gpio 23

                { 0, getKeyCode("'0'") }, // pin 27 = gpio 0   <-- push encoder 0
                { 16, getKeyCode("'1'") }, // pin 36 = gpio 16 <-- push encoder 1
                { 5, getKeyCode("'2'") }, // pin 29 = gpio 5   <-- push encoder 2
                { 14, getKeyCode("'3'") }, // pin 8 = gpio 14  <-- push encoder 3
            };
            if (gpioKey.init() == 0) {
                gpioKey.startThread(); // might want to use the same thread for encoder...
            }
            gpioEncoder.encoders = {
                { 0, 26, 13 }, // pin 37 = gpio 26, pin 33 = gpio 13
                { 1, 20, 6 }, // pin 38 = gpio 20, pin 31 = gpio 6
                { 2, 7, 9 }, // pin 26 = gpio 7, pin 21 = gpio 9
                { 3, 15, 4 }, // pin 10 = gpio 15, pin 7 = gpio 4
            };
            if (gpioEncoder.init() == 0) {
                gpioEncoder.startThread(); // might want to use the same thread for encoder...
            }
        } else if (layout == "rpi3A_4enc_11btn") {
            gpioKey.keys = {
                { 27, getKeyCode("F4") }, // pin 13 = gpio 27
                { 4, getKeyCode("'4'") }, // pin 7 = gpio 4
                { 14, getKeyCode("'r'") }, // pin 8 = gpio 14

                { 16, getKeyCode("'a'") }, // pin 36 = gpio 16
                { 1, getKeyCode("'s'") }, // pin 28 = gpio 1
                { 7, getKeyCode("'d'") }, // pin 26 = gpio 7
                { 15, getKeyCode("'f'") }, // pin 10 = gpio 15

                { 12, getKeyCode("'z'") }, // pin 32 = gpio 12
                { 5, getKeyCode("'x'") }, // pin 29 = gpio 5
                { 0, getKeyCode("'c'") }, // pin 27 = gpio 0
                { 8, getKeyCode("'v'") }, // pin 24 = gpio 8

            };
            if (gpioKey.init() == 0) {
                gpioKey.startThread(); // might want to use the same thread for encoder...
            }
            gpioEncoder.encoders = {
                { 0, 13, 6 }, // gpio 13, gpio 6
                { 1, 24, 23 }, // gpio 24, gpio 23
                { 2, 20, 26 }, // gpio 20, gpio 26
                { 3, 9, 25 }, // gpio 9, gpio 25
            };
            if (gpioEncoder.init() == 0) {
                gpioEncoder.startThread(); // might want to use the same thread for encoder...
            }
        } else if (layout == "rpiCM4_4enc_11btn" || layout == "rpi3A_4enc_11btn_v3") {
            gpioKey.keys = {
                { 27, getKeyCode("F4") }, // pin 13 = gpio 27
                { 4, getKeyCode("'4'") }, // pin 7 = gpio 4
                { 14, getKeyCode("'r'") }, // pin 8 = gpio 14

                { 16, getKeyCode("'a'") }, // pin 36 = gpio 16
                { 1, getKeyCode("'s'") }, // pin 28 = gpio 1
                { 7, getKeyCode("'d'") }, // pin 26 = gpio 7
                { 15, getKeyCode("'f'") }, // pin 10 = gpio 15

                { 12, getKeyCode("'z'") }, // pin 32 = gpio 12
                { 5, getKeyCode("'x'") }, // pin 29 = gpio 5
                { 0, getKeyCode("'c'") }, // pin 27 = gpio 0
                { 8, getKeyCode("'v'") }, // pin 24 = gpio 8

            };
            if (gpioKey.init() == 0) {
                gpioKey.startThread(); // might want to use the same thread for encoder...
            }
            gpioEncoder.encoders = {
                { 0, 13, 6 }, // gpio 13, gpio 6
                { 1, 23, 24 }, // gpio 24, gpio 23
                { 2, 20, 26 }, // gpio 20, gpio 26
                { 3, 25, 9 }, // gpio 9, gpio 25
            };
            if (gpioEncoder.init() == 0) {
                gpioEncoder.startThread(); // might want to use the same thread for encoder...
            }
        } else if (layout == "grid") {
            gpioEncoder.encoders = {
                { 1, 20, 19 },
                { 2, 13, 6 },
                { 3, 5, 7 },
                { 4, 8, 11 },

                { 5, 21, 26 },
                { 6, 9, 10 },
                { 7, 27, 17 },
                { 8, 22, 4 },

                { 9, 16, 12 },
                { 10, 25, 24 },
                { 11, 23, 18 },
                { 12, 15, 14 },
            };
            if (gpioEncoder.init() == 0) {
                gpioEncoder.startThread(); // might want to use the same thread for encoder...
            }

            try {
                trellisKeys1 = {
                    getKeyCode("'f'"),
                    getKeyCode("'d'"),
                    getKeyCode("'s'"),
                    getKeyCode("'a'"),

                    getKeyCode("'r'"),
                    getKeyCode("'e'"),
                    getKeyCode("'w'"),
                    getKeyCode("'q'"),

                    getKeyCode("'4'"),
                    getKeyCode("'3'"),
                    getKeyCode("'2'"),
                    getKeyCode("'1'"),

                    getKeyCode("F4"),
                    getKeyCode("F3"),
                    getKeyCode("F2"),
                    getKeyCode("F1"),
                };
                trellisKeys2 = {
                    getKeyCode("'k'"),
                    getKeyCode("'j'"),
                    getKeyCode("'h'"),
                    getKeyCode("'g'"),

                    getKeyCode("'t'"),
                    getKeyCode("'y'"),
                    getKeyCode("'u'"),
                    getKeyCode("'i'"),

                    getKeyCode("'8'"),
                    getKeyCode("'7'"),
                    getKeyCode("'6'"),
                    getKeyCode("'5'"),

                    getKeyCode("F8"),
                    getKeyCode("F7"),
                    getKeyCode("F6"),
                    getKeyCode("F5"),
                };
                trellisKeys3 = {
                    getKeyCode("'enter'"),
                    getKeyCode("'\''"),
                    getKeyCode("';'"),
                    getKeyCode("'l'"),

                    getKeyCode("'['"),
                    getKeyCode("']'"),
                    getKeyCode("'p'"),
                    getKeyCode("'o'"),

                    getKeyCode("'='"),
                    getKeyCode("'-'"),
                    getKeyCode("'0'"),
                    getKeyCode("'9'"),

                    getKeyCode("F12"),
                    getKeyCode("F11"),
                    getKeyCode("F10"),
                    getKeyCode("F9"),
                };

                trellis1.begin();
                trellis1.startThread("neotrellis1");
                trellis2.begin(0x30);
                trellis2.startThread("neotrellis2");
                trellis3.begin(0x2F);
                trellis3.startThread("neotrellis2");
                setColorCb = [this](int id, Color color) {
                    // search in trellisKeys1 and get the index
                    int index = std::find(trellisKeys1.begin(), trellisKeys1.end(), id) - trellisKeys1.begin();
                    if (index < trellisKeys1.size()) {
                        NeoTrellis::Color neoColor(color.r, color.g, color.b);
                        trellis1.setPixelColorAsync(index, neoColor);
                        trellisUpdated1 = true;
                        return;
                    }
                    // search in trellisKeys2 and get the index
                    index = std::find(trellisKeys2.begin(), trellisKeys2.end(), id) - trellisKeys2.begin();
                    if (index < trellisKeys2.size()) {
                        NeoTrellis::Color neoColor(color.r, color.g, color.b);
                        trellis2.setPixelColorAsync(index, neoColor);
                        trellisUpdated2 = true;
                        return;
                    }
                    // search in trellisKeys3 and get the index
                    index = std::find(trellisKeys3.begin(), trellisKeys3.end(), id) - trellisKeys3.begin();
                    if (index < trellisKeys3.size()) {
                        NeoTrellis::Color neoColor(color.r, color.g, color.b);
                        trellis3.setPixelColorAsync(index, neoColor);
                        trellisUpdated3 = true;
                        return;
                    }
                };
                renderColorsCb = [this]() {
                    if (trellisUpdated1) {
                        trellis1.showAsync();
                        trellisUpdated1 = false;
                    }
                    if (trellisUpdated2) {
                        trellis2.showAsync();
                        trellisUpdated2 = false;
                    }
                    if (trellisUpdated3) {
                        trellis3.showAsync();
                        trellisUpdated3 = false;
                    }
                };
            } catch (const std::exception& e) {
                logError("Application Error: %s", e.what());
                return;
            }
        }
    }

    std::function<void(int id, Color color)> setColorCb = [](int id, Color color) { };
    void setColor(int id, Color color) override
    {
        setColorCb(id, color);
    }

    std::function<void()> renderColorsCb = []() { };
    virtual void renderColors() override
    {
        renderColorsCb();
    }

    Mcp23017Controller mcp23017Controller;
    void setI2c(std::string type)
    {
        if (type == "pixel+_v1") {
            nlohmann::json config;
            config["A0"] = "'c'";
            config["A1"] = "'x'";
            config["A2"] = "'z'";
            config["A3"] = "'s'";
            config["A4"] = "'a'";
            config["A5"] = "'q'";
            config["A6"] = "'1'";
            config["A7"] = "F1";
            config["B0"] = "'v'";
            config["B1"] = "'b'";
            config["B2"] = "'g'";
            config["B3"] = "'f'";
            config["B4"] = "'d'";
            config["B5"] = "'t'";
            config["B6"] = "'5'";
            config["B7"] = "F5";
            mcp23017Controller.config(config);
        }
    }

    void config(nlohmann::json& config) override
    {
        setLayout(config.value("pixelController", "rpi3A_4enc_11btn"));
        if (config.contains("i2c") && config["i2c"].is_array()) {
            for (auto& i2c : config["i2c"]) {
                setI2c(i2c);
            }
        }
    }
};
