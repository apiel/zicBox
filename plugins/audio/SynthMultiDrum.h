#pragma once

#include "plugins/audio/MultiDrumEngine/BassEngine.h"
#include "plugins/audio/MultiDrumEngine/ClapEngine.h"
#include "plugins/audio/MultiDrumEngine/Er1PcmEngine.h"
#include "plugins/audio/MultiDrumEngine/KickEngine.h"
#include "plugins/audio/MultiDrumEngine/MetalicDrumEngine.h"
#include "plugins/audio/MultiDrumEngine/PercussionEngine.h"
#include "plugins/audio/MultiDrumEngine/VolcEngine.h"
#include "plugins/audio/MultiDrumEngine/FmEngine.h"
#include "plugins/audio/MultiDrumEngine/StringEngine.h"
#include "plugins/audio/utils/EnvelopDrumAmp.h"

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
    ClapEngine clapEngine;
    KickEngine kickEngine;
    Er1PcmEngine er1PcmEngine;
    VolcEngine volcanEngine;
    FmEngine fmEngine;
    StringEngine stringEngine;

    static const int ENGINES_COUNT = 9;
    DrumEngine* drumEngines[ENGINES_COUNT] = {
        &metalDrumEngine,
        &percEngine,
        &bassEngine,
        &clapEngine,
        &kickEngine,
        &er1PcmEngine,
        &volcanEngine,
        &fmEngine,
        &stringEngine,
    };
    DrumEngine* drumEngine = drumEngines[0];

    void setEngineVal(Val::CallbackProps p, int index)
    {
        p.val.setFloat(p.value);
        if (index >= drumEngine->mapping.size())
            return;
        // logDebug("setEngineVal (%d) %s %f", index, p.val.key().c_str(), p.val.get());
        ValueInterface* drumEngineVal = drumEngine->mapping[index];
        drumEngineVal->set(p.val.get());
        p.val.setString(drumEngineVal->string());
        p.val.props().label = drumEngineVal->props().label;
        p.val.props().unit = drumEngineVal->props().unit;
        p.val.props().min = drumEngineVal->props().min;
        p.val.props().max = drumEngineVal->props().max;
        p.val.props().floatingPoint = drumEngineVal->props().floatingPoint;
    }

    void copyValues()
    {
        for (int i = 0; i < 10 && i < drumEngine->mapping.size(); i++) {
            ValueInterface* val = drumEngine->mapping[i];
            values[i].val->copy(val);
            values[i].key = val->key();
        }
    }

    SetValFn setVal = [&](std::string key, float value) {
        for (int i = 0; i < 10 && i < drumEngine->mapping.size(); i++) {
            if (values[i].key == key) {
                values[i].val->set(value);
                return;
            }
        }
    };

public:
    /*md **Values**: */

    /*md - `ENGINE` select the drum engine. */
    Val& engine = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = SynthMultiDrum::ENGINES_COUNT - 1, .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        int index = (int)p.val.get();
        drumEngine = drumEngines[index];
        p.val.setString(drumEngine->name);
        drumEngine->initValues();

        // loop through values and update their type
        copyValues();
    });

    struct ValueMap
    {
        std::string key;
        Val* val;
    } values[10] = {
        { "VAL_1", &val(0.0f, "VAL_1", {}, [&](auto p) { setEngineVal(p, 0); }) },
        { "VAL_2", &val(0.0f, "VAL_2", {}, [&](auto p) { setEngineVal(p, 1); }) },
        { "VAL_3", &val(0.0f, "VAL_3", {}, [&](auto p) { setEngineVal(p, 2); }) },
        { "VAL_4", &val(0.0f, "VAL_4", {}, [&](auto p) { setEngineVal(p, 3); }) },
        { "VAL_5", &val(0.0f, "VAL_5", {}, [&](auto p) { setEngineVal(p, 4); }) },
        { "VAL_6", &val(0.0f, "VAL_6", {}, [&](auto p) { setEngineVal(p, 5); }) },
        { "VAL_7", &val(0.0f, "VAL_7", {}, [&](auto p) { setEngineVal(p, 6); }) },
        { "VAL_8", &val(0.0f, "VAL_8", {}, [&](auto p) { setEngineVal(p, 7); }) },
        { "VAL_9", &val(0.0f, "VAL_9", {}, [&](auto p) { setEngineVal(p, 8); }) },
        { "VAL_10", &val(0.0f, "VAL_10", {}, [&](auto p) { setEngineVal(p, 9); }) },
    };
    
    /*md - `DURATION` controls the duration of the envelope. */
    Val& duration = val(500.0f, "DURATION", { "Duration", .min = 50.0, .max = 3000.0, .step = 10.0, .unit = "ms" });

    GraphPointFn ampGraph = [&](float index) { return *envelopAmp.getMorphShape(index); };
    /*md - `AMP_MORPH` morph on the shape of the envelop of the amplitude.*/
    Val& ampMorph = val(0.0f, "AMP_MORPH", { "Amp. Env.", .unit = "%", .graph = ampGraph }, [&](auto p) {
        p.val.setFloat(p.value);
        envelopAmp.morph(p.val.pct());
    });

    SynthMultiDrum(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , metalDrumEngine(props, config)
        , percEngine(props, config)
        , bassEngine(props, config)
        , clapEngine(props, config)
        , kickEngine(props, config)
        , er1PcmEngine(props, config)
        , volcanEngine(props, config)
        , fmEngine(props, config)
        , stringEngine(props, config)
    {
        initValues({ &engine });

        for (int i = 0; i < 10; i++) {
            drumEngines[i]->setValFn = setVal;
        }
        // bassEngine.setValFn = setVal;
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

    void serializeJson(nlohmann::json& json) override
    {
        Mapping::serializeJson(json);
        drumEngine->serializeJson(json);
    }

    void hydrateJson(nlohmann::json& json) override
    {
        Mapping::hydrateJson(json);
        drumEngine->hydrateJson(json);

        // After hydration copy back value in case something changed
        copyValues();
    }
};
