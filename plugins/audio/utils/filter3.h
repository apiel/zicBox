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
