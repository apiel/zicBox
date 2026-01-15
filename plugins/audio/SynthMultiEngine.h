/** Description:
This file defines the `SynthMultiEngine`, a core component designed to be a highly versatile sound generator within an audio application.

The fundamental idea is to combine many different specialized sound creation methods (known as "engines") into a single, unified module. This allows a user to access a vast library of sound types without having to load multiple separate components.

The engines are categorized into two main groups:

1.  **Synth Engines:** These modules are designed for melodic sounds, pads, and complex textures, including methods like Frequency Modulation (FM), Additive synthesis, Supersaw leads, and dedicated bass generation.
2.  **Drum Engines:** These focus on percussive elements, featuring specialized engines for kicks, claps, metallic sounds, and various forms of synthetic drums.

**How It Works:**

The `SynthMultiEngine` acts as a smart wrapper. The user interacts primarily with two types of controls:

1.  **ENGINE Selector:** This control allows instant switching between any of the internal sound modules (both synth and drum).
2.  **Generic Values (VAL\_1 to VAL\_12):** When a user selects a new engine, these generic controls automatically map themselves to the specific, complex parameters of the newly selected sound machine. This ensures the user interface remains consistent while the underlying sound generation changes drastically.

Additionally, the component is designed to be robust, handling basic sound output, note messages (on/off), and includes logic to save and restore the state of both the selected engine and all its parameters when a project is saved or loaded. Certain advanced functions, like those involving audio file loading (such as Wavetable synthesis), are conditionally included based on the system's capabilities.

sha: bc9a9247921a8db2cec18e43d82ce4598ba18d41834a0c2eb78112ec76b762cd
*/
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
#include "plugins/audio/MultiEngine/PhaseDistEngine.h"
#include "plugins/audio/MultiEngine/TrashFMEngine.h"
#include "plugins/audio/MultiEngine/ScrapYardEngine.h"
#include "plugins/audio/MultiEngine/SwarmEngine.h"
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
#include "plugins/audio/MultiDrumEngine/Kick2Engine.h"
#include "plugins/audio/MultiDrumEngine/KickFmEngine.h"
#include "plugins/audio/MultiDrumEngine/KickWaveEngine.h"
#include "plugins/audio/MultiDrumEngine/KickSegmentEngine.h"
#include "plugins/audio/MultiDrumEngine/SnareEngine.h"
#include "plugins/audio/MultiDrumEngine/FreakHatEngine.h"
#include "plugins/audio/MultiDrumEngine/RimshotEngine.h"
#include "plugins/audio/MultiDrumEngine/MetalicDrumEngine.h"
#include "plugins/audio/MultiDrumEngine/PercussionEngine.h"
#include "plugins/audio/MultiDrumEngine/StringEngine.h"
#include "plugins/audio/MultiDrumEngine/VolcEngine.h"
#include "plugins/audio/MultiDrumEngine/CowbellEngine.h"

// Sample
#ifndef SKIP_SNDFILE
#include "audio/fileBrowser.h"
#include "audio/utils/applySampleGain.h"
#include "audio/utils/getStepMultiplier.h"
#include "host/constants.h"
#include "plugins/audio/MultiSampleEngine/AmEngine.h"
#include "plugins/audio/MultiSampleEngine/Grain2Engine.h"
#include "plugins/audio/MultiSampleEngine/MonoEngine.h"
#include "plugins/audio/MultiSampleEngine/StretchEngine.h"
#endif

/*md
## SynthMultiEngine

Synth engine to generate multiple kind of sounds, from drums, to sample, to synth.
*/

class SynthMultiEngine : public Mapping {
protected:
#ifndef SKIP_SNDFILE
    FileBrowser fileBrowser = FileBrowser(AUDIO_FOLDER + "/samples");

    // Hardcoded to 48000, no matter the sample rate
    static const uint64_t bufferSize = 48000 * 30; // 30sec at 48000Hz, 32sec at 44100Hz...
    float sampleData[bufferSize];
    SampleEngine::SampleBuffer sampleBuffer;
    float index = 0.0f;
    float stepMultiplier = 1.0;

