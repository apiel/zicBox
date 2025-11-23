/** Description:
This C++ structure, named `EnvelopRelative`, is designed to manage a flexible, time-based curve, commonly used in areas like audio synthesis to control how a parameter changes over time (for example, how quickly a volume rises and falls).

Think of it as a sophisticated recipe for defining change.

### How It Works

1.  **Defining the Shape:** The envelope is defined by a series of points, where each point specifies a **Modulation Value** and a **Time Point**. By connecting these points, a complex shape is created.
2.  **Calculating Output:** The central function calculates the exact output value at any given moment in time. If the time falls between two defined points, the system smoothly interpolates (calculates the value that falls on the straight line) between them, ensuring a continuous transition.
3.  **Interactive Editing:** The structure supports interactive editing. Users can adjust the specific time and modulation value of individual segments of the curve, referred to as "phases." Safety checks are included to ensure phases don't overlap or move beyond set boundaries.

### Advanced Control and Presets

The system offers powerful features for managing complex shapes:

*   **Modes:** These are built-in presets that allow the envelope to be instantly switched to a new shape (e.g., a simple ramp, or a complex oscillatory pattern).
*   **Macros (A, B, C):** For complex presets, specific controls called "Macros" are provided. These act like simple knobs (A, B, and C) that let the user quickly customize the shape of the active Mode without having to manually adjust every point on the curve.

Finally, the entire configuration—including all custom curve points and active Macro settings—can be easily saved to a file and loaded back later.

sha: 2cc839e37f1dd85d334538025e54e1166191d18e0a32aa365ddc0d7566a0b92f 
*/
#pragma once

#include <sstream>
#include <stdio.h>
#include <vector>
#include <functional>

#include "helpers/clamp.h"

class EnvelopRelative {
public:
    unsigned int index = -1;

    struct Data {
        float modulation;
        float time;
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

    float next(float time, unsigned int& indexRef)
    {
        if (indexRef > data.size() - 1) {
            return 0.0f;
        }

        if (time >= data[indexRef + 1].time) {
            indexRef++;
        }
        // TODO optimize: this could be precalculated using the duration of the envelop
        // So we could calculate what is the step increment for the current time range.
        float timeOffset = data[indexRef + 1].time - data[indexRef].time;
        float timeRatio = (time - data[indexRef].time) / timeOffset;
        return (data[indexRef + 1].modulation - data[indexRef].modulation) * timeRatio + data[indexRef].modulation;
    }

    float next(float time)
    {
        return next(time, index);
    }

    void reset()
    {
        reset(index);
    }

    void reset(unsigned int& indexRef)
    {
        indexRef = 0;
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

            data[currentEditPhase].time = CLAMP(value, 0.0f, 1.0f);
        }
        return &data[currentEditPhase].time;
    }

    float* updatePhaseModulation(int8_t* direction = NULL)
    {
        if (direction && currentEditPhase >= minEditablePhase) {
            float value = data[currentEditPhase].modulation + (*direction * 0.01f);
            data[currentEditPhase].modulation = CLAMP(value, 0.0f, 1.0f);
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
                macro.a = CLAMP(macro.a, 0.0f, 1.0f);
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
                macro.b = CLAMP(macro.b, 0.0f, 1.0f);
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
                macro.c = CLAMP(macro.c, 0.0f, 1.0f);
                setMode(mode, false);
            }
        }
        return &macro.c;
    }

    void setMode(int _mode, bool init = true)
    {
        mode = CLAMP(_mode, 0, (int)modes.size());
        data.clear();

        if (mode >= 0 && mode < modes.size()) {
            modes[mode].func(this, init);
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

    void serialize(FILE* file, std::string separator, std::string name)
    {
        fprintf(file, "%s", name.c_str());
        if (useMacro) {
            fprintf(file, " %d %f %f %f", mode, macro.a, macro.b, macro.c);
        } else {
            fprintf(file, " %d", mode);
            for (EnvelopRelative::Data& phase : data) {
                fprintf(file, " %f:%f", phase.modulation, phase.time);
            }
        }
        fprintf(file, "%s", separator.c_str());
    }

    void hydrate(std::string value)
    {
        std::stringstream ss(value);
        std::string token;

        if (ss >> token) {
            int mode = 0;
            sscanf(token.c_str(), "%d", &mode);
            setMode(mode, false);

            if (useMacro) {
                if (ss >> token) {
                    sscanf(token.c_str(), "%f", &macro.a);
                }
                if (ss >> token) {
                    sscanf(token.c_str(), "%f", &macro.b);
                }
                if (ss >> token) {
                    sscanf(token.c_str(), "%f", &macro.c);
                }
                setMode(mode, false);
            } else {
                data.clear();
                while (ss >> token) {
                    float mod = 0, time = 0;
                    sscanf(token.c_str(), "%f:%f", &mod, &time);
                    data.push_back({ mod, time });
                }
            }
        }
    }
};
