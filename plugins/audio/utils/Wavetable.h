#pragma once

#include <sndfile.h>
#include <string.h>
#include <string>

#include "log.h"

#include "plugins/audio/utils/fileBrowser.h"
#include "plugins/audio/utils/WavetableInterface.h"
#include "plugins/audio/utils/utils.h"
#include "host/constants.h"

#define ZIC_WAVETABLE_WAVEFORMS_COUNT 64

class Wavetable : public WavetableInterface {
protected:
    SF_INFO sfinfo;
    SNDFILE* file = NULL;

    static const uint64_t bufferSize = ZIC_WAVETABLE_WAVEFORMS_COUNT * 2048;
    uint64_t bufferSampleCount = 0;
    uint64_t maxSampleStart = -1;
    float bufferSamples[bufferSize];

    uint64_t sampleStart = 0;

public:
    float sampleIndex = 0.0f;
    FileBrowser fileBrowser = FileBrowser(AUDIO_FOLDER + "/wavetables");

    Wavetable()
        : WavetableInterface(2048)
    {
        memset(&sfinfo, 0, sizeof(sfinfo));
        open(0, true);
    }

    float* samples()
    {
        return bufferSamples + sampleStart;
    }

    float* sample(float* index) override
    {
        return &bufferSamples[(uint16_t)(*index * sampleCount) + sampleStart];
    }

    float sample(float* index, float sampleInc) override
    {
        (*index) += sampleInc;
        while ((*index) >= sampleCount) {
            (*index) -= sampleCount;
        }
        // return bufferSamples[(uint16_t)(*index) + sampleStart];
        return linearInterpolationAbsolute(*index, sampleCount, bufferSamples + sampleStart);
    }

    float sample(float* index, float sampleInc, float lfo)
    {
        (*index) += sampleInc;
        while ((*index) >= sampleCount) {
            (*index) -= sampleCount;
        }
        // int pos = CLAMP(sampleStart + lfo * bufferSampleCount, 0, bufferSampleCount - sampleCount);
        int p = ZIC_WAVETABLE_WAVEFORMS_COUNT * lfo;
        int pos = CLAMP(sampleStart + p * sampleCount, 0, bufferSampleCount - sampleCount);;
        return linearInterpolationAbsolute(*index, sampleCount, bufferSamples + pos);
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
            logError("Error: could not open file %s\n", filename.c_str());
        }
        logTrace("Audio file %s sampleCount %ld sampleRate %d\n", filename.c_str(), (long)sfinfo.frames, sfinfo.samplerate);

        bufferSampleCount = sf_read_float(file, bufferSamples, bufferSize);
        sampleCount = bufferSampleCount / (float)ZIC_WAVETABLE_WAVEFORMS_COUNT;
        maxSampleStart = bufferSampleCount / ZIC_WAVETABLE_WAVEFORMS_COUNT * (ZIC_WAVETABLE_WAVEFORMS_COUNT - 1);
    }

    void open(int position, bool force)
    {
        if (force || position != fileBrowser.position) {
            std::string filepath = fileBrowser.getFilePath(position);
            logTrace("KICK23_SAMPLE_SELECTOR: %d %s\n", position, filepath.c_str());
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

    void morph(int index)
    {
        sampleStart = index * sampleCount;
        if (sampleStart > maxSampleStart) {
            sampleStart = maxSampleStart;
        }
    }

    int getIndex() { return sampleStart / sampleCount; }
};
