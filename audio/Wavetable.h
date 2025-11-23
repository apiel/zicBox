/** Description:
This C++ header defines a crucial component for digital sound synthesis called `Wavetable`. This component is designed to manage and play back a collection of individual sound cycles, allowing for dynamic and complex sound changes.

**Core Concept and Functionality:**

The `Wavetable` class acts as a sophisticated audio loader and playback engine. Instead of treating an audio file as a single continuous track, it assumes the file is composed of 64 distinct, short, looping sound patterns (waveforms) stored sequentially.

1.  **Loading and Storage:** The component includes mechanisms to browse local directories, open audio files (typically WAV format), and read the entire contents into a large internal memory buffer. This buffer holds all 64 waveforms ready for instant access.
2.  **Sound Generation:** The class provides methods to retrieve individual audio samples based on an index. To ensure that playback sounds natural and free of harsh digital artifacts, it employs a mathematical technique called "linear interpolation," which smoothly blends between adjacent audio data points.
3.  **Morphing:** This is the key creative feature. The `morph` function allows the user to instantly change which of the 64 stored waveforms is currently being played. By smoothly adjusting a control value, the system can seamlessly transition, or "morph," from the first waveform to the last, creating rich, evolving sonic textures.

In essence, this component is the engine that transforms a simple audio file containing many cycles into a versatile sound source for a synthesizer.

sha: e06e62e5ef520c78f65a0a88da9371d38c1f0e3a82a343d2cc11ff91e4011cbd 
*/
#pragma once

#include <sndfile.h>
#include <string.h>
#include <string>

#include "log.h"

#include "audio/fileBrowser.h"
#include "audio/WavetableInterface.h"
#include "audio/utils/linearInterpolation.h"
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
