/** Description:
This code defines a specialized software component, a plugin named `EffectVolumeClipping`, designed to process and modify an audio signal. It functions as a versatile tool for controlling loudness and introducing various forms of saturation and distortion.

The core functionality of this plugin is managed by three key user controls:

1.  **Volume:** This sets the overall loudness. Up to 100%, it works as a standard volume percentage. Beyond 100%, it introduces gain, effectively boosting the signal beyond its original level.
2.  **Clipping:** This control dictates the maximum signal level allowed. When used, it hard-limits the sound wave, which can serve as a protective measure or be used creatively to achieve a hard, aggressive tonal quality.
3.  **Mix/Drive Control:** This is a sophisticated parameter that manages two different types of distortion. If the control is centered, it has no effect. Turning it up introduces "Drive," which acts like musical compression or saturation, similar to overdriving an amplifier for warmth and richness. Turning it down applies "Waveshaping," which fundamentally changes the geometry of the sound wave, resulting in more extreme or unique distortion effects.

When the plugin processes audio, it applies the waveshaping or drive first, followed by the clipping limit, and finally adjusts the result according to the set volume level. This sequential application allows users to precisely sculpt the texture and loudness of the audio track.

sha: 019872813e3ec8e88e50a7b8a5594f02e60ccb0ef458b2fd55f3aacb33da9d30 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "audio/effects/applyClipping.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyWaveshape.h"

/*md
## EffectVolumeClipping

EffectVolumeClipping plugin is used to clipping and volume on audio buffer.
*/
class EffectVolumeClipping : public Mapping {
public:
    /*md **Values**: */
    /*md - `VOLUME` to set volume. Till 100, it is the volume percentage, after 100 it is the gain. */
    Val& volume = val(100.0f, "VOLUME", { "Volume", .unit = "%" });

    float scaledClipping = 0.0f;
    /*md - `GAIN_CLIPPING` set the clipping level.*/
    Val& clipping = val(0.0, "GAIN_CLIPPING", { "Clipping", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        scaledClipping = p.val.pct() * p.val.pct() * 20;
    });

    float waveshapeAmount = 0.0f;
    float driveAmount = 0.0f;

    /*md - `DRIVE` to set drive and compression. */
    Val& mix = val(100.0, "DRIVE", { "Shape.|Drive", .type = VALUE_CENTERED | VALUE_STRING, .max = 200.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == 100.0f) {
            driveAmount = 0.0f;
            waveshapeAmount = 0.0f;
            p.val.props().label = "Shape.|Drive";
            p.val.setString("0");
        } else if (p.val.get() > 100.0f) {
            driveAmount = (p.val.get() - 100.0f) / 100.0f;
            waveshapeAmount = 0.0f;
            p.val.props().label = "Drive";
            p.val.setString(std::to_string((int)(driveAmount * 100)) + "%");
        } else {
            driveAmount = 0.0f;
            waveshapeAmount = (100.0f - p.val.get()) / 100.0f;
            p.val.props().label = "Waveshape";
            p.val.setString(std::to_string((int)(waveshapeAmount * 100)) + "%");
        }
    });

    EffectVolumeClipping(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float output = buf[track];
        output = applyWaveshape(output, waveshapeAmount, props.lookupTable);
        output = applyDrive(output, driveAmount);
        output = applyClipping(output, scaledClipping);

        buf[track] = output * volume.pct();
    }
};
