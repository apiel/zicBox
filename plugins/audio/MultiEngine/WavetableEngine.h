#pragma once

#include "helpers/math.h"
#include "plugins/audio/MultiEngine/Engine.h"
#include "plugins/audio/utils/FastWaveform.h"
#include "plugins/audio/utils/MMfilter.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/Wavetable.h"
#include "plugins/audio/utils/val/valMMfilterCutoff.h"

class WavetableEngine : public Engine {
protected:
    MultiFx multiFx;
    Wavetable wavetable;
    MMfilter filter;
    FastWaveform lfo;

    float velocity = 1.0f;
    float sampleIndexLfo = 0.0f;

public:
    // --- 10 parameters ---
    Val& picth = val(0.0f, "PITCH", { "Pitch", VALUE_CENTERED, .min = -32, .max = 32 }, [&](auto p) {
        p.val.setFloat(p.value);
        setBaseFreq(p.val.get());
    });

    // Val& lfoRate = val(50.0f, "LFO_RATE", { "LFO Rate", .min = 0.0f, .max = 50.0f, .step = 0.1, .floatingPoint = 1, .unit = "Hz" });

    Val& lfoRate = val(1.0f, "LFO_RATE", { "LFO Rate", .min = 0.1f, .max = 100.0f, .step = 0.1f, .floatingPoint = 1, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        lfo.setRate(p.val.get());
    });

    Val& lfoWaveform = val(0, "LFO_WAVEFORM", { "LFO", VALUE_STRING, .max = FastWaveform::TYPE_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        lfo.setType((int)p.val.get());
        p.val.setString(lfo.toString());
    });

    Val& lfoFreq = val(0.0f, "LFO_FREQ_MOD", { "Freq. Mod.", VALUE_CENTERED, .min = -100.0f, .unit = "%" });

    Val& wave = val(0, "WAVE", { "Wave", VALUE_STRING }, [&](auto p) {
        p.val.setFloat(p.value);
        int position = p.val.get();
        wavetable.open(position, false);
        p.val.setString(wavetable.fileBrowser.getFileWithoutExtension(position));
    });

    Val& waveEdit = val(0, "WAVE_EDIT", { "Wave Edit", VALUE_STRING, .min = 1.0, .max = ZIC_WAVETABLE_WAVEFORMS_COUNT }, [&](auto p) {
        p.val.setFloat(p.value);
        wavetable.morph((int)p.val.get() - 1);
        p.val.setString(std::to_string((int)p.val.get()) + "/" + std::to_string(ZIC_WAVETABLE_WAVEFORMS_COUNT));
    });

    Val& cutoff = val(0.0f, "CUTOFF", { "LPF | HPF", VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, [&](auto p) {
        valMMfilterCutoff(p, filter);
    });

    Val& resonance = val(0.0f, "RESONANCE", { "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    // --- constructor ---
    WavetableEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : Engine(p, c, "Aditiv")
        , multiFx(props.sampleRate, props.lookupTable)
        , lfo(props.sampleRate)
    {
        initValues();
    }

    void sample(float* buf, float envAmpVal) override
    {
        if (envAmpVal == 0.0f) {
            float out = buf[track];
            out = multiFx.apply(out, fxAmount.pct());
            buf[track] = out;
            return;
        }

        // float out = 0.0f;
        // float lfoHz = lfoRate.get();
        // if (lfoHz > 0.0f) {
        //     float phaseInc = (2.0f * M_PI * lfoHz) / props.sampleRate;
        //     sampleIndexLfo += phaseInc;
        //     if (sampleIndexLfo >= 2.0f * M_PI)
        //         sampleIndexLfo -= 2.0f * M_PI;
        //     // float lfoVal = (fastSin2(sampleIndexLfo) + 1.0f) * 0.5f; // map [-1,1] → [0,1]
        //     float lfoVal = (sinf(sampleIndexLfo) + 1.0f) * 0.5f; // map [-1,1] → [0,1]
        //     out = wavetable.sample(&wavetable.sampleIndex, baseFreq, lfoVal);
        // } else {
        //     out = wavetable.sample(&wavetable.sampleIndex, baseFreq);
        // }

        float modulatedFreq = baseFreq;
        if (lfoFreq.pct() != 0.5f) {
            modulatedFreq += (lfoFreq.pct() - 0.5f) * lfo.process();
            if (modulatedFreq < 0.0f) {
                modulatedFreq = 0.000001f;
            }
        }
        float out = wavetable.sample(&wavetable.sampleIndex, modulatedFreq);
        out = filter.process(out);
        out = out * envAmpVal * velocity;
        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        velocity = _velocity;
        setBaseFreq(picth.get(), note);
        wavetable.sampleIndex = 0;
        sampleIndexLfo = 0.0f;
    }
};
