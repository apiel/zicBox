#ifndef _WAVETABLE_H_
#define _WAVETABLE_H_

#include <sndfile.h>
#include <string>

#include "fileBrowser.h"

#define ZIC_WAVETABLE_WAVEFORMS_COUNT 64

class Wavetable {
protected:
    SF_INFO sfinfo;
    SNDFILE* file = NULL;

    uint64_t sampleCount = 2048; // should this be configurable?
    static const uint64_t bufferSize = ZIC_WAVETABLE_WAVEFORMS_COUNT * 2048;
    uint64_t bufferSampleCount = 0;
    uint64_t maxSampleStart = -1;
    float bufferSamples[bufferSize];

    uint64_t sampleStart = 0;

public:
    float sampleIndex = 0.0f;
    FileBrowser fileBrowser = FileBrowser("./wavetables");

    Wavetable()
    {
        memset(&sfinfo, 0, sizeof(sfinfo));
        open(0, true);
    }

    float sample(float time, float* index, float amp, float freq, float pitch)
    {
        (*index) += pitch * freq;
        while ((*index) >= sampleCount) {
            (*index) -= sampleCount;
        }
        return bufferSamples[(uint16_t)(*index) + sampleStart] * amp;
    }

    void close()
    {
        if (file) {
            sf_close(file);
        }
    }

    void open(std::string filename)
    {
        close();

        if (!(file = sf_open(filename.c_str(), SFM_READ, &sfinfo))) {
            printf("Error: could not open file %s\n", filename.c_str());
        }
        printf("Audio file %s sampleCount %ld sampleRate %d\n", filename.c_str(), (long)sfinfo.frames, sfinfo.samplerate);

        bufferSampleCount = sf_read_float(file, bufferSamples, bufferSize);
        sampleCount = bufferSampleCount / (float)ZIC_WAVETABLE_WAVEFORMS_COUNT;
        maxSampleStart = bufferSampleCount / ZIC_WAVETABLE_WAVEFORMS_COUNT * (ZIC_WAVETABLE_WAVEFORMS_COUNT - 1);
    }

       void open(int position, bool force)
    {
        if (force || position != fileBrowser.position) {
            std::string filepath = fileBrowser.getFilePath(position);
            printf("KICK23_SAMPLE_SELECTOR: %d %s\n", position, filepath.c_str());
            open(filepath);
        }
    } 

    void morph(float pct)
    {
        sampleStart = pct * bufferSampleCount;
        if (sampleStart > maxSampleStart) {
            sampleStart = maxSampleStart;
        }
    }
};

#endif
