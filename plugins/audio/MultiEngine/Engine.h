/** Description:
This code defines the core structure, or blueprint, for a sound generating component within an audio application, likely a synthesizer or instrument plugin.

**Core Functionality:**

1.  **Sound Generation Foundation:** This component inherits abilities from a standard audio processing framework (`MultiEngine`), giving it the necessary methods to integrate into a larger audio system.
2.  **Pitch Management:** It stores the fundamental frequency and the corresponding musical note (like Middle C). It includes logic to accurately calculate the precise sound frequency based on standard MIDI notes, allowing the instrument to play different musical pitches correctly.
3.  **Volume Shaping (Envelope):** A critical part of the sound is its dynamic volume shape, managed by an internal envelope controller.
    *   When a note is activated (`noteOn`), the volume quickly ramps up (the **Attack** phase).
    *   When the note is deactivated (`noteOff`), the volume slowly fades out (the **Release** phase).
4.  **User Parameters:** The design exposes two user-adjustable controls: Attack time and Release time. These parameters, measured in milliseconds, directly influence how quickly the volume changes occur.
5.  **Audio Processing Loop:** During playback, the engine first determines the current volume level dictated by the envelope. It then passes this volume multiplier to the sound generation part, ensuring the sound signal is scaled correctly before reaching the speaker.
6.  **Note Tracking:** It meticulously tracks which note is currently being played, ensuring that the envelope only starts its volume decay (Release) when the specific key that triggered the sound is lifted.

In summary, this blueprint provides the essential control mechanism—tuning, timing, and note tracking—required for a functional digital musical instrument.

sha: 32821aa99770a69540bd919db8b0c1c6cc03cd44a9f77afa6570685c4f7b74ab 
*/
#pragma once

#include "helpers/midiNote.h"
#include "plugins/audio/audioPlugin.h"
#include "plugins/audio/mapping.h"
#include "audio/AsrEnvelop.h"
#include "audio/EnvelopDrumAmp.h"
#include "plugins/audio/MultiEngine.h"

class Engine : public MultiEngine {
protected:
    float baseFreq = 220.0f;
    uint8_t baseFreqNote = 60;
    void setBaseFreq(int8_t pitchSemitones, uint8_t note = 0)
    {
        if (note == 0)
            note = baseFreqNote;

        baseFreqNote = note;
        baseFreq = getMidiNoteFrequency(baseFreqNote + pitchSemitones);
    }

public:
    float attackStep = 0.0f;
    float releaseStep = 0.0f;
    AsrEnvelop envelopAmp = AsrEnvelop(&attackStep, &releaseStep);

    Val& attack = val(50.0f, "ATTACK", { .label = "Attack", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        attackStep = 1.0f / (p.val.get() * 0.001f * props.sampleRate);
    });

    Val& release = val(300.0f, "RELEASE", { .label = "Release", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        releaseStep = 1.0f / (p.val.get() * 0.001f * props.sampleRate);
    });

    Engine(AudioPlugin::Props& props, AudioPlugin::Config& config, std::string name)
        : MultiEngine(props, config, name)
    {
    }

    void sample(float* buf) override
    {
        float envAmp = envelopAmp.next();
        sample(buf, envAmp);
    }

    uint8_t playingNote = 0;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        playingNote = note;
        envelopAmp.attack();
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (note == playingNote) {
            envelopAmp.release();
        }
    }

    virtual void sample(float* buf, float envAmp) = 0;
};
