#pragma once

// Synth
#include "plugins/audio/MultiEngine/Additive2Engine.h"
#include "plugins/audio/MultiEngine/AdditiveEngine.h"
#include "plugins/audio/MultiEngine/BassEngine.h"
#include "plugins/audio/MultiEngine/FmEngine.h"
#include "plugins/audio/MultiEngine/SpaceShipEngine.h"
#include "plugins/audio/MultiEngine/StringEngine.h"
#include "plugins/audio/MultiEngine/SuperSawEngine.h"
#include "plugins/audio/MultiEngine/Wavetable2Engine.h"
#include "plugins/audio/MultiEngine/WavetableEngine.h"

// Drum
#include "plugins/audio/MultiDrumEngine/BassEngine.h"
#include "plugins/audio/MultiDrumEngine/ClapEngine.h"
#include "plugins/audio/MultiDrumEngine/Er1PcmEngine.h"
#include "plugins/audio/MultiDrumEngine/FmEngine.h"
#include "plugins/audio/MultiDrumEngine/KickEngine.h"
#include "plugins/audio/MultiDrumEngine/MetalicDrumEngine.h"
#include "plugins/audio/MultiDrumEngine/PercussionEngine.h"
#include "plugins/audio/MultiDrumEngine/StringEngine.h"
#include "plugins/audio/MultiDrumEngine/VolcEngine.h"

#include "plugins/audio/utils/AsrEnvelop.h"
#include "plugins/audio/utils/EnvelopDrumAmp.h"

/*md
## SynthMultiEngine

Synth engine to generate multiple kind of sounds, from drums, to sample, to synth.
*/

class SynthMultiEngine : public Mapping {
protected:
    float attackStep = 0.0f;
    float releaseStep = 0.0f;
    AsrEnvelop envelopAmp = AsrEnvelop(&attackStep, &releaseStep);

    // Drum
    MetalicDrumEngine metalDrumEngine;
    PercussionEngine percDrumEngine;
    DrumBassEngine bassDrumEngine;
    ClapEngine clapDrumEngine;
    KickEngine kickDrumEngine;
    Er1PcmEngine er1DrumEngine;
    VolcEngine volcanDrumEngine;
    FmDrumEngine fmDrumEngine;
    StringDrumEngine stringDrumEngine;

    // Synth
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
        for (int i = 0; i < 10 && i < selectedEngine->mapping.size(); i++) {
            ValueInterface* val = selectedEngine->mapping[i];
            values[i]->copy(val);
        }
    }

public:
    /*md **Values**: */

    /*md - `ENGINE` select the drum engine. */
    Val& engine = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = SynthMultiEngine::ENGINES_COUNT - 1, .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        int index = (int)p.val.get();
        selectedEngine = engines[index];
        p.val.setString(selectedEngine->name);
        selectedEngine->initValues();

        // loop through values and update their type
        copyValues();
    });

    Val* values[10] = {
        &val(0.0f, "VAL_3", {}, [&](auto p) { setEngineVal(p, 0); }),
        &val(0.0f, "VAL_4", {}, [&](auto p) { setEngineVal(p, 1); }),
        &val(0.0f, "VAL_5", {}, [&](auto p) { setEngineVal(p, 2); }),
        &val(0.0f, "VAL_6", {}, [&](auto p) { setEngineVal(p, 3); }),
        &val(0.0f, "VAL_7", {}, [&](auto p) { setEngineVal(p, 4); }),
        &val(0.0f, "VAL_8", {}, [&](auto p) { setEngineVal(p, 5); }),
        &val(0.0f, "VAL_9", {}, [&](auto p) { setEngineVal(p, 6); }),
        &val(0.0f, "VAL_10", {}, [&](auto p) { setEngineVal(p, 7); }),
        &val(0.0f, "VAL_11", {}, [&](auto p) { setEngineVal(p, 8); }),
        &val(0.0f, "VAL_12", {}, [&](auto p) { setEngineVal(p, 9); }),
    };

    /*md - `ATTACK` controls the duration of the envelope. */
    Val& attack = val(10.0f, "ATTACK", { "Attack", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        attackStep = 1.0f / (p.val.get() * 0.001f * props.sampleRate);
    });

    /*md - `RELEASE` controls the duration of the envelope. */
    Val& release = val(100.0f, "RELEASE", { "Release", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        releaseStep = 1.0f / (p.val.get() * 0.001f * props.sampleRate);
    });

    SynthMultiEngine(AudioPlugin::Props& props, AudioPlugin::Config& config)
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
        float envAmp = envelopAmp.next();
        selectedEngine->sample(buf, envAmp);
    }

    uint8_t playingNote = 0;
    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        playingNote = note;
        envelopAmp.attack();
        selectedEngine->noteOn(note, _velocity);
    }

    void noteOff(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        if (note == playingNote) {
            envelopAmp.release();
        }
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
