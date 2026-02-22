/** Description:
This C++ header defines the blueprint for an advanced audio component called the `MetalicDrumEngine`. This engine is a specialized sound generator designed to synthesize complex, metallic, and resonant drum sounds, such as cymbals, bells, or deep metallic kicks, rather than using recorded audio samples.

### How the Engine Works

The core functionality relies on mathematical synthesis methods to create its signature tone:

1.  **Complex Tone Generation (FM):** The engine uses **Frequency Modulation (FM)** to generate a complex foundational tone. This technique involves using one oscillator (a digital wave creator) to rapidly change the frequency of another, which is crucial for achieving harmonically rich, non-standard sounds typical of metallic objects.
2.  **Resonance and Decay:** The sound is then processed by a **Resonator** component. This digital filter mimics the way a physical object (like a drum shell or metal plate) vibrates and rings, giving the tone its "body" and sustain.
3.  **Envelope Control:** The shape of the sound, from its sharp attack to its smooth decay, is precisely controlled by adjustable mathematical functions, allowing for anything from a short click to a long, ringing tone.

### User Controls and Effects

The engine provides numerous adjustable parameters to shape the sound:

*   **Tonal Control:** Parameters like **Base Frequency**, **FM Frequency/Amplitude**, and **Body Resonance** allow users to dial in the fundamental pitch and how long the sound rings out.
*   **Timbre:** Controls the complexity and harmonic content of the tone.
*   **Integrated Effects:** To finalize the output, the engine includes built-in effects processing for **Drive** (distortion/saturation) or **Compression** for punch, and **Reverb** to add spatial depth.

In essence, the `MetalicDrumEngine` is a complete, self-contained instrument focused on recreating and manipulating synthetic metallic percussion through sophisticated mathematical modeling.

sha: 0259455386f6b5bd45a25cb365b0f0a15f51271c0f27e44047fd13f30609ccb2
*/
#pragma once

#include "audio/engines/DrumMetalic.h"
#include "plugins/audio/MultiEngine.h"

class MetalicDrumEngine : public MultiEngine {
protected:
    DrumMetalic metalic;

    FX_BUFFER

    Val& duration = val("DURATION", metalic.duration);
    GraphPointFn ampGraph = [&](float index) { return *metalic.envelopAmp.getMorphShape(index); };
    Val& ampMorph = val("AMP_MORPH", metalic.ampEnv, ampGraph);
    Val& baseFreq = val("BASE_FREQ", metalic.baseFreq);
    Val& bodyResonance = val("RESONATOR", metalic.bodyResonance);
    Val& timbre = val("TIMBRE", metalic.timbre);
    Val& driveComp = val("DRIVE_COMP", metalic.driveComp);
    Val& toneTension = val("TONE_TENSION", metalic.toneTension);
    Val& reverb = val("REVERB", metalic.reverb);
    Val& fmFreq = val("FM_FREQ", metalic.fmFreq);
    Val& fmAmp = val("FM_AMP", metalic.fmAmp);
    Val& envMod = val("ENV_MOD", metalic.envMod);
    GraphPointFn curveGraph = [&](float t) {
        float shape = metalic.envShape.value;
        if (shape > 1.0f) return Math::pow(1.0f - t, shape);
        return Math::exp(-t * (shape > 0.0f ? shape : 1.0f));
    };
    Val& envShape = val("ENV_SHAPE", metalic.envShape, curveGraph);

public:
    MetalicDrumEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : MultiEngine(p, c, "Metalic")
        , metalic(p.sampleRate, buffer)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        metalic.noteOn(note, _velocity);
    }

    void sample(float* buf) override
    {
        buf[track] = metalic.sample();
    }
};