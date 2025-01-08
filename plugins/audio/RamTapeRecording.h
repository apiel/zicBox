#ifndef _RAM_TAPE_RECORDING_H_
#define _RAM_TAPE_RECORDING_H_

#include "audioPlugin.h"
#include "mapping.h"

#include <filesystem>
#include <mutex>
#include <sndfile.h>
#include <stdexcept>
#include <thread>
#include <vector>

/*md
## RamTapeRecording

RamTapeRecording plugin is used to record audio buffer for a given track.
*/

// TODO provide a way to start recording at the next bar

class RamTapeRecording : public Mapping {
protected:
    std::string folder = "tape";
    std::string filename = "track";
    SNDFILE* sndfile = nullptr;
    bool isPlaying = false;
    size_t sampleCount = 0;
    size_t maxSamples = (20 * 1024 * 1024) / sizeof(float); // 20MB

    std::vector<float> buffer;

    void save()
    {
        std::string filepath = folder + "/tmp/ram_" + filename + ".wav";
        std::filesystem::create_directories(folder + "/tmp");

        SF_INFO sfinfo;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        sfinfo.channels = props.channels;
        sfinfo.samplerate = props.sampleRate;
        sndfile = sf_open(filepath.c_str(), SFM_WRITE, &sfinfo);
        if (!sndfile) {
            throw std::runtime_error("Failed to open audio file for writing");
        }

        // while (!buffer.empty()) {
        //     sf_write_float(sndfile, buffer.data(), 1024);
        //     buffer.erase(buffer.begin(), buffer.begin() + 1024);
        // }
        sf_write_float(sndfile, buffer.data(), buffer.size());

        sf_close(sndfile);
    }

public:
    /*md **Values**: */
    /*md - `TRACK` to set the track number. */
    Val& trackNum = val(0.0f, "TRACK", { "Track", .max = 32 }, [&](auto p) {
        p.val.setFloat(p.value);
        track = p.val.get();
    });

    RamTapeRecording(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        trackNum.props().max = props.maxTracks;
        initValues();
    }

    void sample(float* buf)
    {
        if (isPlaying && sampleCount < maxSamples) {
            buffer.push_back(buf[track]);
            sampleCount++;
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP || event == AudioEventType::PAUSE) {
            save();
        } else if (event == AudioEventType::START) {
            buffer.clear();
        }
        isPlaying = playing;
    }

    /*md **Config**: */
    bool config(char* key, char* value) override
    {
        /*md - `TAPE_FOLDER` set samples folder path. */
        if (strcmp(key, "TAPE_FOLDER") == 0) {
            folder = value;
            return true;
        }

        /*md - `FILENAME: filename` to set filename. By default it is `track`.*/
        if (strcmp(key, "FILENAME") == 0) {
            filename = value;
            return true;
        }

        /*md - `MAX_FILE_SIZE: 200` to set max file size. By default it is `200MB`.*/
        if (strcmp(key, "MAX_FILE_SIZE") == 0) {
            maxSamples = atoi(value) * 1024 * 1024 / sizeof(float);
            return true;
        }

        return AudioPlugin::config(key, value);
    }
};

#endif
