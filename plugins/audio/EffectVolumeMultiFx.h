#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/MultiFx.h"
#include "plugins/audio/utils/utils.h"

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
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    EffectVolumeMultiFx(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , multiFx(props.sampleRate, props.lookupTable)
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
