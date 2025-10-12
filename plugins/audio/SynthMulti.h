#pragma once

#include "plugins/audio/MultiEngine/Additive2Engine.h"
#include "plugins/audio/MultiEngine/AdditiveEngine.h"
#include "plugins/audio/MultiEngine/BassEngine.h"
#include "plugins/audio/MultiEngine/FmEngine.h"
#include "plugins/audio/MultiEngine/SpaceShipEngine.h"
#include "plugins/audio/MultiEngine/StringEngine.h"
#include "plugins/audio/MultiEngine/SuperSawEngine.h"
#include "plugins/audio/MultiEngine/Wavetable2Engine.h"
#include "plugins/audio/MultiEngine/WavetableEngine.h"

/*md
## SynthMulti

Synth engine to generate multiple kind of sounds.
*/

class SynthMulti : public Mapping {
protected:
    FmEngine fmEngine;
    AdditiveEngine additiveEngine;
    Additive2Engine additive2Engine;
    SuperSawEngine superSawEngine;
    SpaceShipEngine spaceShipEngine;
    WavetableEngine wavetableEngine;
    Wavetable2Engine wavetable2Engine;
    BassEngine bassEngine;
    StringEngine stringEngine;

    static const int ENGINES_COUNT = 9;
    Engine* engines[ENGINES_COUNT] = {
        &fmEngine,
        &wavetableEngine,
        &wavetable2Engine,
        &additiveEngine,
        &additive2Engine,
        &superSawEngine,
        &spaceShipEngine,
        &bassEngine,
        &stringEngine,
    };
    Engine* selectedEngine = engines[0];

    void setEngineVal(Val::CallbackProps p, int index)
    {
        p.val.setFloat(p.value);
        if (index >= selectedEngine->mapping.size())
            return;
        // logDebug("setEngineVal (%d) %s %f", index, p.val.key().c_str(), p.val.get());
        ValueInterface* engineVal = selectedEngine->mapping[index];
        engineVal->set(p.val.get());
        p.val.setString(engineVal->string());
        p.val.props().label = engineVal->props().label;
        p.val.props().unit = engineVal->props().unit;
        p.val.props().min = engineVal->props().min;
        p.val.props().max = engineVal->props().max;
        p.val.props().floatingPoint = engineVal->props().floatingPoint;
    }

    void copyValues()
    {
        for (int i = 0; i < 12 && i < selectedEngine->mapping.size(); i++) {
            ValueInterface* val = selectedEngine->mapping[i];
            values[i].val->copy(val);
            values[i].key = val->key();
        }
    }

public:
    /*md **Values**: */

    /*md - `ENGINE` select the drum engine. */
    Val& engine = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = SynthMulti::ENGINES_COUNT - 1, .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        int index = (int)p.val.get();
        selectedEngine = engines[index];
        p.val.setString(selectedEngine->name);
        selectedEngine->initValues();

        // loop through values and update their type
        copyValues();
    });

    struct ValueMap {
        std::string key;
        Val* val;
    } values[12] = {
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
        { "VAL_11", &val(0.0f, "VAL_11", {}, [&](auto p) { setEngineVal(p, 10); }) },
        { "VAL_12", &val(0.0f, "VAL_12", {}, [&](auto p) { setEngineVal(p, 11); }) },
    };

    SynthMulti(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , fmEngine(props, config)
        , additiveEngine(props, config)
        , additive2Engine(props, config)
        , superSawEngine(props, config)
        , spaceShipEngine(props, config)
        , wavetableEngine(props, config)
        , wavetable2Engine(props, config)
        , bassEngine(props, config)
        , stringEngine(props, config)
    {
        initValues({ &engine });
    }

    void sample(float* buf) override
    {
        selectedEngine->sample(buf);
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        selectedEngine->noteOn(note, _velocity);
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        selectedEngine->noteOff(note, _velocity);
    }

    void serializeJson(nlohmann::json& json) override
    {
        Mapping::serializeJson(json);
        selectedEngine->serializeJson(json);
    }

    void hydrateJson(nlohmann::json& json) override
    {
        Mapping::hydrateJson(json);
        selectedEngine->hydrateJson(json);

        // After hydration copy back value in case something changed
        copyValues();
    }
};
