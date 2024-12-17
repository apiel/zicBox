#ifndef _SYNTH_DRUM_SAMPLE_H_
#define _SYNTH_DRUM_SAMPLE_H_

#include <math.h>
#include <sndfile.h>
#include <time.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "mapping.h"

#include "../../helpers/random.h"
#include "utils/ValSerializeSndFile.h"

#ifndef MAX_SAMPLE_VOICES
#define MAX_SAMPLE_VOICES 4
#endif

#ifndef MAX_SAMPLE_DENSITY
#define MAX_SAMPLE_DENSITY 12
#endif

/*md
## SynthDrumSample

SynthDrumSample is a very basic plugin to play one shot sample.
*/
class SynthDrumSample : public Mapping {
protected:
    // Hardcoded to 48000, no matter the sample rate
    static const uint64_t bufferSize = 48000 * 30; // 30sec at 48000Hz, 32sec at 44100Hz...
    float sampleData[bufferSize];
    struct SampleBuffer {
        uint64_t count = 0;
        float* data;
    } sampleBuffer;

    FileBrowser fileBrowser = FileBrowser("./samples");
    float index = 0;
    uint64_t indexStart = 0;
    uint64_t indexEnd = 0;
    float stepIncrement = 1.0;

    float velocity = 1.0;

    void applyGain()
    {
        float max = 0.0;
        for (uint64_t i = 0; i < sampleBuffer.count; i++) {
            if (sampleBuffer.data[i] > max) {
                max = sampleBuffer.data[i];
            } else if (-sampleBuffer.data[i] > max) {
                max = -sampleBuffer.data[i];
            }
        }
        float gain = 1.0 / max;
        // printf("max sample: %f gain: %f\n", max, gain);
        for (uint64_t i = 0; i < sampleBuffer.count; i++) {
            sampleBuffer.data[i] = sampleBuffer.data[i] * gain;
        }
    }

public:
    /*md **Values**: */
    /*md - `START` set the start position of the sample */
    Val& start = val(0.0f, "START", { "Start", .unit = "%" }, [&](auto p) {
        if (p.value < end.get()) {
            p.val.setFloat(p.value);
            indexStart = p.val.pct() * sampleBuffer.count;
        }
    });
    /*md - `END` set the end position of the sample */
    Val& end = val(100.0f, "END", { "End", .unit = "%" }, [&](auto p) {
        if (p.value > start.get()) {
            p.val.setFloat(p.value);
            indexEnd = p.val.pct() * sampleBuffer.count;
        }
    });
    /*md - `BROWSER` to browse between samples to play. */
    Val& browser = val(0.0f, "BROWSER", { "Browser", VALUE_STRING, .max = (float)fileBrowser.count }, [&](auto p) { open(p.value); });

    SynthDrumSample(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        open(browser.get(), true);
        initValues();
    }

    /*md **Config**: */
    bool config(char* key, char* value) override
    {
        /*md - `SAMPLES_FOLDER` set samples folder path. */
        if (strcmp(key, "SAMPLES_FOLDER") == 0) {
            fileBrowser.openFolder(value);
            browser.props().max = fileBrowser.count;
            open(0.0, true);

            return true;
        }

        return AudioPlugin::config(key, value);
    }

    void sample(float* buf) override
    {
        float out = 0.0f;
        if (index < indexEnd) {
            out = sampleBuffer.data[(int)index] * velocity;
            index += stepIncrement;
        } else if (index != sampleBuffer.count) {
            index = sampleBuffer.count;
        }
        buf[track] = out;
    }

    void noteOn(uint8_t note, float _velocity) override
    {
        debug("drum sample noteOn: %d %f\n", note, velocity);
        index = indexStart;
        velocity = _velocity;
    }

    void open(std::string filename)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
        if (!file) {
            debug("Error: could not open file %s [%s]\n", filename.c_str(), sf_strerror(file));
            return;
        }
        // debug("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);
        // printf(".................Audio file chan %d vs prop chan %d\n", sfinfo.channels, props.channels);

        sampleBuffer.count = sf_read_float(file, sampleData, bufferSize);
        sampleBuffer.data = sampleData;

        sf_close(file);

        if (sfinfo.channels < props.channels) {
            stepIncrement = 0.5f;
        } else if (sfinfo.channels > props.channels) {
            stepIncrement = 2.0f;
        } else {
            stepIncrement = 1.0f;
        }

        index = sampleBuffer.count;
        indexEnd = end.pct() * sampleBuffer.count;
        applyGain();
    }

    void open(float value, bool force = false)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            browser.setString(fileBrowser.getFile(position));
            std::string filepath = fileBrowser.getFilePath(position);
            // debug("SAMPLE_SELECTOR: %f %s\n", value, filepath);
            open(filepath);
        }
    }
};

#endif