#ifndef _EFFECT_DRUM_H_
#define _EFFECT_DRUM_H_

#include "audioPlugin.h"
#include "mapping.h"

/*md
## EffectDrum

EffectDrum plugin is used to apply effect to drum tracks.
*/
class EffectDrum : public Mapping {
protected:
    float applyDrive(float input)
    {
        if (driveAmount == 0.0f) {
            return input;
        }
        return tanhLookup(input * (1.0f + driveAmount * 5.0f));
    }

    float tanhLookup(float x)
    {
        x = range(x, -1.0f, 1.0f);
        int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1));
        return props.lookupTable->tanh[index];
    }

    static constexpr int REVERB_BUFFER_SIZE = 48000; // 1 second buffer at 48kHz
    float reverbBuffer[REVERB_BUFFER_SIZE] = { 0.0f };
    int reverbIndex = 0;
    float applyReverb(float signal)
    {
        float reverbAmount = reverb.pct();
        if (reverbAmount == 0.0f) {
            return signal;
        }
        int reverbSamples = static_cast<int>((reverbAmount * 0.5f) * props.sampleRate); // Reverb duration scaled
        float feedback = reverbAmount * 0.7f; // Feedback scaled proportionally
        float mix = reverbAmount * 0.5f; // Mix scaled proportionally

        if (reverbSamples > REVERB_BUFFER_SIZE) {
            reverbSamples = REVERB_BUFFER_SIZE; // Cap the reverb duration to buffer size
        }

        float reverbSignal = reverbBuffer[reverbIndex];
        reverbBuffer[reverbIndex] = signal + reverbSignal * feedback;
        reverbIndex = (reverbIndex + 1) % reverbSamples;

        return signal * (1.0f - mix) + reverbSignal * mix;
    }

public:
    float vol = 1.0;
    float driveAmount = 0.0f;

    /*md **Values**: */
    /*md - `VOLUME_DRIVE` to set volume. Till 100, it is the volume percentage, after 100 it is the gain. */
    Val& volume = val(100.0f, "VOLUME_DRIVE", { "Vol.|Drive", VALUE_STRING, .max = 200.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() <= 100.0f) {
            driveAmount = 0.0f;
            vol = p.val.get() / 100.0f;
            p.val.props().label = "Volume";
            p.val.setString(std::to_string((int)p.val.get()) + "%");
        } else {
            int drive = p.val.get() - 100.0f;
            driveAmount = drive / 100.0f;
            p.val.props().label = "Drive";
            p.val.setString(std::to_string(drive) + "%");
        }
    });
    // TODO negative should do something else...
    /*md - REVERB controls delay time, feedback, and mix with one parameter. */
    Val& reverb = val(0.0f, "REVERB", { "Reverb", .unit = "%" });

    EffectDrum(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float output = buf[track];
        output = applyDrive(output);
        output = applyReverb(output);
        buf[track] = output * vol;
    }
};

#endif
