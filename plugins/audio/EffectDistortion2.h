#ifndef _EFFECT_DISTORTION2_H_
#define _EFFECT_DISTORTION2_H_

#include "audioPlugin.h"
#include "helpers/range.h"
#include "mapping.h"

#include <math.h>

/*//md
## EffectDistortion

EffectDistortion plugin is used to apply distortion effect on audio buffer.
*/
// class EffectDistortion2 : public Mapping {
// public:
//     /*md **Values**: */
//     /*md - `TRANSIENT_GAIN`  Multiplier for transients. */
//     Val& transientGain = val(2.0, "TRANSIENT_GAIN", { "Gain", .min = 1.0, .max = 3.0, .step = 0.1, .unit = "x" });
//     /*md - `BASS_BOOST` Boost for low frequencies. */
//     Val& bassBoost = val(1.5, "BASS_BOOST", { "Boost", .min = 1.0, .max = 3.0, .step = 0.1, .unit = "x" });
//     /*md - `HARMONIC_DRIVE` Saturation intensity for harmonic enhancement. */
//     Val& harmonicDrive = val(1.5, "HARMONIC_DRIVE", { "Drive", .min = 1.0, .max = 3.0, .step = 0.1, .unit = "x" });
//     /*md - `DYNAMIC_PEAK_BOOST` Peak boost for dynamics. */
//     Val& dynamicPeakBoost = val(1.3, "DYNAMIC_PEAK_BOOST", { "Peak Boost", .min = 1.0, .max = 2.0, .step = 0.1, .unit = "x" });

//     EffectDistortion2(AudioPlugin::Props& props, char* _name)
//         : Mapping(props, _name)
//     {
//         initValues();
//     }

//     void sample(float* buf)
//     {
//         float input = buf[track];
//         float output = input;

//         // Step 1: Transient enhancement (amplify attack phase)
//         if (output > 0.0f) {
//             output *= transientGain.get(); // Aggressive transient boost
//         }

//         // Step 2: Bass boost with dynamic tightness
//         float bassEnhanced = output * bassBoost.get();
//         if (bassEnhanced > 1.0f) { // Optional: Tame overly strong bass
//             bassEnhanced = 1.0f + (bassEnhanced - 1.0f) * 0.5f;
//         }

//         // Step 3: Harmonic saturation (target midrange)
//         bassEnhanced *= harmonicDrive.get(); // Apply drive
//         float saturated = std::tanh(bassEnhanced);

//         // Step 4: Dynamic peak boost (more impact)
//         if (saturated > 0.0f) {
//             output = std::pow(saturated, dynamicPeakBoost.get());
//         } else {
//             output = -std::pow(-saturated, dynamicPeakBoost.get());
//         }

//         buf[track] = output;
//     }
// };

// class EffectDistortion2 : public Mapping {
// public:
//     /*md **Values**: */
//     /*md - `DRIVE` controls the overdrive intensity (default 0). */
//     Val& drive = val(50.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `LEVEL` controls how much effect is applied (default 50). */
//     Val& level = val(50.0, "LEVEL", { "Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `COMPRESS` adjusts the compression strength (default 30). */
//     Val& compress = val(30.0, "COMPRESS", { "Compression", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });

//     EffectDistortion2(AudioPlugin::Props& props, char* _name)
//         : Mapping(props, _name)
//     {
//         initValues();
//     }

//     void sample(float* buf)
//     {
//         float input = buf[track];

//         // Get parameters as percentages (0.0 to 1.0)
//         float driveAmount = drive.pct();     // Drive: 0.0 (clean) to 1.0 (maximum overdrive)
//         float levelAmount = level.pct();     // Blend: 0.0 (dry) to 1.0 (fully distorted)
//         float compressAmount = compress.pct() * 2; // Compression: 0.0 (no compression) to 1.0 (max compression)

//         // Step 1: Apply overdrive (nonlinear distortion)
//         float drivenSignal = input * (1.0f + 4.0f * driveAmount); // Amplify signal for overdrive
//         float overdriven = std::tanh(drivenSignal);              // Smooth overdrive

//         // Step 2: Dynamic compression (control peaks for punch)
//         float compressed;
//         if (input > 0.0f) {
//             compressed = std::pow(overdriven, 1.0f - compressAmount); // Reduce peaks for positive values
//         } else {
//             compressed = -std::pow(-overdriven, 1.0f - compressAmount); // Reduce peaks for negative values
//         }

//         // Step 3: Blend overdriven + compressed signal with original (level control)
//         float output = (1.0f - levelAmount) * input + levelAmount * compressed;

