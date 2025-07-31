#pragma once

#include <sstream>
#include <stdio.h>
#include <vector>

#include "log.h"

class EnvelopRelative {
public:
    unsigned int index = -1;
    float currentModulation = 0.0f;

    struct Data {
        float modulation;
        float time;
        float incRatio = 0.0f; // Precomputed increment ratio
    };

    std::vector<Data> data;

    EnvelopRelative(std::vector<Data> data, int8_t _minEditablePhase = 0)
        : data(data)
    {
        setMinEditablePhase(_minEditablePhase);
    }

    struct Mode {
        std::string name;
        std::function<void(EnvelopRelative*, bool)> func;
    };

    std::vector<Mode> modes;

    EnvelopRelative(std::vector<Mode> modes, int8_t _minEditablePhase = 0)
        : modes(modes)
    {
        setMinEditablePhase(_minEditablePhase);
    }

    void setMinEditablePhase(int8_t phase)
    {
        minEditablePhase = phase;
        if (currentEditPhase < minEditablePhase) {
            currentEditPhase = minEditablePhase;
        }
    }

    float next()
    {
        if (index > data.size() - 1) {
            return 0.0f;
        }

        currentModulation += data[index].incRatio;
        if ((data[index].incRatio > 0 && currentModulation >= data[index + 1].modulation) || (data[index].incRatio < 0 && currentModulation <= data[index + 1].modulation)) {
            index++;
            currentModulation = data[index].modulation;
        }
        return currentModulation;
    }

    void reset(float totalSamples)
    {
        index = 0;
        currentModulation = 0.0f;
        calculateIncrements(totalSamples);
    }

    float lastTotalSamples = 1.0f;
    void calculateIncrements(float totalSamples)
    {
        lastTotalSamples = totalSamples;
        for (size_t i = 0; i < data.size() - 1; ++i) {
            float sampleCount = (data[i + 1].time - data[i].time) * totalSamples;
            data[i].incRatio = (sampleCount > 0) ? (data[i + 1].modulation - data[i].modulation) / sampleCount : 0.0f;
        }
    }

protected:
    int8_t minEditablePhase;

public:
    int8_t currentEditPhase = 1;
    int8_t* updateEditPhase(int8_t* direction = NULL)
    {
        if (direction) {
            if (*direction > 0) {
                if (currentEditPhase < data.size() - 1) {
                    currentEditPhase++;
                }
            } else if (currentEditPhase > minEditablePhase) {
                currentEditPhase--;
            }
        }
        return &currentEditPhase;
    }

    float* updatePhaseTime(int8_t* direction = NULL)
    {
        if (direction && currentEditPhase >= minEditablePhase) {
            if (currentEditPhase == data.size() - 1) {
                data.push_back({ 0.0f, 1.0f });
            }
            float value = data[currentEditPhase].time + ((*direction) * 0.01f);

            if (currentEditPhase > 1 && value <= data[currentEditPhase - 1].time) {
                return &data[currentEditPhase].time;
            }
            if (currentEditPhase < data.size() - 1 && value >= data[currentEditPhase + 1].time) {
                return &data[currentEditPhase].time;
            }

            data[currentEditPhase].time = range(value, 0.0f, 1.0f);
        }
        return &data[currentEditPhase].time;
    }

    float* updatePhaseModulation(int8_t* direction = NULL)
    {
        if (direction && currentEditPhase >= minEditablePhase) {
            float value = data[currentEditPhase].modulation + (*direction * 0.01f);
            data[currentEditPhase].modulation = range(value, 0.0f, 1.0f);
        }
        return &data[currentEditPhase].modulation;
    }

    int mode = 0;
    bool useMacro = false;
    struct Macro {
        float a = 0.5;
        float b = 0.5;
        float c = 0.5;
    } macro;

    std::string modeStrPtr;
    std::string* updateMode(int8_t* direction = NULL)
    {
        if (direction != NULL) {
            setMode(mode + *direction);
        }
        modeStrPtr = getModeStr(mode);
        return &modeStrPtr;
    }

    float fEditPhase = 0.0f;

    float* updateMacro1(int8_t* direction = NULL)
    {
        if (direction != NULL) {
            if (!useMacro) {
                fEditPhase = *updateEditPhase(direction);
                return &fEditPhase;
            }
            if (direction != NULL) {
                macro.a += *direction * 0.01f;
                macro.a = range(macro.a, 0.0f, 1.0f);
                setMode(mode, false);
            }
        }
        return &macro.a;
    }

    float* updateMacro2(int8_t* direction = NULL)
    {
        if (direction != NULL) {
            if (!useMacro) {
                return updatePhaseModulation(direction);
            }
            if (direction != NULL) {
                macro.b += *direction * 0.01f;
                macro.b = range(macro.b, 0.0f, 1.0f);
                setMode(mode, false);
            }
        }
        return &macro.b;
    }

    float* updateMacro3(int8_t* direction = NULL)
    {
        if (direction != NULL) {
            if (!useMacro) {
                return updatePhaseTime(direction);
            }
            if (direction != NULL) {
                macro.c += *direction * 0.01f;
                macro.c = range(macro.c, 0.0f, 1.0f);
                setMode(mode, false);
            }
        }
        return &macro.c;
    }

    void setMode(int _mode, bool init = true)
    {
        mode = range(_mode, 0, (int)modes.size());
        data.clear();

        if (mode >= 0 && mode < modes.size()) {
            modes[mode].func(this, init);
            calculateIncrements(lastTotalSamples);
            return;
        }

        // default
        useMacro = false;
        data.push_back({ 1.0f, 0.0f });
        data.push_back({ 0.0f, 1.0f });
    }

    std::string getModeStr(int _mode)
    {
        if (mode >= 0 && mode < modes.size()) {
            return modes[mode].name;
        }

        return "Default";
    }

    void serializeJson(nlohmann::json& json)
    {
        json["mode"] = mode;
        if (useMacro) {
            nlohmann::json m;
            m["a"] = macro.a;
            m["b"] = macro.b;
            m["c"] = macro.c;
            json["macro"] = m;
        } else {
            nlohmann::json phases;
            for (EnvelopRelative::Data& phase : data) {
                nlohmann::json p;
                p["modulation"] = phase.modulation;
                p["time"] = phase.time;
                phases.push_back(p);
            }
            json["phases"] = phases;
        }
    }

    void hydrateJson(nlohmann::json& json)
    {
        mode = json["mode"];
        setMode(mode, false);
        if (useMacro) {
            if (json.contains("macro")) {
                macro.a = json["macro"]["a"];
                macro.b = json["macro"]["b"];
                macro.c = json["macro"]["c"];
            } else {
                logWarn("Missing macro data in json");
            }
        } else {
            data.clear();
            if (json.contains("phases")) {
                for (nlohmann::json& phase : json["phases"]) {
                    data.push_back({ phase["modulation"], phase["time"] });
                }
            }
            // If no phase data, initialize default
            if (data.size() == 0) {
                setMode(mode);
            }
        }
    }
};
