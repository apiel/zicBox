#pragma once

#include <cmath>
#include <cstdint>

#include "audio/utils/math.h"
#include "helpers/clamp.h"

class Lfo {
protected:
    float invSampleRate;

    float lfoPhase = 0.0f;
    float lfoSHValue = 0.0f;
    bool lfoDone = false;
    int shape = 0;

    uint32_t seed = 12345;
    float fastNoise()
    {
        seed = 214013 * seed + 2531011;
        return ((float)((seed >> 16) & 0x7FFF) / 32768.0f) * 2.0f - 1.0f;
    }

public:
    float rateHz = 1.0f;
    int type = 0;
    char typeName[16] = "Sin";
    static const int COUNT = 13;
    static constexpr const char* LFO_NAMES[COUNT] = { "Sin", "Saw", "Tri", "Sqr", "Sin Trg", "Saw Trg", "Tri Trg", "Sqr Trg", "Sin One", "Saw One", "Tri One", "Sqr One", "S&H" };

    Lfo(float sampleRate)
        : invSampleRate(1.0f / sampleRate)
    {
        setType(type);
    }

    float getValAtPhase(float ph_in)
    {
        ph_in -= std::floor(ph_in);
        if (type == 12) return lfoSHValue;
        switch (shape) {
        case 1:
            return (1.0f - 2.0f * ph_in);
        case 2:
            return (ph_in < 0.5f) ? (4.0f * ph_in - 1.0f) : (3.0f - 4.0f * ph_in);
        case 3:
            return (ph_in < 0.5f) ? 1.0f : -1.0f;
        default:
            return Math::fastSin(6.2831853f * ph_in);
        }
    }

    float process()
    {
        if (!lfoDone) {
            lfoPhase += rateHz * invSampleRate;
            if (lfoPhase >= 1.0f) {
                if (type >= 8 && type <= 11) {
                    lfoPhase = 1.0f;
                    lfoDone = true;
                } else lfoPhase -= 1.0f;
                if (type == 12) lfoSHValue = fastNoise();
            }
        }
        return getValAtPhase(lfoPhase);
    }

    void reset()
    {
        lfoDone = false;
        if (type >= 4) {
            lfoPhase = 0.0f;
        }
        if (type == 12) lfoSHValue = fastNoise();
    }

    void setType(int type)
    {
        this->type = CLAMP(type, 0, COUNT - 1); // 12 is S&H type;
        shape = this->type % 4;
        strcpy(typeName, LFO_NAMES[type]);
    }
};