#ifndef _EFFECT_VOLUME_DRIVE_H_
#define _EFFECT_VOLUME_DRIVE_H_

#include "audioPlugin.h"
#include "mapping.h"

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

    EffectVolumeDrive(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void sample(float* buf)
    {
        buf[track] = buf[track] * volumeWithGain;
    }
};

#endif