//         // Step 4: Ensure output stays within [-1.0, 1.0]
//         buf[track] = std::max(-1.0f, std::min(1.0f, output));
//     }
// };

// class EffectDistortion2 : public Mapping {
// public:
//     /*md **Values**: */
//     /*md - `LEVEL` controls how much effect is applied (default 50). */
//     Val& level = val(50.0, "LEVEL", { "Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `DRIVE` controls the amount of distortion (default 50). */
//     Val& drive = val(50.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `COMPRESS` adjusts the compression strength (default 50). */
//     Val& compress = val(50.0, "COMPRESS", { "Compression", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });

//     EffectDistortion2(AudioPlugin::Props& props, char* _name)
//         : Mapping(props, _name)
//     {
//         initValues();
//     }

//     void sample(float* buf)
//     {
//         float input = buf[track];

//         // Get parameters as percentages (0.0 to 1.0)
//         float levelAmount = level.pct();      // Blend: 0.0 (dry) to 1.0 (fully processed)
//         float driveAmount = drive.pct();      // Drive: 0.0 (no distortion) to 1.0 (maximum distortion)
//         float compressAmount = compress.pct(); // Compression: 0.0 (no compression) to 1.0 (max compression)

//         // Step 1: Apply drive (tanh saturation)
//         float driven = std::tanh(input * (1.0f + driveAmount * 5.0f)); // Increase drive for more distortion

//         // Step 2: Apply compression
//         float compressed;
//         if (driven > 0.0f) {
//             compressed = std::pow(driven, 1.0f - compressAmount * 0.8f); // Positive compression
//         } else {
//             compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f); // Negative compression
//         }

//         // Step 3: Blend processed signal with original (level control)
//         float output = (1.0f - levelAmount) * input + levelAmount * compressed;

//         // Step 4: Ensure output stays within [-1.0, 1.0]
//         buf[track] = std::max(-1.0f, std::min(1.0f, output));
//     }
// };

// Nice one
// class EffectDistortion2 : public Mapping {
// public:
//     /*md **Values**: */
//     /*md - `LEVEL` controls how much effect is applied (default 50). */
//     Val& level = val(50.0, "LEVEL", { "Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `DRIVE` controls the amount of distortion (default 50). */
//     Val& drive = val(50.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `COMPRESS` adjusts the compression strength (default 50). */
//     Val& compress = val(50.0, "COMPRESS", { "Compression", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
//     /*md - `BASS` boosts low frequencies for more weight (default 50). */
//     Val& bass = val(50.0, "BASS", { "Bass Boost", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });

//     // State variables for a basic low-pass filter (bass emphasis)
//     float prevInput = 0.0f;
//     float prevOutput = 0.0f;

//     EffectDistortion2(AudioPlugin::Props& props, char* _name)
//         : Mapping(props, _name)
//     {
//         initValues();
//     }

//     void sample(float* buf)
//     {
//         float input = buf[track];

//         // Get parameters as percentages (0.0 to 1.0)
//         float levelAmount = level.pct();      // Blend: 0.0 (dry) to 1.0 (fully processed)
//         float driveAmount = drive.pct();      // Drive: 0.0 (no distortion) to 1.0 (maximum distortion)
//         float compressAmount = compress.pct(); // Compression: 0.0 (no compression) to 1.0 (max compression)
//         float bassBoostAmount = bass.pct();    // Bass boost: 0.0 (no boost) to 1.0 (maximum boost)

//         // Step 1: Apply bass boost using a simple low-pass filter
//         float bassFreq = 0.2f + 0.8f * bassBoostAmount; // Cutoff frequency (higher values retain more bass)
//         float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
//         prevInput = input;
//         prevOutput = bassBoosted;

//         // Amplify the bass-boosted signal
//         bassBoosted *= 1.0f + bassBoostAmount * 4.0f;

//         // Step 2: Apply drive (tanh saturation)
//         float driven = std::tanh(bassBoosted * (1.0f + driveAmount * 5.0f)); // Increase drive for more distortion

//         // Step 3: Apply compression
//         float compressed;
//         if (driven > 0.0f) {
//             compressed = std::pow(driven, 1.0f - compressAmount * 0.8f); // Positive compression
//         } else {
//             compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f); // Negative compression
//         }

//         // Step 4: Blend processed signal with original (level control)
//         float output = (1.0f - levelAmount) * input + levelAmount * compressed;

//         // Step 5: Soft clipping to avoid harsh distortion
//         output = std::tanh(output);

//         // Step 6: Ensure output stays within [-1.0, 1.0]
//         buf[track] = std::max(-1.0f, std::min(1.0f, output));
//     }
// };

