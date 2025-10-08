#pragma once

#include "helpers/clamp.h"
#include "plugins/audio/utils/WavetableInterface.h"
#include "plugins/audio/utils/lookupTable.h"

#include <cmath>
#include <array>
#include <cstdlib>

class KickTransientTableGenerator : public WavetableInterface {
public:
    KickTransientTableGenerator()
        : WavetableInterface(LOOKUP_TABLE_SIZE)
    {
        setMorph(0.0f);
    }

    void setMorph(float m)
    {
        morph = CLAMP(m, 0.0f, 1.0f);
        updateTable();
    }

    float getMorph() const { return morph; }

    float next(float index)
    {
        return linearInterpolation(index, sampleCount, lut);
    }

    float sample(float* index, float /*freq*/) override
    {
        *index += 1.0f / static_cast<float>(sampleCount);
        if (*index >= 1.0f)
            *index = 1.0f; // lock to end
        return linearInterpolation(*index, sampleCount, lut);
    }

    float* sample(float* index) override
    {
        return &lut[static_cast<uint64_t>(*index * sampleCount)];
    }

    float* samples() override
    {
        return lut;
    }

private:
    float morph = 0.0f;
    float lut[LOOKUP_TABLE_SIZE];

    static float linearInterpolation(float index, uint64_t size, const float* table)
    {
        float fIndex = index * size;
        int i0 = static_cast<int>(fIndex) % size;
        int i1 = (i0 + 1) % size;
        float frac = fIndex - static_cast<float>(i0);
        return table[i0] * (1.0f - frac) + table[i1] * frac;
    }

    void updateTable()
    {
        for (uint64_t i = 0; i < sampleCount; ++i) {
            float x = static_cast<float>(i) / static_cast<float>(sampleCount);
            lut[i] = generateTransient(x, morph);
        }
    }

    static float generateTransient(float x, float morph)
    {
        struct Family { float (*func)(float,float); };

        static const Family families[13] = {
            // 1. SnappyClick - noise-based snare-like attack
            { [](float x,float a){
                auto exponentialDecay = [](float t,float rate){ return std::exp(-rate * t); };
                auto whiteNoise = [](){ return ((float)rand()/(float)RAND_MAX)*2.0f-1.0f; };
                float env = exponentialDecay(x, 200.0f*(0.3f+0.7f*a));
                return whiteNoise() * env;
            } },
            // 2. TinyClick
            { [](float x,float a){ return ((float)rand()/RAND_MAX-0.5f)*std::exp(-1500.0f*x*(0.3f+0.7f*a)); } },
            // 3. ShortPop
            { [](float x,float a){ return (float)std::sin(2.0f*M_PI*2000.0f*x)*std::exp(-100.0f*x*(0.3f+0.7f*(1.0f-a))); } },
            // 4. WoodTap
            { [](float x,float a){ return (float)std::sin(2.0f*M_PI*400.0f*x)*std::exp(-60.0f*x*(0.4f+0.6f*a)); } },
            // 5. KnockLow
            { [](float x,float a){ return (float)std::sin(2.0f*M_PI*(80.0f+200.0f*a)*x)*std::exp(-40.0f*x); } },
            // 6. Thump
            { [](float x,float a){ return (float)std::sin(2.0f*M_PI*60.0f*x)*std::exp(-15.0f*x*(0.3f+0.7f*a)); } },
            // 7. BodyPop
            { [](float x,float a){ return (float)std::sin(2.0f*M_PI*150.0f*x)*std::exp(-20.0f*x*(0.5f+0.5f*a)); } },
            // 8. MidSlap
            { [](float x,float a){ return (float)std::sin(2.0f*M_PI*(200.0f+400.0f*a)*x)*std::exp(-25.0f*x); } },
            // 9. Punch
            { [](float x,float a){ return (float)std::sin(2.0f*M_PI*120.0f*x)*std::exp(-10.0f*x*(0.3f+0.7f*a)); } },
            // 10. DeepPunch
            { [](float x,float a){ return (float)std::sin(2.0f*M_PI*80.0f*x)*std::exp(-8.0f*x*(0.4f+0.6f*a)); } },
            // 11. WarmTap
            { [](float x,float a){ return (float)std::tanh(std::sin(2.0f*M_PI*300.0f*x)*(1.0f+3.0f*a))*std::exp(-12.0f*x); } },
            // 12. TapeClick
            { [](float x,float a){ return (float)std::tanh(std::sin(2.0f*M_PI*200.0f*x)*(1.0f+4.0f*a))*std::exp(-15.0f*x); } },
            // 13. AnalogCrack
            { [](float x,float a){ return (float)std::tanh(std::sin(2.0f*M_PI*150.0f*x)*(1.0f+5.0f*a))*std::exp(-20.0f*x); } }
        };

        const float pos = morph * (13 - 1 + 0.9999f); // 13 families now
        const int i0 = static_cast<int>(pos);
        const int i1 = std::min(i0 + 1, 12);
        const float frac = pos - static_cast<float>(i0);

        float v0 = families[i0].func(x, 0.5f);
        float v1 = families[i1].func(x, 0.5f);

        return v0 * (1.0f - frac) + v1 * frac;
    }
};
