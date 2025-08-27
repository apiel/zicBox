#pragma once

#include "plugins/audio/MultiSampleEngine/SampleEngine.h"
#include "plugins/audio/MultiSampleEngine/MonoEngine.h"
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

    static const int ENGINES_COUNT = 1;
    SampleEngine* engines[ENGINES_COUNT] = {
        &monoEngine,
    };
    SampleEngine* engine = engines[0];

    // Hardcoded to 48000, no matter the sample rate
    static const uint64_t bufferSize = 48000 * 30; // 30sec at 48000Hz, 32sec at 44100Hz...
    float sampleData[bufferSize];
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    } sampleBuffer;

    FileBrowser fileBrowser = FileBrowser("./data/audio/samples");
    float index = 0;
    uint64_t indexStart = 0;
    uint64_t indexEnd = 0;
    uint64_t loopStart = 0;
    uint64_t loopEnd = 0;
    uint16_t loopCountRelease = 0;
    int16_t nbOfLoopBeforeRelease = 0;
    float stepIncrement = 1.0;
    float stepMultiplier = 1.0;

    float velocity = 1.0;
    uint8_t sustainedNote = 0;

    uint8_t baseNote = 60;
    float getSampleStep(uint8_t note)
    {
        // https://gist.github.com/YuxiUx/ef84328d95b10d0fcbf537de77b936cd
        // pow(2, ((0) / 12.0)) = 1 for 0 semitone
        // pow(2, ((1) / 12.0)) = 1.059463094 for 1 semitone
        // pow(2, ((2) / 12.0)) = 1.122462048 for 2 semitone
        // ...

        // printf("getSampleStep: %d >> %d = %f\n", note, note - baseNote, pow(2, (note - baseNote) / 12.0));
        return pow(2, ((note - baseNote) / 12.0)) * stepMultiplier;
    }

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
        indexEnd = end.pct() * sampleBuffer.count;
        applyGain(sampleBuffer.data, sampleBuffer.count);
    }

    void open(float value, bool force = false)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            browser.setString(fileBrowser.getFile(position));
            std::string filepath = fileBrowser.getFilePath(position);
            logTrace("SAMPLE_SELECTOR: %f %s", value, filepath.c_str());
            open(filepath);
            initValues();
        }
    }

    void setEngineVal(Val::CallbackProps p, int index)
    {
        p.val.setFloat(p.value);
        if (index >= engine->mapping.size())
            return;
        // logDebug("setEngineVal (%d) %s %f", index, p.val.key().c_str(), p.val.get());
        ValueInterface* drumEngineVal = engine->mapping[index];
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
        for (int i = 0; i < 10 && i < engine->mapping.size(); i++) {
            ValueInterface* val = engine->mapping[i];
            values[i]->copy(val);
        }
    }

