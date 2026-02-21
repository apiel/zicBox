/** Description:
This C++ component acts as a versatile digital effects rack for processing audio signals. It is designed as a centralized control unit, housing over 25 distinct sound modifications, ranging from atmospheric effects like various Reverbs and Delays, to aggressive distortions like Drive, Bitcrusher, and Clipping, as well as utility tools such as Compression and advanced frequency Filters.

The key efficiency of this design lies in its dynamic switching capability. Instead of checking which effect is active for every single audio sample, the class uses an internal dynamic selector (a specialized pointer) that always points directly to the function of the currently chosen effect.

When a user selects a new effect (e.g., moving from ‘Tremolo’ to ‘Flanger’), a dedicated control routine immediately redirects this internal selector to the proper processing logic. The main "apply" function then simply executes whatever code the selector is currently pointing to, ensuring rapid, efficient processing.

For high-quality, real-time results, the class relies on essential background data, including the audio system's clock speed (sample rate) and pre-calculated mathematical tables (like sine waves or complex curves). This architecture makes the system quick to switch effects and robust enough to handle a wide spectrum of sound shaping tasks.

Tags: Multi-effects, Sound Processing, Reverb and Delay, Distortion, Audio Filtering
sha: 158e510ea6c33799bfaf38276413d9c2f1816d1f8f0c2cb7cece6a4fb06fc06c
*/
#pragma once

#include "audio/MFx.h"
#include "plugins/audio/mapping.h"
class MultiFx : public MFx {
public:
    MultiFx(uint64_t sampleRate) : MFx(sampleRate) {}

    Val::CallbackFn setFxType = [&](auto p) {
        p.val.setFloat(p.value);
        this->setEffect(static_cast<int>(p.value));
        p.val.setString(this->getEffectName());
    };
};
