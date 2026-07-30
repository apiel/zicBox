#pragma once

#include <cstdint>

#include "audio/engines/EngineParam.h"
#include "stm32/log.h"

/**
 * @brief Abstract interface for all audio engines.
 * 
 * Defines standard lifecycle, audio rendering, note triggering, 
 * parameter access, and UI visualization methods.
 */
class IEngine {
public:
    virtual ~IEngine() { }

    /**
     * @brief Trigger note playback.
     * @param note MIDI note number (0 - 127).
     * @param velocity Trigger velocity (0.0f - 1.0f).
     */
    virtual void noteOn(uint8_t note, float velocity) = 0;

    /**
     * @brief Release an active note.
     * @param note MIDI note number to release.
     */
    virtual void noteOff(uint8_t note) = 0;

    /**
     * @brief Generate and return the next audio frame sample.
     * @return Mono audio sample, typically in the range [-1.0, 1.0].
     */
    virtual float sample() = 0;

    /**
     * @brief Get pointer to the engine's parameter array.
     */
    virtual Param* getParams() = 0;

    /**
     * @brief Get total number of registered parameters.
     */
    virtual size_t getParamCount() = 0;

    /**
     * @brief Get human-readable name of the engine.
     */
    virtual const char* getName() = 0;

    /**
     * @brief Render visual waveform or response curve point for UI graphs.
     * @param x Normalized X-coordinate [0.0, 1.0].
     * @return Y-coordinate sample value for visualization.
     */
    virtual float draw(float x) = 0;

    /**
     * @brief Get loop length in samples or normalized units (samplers).
     */
    virtual float getLoopLength() = 0;

    /**
     * @brief Get loop start position in samples or normalized units.
     */
    virtual float getLoopStart() = 0;

    /**
     * @brief Set loop start position.
     * @param start New loop start position.
     */
    virtual void setLoopStart(float start) = 0;

    /**
     * @brief Get active polyphonic voice count (default 1 for monophonic).
     */
    virtual int getVoiceCount() = 0;

    /**
     * @brief Get current playhead position for specified voice index (-1.0f if inactive).
     */
    virtual float getPlayhead(int voice) = 0;

    struct XY { float x; float y; };

    /**
     * @brief Set 2D XY pad control coordinates.
     */
    virtual void setXY(XY xy) = 0;

    /**
     * @brief Get current 2D XY pad control coordinates.
     */
    virtual XY getXY() = 0;

    /**
     * @brief Get name of the XY performance mapping.
     */
    virtual const char* getNameXY() = 0;
};

/**
 * @brief CRTP (Curiously Recurring Template Pattern) base class for audio engines.
 * 
 * Performance Design:
 * On microcontrollers like the STM32H7, calling `virtual` functions inside sample-rate 
 * audio processing loops introduces vtable pointer lookup overhead and prevents inlining.
 * 
 * EngineBase uses static polymorphism via CRTP (`EngineBase<Derived>`). Calls to `sample()`,
 * `noteOn()`, etc. dispatch directly to `static_cast<Derived*>(this)->...Impl()` at compile time.
 * This eliminates vtable pointer dereferences, allowing the compiler to inline DSP calls
 * into high-performance audio loops.
 * 
 * Subclasses override `...Impl()` methods (e.g. `sampleImpl`, `noteOnImpl`) to provide specific logic.
 */
template <typename Derived>
class EngineBase : public IEngine {
protected:
    Param* paramsPtr;    ///< Pointer to the subclass's fixed-size Param array
    size_t paramCount;   ///< Total declared size of paramsPtr array
    size_t paramIndex = 0; ///< Index tracker for addParam() registration

public:
    const char* name;
    enum Type {
        Drum,
        Synth,
        Sampler,
    } type;

    /**
     * @brief Construct an EngineBase instance.
     * @tparam N Fixed array size automatically deduced from Param (&p)[N].
     * @param t Engine category type (Drum, Synth, Sampler).
     * @param n Engine identifier name string.
     * @param p Reference to fixed-size Param array in subclass (`Param params[N]`).
     */
    template <size_t N>
    EngineBase(Type t, const char* n, Param (&p)[N])
        : paramsPtr(p)
        , paramCount(N)
        , name(n)
        , type(t)
    {
    }

