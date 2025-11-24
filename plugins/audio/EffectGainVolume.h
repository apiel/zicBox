/** Description:
This header file defines a specialized component called `EffectGainVolume`, which functions as an audio effect plugin. Its sole purpose is to precisely control the loudness of an audio signal as it passes through an audio processing system.

This plugin offers two main parameters that users can adjust:

1.  **Volume:** This sets the standard output level, typically controlled as a percentage (e.g., 0% to 100%).
2.  **Gain:** This provides additional amplification or boost to the signal, allowing for levels higher than the standard volume setting.

**How it Works:**

The plugin is designed to process audio sample-by-sample. When the user adjusts either the volume or the gain, the component instantaneously calculates a single numerical multiplier. This multiplier combines the effects of both parameters into a total scaling factor.

As the audio data (the stream of digital numbers representing the sound) enters the plugin, every incoming sound sample is multiplied by this calculated factor. Multiplication is the standard method for changing the level of a digital audio signal, ensuring that the sound is made louder or quieter according to the user's combined volume and gain settings.

sha: 532a2753a93df4046a25730508f15496a614445ab96779daba4f2326c3c22a10 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"

/*md
## EffectGainVolume

EffectGainVolume plugin is used to apply gain and volume on audio buffer.
*/
class EffectGainVolume : public Mapping {
public:
    /*md **Values**: */
    /*md - `VOLUME` to set volume. */
    Val& volume = val(100.0f, "VOLUME", { "Volume", .unit = "%" }, [&](auto p) { setVolumeWithGain(p.value, gain.get()); });
    /*md - `GAIN` to set gain. */
    Val& gain = val(1.0f, "GAIN", { "Gain", .min = 1.0f, .max = 20.0f, .step = .1f }, [&](auto p) { setVolumeWithGain(volume.get(), p.value); });
    float volumeWithGain = volume.get();

    EffectGainVolume(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    void sample(float* buf)
    {
        buf[track] = buf[track] * volumeWithGain;
    }

    EffectGainVolume& setVolumeWithGain(float vol, float _gain)
    {
        gain.setFloat(_gain);
        volume.setFloat(vol);
        volumeWithGain = (1.0 + (gain.get())) * volume.pct();

        return *this;
    }
};
