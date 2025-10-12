#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"
#include "plugins/audio/utils/MMfilter.h"

#include <cmath>
#include <vector>

class StringEngine : public Engine {
protected:
    MultiFx multiFx;
    MultiFx multiFx2;
    MMfilter filter;

    // Resonator
    std::vector<float> delayLine;
    uint32_t delayLen = 0;
    uint32_t writePos = 0;
    float onePoleState = 0.0f;
    bool resonatorActive = false;

    float velocity = 1.0f;

    // Vibrato
    float vibratoPhase = 0.0f; // 0–1

    static constexpr uint32_t MAX_DELAY = 1 << 16; // 65536

    // --- Resonator tick with vibrato ---
    float resonatorTick(float driver, float vibRatio)
    {
        if (!resonatorActive || delayLen == 0)
            return 0.0f;

        // fractional read position with vibrato applied
        float rp = fmodf((float)writePos * vibRatio, (float)delayLen);
        uint32_t i0 = (uint32_t)rp;
        uint32_t i1 = (i0 + 1) % delayLen;
        float frac = rp - i0;

        float s0 = delayLine[i0];
        float s1 = delayLine[i1];
        float out = s0 + frac * (s1 - s0); // linear interp

        // one-pole lowpass
        float cutoff = std::max(0.005f, tone.pct());
        onePoleState += cutoff * (out - onePoleState);
        float filtered = onePoleState;

        // feedback + external driving
        float fb = decay.get();
        delayLine[writePos % delayLen] = filtered * fb + driver;

        // advance write pointer
        writePos = (writePos + 1) % delayLen;

        return filtered;
    }

public:
    // --- Parameters (exactly 12 now) ---
    Val& pitch = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -24, .max = 24 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq((int)p.val.get());
    });

    Val& decay = val(0.98f, "DECAY", { "Decay", .min = 0.80f, .max = 0.99f, .step = 0.01f, .floatingPoint = 2 });
    Val& tone = val(50.0f, "TONE", { "Tone", .unit = "%" });
    Val& sustainExcite = val(50.0f, "SUSTAIN_EXCITE", { "Sustain Excite", .unit = "%" });

    // Vibrato parameters
    Val& vibratoRate = val(0.0f, "VIBRATO_RATE", { "Vibrato Rate", .min = 0.0f, .max = 30.0f, .step = 0.1f, .floatingPoint = 1, .unit = "Hz" });
    Val& vibratoDepth = val(10.0f, "VIBRATO_DEPTH", { "Vibrato Depth", .min = 0.0f, .max = 100.0f, .unit = "cents" });

    Val& cutoff = val(0.0, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });
    Val& resonance = val(0.0, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { "FX2 type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx2.setFxType);

    Val& fx2Amount = val(0, "FX2_AMOUNT", { "FX2 edit", .unit = "%" });

    // --- constructor ---
    StringEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "String")
        , multiFx(props.sampleRate, props.lookupTable)
        , multiFx2(props.sampleRate, props.lookupTable)
    {
        delayLen = (uint32_t)std::min<uint64_t>((uint64_t)(props.sampleRate * 0.02f), (uint64_t)MAX_DELAY);
        delayLine.assign(delayLen + 4, 0.0f);
        initValues();
    }

    // --- NoteOn: excite resonator ---
    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        setBaseFreq(pitch.get(), note - 24); // remove 2 octaves

        float freq = baseFreq;
        if (freq < 20.0f)
            freq = 20.0f;
        if (freq > props.sampleRate * 0.45f)
            freq = props.sampleRate * 0.45f;

        uint32_t len = (uint32_t)std::max<int>(2, (int)std::round(props.sampleRate / freq));
        len = std::min<uint32_t>(len, MAX_DELAY);

        if (len != delayLen) {
            delayLen = len;
            delayLine.assign(delayLen + 4, 0.0f);
        }

        writePos = 0;
        onePoleState = 0.0f;
        resonatorActive = true;
        driverEnv = 1.0f;
        isNoteHeld = true;
    }

    void noteOff(uint8_t note, float _velocity, void* = nullptr) override
    {
        isNoteHeld = false;
    }

    // --- Sample ---
    bool isNoteHeld = false;
    float driverEnv = 0.0f;

    void sample(float* buf, float envAmpVal) override
    {
        // driver env release
        if (!isNoteHeld) {
            if (driverEnv > 0.0f)
                driverEnv -= 0.05f;
            else
                driverEnv = 0.0f;
            driverEnv = std::min<float>(envAmpVal, driverEnv);
        }

        float vibRatio = 1.0f;
        if (vibratoRate.get() > 0.0f && vibratoDepth.get() > 0.0f) {
            if (vibratoRate.get() < 2.0f) {
                // vibrato LFO (pulse/square)
                float phaseInc = vibratoRate.get() / props.sampleRate;
                vibratoPhase += phaseInc;
                if (vibratoPhase >= 1.0f)
                    vibratoPhase -= 1.0f;

                // pulse: -1 for first half, +1 for second half
                float pulse = (vibratoPhase < 0.1f) ? -1.0f : 1.0f;

                float cents = pulse * vibratoDepth.get();
                vibRatio = powf(2.0f, cents / 1200.0f);
            } else {
                // vibrato LFO (sine)
                float phaseInc = vibratoRate.get() / props.sampleRate;
                vibratoPhase += phaseInc;
                if (vibratoPhase >= 1.0f)
                    vibratoPhase -= 1.0f;

                // sine wave between -1..1
                float sine = sinf(2.0f * M_PI * vibratoPhase);

                // depth in cents -> convert to frequency ratio
                float cents = sine * vibratoDepth.get();
                vibRatio = powf(2.0f, cents / 1200.0f);
            }
        }

        // sustain driver (bow noise)
        float driver = 0.0f;
        if (sustainExcite.pct() > 0.0f) {
            float n = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            n = filter.process(n);
            driver = n * sustainExcite.pct() * velocity * driverEnv;
        }

        float out = resonatorTick(driver, vibRatio);
        out *= velocity * envAmpVal;

        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());

        buf[track] = out;
    }
};
