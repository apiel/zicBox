#pragma once

#include "plugins/audio/MultiEngine/FmEngine.h"
#include "plugins/audio/utils/AsrEnvelop.h"
#include "plugins/audio/utils/EnvelopDrumAmp.h"

/*md
## SynthMulti

Synth engine to generate multiple kind of sounds.
*/

class SynthMulti : public Mapping {
protected:
    float attackStep = 0.0f;
    float releaseStep = 0.0f;
    AsrEnvelop envelopAmp = AsrEnvelop(&attackStep, &releaseStep);

    FmEngine fmEngine;

    static const int ENGINES_COUNT = 1;
    Engine* engines[ENGINES_COUNT] = {
        &fmEngine,
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
    Val& engine = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = ENGINES_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        int index = (int)p.val.get();
        selectedEngine = engines[index];
        p.val.setString(selectedEngine->name);

        // loop through values and update their type
        copyValues();
    });

    Val* values[10] = {
        &val(0.0f, "VAL_1", {}, [&](auto p) { setEngineVal(p, 0); }),
        &val(0.0f, "VAL_2", {}, [&](auto p) { setEngineVal(p, 1); }),
        &val(0.0f, "VAL_3", {}, [&](auto p) { setEngineVal(p, 2); }),
        &val(0.0f, "VAL_4", {}, [&](auto p) { setEngineVal(p, 3); }),
        &val(0.0f, "VAL_5", {}, [&](auto p) { setEngineVal(p, 4); }),
        &val(0.0f, "VAL_6", {}, [&](auto p) { setEngineVal(p, 5); }),
        &val(0.0f, "VAL_7", {}, [&](auto p) { setEngineVal(p, 6); }),
        &val(0.0f, "VAL_8", {}, [&](auto p) { setEngineVal(p, 7); }),
        &val(0.0f, "VAL_9", {}, [&](auto p) { setEngineVal(p, 8); }),
        &val(0.0f, "VAL_10", {}, [&](auto p) { setEngineVal(p, 9); }),
    };

    /*md - `ATTACK` controls the duration of the envelope. */
    Val& attack = val(10.0f, "ATTACK", { "Attack", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        attackStep = p.val.get() * 0.001f / props.sampleRate;
    });

    /*md - `RELEASE` controls the duration of the envelope. */
    Val& release = val(100.0f, "RELEASE", { "Release", .min = 10.0, .max = 3000.0, .step = 10.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        releaseStep = p.val.get() * 0.001f / props.sampleRate;
    });

    SynthMulti(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , fmEngine(props, config)
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