// class EffectDistortion2 : public Mapping {
// public:
//     /*md **Values**: */
//     /*md - `LEVEL` controls how much effect is applied (default 50). */
//     Val& level = val(50.0, "LEVEL", { "Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
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

//     void sample(float* buf)
//     {
//         float input = buf[track];

//         // Get parameters as percentages (0.0 to 1.0)
//         float levelAmount = level.pct();        // Blend: 0.0 (dry) to 1.0 (fully processed)
//         float driveAmount = drive.pct();        // Drive: 0.0 (no distortion) to 1.0 (maximum distortion)
//         float compressAmount = compress.pct();  // Compression: 0.0 (no compression) to 1.0 (max compression)
//         float bassBoostAmount = bass.pct();     // Bass boost: 0.0 (no boost) to 1.0 (maximum boost)
//         float waveshapeAmount = waveshape.pct();// Waveshape: 0.0 (off) to 1.0 (maximum intensity)

//         // Step 1: Apply bass boost using a simple low-pass filter
//         float bassFreq = 0.2f + 0.8f * bassBoostAmount; // Cutoff frequency (higher values retain more bass)
//         float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
//         prevInput = input;
//         prevOutput = bassBoosted;

//         // Amplify the bass-boosted signal
//         bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

//         // Step 2: Apply drive (tanh saturation)
//         float driven = std::tanh(bassBoosted * (1.0f + driveAmount * 5.0f)); // Increase drive for more distortion

//         // Step 3: Apply compression
//         float compressed;
//         if (driven > 0.0f) {
//             compressed = std::pow(driven, 1.0f - compressAmount * 0.8f); // Positive compression
//         } else {
//             compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f); // Negative compression
//         }

//         // Step 4: Apply waveshaping for crazy character
//         float waveshaped = compressed;
//         if (waveshapeAmount > 0.0f) {
//             waveshaped = compressed + waveshapeAmount * std::sin(compressed * M_PI * (1.0f + waveshapeAmount * 2.0f));
//         }

//         // Step 5: Blend processed signal with original (level control)
//         float output = (1.0f - levelAmount) * input + levelAmount * waveshaped;

//         // Step 6: Soft clipping to avoid harsh distortion
//         output = std::tanh(output);

//         // Step 7: Ensure output stays within [-1.0, 1.0]
//         buf[track] = std::max(-1.0f, std::min(1.0f, output));
//     }
// };

// Loouk sin
// class EffectDistortion2 : public Mapping {
// public:
//     /*md **Values**: */
//     /*md - `LEVEL` controls how much effect is applied (default 50). */
//     Val& level = val(50.0, "LEVEL", { "Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
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

//     void sample(float* buf)
//     {
//         float input = buf[track];

//         // Get parameters as percentages (0.0 to 1.0)
//         float levelAmount = level.pct();        // Blend: 0.0 (dry) to 1.0 (fully processed)
//         float driveAmount = drive.pct();        // Drive: 0.0 (no distortion) to 1.0 (maximum distortion)
//         float compressAmount = compress.pct();  // Compression: 0.0 (no compression) to 1.0 (max compression)
//         float bassBoostAmount = bass.pct();     // Bass boost: 0.0 (no boost) to 1.0 (maximum boost)
//         float waveshapeAmount = waveshape.pct();// Waveshape: 0.0 (off) to 1.0 (maximum intensity)

//         // Step 1: Apply bass boost using a simple low-pass filter
//         float bassFreq = 0.2f + 0.8f * bassBoostAmount; // Cutoff frequency (higher values retain more bass)
//         float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
//         prevInput = input;
//         prevOutput = bassBoosted;

//         // Amplify the bass-boosted signal
//         bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

//         // Step 2: Apply drive (tanh saturation)
//         float driven = std::tanh(bassBoosted * (1.0f + driveAmount * 5.0f)); // Increase drive for more distortion

//         // Step 3: Apply compression
//         float compressed;
//         if (driven > 0.0f) {
//             compressed = std::pow(driven, 1.0f - compressAmount * 0.8f); // Positive compression
//         } else {
//             compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f); // Negative compression
//         }

//         // Step 4: Apply waveshaping using the lookup table
//         float waveshaped = compressed;
//         if (waveshapeAmount > 0.0f) {
//             float normalized = compressed * 0.5f + 0.5f; // Map compressed [-1.0, 1.0] to [0.0, 1.0]
//             int index = static_cast<int>(normalized * (LookupTable::size - 1)) % LookupTable::size; // Map to table index
//             float sineValue = props.lookupTable->sine[index]; // Lookup sine value
//             waveshaped = compressed + waveshapeAmount * sineValue;
//         }

