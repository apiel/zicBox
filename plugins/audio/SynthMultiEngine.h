#pragma once

#include "plugins/audio/MultiEngine.h"

// Synth
#include "plugins/audio/MultiEngine/Additive2Engine.h"
#include "plugins/audio/MultiEngine/AdditiveEngine.h"
#include "plugins/audio/MultiEngine/BassEngine.h"
#include "plugins/audio/MultiEngine/FmEngine.h"
#include "plugins/audio/MultiEngine/SpaceShipEngine.h"
#include "plugins/audio/MultiEngine/StringEngine.h"
#include "plugins/audio/MultiEngine/SuperSawEngine.h"
#ifndef SKIP_SNDFILE
#include "plugins/audio/MultiEngine/Wavetable2Engine.h"
#include "plugins/audio/MultiEngine/WavetableEngine.h"
#endif

// Drum
#include "plugins/audio/MultiDrumEngine/BassEngine.h"
#include "plugins/audio/MultiDrumEngine/ClapEngine.h"
#ifndef SKIP_SNDFILE
#include "plugins/audio/MultiDrumEngine/Er1PcmEngine.h"
#endif
#include "plugins/audio/MultiDrumEngine/FmEngine.h"
#include "plugins/audio/MultiDrumEngine/KickEngine.h"
#include "plugins/audio/MultiDrumEngine/MetalicDrumEngine.h"
#include "plugins/audio/MultiDrumEngine/PercussionEngine.h"
#include "plugins/audio/MultiDrumEngine/StringEngine.h"
#include "plugins/audio/MultiDrumEngine/VolcEngine.h"

/*md
## SynthMultiEngine

Synth engine to generate multiple kind of sounds, from drums, to sample, to synth.
*/

class SynthMultiEngine : public Mapping {
protected:
    // Drum
    MetalicDrumEngine metalDrumEngine;
    PercussionEngine percDrumEngine;
    DrumBassEngine bassDrumEngine;
    ClapEngine clapDrumEngine;
    KickEngine kickDrumEngine;
#ifndef SKIP_SNDFILE
    Er1PcmEngine er1DrumEngine;
#endif
    VolcEngine volcanDrumEngine;
    FmDrumEngine fmDrumEngine;
    StringDrumEngine stringDrumEngine;

    // Synth
    FmEngine fmEngine;
    AdditiveEngine additiveEngine;
    Additive2Engine additive2Engine;
    SuperSawEngine superSawEngine;
    SpaceShipEngine spaceShipEngine;
#ifndef SKIP_SNDFILE
    WavetableEngine wavetableEngine;
    Wavetable2Engine wavetable2Engine;
#endif
    BassEngine bassEngine;
    StringEngine stringEngine;

    static const int VALUE_COUNT = 12;
#ifndef SKIP_SNDFILE
    static const int DRUMS_ENGINES_COUNT = 9;
    static const int SYNTH_ENGINES_COUNT = 9;
#else
    static const int DRUMS_ENGINES_COUNT = 8;
    static const int SYNTH_ENGINES_COUNT = 7;
#endif
    static const int ENGINES_COUNT = DRUMS_ENGINES_COUNT + SYNTH_ENGINES_COUNT;
    MultiEngine* engines[ENGINES_COUNT] = {
        // Drum
        &metalDrumEngine,
        &percDrumEngine,
        &bassDrumEngine,
        &clapDrumEngine,
        &kickDrumEngine,
#ifndef SKIP_SNDFILE
        &er1DrumEngine,
#endif
        &volcanDrumEngine,
        &fmDrumEngine,
        &stringDrumEngine,

        // Synth
        &fmEngine,
#ifndef SKIP_SNDFILE
        &wavetableEngine,
        &wavetable2Engine,
#endif
        &additiveEngine,
        &additive2Engine,
        &superSawEngine,
        &spaceShipEngine,
        &bassEngine,
        &stringEngine,
    };
    MultiEngine* selectedEngine = engines[0];

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
        for (int i = 0; i < VALUE_COUNT && i < selectedEngine->mapping.size(); i++) {
            ValueInterface* val = selectedEngine->mapping[i];
            values[i].val->copy(val);
            values[i].key = val->key();
        }
    }

    SetValFn setVal = [&](std::string key, float value) {
        for (int i = 0; i < VALUE_COUNT && i < selectedEngine->mapping.size(); i++) {
            if (values[i].key == key) {
                values[i].val->set(value);
                return;
            }
        }
        // logDebug("in nulti setVal not found %s", key.c_str());
        // for (int i = 0; i < VALUE_COUNT; i++) {
        //     logDebug("%s", values[i].key.c_str());
        // }
    };

public:
    /*md **Values**: */

    /*md - `ENGINE` select the drum engine. */
    Val& engine = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = SynthMultiEngine::ENGINES_COUNT - 1, .unit = "Drum", .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        int index = (int)p.val.get();
        selectedEngine = engines[index];
        p.val.setString(selectedEngine->name);
        selectedEngine->initValues();

        p.val.props().unit = p.val.get() < DRUMS_ENGINES_COUNT ? "Drum" : "Synth";

        // loop through values and update their type
        copyValues();
    });

    struct ValueMap {
        std::string key;
        Val* val;
    } values[VALUE_COUNT] = {
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

    SynthMultiEngine(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        // Drum
        , metalDrumEngine(props, config)
        , percDrumEngine(props, config)
        , bassDrumEngine(props, config)
        , clapDrumEngine(props, config)
        , kickDrumEngine(props, config)
#ifndef SKIP_SNDFILE
        , er1DrumEngine(props, config)
#endif
        , volcanDrumEngine(props, config)
        , fmDrumEngine(props, config)
        , stringDrumEngine(props, config)
        // Synth
        , fmEngine(props, config)
        , additiveEngine(props, config)
        , additive2Engine(props, config)
        , superSawEngine(props, config)
        , spaceShipEngine(props, config)
#ifndef SKIP_SNDFILE
        , wavetableEngine(props, config)
        , wavetable2Engine(props, config)
#endif
        , bassEngine(props, config)
        , stringEngine(props, config)
    {
        initValues({ &engine });

        for (int i = 0; i < VALUE_COUNT; i++) {
            engines[i]->setValFn = setVal;
        }
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
        json["engine"] = selectedEngine->name;
        json["engineType"] = engine.props().unit;
        selectedEngine->serializeJson(json);
    }

    void hydrateJson(nlohmann::json& json) override
    {
        if (json.contains("engine") && json.contains("engineType")) {
            std::string engineName = json["engine"];
            std::string engineType = json["engineType"];
            int indexStart = engineType == "Drum" ? 0 : DRUMS_ENGINES_COUNT;
            for (int i = 0; i < ENGINES_COUNT; i++) {
                if (engines[i]->name == engineName && i >= indexStart) {
                    selectedEngine = engines[i];
                    // Set the value in JSON, so it doesn't get loaded with a different ID.
                    if (json.contains("values")) {
                        auto& values = json["values"];
                        for (int ii = 0; ii < values.size(); ii++) {
                            std::string key = values[ii]["key"];
                            if (key == engine.key()) {
                                values[ii]["value"] = i;
                            }
                        }
                    }
                    break;
                }
            }
        }
        Mapping::hydrateJson(json);
        selectedEngine->hydrateJson(json);

        // After hydration copy back value in case something changed
        copyValues();
    }
};
