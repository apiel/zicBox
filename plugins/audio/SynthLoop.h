#pragma once

#include <math.h>
#include <sndfile.h>
#include <time.h>

#include "audioPlugin.h"
#include "mapping.h"
#include "utils/fileBrowser.h"

#include "host/constants.h"
#include "log.h"
#include "utils/ValSerializeSndFile.h"
#include "utils/utils.h"

#include "plugins/audio/utils/BandEq.h"
#include "plugins/audio/utils/Grains.h"
#include "plugins/audio/utils/MultiFx.h"

/*md
## SynthLoop.
*/

#define ENABLE_CHUNCK_FEATURE

class SynthLoop : public Mapping {
protected:
    BandEq bandEq;
    MultiFx multiFx;
    Grains grains;
    BandEq grainBandEq;

    // Hardcoded to 48000, no matter the sample rate
    static const uint64_t bufferSize = 48000 * 30; // 30sec at 48000Hz, 32sec at 44100Hz...
    float sampleData[bufferSize];
    float eqSampleData[bufferSize];
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    } sampleBuffer;

    FileBrowser fileBrowser = FileBrowser(AUDIO_FOLDER + "/loops");
    float index = 0;
    uint64_t indexStart = 0;
    uint64_t indexEnd = 0;
    float stepIncrement = 1.0;
    float stepMultiplier = 1.0;

    float velocity = 1.0;
    bool isPlaying = false;

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

    float resetValue = -10000.0f; // Use resetValue to avoid freesing CPU when changing FREQ and RANGE (kind of debouncing...)
    void resetEqSampleData()
    {
        for (uint64_t i = 0; i < sampleBuffer.count; i++) {
            eqSampleData[i] = resetValue;
        }
    }

    float getEqSample(uint64_t sampleIndex)
    {
        if (eqSampleData[sampleIndex] == resetValue) {
            eqSampleData[sampleIndex] = bandEq.process(sampleData[sampleIndex]);
        }
        return eqSampleData[sampleIndex];
    }

public:
    /*md **Values**: */
    /*md - `START` set the start position of the sample */
    Val& start = val(0.0f, "START", { "Start", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        if (p.value < end.get()) {
            p.val.setFloat(p.value);
            indexStart = p.val.pct() * sampleBuffer.count;
        }
    });
    /*md - `END` set the end position of the sample */
    Val& end = val(100.0f, "END", { "End", .step = 0.1f, .floatingPoint = 1, .unit = "%" }, [&](auto p) {
        if (p.value > start.get()) {
            p.val.setFloat(p.value);
            indexEnd = p.val.pct() * sampleBuffer.count;
        }
    });
    /*md - `BROWSER` to browse between samples to play. */
    Val& browser = val(1.0f, "BROWSER", { "Browser", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count }, [&](auto p) {
        open(p.value);
        resetEqSampleData();
    });

    /*md - `FREQ` set the center frequency of the effect.*/
    Val& centerFreq = val(1000.0f, "FREQ", { "EQ Frequency", .min = 20.0f, .max = 20000.0f, .step = 20.0f, .unit = "Hz" }, [&](auto p) {
        // p.val.setFloat(p.value > 20.0f && p.value < 100.0f ? 50.0f : p.value);
        p.val.setFloat(p.value);
        bandEq.setCenterFreq(p.val.get());
        resetEqSampleData();
    });

    /*md - `RANGE` set the range of the effect.*/
    Val& rangeHz = val(2000.0f, "RANGE", { "EQ Range", .min = 50.0f, .max = 10000.0f, .step = 50.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        bandEq.setRangeHz(p.val.get());
        resetEqSampleData();
    });

     /*md - `GRAIN_FREQ` set the center frequency for grain effect.*/
    Val& grainCenterFreq = val(1000.0f, "GRAIN_FREQ", { "Grain EQ Freq.", .min = 20.0f, .max = 20000.0f, .step = 20.0f, .unit = "Hz" }, [&](auto p) {
        // p.val.setFloat(p.value > 20.0f && p.value < 100.0f ? 50.0f : p.value);
        p.val.setFloat(p.value);
        grainBandEq.setCenterFreq(p.val.get());
    });

    /*md - `GRAIN_RANGE` set the range of the grain effect.*/
    Val& grainRangeHz = val(2000.0f, "GRAIN_RANGE", { "Grain EQ Range", .min = 50.0f, .max = 10000.0f, .step = 50.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        grainBandEq.setRangeHz(p.val.get());
    });

    /*md - `FX_TYPE` select the effect.*/
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    /*md - `GRAIN_LENGTH` set the duration of the grain.*/
    Val& length = val(100.0f, "GRAIN_LENGTH", { "Grain Length", .min = 5.0, .max = 500.0, .unit = "ms" }, [&](auto p) {
        p.val.setFloat(p.value);
        // grainDuration = props.sampleRate * length.get() * 0.001f;
        grains.setGrainDuration(props.sampleRate * length.get() * 0.001f);
    });

    /*md - `DENSITY` set the density of the effect, meaning how many grains are played at the same time. */
    Val& density = val(1.0f, "DENSITY", { "Density", .min = 1.0, .max = MAX_GRAINS }, [&](auto p) {
        p.val.setFloat(p.value);
        // densityDivider = 1.0f / p.val.get();
        // densityDivider = 1.0f / sqrtf(p.val.get()); // natural loudness
        grains.setDensity(p.val.get());
    });

    /*md - `DENSITY_DELAY` set the delay between each grains. */
    Val& densityDelay = val(10.0f, "DENSITY_DELAY", { "Density Delay", .min = 0.0, .max = 1000, .unit = "ms" }, [&](auto p) {
        densityDelay.setFloat(p.value);
        // grainDelay = props.sampleRate * densityDelay.get() * 0.001f;
        grains.setGrainDelay(props.sampleRate * p.val.get() * 0.001f);
    });

    /*md - `DELAY_RANDOMIZE` set the density delay randomize. */
    Val& delayRandomize = val(0.0f, "DELAY_RANDOMIZE", { "Delay Rand.", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setDelayRandomize(p.val.pct());
    });

    /*md - `PITCH_RANDOMIZE` set the grain pitch randomize. */
    Val& pitchRandomize = val(0.0f, "PITCH_RANDOMIZE", { "Pitch Rand.", .unit = "%" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setPitchRandomize(p.val.pct());
    });

    /*md - `DIRECTION` set the direction of the grain. */
    Val& direction = val(0.0f, "DIRECTION", { "Direction", VALUE_STRING, .max = 2 }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setDirection((Grains::DIRECTION)p.val.get());
        if (grains.direction == Grains::DIRECTION::BACKWARD) {
            p.val.setString("Backward");
        } else if (grains.direction == Grains::DIRECTION::FORWARD) {
            p.val.setString("Forward");
        } else if (grains.direction == Grains::DIRECTION::RANDOM) {
            p.val.setString("Random");
        }
    });

    /*md - `DETUNE` set the pitch spread across grains (in semitones). */
    Val& detune = val(0.0f, "DETUNE", { "Detune", VALUE_CENTERED, .min = -12.0f, .max = 12.0f, .step = 0.1f, .floatingPoint = 1, .unit = "st" }, [&](auto p) {
        p.val.setFloat(p.value);
        grains.setDetune(p.val.get());
    });