//         // Step 5: Blend processed signal with original (level control)
//         float output = (1.0f - levelAmount) * input + levelAmount * waveshaped;

//         // Step 6: Soft clipping to avoid harsh distortion
//         output = std::tanh(output);

//         // Step 7: Ensure output stays within [-1.0, 1.0]
//         buf[track] = std::max(-1.0f, std::min(1.0f, output));
//     }
// };

// Loouk tanh and sin
class EffectDistortion2 : public Mapping {
public:
    /*md **Values**: */
    /*md - `LEVEL` controls how much effect is applied (default 50). */
    Val& level = val(50.0, "LEVEL", { "Level", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `DRIVE` controls the amount of distortion (default 50). */
    Val& drive = val(50.0, "DRIVE", { "Drive", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `COMPRESS` adjusts the compression strength (default 50). */
    Val& compress = val(50.0, "COMPRESS", { "Compression", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `BASS` boosts low frequencies for more weight (default 50). */
    Val& bass = val(50.0, "BASS", { "Bass Boost", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });
    /*md - `WAVESHAPE` adjusts the intensity of waveshaping (default 0, off). */
    Val& waveshape = val(0.0, "WAVESHAPE", { "Waveshape", .min = 0.0, .max = 100.0, .step = 1.0, .unit = "%" });

    // State variables for a basic low-pass filter (bass emphasis)
    float prevInput = 0.0f;
    float prevOutput = 0.0f;

    EffectDistortion2(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
    }

    // Lookup-based tanh function
    float tanhLookup(float x, LookupTable* table) {
        x = std::max(-1.0f, std::min(1.0f, x)); // Clamp x to [-1.0, 1.0]
        int index = static_cast<int>((x + 1.0f) * 0.5f * (LookupTable::size - 1)); // Map x to [0, size-1]
        return table->tanh[index];
    }

    void sample(float* buf)
    {
        float input = buf[track];

        // Get parameters as percentages (0.0 to 1.0)
        float levelAmount = level.pct();        // Blend: 0.0 (dry) to 1.0 (fully processed)
        float driveAmount = drive.pct();        // Drive: 0.0 (no distortion) to 1.0 (maximum distortion)
        float compressAmount = compress.pct();  // Compression: 0.0 (no compression) to 1.0 (max compression)
        float bassBoostAmount = bass.pct();     // Bass boost: 0.0 (no boost) to 1.0 (maximum boost)
        float waveshapeAmount = waveshape.pct();// Waveshape: 0.0 (off) to 1.0 (maximum intensity)

        // Step 1: Apply bass boost using a simple low-pass filter
        float bassFreq = 0.2f + 0.8f * bassBoostAmount; // Cutoff frequency (higher values retain more bass)
        float bassBoosted = (1.0f - bassFreq) * prevOutput + bassFreq * (input + prevInput) * 0.5f;
        prevInput = input;
        prevOutput = bassBoosted;

        // Amplify the bass-boosted signal
        bassBoosted *= 1.0f + bassBoostAmount * 2.0f;

        // Step 2: Apply drive (tanh saturation using lookup table)
        float driven = tanhLookup(bassBoosted * (1.0f + driveAmount * 5.0f), props.lookupTable);

        // Step 3: Apply compression
        float compressed;
        if (driven > 0.0f) {
            compressed = std::pow(driven, 1.0f - compressAmount * 0.8f); // Positive compression
        } else {
            compressed = -std::pow(-driven, 1.0f - compressAmount * 0.8f); // Negative compression
        }

        // Step 4: Apply waveshaping using the sine lookup table
        float waveshaped = compressed;
        if (waveshapeAmount > 0.0f) {
            float normalized = compressed * 0.5f + 0.5f; // Map compressed [-1.0, 1.0] to [0.0, 1.0]
            int index = static_cast<int>(normalized * (LookupTable::size - 1)) % LookupTable::size; // Map to table index
            float sineValue = props.lookupTable->sine2[index]; // Lookup sine value
            waveshaped = compressed + waveshapeAmount * sineValue;
        }

        // Step 5: Blend processed signal with original (level control)
        float output = (1.0f - levelAmount) * input + levelAmount * waveshaped;

        // Step 6: Soft clipping to avoid harsh distortion
        output = tanhLookup(output, props.lookupTable);

        // Step 7: Ensure output stays within [-1.0, 1.0]
        buf[track] = std::max(-1.0f, std::min(1.0f, output));
    }
};




#endif