    // must be defined before engines, else when passing browser to engines, it will be defined as nullptr
    Val browser = val(1.0f, "BROWSER", { "Browser", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count }, [&](auto p) {
        p.val.setFloat(p.value);
        int position = p.val.get();
        // p.val.setString(fileBrowser.getFile(position));
        if (position != fileBrowser.position) {
            p.val.setString(fileBrowser.getFile(position));
            // logDebug(">>>>>>>>> browser str %s", p.val.string().c_str());
            std::string filepath = fileBrowser.getFilePath(position);
            logTrace("SAMPLE_SELECTOR: %f %s", p.value, filepath.c_str());
            // open(filepath);

            SF_INFO sfinfo;
            SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sfinfo);
            if (!file) {
                logWarn("Could not open file %s [%s]\n", filepath.c_str(), sf_strerror(file));
                return;
            }
            // logTrace("Audio file %s sampleCount %ld sampleRate %d channel %d\n", filepath.c_str(), (long)sfinfo.frames, sfinfo.samplerate, sfinfo.channels);
            logDebug("Audio file %s sampleCount %ld sampleRate %d channel %d", filepath.c_str(), (long)sfinfo.frames, sfinfo.samplerate, sfinfo.channels);

            sampleBuffer.count = sf_read_float(file, sampleData, bufferSize);
            sampleBuffer.data = sampleData;

            sf_close(file);

            stepMultiplier = getStepMultiplierMonoTrack(sfinfo.channels, props.channels);

            index = sampleBuffer.count;

            applySampleGain(sampleBuffer.data, sampleBuffer.count);
            if (selectedEngine) selectedEngine->opened();

            initValues({ &browser });
        } else {
            // Need to be set like this, even if code is repeated in the other if statement
            p.val.setString(fileBrowser.getFile(position));
        }
    });
#endif

    // Drum
    MetalicDrumEngine metalDrumEngine;
    PercussionEngine percDrumEngine;
    DrumBassEngine bassDrumEngine;
    ClapEngine clapDrumEngine;
    KickEngine kickDrumEngine;
    Kick2Engine kick2DrumEngine;
    KickFmEngine kickFmDrumEngine;
    KickWaveEngine kickWaveDrumEngine;
    KickSegmentEngine kickSegmentEngine;
    SnareEngine snareDrumEngine;
    FreakHatEngine freakHatDrumEngine;
    VolcEngine volcanDrumEngine;
    FmDrumEngine fmDrumEngine;
    StringDrumEngine stringDrumEngine;
    RimshotEngine rimshotDrumEngine;
    CowbellEngine cowbellDrumEngine;
#ifndef SKIP_SNDFILE
    Er1PcmEngine er1DrumEngine;
#endif

    // Synth
    FmEngine fmEngine;
    AdditiveEngine additiveEngine;
    Additive2Engine additive2Engine;
    SuperSawEngine superSawEngine;
    SpaceShipEngine spaceShipEngine;
    BassEngine bassEngine;
    StringEngine stringEngine;
    PhaseDistEngine phaseDistEngine;
    TrashFMEngine trashFmEngine;
    ScrapYardEngine scrapYardEngine;
    SwarmEngine swarmEngine;
#ifndef SKIP_SNDFILE
    WavetableEngine wavetableEngine;
    Wavetable2Engine wavetable2Engine;

    // Sample
    AmEngine amEngine;
    MonoEngine monoEngine;
    Grain2Engine grain2Engine;
    StretchEngine stretchEngine;
#endif

    static const int VALUE_COUNT = 12;
#ifndef SKIP_SNDFILE
    static const int DRUMS_ENGINES_COUNT = 17;
    static const int SYNTH_ENGINES_COUNT = 13;
    static const int SAMPLE_ENGINES_COUNT = 4;
#else
    static const int DRUMS_ENGINES_COUNT = 16; // -1
    static const int SYNTH_ENGINES_COUNT = 11; // -2
    static const int SAMPLE_ENGINES_COUNT = 0;
