#pragma once

#include "plugins/audio/utils/EnvelopDrumAmp.h"
#include "plugins/audio/MultiDrumEngine/MetalicDrumEngine.h"
#include "plugins/audio/MultiDrumEngine/PercussionEngine.h"
#include "plugins/audio/MultiDrumEngine/BassEngine.h"

/*md
## SynthMultiDrum

Synth engine to generate multiple kind of drum sounds.
*/

class SynthMultiDrum : public Mapping {
protected:
    EnvelopDrumAmp envelopAmp;

    MetalicDrumEngine metalDrumEngine;
    PercussionEngine percEngine;
    BassEngine bassEngine;

    DrumEngine* drumEngines[3] = {
        &metalDrumEngine,
        &percEngine,
        &bassEngine,
    };
    DrumEngine* drumEngine = drumEngines[0];

public:
    /*md **Values**: */

    Val* values[10] = {
        &val(0.0f, "VAL_1", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[0]->set(p.val.get()); p.val.setString(drumEngine->mapping[0]->string()); }),
        &val(0.0f, "VAL_2", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[1]->set(p.val.get()); p.val.setString(drumEngine->mapping[1]->string()); }),
        &val(0.0f, "VAL_3", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[2]->set(p.val.get()); p.val.setString(drumEngine->mapping[2]->string()); }),
        &val(0.0f, "VAL_4", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[3]->set(p.val.get()); p.val.setString(drumEngine->mapping[3]->string()); }),
        &val(0.0f, "VAL_5", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[4]->set(p.val.get()); p.val.setString(drumEngine->mapping[4]->string()); }),
        &val(0.0f, "VAL_6", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[5]->set(p.val.get()); p.val.setString(drumEngine->mapping[5]->string()); }),
        &val(0.0f, "VAL_7", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[6]->set(p.val.get()); p.val.setString(drumEngine->mapping[6]->string()); }),
        &val(0.0f, "VAL_8", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[7]->set(p.val.get()); p.val.setString(drumEngine->mapping[7]->string()); }),
        &val(0.0f, "VAL_9", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[8]->set(p.val.get()); p.val.setString(drumEngine->mapping[8]->string()); }),
        &val(0.0f, "VAL_10", {}, [&](auto p) { p.val.setFloat(p.value); drumEngine->mapping[9]->set(p.val.get()); p.val.setString(drumEngine->mapping[9]->string()); }),
    };

    /*md - `ENGINE` select the drum engine. */
    Val& engine = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = 2 }, [&](auto p) {
        p.val.setFloat(p.value);
        drumEngine = drumEngines[(int)p.val.get()];
        p.val.setString(drumEngine->name);

        // loop through values and update their type
        for (int i = 0; i < 10 && i < drumEngine->mapping.size(); i++) {
            ValueInterface* val = drumEngine->mapping[i];
            values[i]->copy(val);
        }
    });

    /*md - `DURATION` controls the duration of the envelope. */
    Val& duration = val(500.0f, "DURATION", { "Duration", .min = 50.0, .max = 3000.0, .step = 10.0, .unit = "ms" });

    /*md - `AMP_MORPH` morph on the shape of the envelop of the amplitude.*/
    Val& ampMorph = val(0.0f, "AMP_MORPH", { "Amp. Morph", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        envelopAmp.morph(p.val.pct());
    });

    SynthMultiDrum(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , metalDrumEngine(props, config)
        , percEngine(props, config)
        , bassEngine(props, config)
    {
        initValues();
    }

    int totalSamples = 0;
    int i = 0;
    // float phase = 0.0f;
    // float phaseIncrement = 0.0f;
    // float resonatorState = 0.0f;
    // float noteFreq = 440.0f;
    void sample(float* buf) override
    {
        if (i < totalSamples) {
            float envAmp = envelopAmp.next();
            drumEngine->sampleOn(buf, envAmp, i, totalSamples);
            i++;
        } else {
            // buf[track] = applyReverb(buf[track]);
            drumEngine->sampleOff(buf);
        }
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        const float sampleRate = props.sampleRate;
        totalSamples = static_cast<int>(sampleRate * (duration.get() / 1000.0f));
        envelopAmp.reset(totalSamples);
        i = 0;
        drumEngine->noteOn(note, _velocity);
    }

    DataFn dataFunctions[1] = {
        { "ENV_AMP_FORM", [this](void* userdata) {
             float* index = (float*)userdata;
             return (void*)envelopAmp.getMorphShape(*index);
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
