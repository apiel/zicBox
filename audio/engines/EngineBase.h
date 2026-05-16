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
    virtual float draw(float x) = 0;
    virtual float getLoopLength() = 0;
    virtual float getLoopStart() = 0;
    virtual void setLoopStart(float start) = 0;
    virtual int getVoiceCount() = 0;
    virtual float getPlayhead(int voice) = 0;

    struct XY { float x; float y; };
    virtual void setXY(XY xy) = 0;
    virtual XY getXY() = 0;
};

template <typename Derived>
class EngineBase : public IEngine {
protected:
    Param* paramsPtr;
    size_t paramCount;
    size_t paramIndex = 0;

public:
    const char* name;
    enum Type {
        Drum,
        Synth,
        Sampler,
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

    float draw(float x) { return static_cast<Derived*>(this)->drawImpl(x); }
    float drawImpl(float x) { return 0.0f; }

    float getLoopLength() { return static_cast<Derived*>(this)->getLoopLengthImpl(); }
    float getLoopLengthImpl() { return 0.0f; }
    
    float getLoopStart() { return static_cast<Derived*>(this)->getLoopStartImpl(); }
    float getLoopStartImpl() { return 0.0f; }

    void setLoopStart(float start) { static_cast<Derived*>(this)->setLoopStartImpl(start); }
    void setLoopStartImpl(float start) { }

    int getVoiceCount() { return static_cast<Derived*>(this)->getVoiceCountImpl(); }
    int getVoiceCountImpl() { return 1; }

    float getPlayhead(int voice) { return static_cast<Derived*>(this)->getPlayheadImpl(voice); }
    float getPlayheadImpl(int voice) { return -1.0f; }

    void setXY(XY xy) { static_cast<Derived*>(this)->setXYImpl(xy); }
    void setXYImpl(XY xy) { }

    XY getXY() { return static_cast<Derived*>(this)->getXYImpl(); }
    XY getXYImpl() { return { 0.0f, 0.0f }; }

    Param* getParams() { return paramsPtr; }
    size_t getParamCount() { return paramCount; }
    Param& addParam(Param p, bool autoFinalize = true)
    {
        p.context = this;
        if (autoFinalize) p.finalize();
        return paramsPtr[paramIndex++] = p;
    }

    float pct(Param& p) { return (p.value - p.min) / (p.max - p.min); }
};