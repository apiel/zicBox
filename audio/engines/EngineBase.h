#pragma once

#include <cstdint>

#include "audio/engines/EngineParam.h"

template <typename Derived>
class EngineBase {
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
        : m_params(p)
        , paramCount(N)
        , name(n)
        , type(t)
    {
        for (size_t i = 0; i < N; i++) {
            p[i].finalize();
        }
    }

    void noteOff(uint8_t note) { static_cast<Derived*>(this)->noteOffImpl(note); }
    void noteOffImpl(uint8_t note) { } // Default empty implementation so subclasses don't HAVE to implement it

    void noteOn(uint8_t note, float velocity) { static_cast<Derived*>(this)->noteOnImpl(note, velocity); }

    float sample() { return static_cast<Derived*>(this)->sampleImpl(); }

    Param* getParams() { return paramsPtr; }
    size_t getParamCount() { return paramCount; }

    float getPct(Param& p)
    {
        return (p.value - p.min) / (p.max - p.min);
    }
};