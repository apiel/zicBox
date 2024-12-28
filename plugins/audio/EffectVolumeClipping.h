#ifndef _EFFECT_VOLUME_CLIPPING_H_
#define _EFFECT_VOLUME_CLIPPING_H_

#include "audioPlugin.h"
#include "mapping.h"
#include "plugins/audio/utils/utils.h"

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

    EffectVolumeClipping(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    void sample(float* buf)
    {
        float output = buf[track];
        output = applyWaveshape(output);
        output = applyDrive(output);
        output = applyClipping(output);

        buf[track] = output * volume.pct();
    }

    float applyClipping(float input)
    {
        if (scaledClipping == 0.0f) {
            return input;
        }
        return range(input + input * scaledClipping, -1.0f, 1.0f);
    }

    float sineLookupInterpolated(float x)
    {
        x -= std::floor(x);
        return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
    }

    float applyWaveshape(float input)
    {
        if (waveshapeAmount > 0.0f) {
            float sineValue = sineLookupInterpolated(input);
            return input + waveshapeAmount * sineValue;
        }
        return input;
    }

    float applyDrive(float input)
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