#ifdef ENABLE_CHUNCK_FEATURE
    /*md - `CHUNKS` set the number of chunks to play randomly.*/
    Val& chunkCount = val(1.0f, "CHUNKS", { "Rand.Chunks", .min = 1.0f, .max = 64.0f, .incType = INC_EXP }, [&](auto p) {
        p.val.setFloat(p.value);
        chunkSize = (indexEnd - indexStart) / p.val.get();
        currentChunk = currentChunk % (uint8_t)p.val.get();
        updateChunkBoundaries();
    });
#endif

    SynthLoop(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , bandEq(props.sampleRate)
        , grainBandEq(props.sampleRate)
        , multiFx(props.sampleRate, props.lookupTable)
        , grains(props.lookupTable, [this](uint64_t idx) -> float { return sampleData[idx]; })
    {
        open(browser.get(), true);
        initValues();

        //md **Config**:
        auto& json = config.json;

        //md - `"samplesFolder": "samples"` set samples folder path.
        if (json.contains("samplesFolder")) {
            fileBrowser.openFolder(json["samplesFolder"].get<std::string>());
            browser.props().max = fileBrowser.count;
            open(0.0, true);
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        isPlaying = playing;
        // if (event == AudioEventType::STOP) {
        // } else if (event == AudioEventType::PAUSE) {
        // } else if (event == AudioEventType::TOGGLE_PLAY_PAUSE || event == AudioEventType::START) {
        // }
    }

#ifdef ENABLE_CHUNCK_FEATURE
    uint64_t chunkSize = 0;
    uint8_t currentChunk = 0;
    uint64_t chunkStart = 0;
    uint64_t chunkEnd = 0;
    void updateChunkBoundaries()
    {
        chunkStart = indexStart + currentChunk * chunkSize;
        chunkEnd = chunkStart + chunkSize;
    }
#endif
    void sample(float* buf) override
    {
        if (!isPlaying)
            return;

        float out = 0.0f;
        out = grains.getGrainSample(stepIncrement, index, sampleBuffer.count);
        index += stepIncrement;

#ifdef ENABLE_CHUNCK_FEATURE
        if (index >= chunkEnd) {
            currentChunk = rand() % (uint8_t)chunkCount.get();
            index = indexStart + currentChunk * chunkSize;
            updateChunkBoundaries();
        }
#else
        if (index >= indexEnd) {
            index = indexStart;
        }
#endif

        out = grainBandEq.process(out);

        out = multiFx.apply(out, fxAmount.pct());
        buf[track] = out * velocity;
    }

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        // printf("[%d] drum sample noteOn: %d %f\n", track, note, _velocity);
        // logTrace("drum sample noteOn: %d %f", note, velocity);
        // index = indexStart;
        // stepIncrement = getSampleStep(note);
        // velocity = _velocity;
    }

    // void noteOff(uint8_t note, float velocity, void* userdata = NULL) override
    // {
    // }

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

    void serializeJson(nlohmann::json& json) override
    {
        Mapping::serializeJson(json);
        json["sampleFile"] = fileBrowser.getFile(browser.get());
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