#endif
    static const int ENGINES_COUNT = DRUMS_ENGINES_COUNT + SYNTH_ENGINES_COUNT + SAMPLE_ENGINES_COUNT;
    MultiEngine* engines[ENGINES_COUNT] = {
        // Drum
        &metalDrumEngine,
        &percDrumEngine,
        &bassDrumEngine,
        &clapDrumEngine,
        &kickDrumEngine,
        &kick2DrumEngine,
        &kickFmDrumEngine,
        &kickWaveDrumEngine,
        &kickSegmentEngine,
        &snareDrumEngine,
        &freakHatDrumEngine,
        &volcanDrumEngine,
        &fmDrumEngine,
        &stringDrumEngine,
        &rimshotDrumEngine,
        &cowbellDrumEngine,
#ifndef SKIP_SNDFILE
        &er1DrumEngine,
#endif

        // Synth
        &fmEngine,
        &additiveEngine,
        &additive2Engine,
        &superSawEngine,
        &spaceShipEngine,
        &bassEngine,
        &stringEngine,
        &phaseDistEngine,
        &trashFmEngine,
        &scrapYardEngine,
        &swarmEngine,
#ifndef SKIP_SNDFILE
        &wavetableEngine,
        &wavetable2Engine,

        // Sample
        &monoEngine,
        &grain2Engine,
        &stretchEngine,
        &amEngine,
#endif
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
        p.val.copy(engineVal);

        if (selectedEngine->needCopyValues) {
            // logDebug("need to copy values");
            copyValues();
            selectedEngine->needCopyValues = false;
        }
    }

    bool alreadyCopyingValues = false;
    void copyValues()
    {
        if (alreadyCopyingValues) return;

        alreadyCopyingValues = true;
        for (int i = 0; i < VALUE_COUNT && i < selectedEngine->mapping.size(); i++) {
            ValueInterface* val = selectedEngine->mapping[i];
            values[i].val->copy(val);
            //     logDebug("copy %s to %s", val->key().c_str(), values[i].val->key().c_str());
            //     logDebug("---> copy str %s to %s", val->string().c_str(), values[i].val->string().c_str());
            values[i].key = val->key();
            values[i].val->set(val->get());
        }
        alreadyCopyingValues = false;
    }

    SetValFn setVal = [&](std::string key, float value) {
        for (int i = 0; i < VALUE_COUNT && i < selectedEngine->mapping.size(); i++) {
            if (values[i].key == key) {
                values[i].val->set(value);
                return;
            }
        }
    };

    float engineTypeId = 0.0f;

public:
    /*md **Values**: */

    /*md - `ENGINE` select the drum engine. */
    Val& engine = val(0, "ENGINE", { .label = "Engine", .type = VALUE_STRING, .min = 0, .max = SynthMultiEngine::ENGINES_COUNT - 1, .unit = "Drum", .incType = INC_ONE_BY_ONE }, [&](auto p) {
        p.val.setFloat(p.value);
        int index = (int)p.val.get();
        selectedEngine = engines[index];
        p.val.setString(selectedEngine->name);

        if (p.val.get() < DRUMS_ENGINES_COUNT) {
            p.val.props().unit = "Drum";
            engineTypeId = 1.0f;
        } else if (p.val.get() < DRUMS_ENGINES_COUNT + SYNTH_ENGINES_COUNT) {
            p.val.props().unit = "Synth";
            engineTypeId = 2.0f;
        } else {
            p.val.props().unit = "Sample";
            engineTypeId = 3.0f;
        }

        selectedEngine->initValues({ &browser });

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
        , kick2DrumEngine(props, config)
        , kickFmDrumEngine(props, config)
        , kickWaveDrumEngine(props, config)
        , kickSegmentEngine(props, config)
        , snareDrumEngine(props, config)
        , freakHatDrumEngine(props, config)
        , volcanDrumEngine(props, config)
        , fmDrumEngine(props, config)
        , stringDrumEngine(props, config)
        , rimshotDrumEngine(props, config)
        , cowbellDrumEngine(props, config)
#ifndef SKIP_SNDFILE
        , er1DrumEngine(props, config)
#endif
        // Synth
        , fmEngine(props, config)
        , additiveEngine(props, config)
        , additive2Engine(props, config)
        , superSawEngine(props, config)
        , spaceShipEngine(props, config)
        , bassEngine(props, config)
        , stringEngine(props, config)
        , phaseDistEngine(props, config)
        , trashFmEngine(props, config)
        , scrapYardEngine(props, config)
        , swarmEngine(props, config)
#ifndef SKIP_SNDFILE
        , wavetableEngine(props, config)
        , wavetable2Engine(props, config)
        , monoEngine(props, config, sampleBuffer, index, stepMultiplier, &browser)
        , grain2Engine(props, config, sampleBuffer, index, stepMultiplier, &browser)
        , amEngine(props, config, sampleBuffer, index, stepMultiplier, &browser)
        , stretchEngine(props, config, sampleBuffer, index, stepMultiplier, &browser)
#endif
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
        json["sampleFile"] = fileBrowser.getFile(browser.get());
        selectedEngine->serializeJson(json);
    }

    void hydrateJson(nlohmann::json& json) override
    {
        if (json.contains("sampleFile")) {
            int position = fileBrowser.find(json["sampleFile"]);
            if (position != 0) {
                browser.set(position);
            }
            // } else {
            //     browser.set(1);
        }
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

    DataFn dataFunctions[3] = {
        { "SAMPLE_BUFFER", [this](void* userdata) {
             return &sampleBuffer;
         } },
        { "SAMPLE_INDEX", [this](void* userdata) {
             return &index;
         } },
        { "GET_ENGINE_TYPE_ID", [this](void* userdata) {
             return &engineTypeId;
         } },
    };
    DEFINE_GETDATAID_AND_DATA
};
