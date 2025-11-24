/** Description:
This specialized software component, called `TomEngine`, is designed to digitally create the realistic sound of a Tom drum, suitable for use within an audio plugin or sound production software.

It acts as a sound synthesizer, inheriting capabilities from a standard `DrumEngine`. Instead of using pre-recorded audio samples, it generates the drum sound in real-time using mathematical formulas, a technique known as physical modeling synthesis.

Users can precisely shape the resulting sound using several adjustable parameters:

*   **Pitch** and **Decay** control the fundamental frequency and how long the sound rings out.
*   **Punch** adds a sharp, initial transient attack, mimicking the drumstick hit.
*   **Bend** allows the pitch to quickly drop after the initial strike, a characteristic of many tensioned drums.
*   **Body** and **Harmonics** control the richness and complexity of the drum’s tone (the simulated overtones and membrane resonance).

The core mechanism works by generating complex wave patterns based on the fundamental frequency and its multiples, simulating the vibration of the drum head. It then applies integrated audio effects like **Drive** (distortion/overdrive), **Boost** (gain control), and **Reverb** (simulating echo in a space) before outputting the final audio signal. This structure ensures the generated drum sound is fully customizable and reacts dynamically to control changes.

sha: a20ee0212b1a91de19dd76a2fe6ca86720555e9ebd47abebfef97ab3f0b57b59 
*/
#pragma once
#include "plugins/audio/MultiDrumEngine/DrumEngine.h"
#include "audio/effects/applyBoost.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyReverb.h"

class TomEngine : public DrumEngine {
public:
    Val& pitch = val(120.f, "PITCH", { "Pitch", .min = 40.f, .max = 400.f, .step = 1.f, .unit = "Hz" });
    Val& decay = val(0.5f, "DECAY", { "Decay", .min = 0.05f, .max = 1.5f, .step = 0.01f });
    Val& punch = val(0.6f, "PUNCH", { "Punch", .unit = "%" });
    Val& bend = val(0.4f, "BEND", { "Bend", .unit = "%" });
    Val& bodyMix = val(0.7f, "BODY", { "Body", .unit = "%" });
    Val& harmonics = val(0.3f, "HARM", { "Harm", .unit = "%" });
    Val& drive = val(0.f, "DRIVE", { "Drive", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& boost = val(0.f, "BOOST", { "Boost", .type = VALUE_CENTERED, .min = -100.f, .max = 100.f, .unit = "%" });
    Val& reverb = val(0.2f, "REVERB", { "Reverb", .unit = "%" });
    Val& volume = val(0.9f, "VOL", { "Vol", .unit = "%" });

    TomEngine(AudioPlugin::Props& p, AudioPlugin::Config& c)
        : DrumEngine(p, c, "Tom")
    {
        initValues();
    }

    static constexpr int REVBUF = 48000;
    float reverbBuf[REVBUF] = {};
    int rIdx = 0;

    float phase = 0.f;
    float memState[4] = {};

    void sampleOn(float* buf, float envAmp, int sc, int ts) override
    {
        const float t = float(sc) / ts;
        const float env = expf(-t * 4.f / decay.get());
        const float bendEnv = 1.f - bend.pct() * t;
        const float freq = pitch.get() * bendEnv;

        // --- Fundamental + 2 harmonics
        float out = 0.f;
        for (int h = 1; h <= 3; ++h) {
            float amp = (h == 1) ? 1.f : (harmonics.pct() / h);
            out += amp * sinf(phase * h);
        }
        phase += 2.f * M_PI * freq / props.sampleRate;

        // --- Membrane resonance (2-pole)
        for (int i = 0; i < 2; ++i) {
            memState[i * 2] += freq * (i + 1) / props.sampleRate;
            out += sinf(memState[i * 2]) * expf(-t * (i + 1) / decay.get()) * bodyMix.pct() * 0.3f;
        }

        out *= envAmp * env * volume.pct();

        // --- Punch transient
        if (t < 0.02f)
            out *= 1.f + punch.pct() * 2.f;

        out = applyEffects(out);
        buf[track] = out;
    }

    void sampleOff(float* buf) override
    {
        buf[track] = applyReverb(buf[track], reverb.pct(), reverbBuf, rIdx, REVBUF);
    }

    void noteOn(uint8_t note, float, void* = nullptr) override
    {
        phase = 0.f;
        for (auto& s : memState)
            s = 0.f;
        pitch.set(pitch.get() * powf(2.f, (note - 60) / 12.f));
    }

private:
    float prevInput = 0.0f;
    float prevOutput = 0.0f;
    float applyEffects(float x)
    {
        float d = drive.pct() * 2.f - 1.f;
        if (d > 0.f)
            x = applyDrive(x, d, props.lookupTable);
        else
            x = applyBoost(x, -d, prevInput, prevOutput);
        return applyReverb(x, reverb.pct(), reverbBuf, rIdx, REVBUF);
    }
};