    /**
     * @brief Initialize parameters, bind engine context, and calculate precision.
     */
    void init()
    {
        for (size_t i = 0; i < paramCount; i++) {
            paramsPtr[i].context = this;
            paramsPtr[i].finalize();
        }
    }

    /// Returns engine identifier name.
    const char* getName() { return name; }

    /// Triggers note release in subclass implementation.
    void noteOff(uint8_t note) { static_cast<Derived*>(this)->noteOffImpl(note); }
    /// Default empty noteOff implementation (subclasses override if needed).
    void noteOffImpl(uint8_t note) { }

    /// Triggers note attack in subclass implementation.
    void noteOn(uint8_t note, float velocity) { static_cast<Derived*>(this)->noteOnImpl(note, velocity); }

    /// Generates next audio sample frame from subclass implementation.
    float sample() { return static_cast<Derived*>(this)->sampleImpl(); }

    /// Graph drawing query dispatched to subclass implementation.
    float draw(float x) { return static_cast<Derived*>(this)->drawImpl(x); }
    /// Default draw implementation (returns flat line 0.0f).
    float drawImpl(float x) { return 0.0f; }

    /// Query loop length from subclass implementation.
    float getLoopLength() { return static_cast<Derived*>(this)->getLoopLengthImpl(); }
    /// Default loop length implementation.
    float getLoopLengthImpl() { return 0.0f; }

    /// Query loop start position from subclass implementation.
    float getLoopStart() { return static_cast<Derived*>(this)->getLoopStartImpl(); }
    /// Default loop start implementation.
    float getLoopStartImpl() { return 0.0f; }

    /// Set loop start position in subclass implementation.
    void setLoopStart(float start) { static_cast<Derived*>(this)->setLoopStartImpl(start); }
    /// Default setLoopStart implementation.
    void setLoopStartImpl(float start) { }

    /// Query voice count from subclass implementation.
    int getVoiceCount() { return static_cast<Derived*>(this)->getVoiceCountImpl(); }
    /// Default voice count implementation (1 voice = monophonic).
    int getVoiceCountImpl() { return 1; }

    /// Query playhead position for specified voice index.
    float getPlayhead(int voice) { return static_cast<Derived*>(this)->getPlayheadImpl(voice); }
    /// Default playhead implementation (-1.0f = inactive).
    float getPlayheadImpl(int voice) { return -1.0f; }

    /// Set XY pad coordinates in subclass implementation.
    void setXY(XY xy) { static_cast<Derived*>(this)->setXYImpl(xy); }
    /// Default setXY implementation.
    void setXYImpl(XY xy) { }

    /// Query XY pad coordinates from subclass implementation.
    XY getXY() { return static_cast<Derived*>(this)->getXYImpl(); }
    /// Default getXY implementation.
    XY getXYImpl() { return { 0.0f, 0.0f }; }

    /// Query XY control mapping name from subclass implementation.
    const char* getNameXY() { return static_cast<Derived*>(this)->getNameXYImpl(); }
    /// Default getNameXY implementation.
    const char* getNameXYImpl() { return nullptr; }

    /// Returns pointer to array of registered parameters.
    Param* getParams() { return paramsPtr; }

    /// Returns total count of declared parameters.
    size_t getParamCount() { return paramCount; }

    /**
     * @brief Register a parameter into the paramsPtr array.
     * @note CRITICAL: Must not call addParam() more times than declared array size N in `Param params[N]`,
     *       otherwise out-of-bounds array access will cause a segmentation fault!
     * @param p Parameter configuration object.
     * @param autoFinalize Automatically compute precision and defaults.
     * @return Reference to the registered Param object inside paramsPtr array.
     */
    Param& addParam(Param p, bool autoFinalize = true)
    {
        p.context = this;
        if (autoFinalize) p.finalize();
        return paramsPtr[paramIndex++] = p;
    }

    /// Utility helper: Calculates normalized parameter position in range [0.0, 1.0].
    float pct(Param& p) { return (p.value - p.min) / (p.max - p.min); }
};