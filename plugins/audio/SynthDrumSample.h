#ifndef _SYNTH_DRUM_SAMPLE_H_
#define _SYNTH_DRUM_SAMPLE_H_

#include <math.h>
#include <sndfile.h>
#include <time.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "mapping.h"

#include "helpers/random.h"
#include "log.h"
#include "utils/ValSerializeSndFile.h"
#include "utils/utils.h"

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
    float stepMultiplier = 1.0;

    float velocity = 1.0;

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
        // printf("[%d] drum sample noteOn: %d %f\n", track, note, _velocity);
        // logDebug("drum sample noteOn: %d %f", note, velocity);
        index = indexStart;
        stepIncrement = getSampleStep(note);
        velocity = _velocity;
    }

    void open(std::string filename)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
        if (!file) {
            logDebug("Error: could not open file %s [%s]\n", filename.c_str(), sf_strerror(file));
            return;
        }
        // logDebug("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);
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
            // logDebug("SAMPLE_SELECTOR: %f %s\n", value, filepath);
            open(filepath);
        }
    }
};

#endif