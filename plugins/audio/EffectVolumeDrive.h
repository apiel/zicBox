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

    float compressAmount = 0.0f;
    float driveAmount = 0.0f;

    /*md - `DRIVE` to set drive and compression. */
    Val& mix = val(100.0, "DRIVE", { "Comp. | Drive", .type = VALUE_CENTERED, .max = 200.0f, .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == 100.0f) {
            driveAmount = 0.0f;
            compressAmount = 0.0f;
        } else if (p.val.get() > 100.0f) {
            driveAmount = (p.val.get() - 100.0f) / 100.0f;
            compressAmount = 0.0f;
        } else {
            driveAmount = 0.0f;
            compressAmount = (100.0f - p.val.get()) / 100.0f;
        }
    });

    EffectVolumeDrive(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float output = buf[track];
        output = applyCompression(output, compressAmount);
        output = applyDrive(output, driveAmount);
        buf[track] = output * volumeWithGain;
    }

    float applyCompression(float input, float compressAmount)
    {
        if (compressAmount == 0.0f) {
            return input;
        }
        return std::pow(input, 1.0f - compressAmount * 0.8f);
    }

    float applyDrive(float input, float driveAmount)
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
};

#endif