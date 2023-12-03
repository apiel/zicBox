#ifndef _SYNTH_SAMPLE_H_
#define _SYNTH_SAMPLE_H_

#include <math.h>
#include <sndfile.h>
#include <time.h>

#include "audioPlugin.h"
#include "fileBrowser.h"
#include "mapping.h"

#include "utils/ValSerializeSndFile.h"

// Each voice should hold his own wave file
// Sequencer can play different wave files on each step
//      ?? The sample is loaded on note on if it's not already loaded
//      Wavefile contain patch settings
//      Need to be able to edit patch while playing for a specific wave file
// There is 2 modulations...

class SynthSample : public Mapping {
protected:
    FileBrowser fileBrowser = FileBrowser("./samples");

    // Use to restore sustain in case it was move by another parameter
    float sustainPositionOrigin = -255.0f;
    float sustainLengthOrigin = -255.0f;

public:
    Val& start = val(0.0f, "START", { "Start", .unit = "%" }, [&](auto p) { setStart(p.value); });
    Val& end = val(100.0f, "END", { "End", .unit = "%" }, [&](auto p) { setEnd(p.value); });
    Val& sustainPosition = val(0.0f, "SUSTAIN_POSITION", { "Sustain position", .unit = "%" }, [&](auto p) { setSustainPosition(p.value, true); });
    // Where -1 is no sustain
    Val& sustainLength = val(0.0f, "SUSTAIN_LENGTH", { "Sustain length", .unit = "%" }, [&](auto p) { setSustainLength(p.value, true); });

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
        return AudioPlugin::config(key, value);
    }

    void sample(float* buf)
    {
    }

    void open(float value, bool force = false)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            char* filepath = fileBrowser.getFilePath(position);
            browser.setString(fileBrowser.getFile(position));
            debug("SAMPLE_SELECTOR: %f %s\n", value, filepath);
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

    void setSustainPosition(float value, bool setOrigin = false)
    {
        if (value < start.get() || value + sustainLength.get() > end.get()) {
            return;
        }
        sustainPosition.setFloat(value);
        if (setOrigin) {
            sustainPositionOrigin = sustainPosition.get();
        }
    }

    void setSustainLength(float value, bool setOrigin = false)
    {
        if (value + sustainPosition.get() > end.get()) {
            return;
        }
        sustainLength.setFloat(value);
        if (setOrigin) {
            sustainLengthOrigin = sustainLength.get();
        }
    }

    void setValueBoundaries()
    {
        float sustain = sustainPosition.get();
        if (start.get() > sustain) {
            sustainLength.set(sustainLength.get() - (start.get() - sustain));
            sustainPosition.set(start.get());
        }

        float sustainLen = sustainLength.get();
        if (sustain + sustainLen > end.get()) {
            sustainLength.set(end.get() - sustain);
        }

        restoreSustainlength();
        restoreSustainPosition();
    }

    void restoreSustainlength()
    {
        if (sustainLengthOrigin != sustainLength.get()) {
            printf("restoreSustainlength\n");
            int sustainLen = end.get() - sustainPosition.get();
            if (sustainLen >= sustainLengthOrigin) {
                sustainLength.set(sustainLengthOrigin);
            } else {
                sustainLength.set(sustainLen);
            }
        }
    }

    void restoreSustainPosition()
    {
        // if (sustainPositionOrigin != -255.0f) {
        //     int sustainPos = start.get();
        //     if (sustainPos <= sustainPositionOrigin) {
        //         sustainPosition.set(sustainPositionOrigin);
        //         sustainPositionOrigin = -255.0f;
        //     } else {
        //         sustainLength.set(sustainPositionOrigin - sustainPos);
        //         sustainPosition.set(sustainPos);
        //     }
        // }
    }
};

#endif