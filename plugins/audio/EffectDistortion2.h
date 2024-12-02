#ifndef _EFFECT_DISTORTION2_H_
#define _EFFECT_DISTORTION2_H_

#include "audioPlugin.h"
#include "helpers/range.h"
#include "plugins/audio/utils/utils.h"
#include "mapping.h"

#include <math.h>

/*md
## EffectDistortion2

EffectDistortion2 plugin is used to apply distortion effect on audio buffer.
*/

// class EffectDistortion2 : public Mapping {
// public:
//     /*md **Values**: */
//     /*md - `LEVEL` controls how much effect is applied (default 50). */
//     Val& level = val(50.0, "LEVEL", { "FX Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `DRIVE` controls the amount of distortion (default 50). */
//     Val& drive = val(50.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `COMPRESS` adjusts the compression strength (default 50). */
//     Val& compress = val(50.0, "COMPRESS", { "Compression", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `BASS` boosts low frequencies for more weight (default 50). */
//     Val& bass = val(50.0, "BASS", { "Bass Boost", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `WAVESHAPE` adjusts the intensity of waveshaping (default 0, off). */
//     Val& waveshape = val(0.0, "WAVESHAPE", { "Waveshape", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });

//     // State variables for a basic low-pass filter (bass emphasis)
//     float prevInput = 0.0f;
//     float prevOutput = 0.0f;

//     EffectDistortion2(AudioPlugin::Props& props, char* _name)
//         : Mapping(props, _name)
//     {
//         initValues();
//     }

//     float tanhLookup(float x)
//     {
//         x = range(x, -1.0f, 1.0f); // Clamp x to [-1.0, 1.0]
//         int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1)); // Map x to [0, size-1]
//         return props.lookupTable->tanh[index];
//     }

//     float sineLookupInterpolated(float x)
//     {
//         x -= std::floor(x); // Wrap x to [0, 1)
//         return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
//     }

//     void sample(float* buf)
//     {
//         float input = buf[track];

//         // Get parameters as percentages (0.0 to 1.0)
//         float levelAmount = level.pct(); // Blend: 0.0 (dry) to 1.0 (fully processed)
//         float driveAmount = drive.pct(); // Drive: 0.0 (no distortion) to 1.0 (maximum distortion)
//         float compressAmount = compress.pct(); // Compression: 0.0 (no compression) to 1.0 (max compression)
//         float bassBoostAmount = bass.pct(); // Bass boost: 0.0 (no boost) to 1.0 (maximum boost)
//         float waveshapeAmount = waveshape.pct(); // Waveshape: 0.0 (off) to 1.0 (maximum intensity)

//         // Step 1: Apply bass boost using a simple low-pass filter
//         float bassFreq = 0.2f + 0.8f * bassBoostAmount; // Cutoff frequency (higher values retain more bass)
//         float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
//         prevInput = input;
//         prevOutput = bassBoosted;

//         // Amplify the bass-boosted signal
//         bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

//         // Step 2: Apply drive (tanh saturation using lookup table)
//         float driven = tanhLookup(bassBoosted * (1.0f + driveAmount * 5.0f));

//         // Step 3: Apply compression
//         float compressed;
//         if (driven > 0.0f) {
//             compressed = std::pow(driven, 1.0f - compressAmount * 0.8f); // Positive compression
//         } else {
//             compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f); // Negative compression
//         }

//         // Step 4: Apply waveshaping using the sine lookup table
//         float waveshaped = compressed;
//         if (waveshapeAmount > 0.0f) {
//             float normalized = compressed * 0.5f + 0.5f; // Map compressed [-1.0, 1.0] to [0.0, 1.0]
//             float sineValue = sineLookupInterpolated(compressed); // Use interpolated LUT
//             waveshaped = compressed + waveshapeAmount * sineValue;
//         }

//         // Step 5: Blend processed signal with original (level control)
//         float output = (1.0f - levelAmount) * input + levelAmount * waveshaped;

//         // Step 6: Soft clipping to avoid harsh distortion
//         output = tanhLookup(output);

//         // Step 7: Ensure output stays within [-1.0, 1.0]
//         buf[track] = std::max(-1.0f, std::min(1.0f, output));
//     }
// };




// class EffectDistortion2 : public Mapping {
// public:
//     /*md **Values**: */
//     /*md - `LEVEL` controls how much effect is applied (default 50). */
//     Val& level = val(50.0, "LEVEL", { "FX Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `DRIVE` controls the amount of distortion (default 50). */
//     Val& drive = val(50.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `COMPRESS` adjusts the compression strength (default 50). */
//     Val& compress = val(50.0, "COMPRESS", { "Compression", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `BASS` boosts low frequencies for more weight (default 50). */
//     Val& bass = val(50.0, "BASS", { "Bass Boost", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `WAVESHAPE` adjusts the intensity of waveshaping (default 0, off). */
//     Val& waveshape = val(0.0, "WAVESHAPE", { "Waveshape", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });

//     // State variables for a basic low-pass filter (bass emphasis)
//     float prevInput = 0.0f;
//     float prevOutput = 0.0f;

//     EffectDistortion2(AudioPlugin::Props& props, char* _name)
//         : Mapping(props, _name)
//     {
//         initValues();
//     }

//     float tanhLookup(float x)
//     {
//         x = range(x, -1.0f, 1.0f); // Clamp x to [-1.0, 1.0]
//         int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1)); // Map x to [0, size-1]
//         return props.lookupTable->tanh[index];
//     }

//     float sineLookupInterpolated(float x)
//     {
//         x -= std::floor(x); // Wrap x to [0, 1)
//         return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
//     }

//     void sample(float* buf)
//     {
//         float input = buf[track];

//         // Get parameters as percentages (0.0 to 1.0)
//         float levelAmount = level.pct(); // Blend: 0.0 (dry) to 1.0 (fully processed)
//         float driveAmount = drive.pct(); // Drive: 0.0 (no distortion) to 1.0 (maximum distortion)
//         float compressAmount = compress.pct(); // Compression: 0.0 (no compression) to 1.0 (max compression)
//         float bassBoostAmount = bass.pct(); // Bass boost: 0.0 (no boost) to 1.0 (maximum boost)
//         float waveshapeAmount = waveshape.pct(); // Waveshape: 0.0 (off) to 1.0 (maximum intensity)

//         // Step 1: Apply bass boost using a simple low-pass filter
//         float bassFreq = 0.2f + 0.8f * bassBoostAmount; // Cutoff frequency (higher values retain more bass)
//         float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
//         prevInput = input;
//         prevOutput = bassBoosted;

//         // Amplify the bass-boosted signal
//         bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

//         // Step 2: Apply drive (tanh saturation using lookup table)
//         float driven = tanhLookup(bassBoosted * (1.0f + driveAmount * 5.0f));

//         // Step 3: Apply compression
//         float compressed;
//         if (driven > 0.0f) {
//             compressed = std::pow(driven, 1.0f - compressAmount * 0.8f); // Positive compression
//         } else {
//             compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f); // Negative compression
//         }

//         // Step 4: Apply waveshaping using the sine lookup table
//         float waveshaped = compressed;
//         if (waveshapeAmount > 0.0f) {
//             float normalized = compressed * 0.5f + 0.5f; // Map compressed [-1.0, 1.0] to [0.0, 1.0]
//             float sineValue = sineLookupInterpolated(compressed); // Use interpolated LUT
//             waveshaped = compressed + waveshapeAmount * sineValue;
//         }

//         // Step 5: Blend processed signal with original (level control)
//         float output = (1.0f - levelAmount) * input + levelAmount * waveshaped;

//         // Step 6: Soft clipping to avoid harsh distortion
//         output = tanhLookup(output);

//         // Step 7: Ensure output stays within [-1.0, 1.0]
//         buf[track] = std::max(-1.0f, std::min(1.0f, output));
//     }
// };


