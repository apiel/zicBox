#pragma once

#include "plugins/audio/MultiSampleEngine/AmEngine.h"
#include "plugins/audio/MultiSampleEngine/GrainEngine.h"
#include "plugins/audio/MultiSampleEngine/MonoEngine.h"
#include "plugins/audio/MultiSampleEngine/StretchEngine.h"
#include "plugins/audio/utils/EnvelopDrumAmp.h"
#include "plugins/audio/utils/utils.h"
#include "utils/fileBrowser.h"

#include <sndfile.h>

/*md
## SynthMultiSample

Multiple engines to play with samples.
*/

class SynthMultiSample : public Mapping {
protected:
    MonoEngine monoEngine;
    GrainEngine grainEngine;
    AmEngine amEngine;
    StretchEngine stretchEngine;

    static const int ENGINES_COUNT = 4;
    SampleEngine* engines[ENGINES_COUNT] = {
        &monoEngine,
        &grainEngine,
        &amEngine,
        &stretchEngine,
    };
    SampleEngine* engine = engines[0];

    // Hardcoded to 48000, no matter the sample rate
    static const uint64_t bufferSize = 48000 * 30; // 30sec at 48000Hz, 32sec at 44100Hz...
    float sampleData[bufferSize];
    SampleEngine::SampleBuffer sampleBuffer;
    float index = 0.0f;
    float stepMultiplier = 1.0;

    FileBrowser fileBrowser = FileBrowser("./data/audio/samples");

    void open(std::string filename)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
        if (!file) {
            logDebug("Error: could not open file %s [%s]\n", filename.c_str(), sf_strerror(file));
            return;
        }
        logTrace("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);
        // printf(".................Audio file chan %d vs prop chan %d\n", sfinfo.channels, props.channels);

        sampleBuffer.count = sf_read_float(file, sampleData, bufferSize);
        sampleBuffer.data = sampleData;

        sf_close(file);

        if (sfinfo.channels < props.channels) {
            stepMultiplier = 0.5f;
        } else if (sfinfo.channels > props.channels) {
            stepMultiplier = 2.0f;
        } else {
            stepMultiplier = 1.0f;
        }

        index = sampleBuffer.count;

        applyGain(sampleBuffer.data, sampleBuffer.count);
        engine->opened();
    }

    static const int valCount = 11;

    void setEngineVal(Val::CallbackProps p, int index)
    {
        p.val.setFloat(p.value);
        if (index >= engine->mapping.size()) {
            return;
        }
        // logDebug("setEngineVal (%d) %s %f", index, p.val.key().c_str(), p.val.get());
        ValueInterface* drumEngineVal = engine->mapping[index];
        drumEngineVal->set(p.val.get());
        p.val.setString(drumEngineVal->string());
        p.val.props().label = drumEngineVal->props().label;
        p.val.props().unit = drumEngineVal->props().unit;
        p.val.props().min = drumEngineVal->props().min;
        p.val.props().max = drumEngineVal->props().max;
        p.val.props().floatingPoint = drumEngineVal->props().floatingPoint;

        if (engine->needCopyValues) {
            engine->needCopyValues = false;
            copyValues();
        }
    }

    void copyValues()
    {
        for (int i = 0; i < valCount && i < engine->mapping.size(); i++) {
            ValueInterface* val = engine->mapping[i];
            values[i]->copy(val);
        }
    }

public:
    /*md **Values**: */

    /*md - `ENGINE` select the drum engine. */
    Val& engineVal = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = SynthMultiSample::ENGINES_COUNT - 1, .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        int index = (int)p.val.get();
        engine = engines[index];
        p.val.setString(engine->name);
        engine->initValues();

        // loop through values and update their type
        copyValues();
    });

    Val* values[valCount] = {
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
        &val(0.0f, "VAL_11", {}, [&](auto p) { setEngineVal(p, 10); }),
    };

    /*md - `BROWSER` to browse between samples to play. */
    Val& browser = val(1.0f, "BROWSER", { "Browser", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count }, [&](auto p) {
        p.val.setFloat(p.value);
        int position = p.val.get();
        if (position != fileBrowser.position) {
            p.val.setString(fileBrowser.getFile(position));
            std::string filepath = fileBrowser.getFilePath(position);
            logTrace("SAMPLE_SELECTOR: %f %s", p.value, filepath.c_str());
            open(filepath);
            initValues({ &browser });
        }
    });

    SynthMultiSample(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , monoEngine(props, config, sampleBuffer, index, stepMultiplier)
        , grainEngine(props, config, sampleBuffer, index, stepMultiplier)
        , amEngine(props, config, sampleBuffer, index, stepMultiplier)
        , stretchEngine(props, config, sampleBuffer, index, stepMultiplier)
    {
        initValues({ &engineVal });
    }

    void sample(float* buf) override
    {
        engine->sample(buf);
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        engine->noteOn(note, _velocity);
    }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        engine->noteOff(note, velocity);
    }

    void serializeJson(nlohmann::json& json) override
    {
        Mapping::serializeJson(json);
        json["sampleFile"] = fileBrowser.getFile(browser.get());
        engine->serializeJson(json);
    }

    void hydrateJson(nlohmann::json& json) override
    {
        Mapping::hydrateJson(json);
        if (json.contains("sampleFile")) {
            int position = fileBrowser.find(json["sampleFile"]);
            if (position != 0) {
                browser.set(position);
            }
        }

        engine->hydrateJson(json);
        // After hydration copy back value in case something changed
        copyValues();
    }

    enum DATA_ID {
        SAMPLE_BUFFER,
        SAMPLE_INDEX,
    };

    /*md **Data ID**: */
    uint8_t getDataId(std::string name) override
    {
        /*md - `SAMPLE_BUFFER` return a representation of the current sample loaded in buffer */
        if (name == "SAMPLE_BUFFER")
            return DATA_ID::SAMPLE_BUFFER;
        /*md - `SAMPLE_INDEX` return the current index of the playing sample */
        if (name == "SAMPLE_INDEX")
            return DATA_ID::SAMPLE_INDEX;
        return atoi(name.c_str());
    }

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case DATA_ID::SAMPLE_BUFFER:
            return &sampleBuffer;
        case DATA_ID::SAMPLE_INDEX:
            return &index;
        }
        return NULL;
    }
};
