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
#include "plugins/audio/utils/MultiFx.h"

#define MAX_GRAINS 16

/*md
## SynthLoop.
*/
class SynthLoop : public Mapping {
protected:
    BandEq bandEq;
    MultiFx multiFx;

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

    void updateEqSampleData()
    {
        for (uint64_t i = 0; i < sampleBuffer.count; i++) {
            eqSampleData[i] = bandEq.process(sampleData[i]);
        }
    }

    struct Grain {
        bool active = false;
        float position = 0.0f;
        float step = 1.0f;
        uint64_t startTime = 0;
        uint64_t lifeSamples = 0;
        float amplitude = 1.0f;
        bool reverse = false;
    } grains[MAX_GRAINS];

    uint64_t globalSampleTime = 0;
    uint64_t lastGrainSpawn = 0;

    void spawnGrain()
    {
        for (int i = 0; i < MAX_GRAINS; ++i) {
            if (!grains[i].active) {
                grains[i].active = true;

                // Convert ms to samples (fixed 48000Hz)
                grains[i].lifeSamples = (uint64_t)((length.get() / 1000.0f) * props.sampleRate);

                // Apply random delay and direction
                float delayRand = delayRandomize.pct();
                float delayMs = densityDelay.get() * (1.0f + ((rand() / (float)RAND_MAX - 0.5f) * 2.0f * delayRand));
                grains[i].startTime = globalSampleTime + (uint64_t)((delayMs / 1000.0f) * props.sampleRate);

                // Direction handling
                int dirMode = (int)direction.get();
                if (dirMode == 2) // Random
                    grains[i].reverse = rand() % 2;
                else
                    grains[i].reverse = (dirMode == 0);

                // Random pitch
                float pitchRand = pitchRandomize.pct();
                float randSemitone = ((rand() / (float)RAND_MAX - 0.5f) * 2.0f * 12.0f * pitchRand);
                grains[i].step = pow(2, (randSemitone / 12.0f)) * (grains[i].reverse ? -1.0f : 1.0f);

                // Random start point inside region
                float regionLen = (float)(indexEnd - indexStart);
                grains[i].position = indexStart + (rand() / (float)RAND_MAX) * regionLen;

                // Amplitude normalized by density
                grains[i].amplitude = velocity / density.get();

                return;
            }
        }
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
        updateEqSampleData();
    });

    /*md - `FREQ` set the center frequency of the effect.*/
    Val& centerFreq = val(1000.0f, "FREQ", { "Center Frequency", .min = 20.0f, .max = 20000.0f, .step = 50.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value > 20.0f && p.value < 100.0f ? 50.0f : p.value);
        bandEq.setCenterFreq(p.val.get());
        updateEqSampleData();
    });

    /*md - `RANGE` set the range of the effect.*/
    Val& rangeHz = val(2000.0f, "RANGE", { "Range", .min = 50.0f, .max = 10000.0f, .step = 50.0f, .unit = "Hz" }, [&](auto p) {
        p.val.setFloat(p.value);
        bandEq.setRangeHz(p.val.get());
        updateEqSampleData();
    });

    /*md - `FX_TYPE` select the effect.*/
    Val& fxType = val(0, "FX_TYPE", { "FX type", VALUE_STRING, .max = MultiFx::FXType::FX_COUNT - 1 }, [&](auto p) {
        multiFx.setFxType(p);
    });

    /*md - `FX_AMOUNT` set the effect amount.*/
    Val& fxAmount = val(0, "FX_AMOUNT", { "FX edit", .unit = "%" });

    /*md - `GRAIN_LENGTH` set the duration of the grain.*/
    Val& length = val(100.0f, "GRAIN_LENGTH", { "Grain Length", .min = 5.0, .max = 100.0, .unit = "ms" });

    /*md - `DENSITY` set the density of the effect, meaning how many grains are played at the same time. */
    Val& density = val(1.0f, "DENSITY", { "Density", .min = 1.0, .max = MAX_GRAINS });

    /*md - `DENSITY_DELAY` set the delay between each grains. */
    Val& densityDelay = val(10.0f, "DENSITY_DELAY", { "Density Delay", .min = 1.0, .max = 1000, .unit = "ms" });

    /*md - `DELAY_RANDOMIZE` set the density delay randomize. */
    Val& delayRandomize = val(0.0f, "DELAY_RANDOMIZE", { "Delay Rand.", .unit = "%" });

    /*md - `PITCH_RANDOMIZE` set the grain pitch randomize. */
    Val& pitchRandomize = val(0.0f, "PITCH_RANDOMIZE", { "Pitch Rand.", .unit = "%" });

    /*md - `DIRECTION` set the direction of the grain. */
    Val& direction = val(1, "DIRECTION", { "Direction", VALUE_STRING, .max = 2 }, [&](auto p) {
        p.val.setFloat(p.value);
        if (p.val.get() == 0) {
            p.val.setString("Backward");
        } else if (p.val.get() == 1) {
            p.val.setString("Forward");
        } else if (p.val.get() == 2) {
            p.val.setString("Random");
        }
    });

    SynthLoop(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
        , bandEq(props.sampleRate)
        , multiFx(props.sampleRate, props.lookupTable)
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

    // void sample(float* buf) override
    // {
    //     if (!isPlaying) {
    //         return;
    //     }

    //     float out = 0.0f;
    //     if (index >= indexEnd) {
    //         index = indexStart;
    //     }
    //     out = eqSampleData[(int)index] * velocity;
    //     index += stepIncrement;

    //     out = multiFx.apply(out, fxAmount.pct());
    //     buf[track] = out;
    // }

    void sample(float* buf) override
    {
        if (!isPlaying) {
            return;
        }

        float output = 0.0f;
        globalSampleTime++;

        // Grain spawning based on density and delay
        uint64_t delaySamples = (uint64_t)((densityDelay.get() / 1000.0f) * props.sampleRate);
        if (globalSampleTime - lastGrainSpawn >= delaySamples) {
            int grainsToSpawn = (int)density.get();
            for (int g = 0; g < grainsToSpawn; ++g)
                spawnGrain();
            lastGrainSpawn = globalSampleTime;
        }

        // Mix all active grains
        for (int i = 0; i < MAX_GRAINS; ++i) {
            Grain& gr = grains[i];
            if (!gr.active)
                continue;

            if (globalSampleTime >= gr.startTime) {
                uint64_t age = globalSampleTime - gr.startTime;

                if (age >= gr.lifeSamples) {
                    gr.active = false;
                    continue;
                }

                // Linear envelope
                float env = 1.0f - (age / (float)gr.lifeSamples);

                // Clamp sample index
                int sampleIndex = (int)gr.position;
                if (sampleIndex < 0 || sampleIndex >= (int)sampleBuffer.count) {
                    gr.active = false;
                    continue;
                }

                // Read and advance
                float sampleValue = eqSampleData[sampleIndex] * env * gr.amplitude;
                output += sampleValue;
                gr.position += gr.step;

                // Loop within region
                if (gr.position < (float)indexStart || gr.position >= (float)indexEnd) {
                    gr.active = false;
                }
            }
        }

        // Apply FX chain
        output = multiFx.apply(output, fxAmount.pct());
        buf[track] = output;
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
