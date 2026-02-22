/** Description:
This technical blueprint defines the **KickEngine**, a specialized digital audio generator responsible for creating and shaping electronic kick drum sounds within a larger music application. It functions as an advanced sound synthesis module that allows users to precisely control every aspect of the final drum hit.

### How the KickEngine Works

The engine constructs the kick drum sound by combining several distinct sonic layers and processing them sequentially:

1.  **Core Tone Generation:** The fundamental sound is created by a digital oscillator (waveform generator). Users can morph between different base tone shapes, influencing the overall character of the kick, from smooth sine waves to aggressive square waves.
2.  **Pitch and Envelope:** A critical component is the pitch envelope. A dedicated generator controls the frequency over time, producing the characteristic downward pitch sweep necessary for a powerful bass "thump."
3.  **Attack Transient:** To give the kick definition and "punch," a short, sharp attack sound (transient) is blended with the core tone. This is the initial high-frequency "click" heard at the very beginning of the sound.
4.  **Filtering and Shaping:** The combined signal is passed through a versatile filter that allows users to adjust the **Cutoff** (determining how bright or muffled the sound is) and **Resonance** (adding emphasis or ringing at the cutoff frequency).
5.  **Dual Effects Chain:** The resulting filtered sound travels through two separate, independent multi-effects units. This provides layered post-processing, such as distortion, compression, or other time-based effects, to finalize the sound's character and presence.

All aspects of the sound—the waveform type, the intensity of the transient, the speed of the pitch envelope, and all filter and effect settings—are exposed as user-adjustable parameters, offering detailed control to sculpt a wide variety of kick drum sounds.

sha: c9b2fd004ba7ac74b959c9ca45d8ea0b6bbe4bf778bc1d9663977f009de5036c 
*/
#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/KickEnvTableGenerator.h"
#include "audio/KickTransientTableGenerator.h"
#include "audio/WavetableGenerator2.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyCompression.h"
#include "plugins/audio/utils/valMMfilterCutoff.h"
#include "plugins/audio/utils/valMultiFx.h"

class KickEngine : public DrumEngine {
protected:
    float velocity = 1.0f;

    MMfilter filter;
    MultiFx multiFx;
    MultiFx multiFx2;

    WavetableGenerator waveform;
    KickTransientTableGenerator transient;
    KickEnvTableGenerator kickEnv;

#define ENVELOP_COUNT 20

public:
    GraphPointFn freqGraph = [&](float index) { return *kickEnv.sample(&index); };
    Val& freqModulation = val(10.0f, "ENVELOPE_SHAPE", { .label = "Freq. mod.", .type = VALUE_BASIC, .step = 0.05f, .unit = "%", .graph = freqGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        kickEnv.setMorph(p.val.pct());
    });

    GraphPointFn waveGraph = [&](float index) { return *waveform.sample(&index); };
    Val& waveformType = val(250.0f, "WAVEFORM_TYPE", { .label = "Waveform", .type = VALUE_STRING, .max = (int)WavetableGenerator::Type::COUNT * 100 - 1, .graph = waveGraph }, [&](auto p) {
        int currentWave = (int)p.val.get() / 100;
        p.val.setFloat(p.value);
        int newWave = (int)p.val.get() / 100;
        if (currentWave != newWave) {
            waveform.setType((WavetableGenerator::Type)newWave);
            p.val.props().unit = waveform.getTypeName();
        }
        int morph = p.val.get() - newWave * 100;
        waveform.setMorph(morph / 100.0f);
        p.val.setString(std::to_string(morph) + "%");
    });

    Val& pitch = val(0, "PITCH", { .label = "Pitch", .type = VALUE_CENTERED, .min = -24, .max = 24, .incType = INC_ONE_BY_ONE });

    GraphPointFn transientGraph = [&](float index) { return *transient.sample(&index); };
    Val& transientMorph = val(100.0, "TRANSIENT", { .label = "Transient", .step = 0.05f, .unit = "%", .graph = transientGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        transient.setMorph(p.val.pct());
    });

    Val& cutoff = val(0.0, "CUTOFF", { .label = "LPF | HPF", .type = VALUE_CENTERED | VALUE_STRING, .min = -100.0, .max = 100.0 }, valMMfilterCutoff(filter));
    Val& resonance = val(0.0, "RESONANCE", { .label = "Resonance", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        filter.setResonance(p.val.pct());
    });

    Val& fxType = val(0, "FX_TYPE", { .label = "FX type", .type = VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx));
    Val& fxAmount = val(0, "FX_AMOUNT", { .label = "FX edit", .unit = "%" });

    Val& fx2Type = val(0, "FX2_TYPE", { .label = "FX2 type", .type = VALUE_STRING, .max = MultiFx::FX_COUNT - 1 }, valMultiFx(multiFx2));
    Val& fx2Amount = val(0, "FX2_AMOUNT", { .label = "FX2 edit", .unit = "%" });

    KickEngine(AudioPlugin::Props& p, AudioPlugin::Config& c, float* fxBuffer1, float* fxBuffer2)
        : DrumEngine(p, c, "Kick")
        , multiFx(props.sampleRate, fxBuffer1)
        , multiFx2(props.sampleRate, fxBuffer2)
        , waveform(props.lookupTable, props.sampleRate)
    {
        initValues();
    }

    float scaledClipping = 0.0f;
    float freq = 1.0f;

    float sampleIndex = 0.0f;
    float lpState = 0.f;
    void sampleOn(float* buf, float envAmp, int sampleCounter, int totalSamples) override
    {

        float t = float(sampleCounter) / totalSamples;
        float envFreq = kickEnv.next(t);
        float modulatedFreq = freq + envFreq;
        float out = waveform.sample(&sampleIndex, modulatedFreq) * envAmp * 0.5f;

        if (t < 0.01f && transientMorph.get() > 0.0f) {
            out = out + transient.next(t) * envAmp;
        }

        out = filter.process(out);
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());

        buf[track] = out * velocity;
    }

    void sampleOff(float* buf) override
    {
        float out = buf[track];
        out = multiFx.apply(out, fxAmount.pct());
        out = multiFx2.apply(out, fx2Amount.pct());
        buf[track] = out;
    }

    // Higher base note is, lower pitch will be
    //
    // Usualy our base note is 60
    // but since we want a kick sound we want bass and we remove one octave (12 semitones)
    // So we send note 60, we will play note 48...
    uint8_t baseNote = 60 + 12;
    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        DrumEngine::noteOn(note, _velocity);
        velocity = _velocity;
        sampleIndex = 0;
        lpState = 0.f;
        freq = pow(2, ((note - baseNote + pitch.get()) / 12.0));
    }
};
