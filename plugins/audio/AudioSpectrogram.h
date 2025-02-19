#ifndef _AUDIO_SPECTROGRAM_H_
#define _AUDIO_SPECTROGRAM_H_

#include "audioPlugin.h"

/*md
## AudioSpectrogram

AudioSpectrogram plugin is used to keep track of audio buffer.

*/
class AudioSpectrogram : public AudioPlugin {
protected:
    float buffer[1024];
    int bufferIndex = 0;

public:
    AudioSpectrogram(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : AudioPlugin(props, config)
    {
    }

    void sample(float* buf)
    {
        if (bufferIndex >= 1024) {
            bufferIndex = 0;
        }
        buffer[bufferIndex++] = buf[track];
    }

    enum DATA_ID {
        BUFFER,
    };

    /*md **Data ID**: */
    uint8_t getDataId(std::string name) override
    {
        /*md - `BUFFER` return a representation of the current audio track */
        if (name == "BUFFER")
            return BUFFER;
        return atoi(name.c_str());
    }

    void* data(int id, void* userdata = NULL) override
    {
        switch (id) {
        case BUFFER: {
            // cut the buffer to the current index
            // and move the index to the end
            return &buffer;
        }
        }
        return NULL;
    }
};

#endif
