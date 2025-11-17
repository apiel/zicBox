#pragma once

#include <math.h>
#include <sndfile.h>
#include <time.h>

#include "audioPlugin.h"
#include "audio/fileBrowser.h"
#include "mapping.h"

#include "helpers/random.h"
#include "log.h"
#include "plugins/audio/utils/ValSerializeSndFile.h"
#include "host/constants.h"
#include "audio/utils/getStepMultiplier.h"
#include "audio/utils/applySampleGain.h"

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

    FileBrowser fileBrowser = FileBrowser(AUDIO_FOLDER + "/samples");
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
    Val& browser = val(1.0f, "BROWSER", { "Browser", VALUE_STRING, .min = 1.0f, .max = (float)fileBrowser.count }, [&](auto p) { open(p.value); });

    SynthDrumSample(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
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

    void noteOn(uint8_t note, float _velocity, void* userdata = NULL) override
    {
        // printf("[%d] drum sample noteOn: %d %f\n", track, note, _velocity);
        logTrace("drum sample noteOn: %d %f", note, velocity);
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
        logTrace("Audio file %s sampleCount %ld sampleRate %d\n", filename, (long)sfinfo.frames, sfinfo.samplerate);
        // printf(".................Audio file chan %d vs prop chan %d\n", sfinfo.channels, props.channels);

        sampleBuffer.count = sf_read_float(file, sampleData, bufferSize);
        sampleBuffer.data = sampleData;

        sf_close(file);

        stepMultiplier = getStepMultiplierMonoTrack(sfinfo.channels, props.channels);

        index = sampleBuffer.count;
        indexEnd = end.pct() * sampleBuffer.count;
        applySampleGain(sampleBuffer.data, sampleBuffer.count);
    }

    void open(float value, bool force = false)
    {
        browser.setFloat(value);
        int position = browser.get();
        if (force || position != fileBrowser.position) {
            browser.setString(fileBrowser.getFile(position));
            std::string filepath = fileBrowser.getFilePath(position);
            logTrace("SAMPLE_SELECTOR: %f %s\n", value, filepath);
            open(filepath);
        }
    }
};
