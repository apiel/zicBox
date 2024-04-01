#ifndef _EFFECT_GAIN_VOLUME_H_
#define _EFFECT_GAIN_VOLUME_H_

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

    EffectGainVolume(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
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

#endif
