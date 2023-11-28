#ifndef _EFFECT_GAIN_VOLUME_H_
#define _EFFECT_GAIN_VOLUME_H_

#include "audioPlugin.h"
#include "mapping.h"

class EffectGainVolume : public Mapping {
public:
    Val& volume = val(100.0f, "VOLUME", [&](float value) { setVolumeWithGain(value, gain.get()); }, { "Volume" });
    Val& gain = val(0.0f, "GAIN", [&](float value) { setVolumeWithGain(volume.get(), value); }, { "Gain" });
    float volumeWithGain = volume.get();

    EffectGainVolume(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        setVolumeWithGain(volume.get(), gain.get());
    }

    void sample(float* buf)
    {
        buf[track] = buf[track] * volumeWithGain;
    }

    EffectGainVolume& setVolumeWithGain(float vol, float _gain)
    {
        gain.setFloat(_gain);
        volume.setFloat(vol);
        volumeWithGain = (1.0 + (gain.pct() * 4.0)) * volume.pct();

        return *this;
    }
};

#endif
