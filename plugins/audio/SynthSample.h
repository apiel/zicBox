#ifndef _SYNTH_SAMPLE_H_
#define _SYNTH_SAMPLE_H_

#include <math.h>
#include <sndfile.h>
#include <time.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "mapping.h"

#include "utils/ValSerializeSndFile.h"

// Sequencer can play different wave files on each step
//          Use a common cache to be able to load patch changes
//          There would be 4 instance SynthSample for the sequencer
//          + 1 instance SynthSample for patch editing
//          each instance would have 4 voices
//              voice load there own buffer of wave file and keep patch settings
//              to allowed to keep playing sample till the end of the buffer even if change patch
//              do we even want this???
// There is 2 modulations...

class SynthSample : public Mapping {
protected:
    // We hardcode it to 48000, no matter the sample rate
    static const uint64_t bufferSize = 48000 * 30; // 30sec at 48000Hz, 32sec at 44100Hz...
    float sampleData[bufferSize];
    struct SampleBuffer {
        uint64_t count = 0;
        float *data;
    } sampleBuffer;

    FileBrowser fileBrowser = FileBrowser("./samples");

    // Use to restore sustain in case it was move by another parameter
    float sustainPositionOrigin = 0.0f;
    float sustainLengthOrigin = -1.0f;
    bool skipOrigin = false; // Used as point to skip set origin

public:
    Val& start = val(0.0f, "START", { "Start", .unit = "%" }, [&](auto p) { setStart(p.value); });
    Val& end = val(100.0f, "END", { "End", .unit = "%" }, [&](auto p) { setEnd(p.value); });
    Val& sustainPosition = val(0.0f, "SUSTAIN_POSITION", { "Sustain position", .unit = "%" }, [&](auto p) { setSustainPosition(p.value, (bool*)p.data); });
    // Where -1 is no sustain
    Val& sustainLength = val(0.0f, "SUSTAIN_LENGTH", { "Sustain length", .unit = "%" }, [&](auto p) { setSustainLength(p.value, (bool*)p.data); });

    Val& browser = val(0.0f, "BROWSER", { "Browser", VALUE_STRING, .max = (float)fileBrowser.count }, [&](auto p) { open(p.value); });

    SynthSample(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        open(browser.get(), true);
        initValues();
    }

    bool config(char* key, char* value) override
    {
        if (strcmp(key, "SAMPLES_FOLDER") == 0) {
            fileBrowser.openFolder(value);
            browser.props().max = fileBrowser.count;
            open(0.0, true);

            return true;
        }

        // if (strcmp(key, "DATA_STATE") == 0) {
        //     char* pluginName = strtok(value, " ");
        //     dataId = atoi(strtok(NULL, " "));
        //     plugin = &getPlugin(pluginName);
        //     return true;
        // }

        return AudioPlugin::config(key, value);
    }

    void sample(float* buf)
    {
    }

    void open(const char* filename)
    {
        SF_INFO sfinfo;
        SNDFILE* file = sf_open(filename, SFM_READ, &sfinfo);
        if (!file) {
            debug("Error: could not open file %s [%s]\n", filename, sf_strerror(file));
            return;
        }
        // debug("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);

        sampleBuffer.count = sf_read_float(file, sampleData, bufferSize);
        sampleBuffer.data = sampleData;

        sf_close(file);

        // FIXME
        // ValSerializeSndFile serialize(mapping);
        // serialize.loadSetting(filename);
    }

    void open(float value, bool force = false)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            char* filepath = fileBrowser.getFilePath(position);
            browser.setString(fileBrowser.getFile(position));
            // debug("SAMPLE_SELECTOR: %f %s\n", value, filepath);
            open(filepath);
        }
    }

    void setStart(float value)
    {
        if (value >= end.get()) {
            return;
        }
        start.setFloat(value);
        setValueBoundaries();
    }

    void setEnd(float value)
    {
        if (value <= start.get()) {
            return;
        }
        end.setFloat(value);
        setValueBoundaries();
    }

    void setSustainPosition(float value, bool* setOrigin)
    {
        if (value < start.get() || value + sustainLength.get() > end.get()) {
            return;
        }
        sustainPosition.setFloat(value);
        if (setOrigin == NULL || *setOrigin) {
            sustainPositionOrigin = sustainPosition.get();
            setValueBoundaries();
        }
    }

    void setSustainLength(float value, bool* setOrigin)
    {
        if (value + sustainPosition.get() > end.get()) {
            return;
        }
        sustainLength.setFloat(value);
        if (setOrigin == NULL || *setOrigin) {
            sustainLengthOrigin = sustainLength.get();
        }
    }

    void setValueBoundaries()
    {
        float sustain = sustainPosition.get();
        if (start.get() > sustain) {
            sustainLength.set(sustainLength.get() - (start.get() - sustain), &skipOrigin);
            sustainPosition.set(start.get(), &skipOrigin);
        }

        float sustainLen = sustainLength.get();
        if (sustain + sustainLen > end.get()) {
            sustainLength.set(end.get() - sustain, &skipOrigin);
        }

        restoreSustainPosition();
        restoreSustainlength();
    }

    void restoreSustainlength()
    {
        if (sustainLengthOrigin != sustainLength.get()) {
            int sustainLen = end.get() - sustainPosition.get();
            if (sustainLen >= sustainLengthOrigin) {
                sustainLength.set(sustainLengthOrigin, &skipOrigin);
            } else {
                sustainLength.set(sustainLen, &skipOrigin);
            }
        }
    }

    void restoreSustainPosition()
    {
        if (sustainPositionOrigin != sustainPosition.get()) {
            int sustainPos = start.get();
            if (sustainPos <= sustainPositionOrigin) {
                sustainPosition.set(sustainPositionOrigin, &skipOrigin);
            } else {
                sustainPosition.set(sustainPos, &skipOrigin);
            }
        }
    }

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case 0:
            return &sampleBuffer;
        }
        return NULL;
    }
};

#endif