public:
    /*md **Values**: */

    /*md - `ENGINE` select the drum engine. */
    Val& engineVal = val(0, "ENGINE", { "Engine", VALUE_STRING, .min = 0, .max = ENGINES_COUNT - 1 }, [&](auto p) {
        p.val.setFloat(p.value);
        int index = (int)p.val.get();
        engine = engines[index];
        p.val.setString(engine->name);

        // loop through values and update their type
        copyValues();
    });

    Val* values[6] = {
        &val(0.0f, "VAL_1", {}, [&](auto p) { setEngineVal(p, 0); }),
        &val(0.0f, "VAL_2", {}, [&](auto p) { setEngineVal(p, 1); }),
        &val(0.0f, "VAL_3", {}, [&](auto p) { setEngineVal(p, 2); }),
        &val(0.0f, "VAL_4", {}, [&](auto p) { setEngineVal(p, 3); }),
        &val(0.0f, "VAL_5", {}, [&](auto p) { setEngineVal(p, 4); }),
        &val(0.0f, "VAL_6", {}, [&](auto p) { setEngineVal(p, 5); }),
    };

    /*md **Values**: */
    /*md - `START` set the start position of the sample */
    Val& start = val(0.0f, "START", { "Start", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        if (p.value < end.get()) {
            bool sustainEq = p.val.get() == sustainPosition.get();
            p.val.setFloat(p.value);
            indexStart = p.val.pct() * sampleBuffer.count;
            logTrace("- START: %d", indexStart);
            if (p.val.get() > sustainPosition.get() || sustainEq) {
                sustainPosition.set(p.val.get());
            }
        }
    });
    /*md - `END` set the end position of the sample */
    Val& end = val(100.0f, "END", { "End", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        if (p.value > start.get()) {
            p.val.setFloat(p.value);
            indexEnd = p.val.pct() * sampleBuffer.count;
            logTrace("- END: %d", indexEnd);
            if (p.val.get() < sustainPosition.get() + sustainLength.get()) {
                sustainPosition.set(p.val.get() - sustainLength.get());
            }
        }
    });
    /*md - `BROWSER` to browse between samples to play. */
    Val& browser = val(1.0f, "BROWSER", { "Browser", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count }, [&](auto p) { open(p.value); });

    /*md - `LOOP_POSITION` set the position of the sustain loop */
    Val& sustainPosition = val(0.0f, "LOOP_POSITION", { "Loop position", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        if (p.value < start.get()) {
            p.value = start.get();
        }
        if (p.value + sustainLength.get() > end.get()) {
            return;
        }
        sustainPosition.setFloat(p.value);
        if (sustainLength.get() == 0.0f && p.val.get() > start.get()) {
            sustainLength.set(5);
        }
        loopStart = p.val.pct() * sampleBuffer.count;
        logTrace("- LOOP_POSITION: %d", loopStart);
        sustainLength.set(sustainLength.get());
    });
    /*md - `LOOP_LENGTH` set the length of the sustain loop */
    Val& sustainLength = val(0.0f, "LOOP_LENGTH", { "Loop length", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        if (p.value + sustainPosition.get() > end.get()) {
            return;
        }
        sustainLength.setFloat(p.value);
        loopEnd = p.val.pct() * sampleBuffer.count + sustainPosition.pct() * sampleBuffer.count;
        logTrace("- LOOP_LENGTH: %d", loopEnd);
        sustainRelease.set(sustainRelease.get());
        if (sustainLength.get() == 0) {
            sustainedNote = 0;
        }
    });

    bool showNumberOfLoopsInUnit = true;
    /*md - `LOOP_RELEASE` set a delay before the sustain loop ends when note off is triggered */
    Val& sustainRelease = val(0.0f, "LOOP_RELEASE", { "Loop Release", .min = 0.0, .max = 5000.0, .step = 50.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() > 0) {
            uint64_t loopLength = sustainLength.pct() * sampleBuffer.count;
            float msLoopLength = loopLength / (float)props.sampleRate * 1000;
            loopCountRelease = msLoopLength > 0 ? p.val.get() / msLoopLength : 0;
            // printf(">>>> sustainRelease: %f loopLength: %ld msLoopLength: %f loopCountRelease: %d\n", p.val.get(), loopLength, msLoopLength, loopCountRelease);
        }
        if (showNumberOfLoopsInUnit) {
            p.val.props().unit = "ms(" + std::to_string(loopCountRelease) + ")";
        }
    });

    SynthMultiSample(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , monoEngine(props, config)
    {
        initValues({ &engineVal });
    }

    void sample(float* buf) override
    {
        float out = 0.0f;
        if (sustainedNote || nbOfLoopBeforeRelease > 0) {
            // out = sampleBuffer.data[(int)index] * velocity * envFactor;
            out = sampleBuffer.data[(int)index] * velocity;
            index += stepIncrement;
            if (index >= loopEnd) {
                index = loopStart;
                nbOfLoopBeforeRelease--;
            }
        } else if (index < indexEnd) {
            // out = sampleBuffer.data[(int)index] * velocity * envFactor;
            out = sampleBuffer.data[(int)index] * velocity;
            index += stepIncrement;
        } else if (index != sampleBuffer.count) {
            index = sampleBuffer.count;
        }
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        index = indexStart;
        stepIncrement = getSampleStep(note);
        velocity = _velocity;
        if (sustainLength.get() > 0.0f) {
            sustainedNote = note;
        }
        engine->noteOn(note, _velocity);
    }

    void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    {
        if (note == sustainedNote) {
            nbOfLoopBeforeRelease = loopCountRelease;
            sustainedNote = 0;
        }
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
