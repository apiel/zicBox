#ifndef _ZIC_ENVELOP_RELATIVE_H_
#define _ZIC_ENVELOP_RELATIVE_H_

#include <vector>

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
        // setMode(1);
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

    char* updateMode(int8_t* direction = NULL)
    {
        setMode(range(mode + *direction, 0, 2));
        return (char *)getModeStr(mode).c_str();
    }

    Macro* updateMacro(Macro _macro)
    {
        macro = _macro;
        return &macro;
    }

    enum MODE {
        DEFAULT,
        KICK,
        EXPO_DECAY,
    };

    void setMode(int _mode)
    {
        mode = _mode;
        data.clear();

        if (mode == MODE::KICK) {
            useMacro = false;
            data.push_back({ 1.0f, 0.0f });
            data.push_back({ 0.5f, 0.03f });
            data.push_back({ 0.3f, 0.07f });
            data.push_back({ 0.09f, 0.19f });
            data.push_back({ 0.0f, 1.0f });
            return;
        }

        if (mode == MODE::EXPO_DECAY) {
            useMacro = true;
            for (float x = 0.0f; x <= 1.0f; x += 0.01f) {
                // data.push_back({ 1.0f * exp(-15 * x) + 0.2f - 0.2f * x, x });
                // float y = 1 * exp(-15 * x) +  (0.2 - 0.2 * pow(x , 4));
                float a = 70 * macro.a;
                float b = 0.5 * macro.b;
                float c = 100 * macro.c;
                float y = 1 * exp(-a * x) + (b - b * pow(x, c));
                data.push_back({ y, x });
            }
        }

        // default
        useMacro = false;
        data.push_back({ 1.0f, 0.0f });
        data.push_back({ 0.0f, 1.0f });
    }

    std::string getModeStr(int _mode)
    {
        if (_mode == MODE::KICK)
            return "Kick";
        if (_mode == MODE::EXPO_DECAY)
            return "Expo decay";

        return "Default";
    }
};

#endif

// 100 * exp(-1.0 * x) + 1.0
/// --> "100 * exp(-0.2 * x) +  (10 - 0.1 * x)",
// ----> 1 * exp(-10 * x) +  (0.2 - 0.2 * x)

// double exponentialDecay(double x, double a, double b, double c) {
//     return a * exp(-b * x) + c;
// }

// int main() {
//     // Parameters for the function
//     double a = 10.0; // Initial value
//     double b = 1.0;  // Decay rate
//     double c = 1.0;  // Asymptote value

//     // Print the function values for a range of x
//     for (double x = 0; x <= 10; x += 0.5) {
//         double y = exponentialDecay(x, a, b, c);
//         std::cout << "x: " << x << ", y: " << y << std::endl;
//     }

//     return 0;
// }
