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

SynthDrumSample is a very basic plugin to play one shot sample with a builtin sequencer.
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
    float stepIncrement = 1.0;

public:
    /*md **Values**: */
    /*md - `END` set the end position of the sample */
    Val& end = val(100.0f, "END", { "End", .unit = "%" });
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
        if (index < sampleBuffer.count) {
            out = sampleBuffer.data[(int)index];
            index += stepIncrement;
        }
        buf[track] = out;
    }

    void noteOn(uint8_t note, float velocity) override
    {
        debug("drum sample noteOn: %d %f\n", note, velocity);
        index = 0.0;
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