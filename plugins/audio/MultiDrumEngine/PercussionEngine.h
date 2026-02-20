/** Description:
This C++ header file defines the blueprint for `PercussionEngine`, an advanced digital sound generator specifically designed to synthesize percussion sounds, such as kicks, snares, or toms, within an audio processing framework.

This engine inherits functionality from a base `DrumEngine`, providing a complex structure for generating high-quality rhythmic tones.

**Core Functionality:**

The engine creates sound by mixing two primary components: a **Tonal Body** and a **Noise Component**.

1.  **Tonal Body Generation:** This controls the fundamental pitch of the drum. It includes a `Pitch Bend` feature that allows the frequency to quickly sweep up or down right after the sound starts, essential for generating classic tight drum sounds. It also uses `Harmonics` to enrich the tone, adding brightness or complexity to the sound wave.
2.  **Noise Component (Snare/Attack):** This generates texture, crucial for attack transients or snare-like sounds. The `Noise Character` control is key, allowing the engine to manipulate basic white noise. It can filter the noise to be smoother and darker (similar to "Pink Noise") or digitally degrade it using a technique similar to bitcrushing for a harsh, aggressive sound. This shaped noise is then run through a digital **resonator** to give it a defined pitch and natural decay.

**User Controls and Effects:**

The `PercussionEngine` provides numerous controls to shape the final output:

*   **Pitch & Bend:** Determines the base frequency and how that frequency sweeps over time.
*   **Mix:** Balances the volume between the synthesized Tonal Body and the Noise Component.
*   **Punch:** Adds a momentary volume boost at the very beginning of the note, simulating the impact of a real drum hit.
*   **Drive & Boost:** Applies distortion or saturation effects to add warmth or grit to the signal.
*   **Reverb:** Adds spatial ambience to the final sound.

In essence, this class defines a highly flexible instrument that mathematically generates a complex percussive sound, blends tones and custom-shaped noise, and applies real-time audio effects.

sha: 08c5ec425f2d4fb8519b1401d7c3c198a1510f7c4fa81312b43575895e4a86f4 
*/
#pragma once

#include "audio/engines/DrumPercussion.h"
#include "plugins/audio/MultiEngine.h"

class PercussionEngine : public MultiEngine {
protected:
    DrumPercussion perc;
    REVERB_BUFFER

    Val& duration = val("DURATION", perc.duration);
    GraphPointFn ampGraph = [&](float i) { return *perc.envelopAmp.getMorphShape(i); };
    Val& ampMorph = val("AMP_MORPH", perc.ampEnv, ampGraph);

    Val& pitch = val("PITCH", perc.pitch);
    GraphPointFn bendGraph = [&](float i) { 
        float bVal = perc.bend.value * 0.02f - 1.0f; // Centered mapping
        if (bVal == 0.0f) return 1.0f;
        return (bVal < 0.0f) ? 1.0f + bVal * (1.0f - i) : 1.0f - bVal * i;
    };
    Val& bend = val("BEND", perc.bend, bendGraph);
    Val& harmonics = val("HARMONICS", perc.harmonics);

    Val& snareTune = val("NOISE_TUNE", perc.snareTune);
    Val& noiseCharacter = val("NOISE_SHAPE", perc.noiseCharacter);
    Val& snareDecay = val("NOISE_DEC", perc.snareDecay);
    Val& mix = val("MIX", perc.mix);

    Val& punch = val("PUNCH", perc.punch);
    Val& drive = val("DRIVE", perc.drive);
    Val& reverb = val("REVERB", perc.reverb);

public:
    PercussionEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : MultiEngine(p, c, "Perc"), perc(p.sampleRate, buffer) {
        initValues();
    }

    void noteOn(uint8_t note, float velocity, void* = nullptr) override {
        perc.noteOn(note, velocity);
    }

    void sample(float* buf) override {
        buf[track] = perc.sample();
    }
};