class EffectDistortion2 : public Mapping {
public:
    Val& level = val(50.0, "LEVEL", { "FX Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    Val& drive = val(50.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    Val& compress = val(50.0, "COMPRESS", { "Compression", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    Val& bass = val(50.0, "BASS", { "Bass Boost", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    Val& waveshape = val(0.0, "WAVESHAPE", { "Waveshape", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });

    float prevInput1 = 0.0f, prevOutput1 = 0.0f; // State for pass 1
    float prevInput2 = 0.0f, prevOutput2 = 0.0f; // State for pass 2

    EffectDistortion2(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    float tanhLookup(float x) {
        x = range(x, -1.0f, 1.0f);
        int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1));
        return props.lookupTable->tanh[index];
    }

    float sineLookupInterpolated(float x) {
        x -= std::floor(x);
        return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
    }

    void sample(float* buf) {
        float input = buf[track];

        // Get parameters
        float levelAmount = level.pct();
        float driveAmount = drive.pct();
        float compressAmount = compress.pct();
        float bassBoostAmount = bass.pct();
        float waveshapeAmount = waveshape.pct();

        // First pass
        float intermediate = processSample(input, levelAmount, driveAmount, compressAmount, bassBoostAmount, waveshapeAmount, prevInput1, prevOutput1);

        // Second pass
        float output = processSample(intermediate, levelAmount, driveAmount, compressAmount, bassBoostAmount, waveshapeAmount, prevInput2, prevOutput2);

        // Ensure output stays within [-1.0, 1.0]
        buf[track] = std::max(-1.0f, std::min(1.0f, output));
    }

private:
    float processSample(float input, float levelAmount, float driveAmount, float compressAmount, float bassBoostAmount, float waveshapeAmount, float& prevInput, float& prevOutput) {
        // Step 1: Apply bass boost
        float bassFreq = 0.2f + 0.8f * bassBoostAmount;
        float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
        prevInput = input;
        prevOutput = bassBoosted;
        bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

        // Step 2: Apply drive
        float driven = tanhLookup(bassBoosted * (1.0f + driveAmount * 5.0f));

        // Step 3: Apply compression
        float compressed;
        if (driven > 0.0f) {
            compressed = std::pow(driven, 1.0f - compressAmount * 0.8f);
        } else {
            compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f);
        }

        // Step 4: Apply waveshaping
        float waveshaped = compressed;
        if (waveshapeAmount > 0.0f) {
            float sineValue = sineLookupInterpolated(compressed);
            waveshaped = compressed + waveshapeAmount * sineValue;
        }

        // Step 5: Blend with original signal
        float processed = (1.0f - levelAmount) * input + levelAmount * waveshaped;

        // Step 6: Soft clipping
        return tanhLookup(processed);
    }
};




// class EffectDistortion2 : public Mapping {
// public:
//     // Parameters for the effect
//     Val& level = val(50.0, "LEVEL", { "FX Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     Val& drive = val(50.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     Val& compress = val(50.0, "COMPRESS", { "Compression", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     Val& bass = val(50.0, "BASS", { "Bass Boost", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     Val& waveshape = val(0.0, "WAVESHAPE", { "Waveshape", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });

//     float prevInput = 0.0f;
//     float prevOutput = 0.0f;

//     EffectDistortion2(AudioPlugin::Props& props, char* _name)
//         : Mapping(props, _name)
//     {
//         initValues();
//     }

//     float tanhLookup(float x) {
//         x = range(x, -1.0f, 1.0f);
//         int index = static_cast<int>((x + 1.0f) * 0.5f * (props.lookupTable->size - 1));
//         return props.lookupTable->tanh[index];
//     }

//     float sineLookupInterpolated(float x) {
//         x -= std::floor(x);
//         return linearInterpolation(x, props.lookupTable->size, props.lookupTable->sine);
//     }

//     void sample(float* buf) {
//         float input = buf[track];

//         // Get parameters as percentages
//         float levelAmount = level.pct();
//         float driveAmount = drive.pct();
//         float compressAmount = compress.pct();
//         float bassBoostAmount = bass.pct();
//         float waveshapeAmount = waveshape.pct();

//         // Process the signal once
//         float output = processSample(input, levelAmount, driveAmount, compressAmount, bassBoostAmount, waveshapeAmount);

//         // Process the output again to simulate chaining
//         output = processSample(output, levelAmount, driveAmount, compressAmount, bassBoostAmount, waveshapeAmount);

//         // Ensure output stays within [-1.0, 1.0]
//         buf[track] = std::max(-1.0f, std::min(1.0f, output));
//     }

// protected:
//     float processSample(float input, float levelAmount, float driveAmount, float compressAmount, float bassBoostAmount, float waveshapeAmount) {
//         // Step 1: Apply bass boost
//         float bassFreq = 0.2f + 0.8f * bassBoostAmount;
//         float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
//         prevInput = input;
//         prevOutput = bassBoosted;
//         bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

//         // Step 2: Apply drive
//         float driven = tanhLookup(bassBoosted * (1.0f + driveAmount * 5.0f));

//         // Step 3: Apply compression
//         float compressed;
//         if (driven > 0.0f) {
//             compressed = std::pow(driven, 1.0f - compressAmount * 0.8f);
//         } else {
//             compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f);
//         }

//         // Step 4: Apply waveshaping
//         float waveshaped = compressed;
//         if (waveshapeAmount > 0.0f) {
//             float sineValue = sineLookupInterpolated(compressed);
//             waveshaped = compressed + waveshapeAmount * sineValue;
//         }

//         // Step 5: Blend with original signal
//         float processed = (1.0f - levelAmount) * input + levelAmount * waveshaped;

//         // Step 6: Soft clipping
//         return tanhLookup(processed);
//     }
// };


#endif
