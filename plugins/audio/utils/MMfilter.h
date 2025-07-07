#pragma once

#include "filter.h"

class MMfilter {
protected:
    EffectFilterData filter;
    float mix = 0.0;
    float cutoffAmount = 0.0;

public:
    void setResonance(float amount) { 
        filter.resonance = amount; 
        setCutoff(cutoffAmount);
    }

    void setCutoff(float amount)
    {
        cutoffAmount = amount;
        if (amount > 0.0) {
            filter.setType(EffectFilterData::Type::HP);
            filter.setHp(amount, filter.resonance);
            if (amount < 0.5) {
                mix = amount * 2;
            }
        } else {
            filter.setType(EffectFilterData::Type::LP);
            filter.setLp(-amount, filter.resonance);
            if (amount > -0.20) {
                mix = -amount * 5;
            }
            // if (amount > -0.10) {
            //     mix = -amount * 10;
            // }
        }
    }

    float process(float inputValue)
    {
        if (inputValue == 0 || mix == 0.0) {
            return inputValue;
        }

        if (mix < 1.0) {
            return inputValue * (1.0 - mix) + filter.process(inputValue) * mix;
        }

        return filter.process(inputValue);
    }
};
