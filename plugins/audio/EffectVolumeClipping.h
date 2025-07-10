#pragma once

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/utils.h"
#include "plugins/audio/utils/effects/applyClipping.h"
#include "plugins/audio/utils/effects/applyDrive.h"
#include "plugins/audio/utils/effects/applyWaveshape.h"

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
        output = applyDrive(output, driveAmount, props.lookupTable);
        output = applyClipping(output, scaledClipping);

        buf[track] = output * volume.pct();
    }
};
