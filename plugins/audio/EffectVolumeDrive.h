/** Description:
This C++ header file defines an essential component for sound manipulation, packaged as an audio effect plugin named `EffectVolumeDrive`.

**Purpose:**
This plugin is designed to control both the overall loudness and the dynamic intensity of an audio signal. It combines standard volume control with powerful features like audio distortion (drive) and dynamic range compression.

**How It Works (The Basic Idea):**
When audio passes through this plugin, it processes the sound in three main steps:
1.  **Compression:** First, the sound level is adjusted to smooth out overly loud parts if compression is active.
2.  **Drive/Distortion:** Next, based on user settings, subtle warmth or heavy distortion is applied to the signal.
3.  **Final Volume:** Finally, the result is multiplied by the overall volume setting, which can either reduce or dramatically boost the final output level (gain).

**Key User Controls:**

1.  **VOLUME (Loudness/Gain):** This single control manages two distinct functions:
    *   Below 100%: Acts as a standard percentage volume reduction.
    *   Above 100%: Shifts into a powerful Gain mode, significantly boosting the audio signal up to 21 times the original volume.

2.  **DRIVE (Intensity/Mix):** This is a specialized control centered around a neutral value (100%):
    *   100%: The effect is bypassed for drive and compression.
    *   Below 100%: Applies **Compression**, which helps balance loud and quiet parts of the music, making the sound smoother and denser.
    *   Above 100%: Applies **Drive/Distortion**, which adds grit, harmonics, or saturation to the signal.

In essence, `EffectVolumeDrive` provides musicians and engineers precise control over how loud, compressed, or distorted their audio track should be, all within a single, highly flexible unit.

sha: 5d0d6106e1010903f1aeb9011921802d5ba64c4c3ed29c419d4856f8fca0d421 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "audio/effects/applyDrive.h"
#include "audio/effects/applyCompression.h"

/*md
## EffectVolumeDrive

EffectVolumeDrive plugin is used to apply gain and volume on audio buffer.
*/
class EffectVolumeDrive : public Mapping {
public:
    float volumeWithGain = 1.0;

    /*md **Values**: */
    /*md - `VOLUME` to set volume. Till 100, it is the volume percentage, after 100 it is the gain. */
    Val& volume = val(100.0f, "VOLUME", { "Volume", VALUE_STRING, .max = 200.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() <= 100.0f) {
            volumeWithGain = 100.0f / p.val.get();
            p.val.props().label = "Volume";
            p.val.setString(std::to_string((int)p.val.get()) + "%");
        } else {
            // take difference between 100 and 200
            // then multiply by 0.20 to get a max value of 20.0f
            // finally add 1.0f to always be greater than 1.0f meaning that 101 will be 1.20 and 200 will be 21.0
            volumeWithGain = (p.val.get() - 100.0f) * 0.20f + 1.0f;
            p.val.props().label = "Gain";
            std::string valStr = std::to_string(volumeWithGain);
            p.val.setString(valStr.substr(0, valStr.find(".") + 2));
        }
    });

    float compressAmount = 0.0f;
    float driveAmount = 0.0f;

    /*md - `DRIVE` to set drive and compression. */
    Val& mix = val(100.0, "DRIVE", { "Comp.|Drive", .type = VALUE_CENTERED | VALUE_STRING, .max = 200.0f }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == 100.0f) {
            driveAmount = 0.0f;
            compressAmount = 0.0f;
            p.val.props().label = "Comp.|Drive";
            p.val.setString("0");
        } else if (p.val.get() > 100.0f) {
            driveAmount = (p.val.get() - 100.0f) / 100.0f;
            compressAmount = 0.0f;
            p.val.props().label = "Drive";
            p.val.setString(std::to_string((int)(driveAmount * 100)) + "%");
        } else {
            driveAmount = 0.0f;
            compressAmount = (100.0f - p.val.get()) / 100.0f;
            p.val.props().label = "Compressor";
            p.val.setString(std::to_string((int)(compressAmount * 100)) + "%");
        }
    });

    EffectVolumeDrive(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float output = buf[track];
        output = applyCompression(output, compressAmount);
        output = applyDrive(output, driveAmount, props.lookupTable);
        buf[track] = output * volumeWithGain;
    }
};
