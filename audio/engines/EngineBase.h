#pragma once

#include <cstdint>

#include "audio/engines/EngineParam.h"
#include "stm32/log.h"

class IEngine {
public:
    virtual ~IEngine() { }
    virtual void noteOn(uint8_t note, float velocity) = 0;
    virtual void noteOff(uint8_t note) = 0;
    virtual float sample() = 0;
    virtual Param* getParams() = 0;
    virtual size_t getParamCount() = 0;
    virtual const char* getName() = 0;
};

template <typename Derived>
class EngineBase : public IEngine {
protected:
    Param* paramsPtr;
    size_t paramCount;

public:
    const char* name;
    enum Type {
        Drum,
        Synth,
    } type;

    template <size_t N>
    EngineBase(Type t, const char* n, Param (&p)[N])
        : paramsPtr(p)
        , paramCount(N)
        , name(n)
        , type(t)
    {
    }

    void init()
    {
        for (size_t i = 0; i < paramCount; i++) {
            paramsPtr[i].context = this;
            paramsPtr[i].finalize();
            // log("Param %d: %s --> %d", i, paramsPtr[i].label, paramsPtr[i].precision);
        }
    }

    const char* getName() { return name; }
    void noteOff(uint8_t note) { static_cast<Derived*>(this)->noteOffImpl(note); }
    void noteOffImpl(uint8_t note) { } // Default empty implementation so subclasses don't HAVE to implement it

    void noteOn(uint8_t note, float velocity) { static_cast<Derived*>(this)->noteOnImpl(note, velocity); }

    float sample() { return static_cast<Derived*>(this)->sampleImpl(); }

    Param* getParams() { return paramsPtr; }
    size_t getParamCount() { return paramCount; }

    float pct(Param& p) { return (p.value - p.min) / (p.max - p.min); }
};