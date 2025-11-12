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

#ifndef FS_ROOT_FOLDER
#define FS_ROOT_FOLDER "."
#endif

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
    }

    void init()
    {
        hydratePreset();
        setView(*selectedEngineView);
    }

    int8_t getSelectedEngine() override { return selectedEngine; }
    int8_t getEngineCount() override { return engineCount; }
    void selectEngine(int8_t index) override
    {
        selectedEngine = CLAMP(index, 0, engineCount - 1);
        selectedEngineView = &engineAndViews[selectedEngine].view;
        audio.setEngine(&engineAndViews[selectedEngine].engine);
        // setView(*selectedEngineView);
        // currentView = selectedEngineView;
    }

    void commonRender()
    {
        if (audio.isPlaying) {
            uint8_t x = 120;
            draw.line({ x, 0 }, { x + 5, 3 });
            draw.line({ x + 5, 3 }, { x, 6 });
            draw.line({ x, 0 }, { x, 6 });
        }
    }

    bool render()
    {
        bool shouldRender = draw.shouldRender();
        if (shouldRender) {
            currentView->render();
            commonRender();
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

    bool shift = false;
    void onKey(uint16_t id, int key, int8_t state)
    {
        if (!currentView->onKey(id, key, state)) {
            // logDebug("onKey id %d key %d state %d", id, key, state);
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
                if (state == 1) {
                    serializePreset();
                }
            } else if (key == 29) { // z
                if (shift) {
                    if (state == 1) {
                        audio.isPlaying = !audio.isPlaying;
                        // logDebug("playing %d", audio.isPlaying);
                        draw.renderNext();
                    }
                } else if (state == 1) {
                    audio.noteOn(60, 1.0f);
                } else {
                    audio.noteOff(60);
                }
            } else if (key == 27) { // x
                if (state == 0) {
                    // setView(metalicView);
                }
            } else if (key == 6) { // c
                shift = state == 1;
            }
        }
    }

    void setView(View& view) override
    {
        currentView = &view;
        draw.renderNext();
    }

    void serializePreset()
    {
        const std::string filename = std::string(FS_ROOT_FOLDER) + "/preset.bin";
        std::ofstream out(filename, std::ios::binary);

        if (!out) {
            logError("Could not open file %s", filename.c_str());
            return;
        }

        std::vector<Engine::KeyValue> preset = audio.engine->serialize();
        preset.push_back({ "engine", audio.engine->shortName });
        preset.push_back({ "volume", audio.volume });
        preset.push_back({ "fx1", audio.fx1.getShortName() });
        preset.push_back({ "fx1Amount", audio.fx1Amount });
        preset.push_back({ "fx2", audio.fx2.getShortName() });
        preset.push_back({ "fx2Amount", audio.fx2Amount });
        preset.push_back({ "fx3", audio.fx3.getShortName() });
        preset.push_back({ "fx3Amount", audio.fx3Amount });

        logDebug("serializePreset %s size: %d (%s)", audio.engine->shortName.c_str(), preset.size(), filename.c_str());
        for (const auto& kv : preset) {
            out.write("!", 1);
            uint8_t key_len = kv.key.size();
            out.write((char*)&key_len, 1);
            out.write(kv.key.data(), key_len);

            if (std::holds_alternative<float>(kv.value)) {
                char type = 'f';
                out.write(&type, 1);
                float f = std::get<float>(kv.value);
                out.write(reinterpret_cast<const char*>(&f), sizeof(f));
                logDebug("- key: %s value: %f", kv.key.c_str(), f);
            } else if (std::holds_alternative<std::string>(kv.value)) {
                char type = 's';
                out.write(&type, 1);
                const std::string& s = std::get<std::string>(kv.value);
                uint16_t len = s.size();
                out.write(reinterpret_cast<const char*>(&len), sizeof(len));
                out.write(s.data(), len);
                logDebug("- key: %s value: %s", kv.key.c_str(), s.c_str());
            }
        }

        std::vector<std::vector<Sequencer::KeyValue>> sequences = audio.seq.serialize();
        for (const auto& sequence : sequences) {
            out.write("$", 1);
            uint8_t len = sequence.size();
            out.write(reinterpret_cast<const char*>(&len), 1);
            for (const auto& kv : sequence) {
                out.write(reinterpret_cast<const char*>(&kv.key), 1);
                out.write(reinterpret_cast<const char*>(&kv.value), sizeof(kv.value));
            }
        }
    }

    void hydratePreset()
    {
        const std::string filename = std::string(FS_ROOT_FOLDER) + "/preset.bin";
        std::ifstream in(filename, std::ios::binary);

        logDebug("hydratePreset %s", filename.c_str());

        if (!in) {
            logError("Could not open file %s", filename.c_str());
            return;
        }

        std::vector<Engine::KeyValue> preset;
        std::vector<std::vector<Sequencer::KeyValue>> sequences;
        while (in.peek() != EOF) {
            char c;
            in.read(&c, 1);
            if (c == '!') { // preset engine
                uint8_t key_len;
                in.read((char*)&key_len, 1);
                std::string key(key_len, 0);
                in.read(key.data(), key_len);

                uint8_t type;
                in.read((char*)&type, 1);

                if (type == 'f') { // float
                    float f;
                    in.read((char*)&f, sizeof(f));
                    preset.push_back({ key, f });
                    logDebug("- key: %s value: %f", key.c_str(), f);
                } else if (type == 's') { // string
                    uint16_t str_len;
                    in.read((char*)&str_len, sizeof(str_len));
                    std::string value(str_len, 0);
                    in.read(value.data(), str_len);
                    preset.push_back({ key, value });
                    logDebug("- key: %s value: %s", key.c_str(), value.c_str());

                    if (key == "engine") {
                        logDebug("loadEngine %s", value.c_str());
                        loadEngine(value);
                    }
                }
            } else if (c == '$') { // sequences
                std::vector<Sequencer::KeyValue> sequence;
                uint8_t len;
                in.read((char*)&len, 1);
                for (uint8_t i = 0; i < len; i++) {
                    Sequencer::KeyValue kv;
                    in.read((char*)&kv.key, 1);
                    in.read((char*)&kv.value, sizeof(kv.value));
                    sequence.push_back(kv);
                }
                sequences.push_back(sequence);
            }
        }

        for (auto& kv : preset) {
            if (kv.key == "volume") audio.volume = (std::get<float>(kv.value));
            else if (kv.key == "fx1") audio.fx1.set(std::get<std::string>(kv.value));
            else if (kv.key == "fx1Amount") audio.fx1Amount = (std::get<float>(kv.value));
            else if (kv.key == "fx2") audio.fx2.set(std::get<std::string>(kv.value));
            else if (kv.key == "fx2Amount") audio.fx2Amount = (std::get<float>(kv.value));
            else if (kv.key == "fx3") audio.fx3.set(std::get<std::string>(kv.value));
            else if (kv.key == "fx3Amount") audio.fx3Amount = (std::get<float>(kv.value));
        }

        audio.engine->hydrate(preset);
        audio.seq.hydrate(sequences);
    }

    void loadEngine(std::string engineName)
    {
        logDebug("loadEngine %s", engineName.c_str());
        for (int8_t i = 0; i < engineCount; i++) {
            if (engineAndViews[i].engine.shortName == engineName) {
                selectEngine(i);
                return;
            }
        }
    }
};