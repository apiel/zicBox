#pragma once

#include "audioPlugin.h"
#include "helpers/range.h"
#include "mapping.h"
#include "plugins/audio/utils/utils.h"
#include "utils/effects/applyBoost.h"
#include "utils/effects/applyDrive.h"
#include "utils/effects/applyWaveshape.h"
#include "utils/effects/applySoftClipping.h"
#include "utils/effects/applyCompression.h"

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
            output = applyDrive(output, driveAmount, props.lookupTable);
            output = applyCompression(output, compressAmount);
            output = applyWaveshape(output, waveshapeAmount, props.lookupTable);
            output = blend(input, output, levelAmount);
            output = applySoftClipping(output, props.lookupTable);

            buf[track] = CLAMP(output, -1.0f, 1.0f);
        }
    }

protected:
    float blend(float originalInput, float processedInput, float levelAmount)
    {
        return (1.0f - levelAmount) * originalInput + levelAmount * processedInput;
    }
};
