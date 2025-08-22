#pragma once

#include <functional>

class AsrEnvelop {
protected:
    float (AsrEnvelop::*nextFn)() = &AsrEnvelop::envelopSustain;

    float env = 0.0f;
    float* attackStep;
    float* releaseStep;

    std::function<void()> onFinish;

    float envelopAttack()
    {
        env += *attackStep;
        if (env >= 1.0f) {
            env = 1.0f;
            nextFn = &AsrEnvelop::envelopSustain;
        }
        return env;
    }

    float envelopSustain()
    {
        return env;
    }

    float envelopRelease()
    {
        env -= *releaseStep;
        if (env <= 0.0f) {
            env = 0.0f;
            if (onFinish) {
                onFinish();
            }
        }
        return env;
    }

public:
    AsrEnvelop(float* attackStep, float* releaseStep, std::function<void()> onFinish = NULL)
        : attackStep(attackStep)
        , releaseStep(releaseStep)
        , onFinish(onFinish)
    {
    }

    float next()
    {
        return (this->*nextFn)();
    }

    float get()
    {
        return env;
    }

    void attack()
    {
        nextFn = &AsrEnvelop::envelopAttack;
    }

    void release()
    {
        nextFn = &AsrEnvelop::envelopRelease;
    }

    bool isRelease()
    {
        return nextFn == &AsrEnvelop::envelopRelease;
    }
};
