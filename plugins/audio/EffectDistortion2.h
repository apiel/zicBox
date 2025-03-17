#pragma once

#include "audioPlugin.h"
#include "helpers/range.h"
#include "mapping.h"
#include "plugins/audio/utils/utils.h"

#include <math.h>

/*md
## EffectDistortion2

EffectDistortion2 plugin is used to apply distortion effect on audio buffer.
*/

class EffectDistortion2 : public Mapping {
public:
    /*md **Values**: */
    /*md - `LEVEL` to set distortion mix level. */
    Val& level = val(100.0, "LEVEL", { "Dist. Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `DRIVE` to set distortion drive. */
    Val& drive = val(0.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `COMPRESS` to set distortion compression. */
    Val& compress = val(0.0, "COMPRESS", { "Compression", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `BASS` to set bass boost. */
    Val& bass = val(0.0, "BASS", { "Bass Boost", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `WAVESHAPE` to set waveshape. */
    Val& waveshape = val(0.0, "WAVESHAPE", { "Waveshape", .type = VALUE_CENTERED, .min = -100.0, .max = 100.0, .step = 1.0, .unit = "%" });

    float prevInput1 = 0.0f, prevOutput1 = 0.0f; // State for pass 1
    // float prevInput2 = 0.0f, prevOutput2 = 0.0f; // State for pass 2

    EffectDistortion2(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        initValues();
    }

    float tanhLookup(float x)
    {
        x = range(x, -1.0f, 1.0f);
        int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1));
        return props.lookupTable->tanh[index];
    }

    float sineLookupInterpolated(float x)
    {
        x -= std::floor(x);
        return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
    }

    void sample(float* buf)
    {
        float input = buf[track];
        if (input != 0.0f) { // <--- could this be a problem?
            // Get parameters
            float levelAmount = level.pct();
            float driveAmount = drive.pct();
            float compressAmount = compress.pct() * 2 - 1.0f;
            float bassBoostAmount = bass.pct();
            float waveshapeAmount = waveshape.pct() * 2 - 1.0f;

            float output = input;
            output = applyBoost(output, bassBoostAmount, prevInput1, prevOutput1);
            output = applyDrive(output, driveAmount);
            output = applyCompression(output, compressAmount);
            output = applyWaveshape(output, waveshapeAmount);
            output = blend(input, output, levelAmount);
            output = applySoftClipping(output);

            buf[track] = range(output, -1.0f, 1.0f);
        }
    }

protected:
    float blend(float originalInput, float processedInput, float levelAmount)
    {
        return (1.0f - levelAmount) * originalInput + levelAmount * processedInput;
    }

    float applySoftClipping(float input)
    {
        return tanhLookup(input);
    }

    float applyWaveshape(float input, float waveshapeAmount)
    {
        if (waveshapeAmount > 0.0f) {
            // float sineValue = sineLookupInterpolated(input);
            float sineValue = sinf(input);
            return input + waveshapeAmount * sineValue * 2;
        }
        if (waveshapeAmount < 0.0f) {
            float sineValue = sineLookupInterpolated(input);
            return input + (-waveshapeAmount) * sineValue;
        }
        return input;
    }

    float applyBoost(float input, float bassBoostAmount, float& prevInput, float& prevOutput)
    {
        if (bassBoostAmount == 0.0f) {
            return input;
        }
        float bassFreq = 0.2f + 0.8f * bassBoostAmount;
        float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
        prevInput = input;
        prevOutput = bassBoosted;
        bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

        return bassBoosted;
    }

    float applyDrive(float input, float driveAmount)
    {
        if (driveAmount == 0.0f) {
            return input;
        }
        return tanhLookup(input * (1.0f + driveAmount * 5.0f));
    }

    float applyCompression(float input, float compressAmount)
    {
        if (compressAmount == 0.0f) {
            return input;
        }
        if (input > 0.0f) {
            return std::pow(input, 1.0f - compressAmount * 0.8f);
        }
        return -std::pow(-input, 1.0f - compressAmount * 0.8f);
    }
};
