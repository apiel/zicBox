/** Description:
This header file defines a specialized digital audio processing unit, acting like a flexible effects pedal combined with a volume controller. Its primary purpose is to modify an incoming audio signal by simultaneously applying amplification and a selectable sound effect.

### Core Functionality

The unit performs two main actions on the audio signal:

1.  **Loudness Control:** It provides two interdependent controls: standard **Volume** (expressed as a percentage) and an additional **Gain** setting, which acts as a powerful amplifier boost. These two values are combined internally to determine the final output level.
2.  **Multi-Effect Processing:** It contains a library of selectable audio effects. The user can choose the **Effect Type** and then dial in the **Effect Amount** to control the intensity or wetness of that specific sound manipulation.

### How It Works

When a piece of audio data passes through this processor, the sound first goes through the selected multi-effect transformation. Once the effect is applied, the system then scales the resulting sound by the combined Volume and Gain factor. This ensures that the effect is applied correctly before the final output loudness is set.

Essentially, this single component simplifies audio chain design by packaging powerful amplification controls with a versatile, user-configurable audio effect engine.

sha: a26a11548eb73b6a05dbae3868e2e0356ac97f58b9acc6c26e28c2e3aef23f6c 
*/
#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "audio/MultiFx.h"

/*md
## EffectVolumeMultiFx

EffectVolumeMultiFx plugin is used to apply gain and volume on audio buffer, as well to apply a selected effect.
*/
class EffectVolumeMultiFx : public Mapping {
protected:
    MultiFx multiFx;

public:
    /*md **Values**: */
    /*md - `VOLUME` to set volume. */
    Val& volume = val(100.0f, "VOLUME", { "Volume", .unit = "%" }, [&](auto p) { setVolumeWithGain(p.value, gain.get()); });
    /*md - `GAIN` to set gain. */
    Val& gain = val(1.0f, "GAIN", { "Gain", .min = 1.0f, .max = 20.0f, .step = .1f }, [&](auto p) { setVolumeWithGain(volume.get(), p.value); });
    float volumeWithGain = volume.get();

    /*md - `FX_TYPE` select the effect.*/
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, multiFx.setFxType);

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    EffectVolumeMultiFx(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , multiFx(props.sampleRate)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float out = multiFx.apply(buf[track], fxAmount.pct());
        buf[track] = out * volumeWithGain;
    }

    EffectVolumeMultiFx& setVolumeWithGain(float vol, float _gain)
    {
        gain.setFloat(_gain);
        volume.setFloat(vol);
        volumeWithGain = (1.0 + (gain.get())) * volume.pct();

        return *this;
    }
};
