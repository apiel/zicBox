/** Description:
This code defines a comprehensive audio component named `SynthMultiDrum`, designed to be a highly versatile drum machine capable of generating many different types of percussive sounds.

**Core Functionality:**

The `SynthMultiDrum` does not generate sound itself but acts as a central hub for nine separate, specialized sound generators. These internal "engines" include dedicated modules for bass drums, claps, kicks, metallic hits, and various synthesized percussion sounds.

**How it Works:**

1.  **Engine Selection:** The user controls a main parameter called `ENGINE`. This knob acts as a selector switch, instantly choosing which of the nine specialized sound generators is currently active.
2.  **Dynamic Controls:** To simplify the user interface, the component provides 12 generic controls (labeled VAL\_1 through VAL\_12). When the user switches the active sound engine (e.g., from the Kick Engine to the Clap Engine), these 12 controls immediately adapt. They dynamically change their function, label, and adjustable range to reflect the specific parameters needed for the newly selected engine.
3.  **Sound Generation:** When a trigger signal (like a MIDI note) is received, the component simply passes that command directly to the currently selected engine, which then generates the appropriate sound signal.

In essence, `SynthMultiDrum` is a smart switchboard that provides a single, unified control interface for a complete suite of specialized drum synthesis tools. It also handles saving and loading the settings for the currently active sound generator.

sha: 183db38d9bc8c270456d82421014fab85502b00a04b2c44f44e23365758ec1e4 
*/
#pragma once

#include "plugins/audio/MultiDrumEngine/BassEngine.h"
#include "plugins/audio/MultiDrumEngine/ClapEngine.h"
#include "plugins/audio/MultiDrumEngine/Er1PcmEngine.h"
#include "plugins/audio/MultiDrumEngine/FmEngine.h"
#include "plugins/audio/MultiDrumEngine/KickEngine.h"
#include "plugins/audio/MultiDrumEngine/MetalicDrumEngine.h"
#include "plugins/audio/MultiDrumEngine/PercussionEngine.h"
#include "plugins/audio/MultiDrumEngine/StringEngine.h"
#include "plugins/audio/MultiDrumEngine/VolcEngine.h"

/*md
## SynthMultiDrum

Synth engine to generate multiple kind of drum sounds.
*/

class SynthMultiDrum : public Mapping {
protected:
    MetalicDrumEngine metalDrumEngine;
    PercussionEngine percEngine;
    DrumBassEngine bassEngine;
    ClapEngine clapEngine;
    KickEngine kickEngine;
    Er1PcmEngine er1PcmEngine;
    VolcEngine volcanEngine;
    FmDrumEngine fmEngine;
    StringDrumEngine stringEngine;

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
        for (int i = 0; i < 12 && i < drumEngine->mapping.size(); i++) {
            ValueInterface* val = drumEngine->mapping[i];
            values[i].val->copy(val);
            values[i].key = val->key();
        }
    }

    SetValFn setVal = [&](std::string key, float value) {
        for (int i = 0; i < 12 && i < drumEngine->mapping.size(); i++) {
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

    void sample(float* buf) override
    {
        drumEngine->sample(buf);
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
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
