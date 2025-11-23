/** Description:
This code defines an advanced audio processing component, specifically a versatile digital filter named `EffectFilter3`. Its purpose is to take raw sound input and precisely modify its frequency content before it reaches the listener.

### **Core Functionality: Audio Filtering**

The component uses mathematical calculations to shape sound. It supports three fundamental types of filtering, often used in music synthesis and effects:

1.  **Low Pass (LP):** Allows low frequencies to pass while reducing or eliminating high frequencies (making the sound muffled).
2.  **High Pass (HP):** Allows high frequencies to pass while reducing or eliminating low frequencies (making the sound thin).
3.  **Band Pass (BP):** Allows only a specific range of mid-frequencies to pass, cutting off both highs and lows.

### **Advanced Stacking and Modes**

This filter offers nine distinct operating modes, achieved by stacking the basic filter types. The suffixes (e.g., LP2, HP3) mean the audio signal is passed through two or three identical filter stages sequentially. Stacking the filters creates a much steeper and more dramatic frequency cutoff, fundamentally altering the character of the sound.

### **How the Processing Works**

1.  **Configuration:** The component stores internal calculation data in three separate memory slots to manage these stacked stages.
2.  **Selection (`setType`):** When a user selects a filter type (like HP2), the component instantly points its main processing engine to the specific mathematical routine needed for that two-stage high-pass calculation.
3.  **Processing:** Whenever a piece of audio data enters the main `process` function, the component immediately executes the exact, pre-selected routine (e.g., LP3). If a stacked mode is chosen, the output from the first filter stage feeds directly into the input of the next stage, ensuring complex and robust audio manipulation.

sha: ec8fb590ad6631595717f580940ca53f686ea98659260c45d9757f6135d2c768 
*/
#pragma once

#include "../audioPlugin.h"
#include "filter.h"

#include <math.h> // fabs

class EffectFilter3: public EffectFilterData {
public:
    float buf[3] = { 0.0 };
    float lp[3] = { 0.0 };
    float hp[3] = { 0.0 };
    float bp[3] = { 0.0 };

    typedef float (EffectFilter3::*ProcessFnPtr)(float);
    ProcessFnPtr processFn;

    enum Type {
        LP,
        LP2,
        LP3,
        HP,
        HP2,
        HP3,
        BP,
        BP2,
        BP3,
    };

    void setType(Type type)
    {
        if (type == LP) {
            setFn = &EffectFilterData::setLp;
            processFn = &EffectFilter3::processLp;
        } else if (type == LP2) {
            setFn = &EffectFilterData::setLp;
            processFn = &EffectFilter3::processLp2;
        } else if (type == LP3) {
            setFn = &EffectFilterData::setLp;
            processFn = &EffectFilter3::processLp3;            
        } else if (type == HP) {
            setFn = &EffectFilterData::setHp;
            processFn = &EffectFilter3::processHp;
        } else if (type == HP2) {
            setFn = &EffectFilterData::setHp;
            processFn = &EffectFilter3::processHp2;
        } else if (type == HP3) {
            setFn = &EffectFilterData::setHp;
            processFn = &EffectFilter3::processHp3;         
        } else if (type == BP) {
            setFn = &EffectFilterData::setBp;
            processFn = &EffectFilter3::processBp;
        } else if (type == BP2) {
            setFn = &EffectFilterData::setBp;
            processFn = &EffectFilter3::processBp2;
        } else if (type == BP3) {
            setFn = &EffectFilterData::setBp;
            processFn = &EffectFilter3::processBp3;
        }
    }

    float processLp(float inputValue)
    {
        setSampleData(inputValue, buf[0], hp[0], bp[0], lp[0]);
        return lp[0];
    }

    float processHp(float inputValue)
    {
        setSampleData(inputValue, buf[0], hp[0], bp[0], lp[0]);
        return hp[0];
    }

    float processBp(float inputValue)
    {
        setSampleData(inputValue, buf[0], hp[0], bp[0], lp[0]);
        return bp[0];
    }

    float processLp2(float inputValue)
    {
        setSampleData(processLp(inputValue), buf[1], hp[1], bp[1], lp[1]);
        return lp[1];
    }

    float processHp2(float inputValue)
    {
        setSampleData(processHp(inputValue), buf[1], hp[1], bp[1], lp[1]);
        return hp[1];
    }

    float processBp2(float inputValue)
    {
        setSampleData(processBp(inputValue), buf[1], hp[1], bp[1], lp[1]);
        return bp[1];
    }

    float processLp3(float inputValue)
    {
        setSampleData(processLp2(inputValue), buf[2], hp[2], bp[2], lp[2]);
        return lp[2];
    }

    float processHp3(float inputValue)
    {
        setSampleData(processHp2(inputValue), buf[2], hp[2], bp[2], lp[2]);
        return hp[2];
    }

    float processBp3(float inputValue)
    {
        setSampleData(processBp2(inputValue), buf[2], hp[2], bp[2], lp[2]);
        return bp[2];
    }

    float process(float inputValue)
    {
        return (this->*processFn)(inputValue);
    }
};
