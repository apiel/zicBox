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

public:
    Val& start = val(0.0f, "START", { "Start", .unit = "%" });

    Val& browser = val(0.0f, "BROWSER", { "Browser", VALUE_STRING, .max = (float)fileBrowser.count }, [&](float value) { open(value); });

    SynthSample(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
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
};

#endif