#pragma once

#include <fstream>

#include "audio.h"
#include "draw/drawMono.h"
#include "helpers/enc.h"
#include "log.h"

#include "clapView.h"
#include "engineView.h"
#include "fxView.h"
#include "metalicView.h"
#include "snareHatView.h"
#include "stringView.h"
#include "toneView.h"

#include "uiManagerInterface.h"

class UIManager : public UIManagerInterface {
public:
    Audio& audio = Audio::get();

    static const uint8_t encoderCount = 9;
    static const int width = 128;
    static const int height = 128;

    DrawMono<width, height> draw;

    EngineView engineView;
    ToneView toneView;
    ClapView clapView;
    StringView stringView;
    MetalicView metalicView;
    SnareHatView snareHatView;
    FxView fxView;

    View* currentView = &toneView;

    static const int8_t engineCount = 5;
    struct EngineAndView {
        View& view;
        Engine& engine;
    } engineAndViews[engineCount] = {
        { toneView, audio.tone },
        { clapView, audio.clap },
        { stringView, audio.drumString },
        { metalicView, audio.metalic },
        { snareHatView, audio.snareHat },
    };
    int8_t selectedEngine = 0;
    View* selectedEngineView = &engineAndViews[selectedEngine].view;

    UIManager()
        : engineView(draw, *this)
        , toneView(draw)
        , clapView(draw)
        , stringView(draw)
        , metalicView(draw)
        , snareHatView(draw)
        , fxView(draw)
    {
        currentView->render();
        draw.renderNext();
        selectEngine(selectedEngine);

        hydratePreset();
    }

    int8_t getSelectedEngine() override { return selectedEngine; }
    int8_t getEngineCount() override { return engineCount; }
    void selectEngine(int8_t index) override
    {
        selectedEngine = CLAMP(index, 0, engineCount - 1);
        selectedEngineView = &engineAndViews[selectedEngine].view;
        audio.setEngine(&engineAndViews[selectedEngine].engine);
        setView(*selectedEngineView);
    }

    bool render()
    {
        bool shouldRender = draw.shouldRender();
        if (shouldRender) {
            currentView->render();
        }
        return shouldRender;
    }

    uint64_t lastEncoderTick[encoderCount] = { 0 };
    void onEncoder(int id, int8_t direction, uint64_t tick)
    {
        int scaledDirection = encGetScaledDirection(direction, tick, lastEncoderTick[id]);
        lastEncoderTick[id] = tick;
        logDebug("onEncoder %d dir:%d", id, scaledDirection);
        currentView->onEncoder(id, scaledDirection, tick);
    }

    void onKey(uint16_t id, int key, int8_t state)
    {
        if (!currentView->onKey(id, key, state)) {
            logDebug("onKey id %d key %d state %d", id, key, state);
            if (key == 4) { // a
                if (state == 0) {
                    setView(*selectedEngineView);
                } else {
                    setView(engineView);
                }
            } else if (key == 22) { // s
                if (state == 0) {
                    setView(fxView);
                }
            } else if (key == 7) { // d
                if (state == 0) {
                    // setView(stringView);
                }
            } else if (key == 9) { // f
                if (state == 0) {
                    // setView(fxView);
                }
            } else if (key == 29) { // z
                if (state == 1) {
                    audio.noteOn(60, 1.0f);
                } else {
                    audio.noteOff(60);
                }
            } else if (key == 27) { // x
                if (state == 0) {
                    // setView(metalicView);
                }
            } else if (key == 6) { // c
                if (state == 0) {
                    serializePreset();
                }
            }
        }
    }

    void setView(View& view) override
    {
        currentView = &view;
        draw.renderNext();
    }


    void serializePreset() {
        const std::string filename = "preset.bin";
        std::ofstream out(filename, std::ios::binary);

        std::vector<Engine::KeyValue> preset = audio.engine->serialize();
        preset.push_back({ "engine", audio.engine->shortName });
        logDebug("serializePreset %s size: %d", audio.engine->shortName.c_str(), preset.size());
        for (const auto &kv : preset) {
            uint8_t key_len = kv.key.size();
            out.write((char*)&key_len, 1);
            out.write(kv.key.data(), key_len);
            logDebug("- key: %s", kv.key.c_str());

            if (std::holds_alternative<float>(kv.value)) {
                char type = 'f';
                out.write(&type, 1);
                float f = std::get<float>(kv.value);
                out.write(reinterpret_cast<const char*>(&f), sizeof(f));
            } else if (std::holds_alternative<std::string>(kv.value)) {
                char type = 's';
                out.write(&type, 1);
                const std::string &s = std::get<std::string>(kv.value);
                uint16_t len = s.size();
                out.write(reinterpret_cast<const char*>(&len), sizeof(len));
                out.write(s.data(), len);
            }
        }
    }

    void hydratePreset() {
        const std::string filename = "preset.bin";
        std::ifstream in(filename, std::ios::binary);

        std::vector<Engine::KeyValue> preset;
        while (in.peek() != EOF) {
            uint8_t key_len;
            in.read((char*)&key_len, 1);
            std::string key(key_len, 0);
            in.read(key.data(), key_len);

            uint8_t type;
            in.read((char*)&type, 1);

            if (type == 'f') { // float
                float f;
                in.read((char*)&f, sizeof(f));
                preset.push_back({key, f});
            } else if (type == 's') { // string
                uint16_t str_len;
                in.read((char*)&str_len, sizeof(str_len));
                std::string value(str_len, 0);
                in.read(value.data(), str_len);
                preset.push_back({key, value});

                if (key == "engine") {
                    loadEngine(value);
                }
            }
        }

        audio.engine->hydrate(preset);
    }

    void loadEngine(std::string engineName) {
        logDebug("loadEngine %s", engineName.c_str());
        for (int8_t i = 0; i < engineCount; i++) {
            if (engineAndViews[i].engine.shortName == engineName) {
                selectEngine(i);
                return;
            }
        }
    }
};