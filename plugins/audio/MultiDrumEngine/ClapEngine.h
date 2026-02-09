/** Description:
This file defines a specialized software component, the `ClapEngine`, designed to synthesize a realistic, customizable hand-clap sound. It builds upon a fundamental `DrumEngine` structure, meaning it manages how audio plugins trigger and stop drum sounds.

The core idea is to simulate a clap, which is technically a rapid succession of short, noisy bursts. The engine does this by generating carefully filtered noise, turning it on and off according to precise timing parameters.

**How the Synthesis Works:**
When a musical trigger (like a note-on signal) is received, the engine starts a rapid sequence of digital noise bursts. The timing of these bursts (Burst Count and Spacing) and their fade-out speed (Decay) are fully adjustable. This noise is then processed through a critical **Bandpass Filter**, which sculpts the raw noise into the punchy, midrange frequencies typical of a clap.

**User Customization:**
The engine offers ten detailed controls for shaping the final sound:

1.  **Burst:** Controls the number, spacing, and decay time of the individual noise hits.
2.  **Filtering:** Adjusts the Cutoff frequency and Resonance of the Bandpass Filter to set the tone.
3.  **Impact:** Parameters like "Punch" and "Transient" allow users to aggressively shape the initial attack and sharpness of the sound.
4.  **Effects:** Integrated effects include adjustable Boost/Compression (for volume and dynamics control) and Reverb (to add space and depth).

In summary, this class acts as the complete blueprint for generating a professional, dynamic clap sound by managing burst timing, complex noise generation, filtering, and final effect processing.

sha: 5e41dddebf01f8d30d1cb478c2ac252c6a3cc54aec287e458ad32d045374afbe
*/
#pragma once
#include "audio/engines/DrumClap.h"
#include "plugins/audio/MultiEngine.h"
class ClapEngine : public MultiEngine {
protected:
    DrumClap clap;
    
    Val& duration = val("DURATION", clap.duration);
    GraphPointFn ampGraph = [&](float index) { return *clap.envelopAmp.getMorphShape(index); };
    Val& ampMorph = val("AMP_MORPH", clap.ampEnv, ampGraph);
    Val& burstCount = val("BURSTS", clap.burstCount);
    Val& burstSpacing = val("SPACING", clap.burstSpacing);
    Val& decay = val("DECAY", clap.burstDecay);
    Val& noiseColor = val("NOISE_COLOR", clap.noiseColor);
    Val& cutoff = val("CUTOFF", clap.filterFreq);
    Val& resonance = val("RESONANCE", clap.filterReso);
    Val& punch = val("PUNCH", clap.punch);
    Val& transient = val("TRANSIENT", clap.transient);
    Val& boost = val("BOOST", clap.boost);
    Val& reverb = val("REVERB", clap.reverb);

public:
    ClapEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : MultiEngine(p, c, "Clap")
        , clap(p.sampleRate)
    {
        initValues();
    }

    void noteOn(uint8_t note, float _velocity, void* = nullptr) override
    {
        clap.noteOn(note, _velocity);
    }

    void sample(float* buf) override
    {
        buf[track] = clap.sample();
    }
};
