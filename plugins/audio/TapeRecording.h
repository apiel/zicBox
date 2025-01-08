#ifndef _TAPE_RECORDING_H_
#define _TAPE_RECORDING_H_

#include "audioPlugin.h"
#include "mapping.h"

#include <filesystem>
#include <mutex>
#include <sndfile.h>
#include <stdexcept>
#include <thread>
#include <vector>

/*md
## TapeRecording

TapeRecording plugin is used to record audio buffer for a given track.
*/
class TapeRecording : public Mapping {
protected:
    std::string folder = "tape";
    std::string filename = "track";
    SNDFILE* sndfile = nullptr;
    SF_INFO sfinfo;
    std::thread writerThread;
    bool loopRunning = true;

    std::vector<float> buffer;

    size_t maxFileSize = 20 * 1024 * 1024; // 200MB

    void writerLoop()
    {
        std::string filepath = folder + "/.tmp/" + filename + ".wav";

        std::filesystem::create_directories(folder + "/.tmp");

        sndfile = sf_open(filepath.c_str(), SFM_WRITE, &sfinfo);
        if (!sndfile) {
            throw std::runtime_error("Failed to open audio file for writing");
        }

        size_t maxSamples = maxFileSize / sizeof(float);
        size_t sampleCount = 0;
        while (loopRunning && sampleCount < maxSamples) {
            if (buffer.size() > 1024) {
                sf_write_float(sndfile, buffer.data(), 1024);
                buffer.erase(buffer.begin(), buffer.begin() + 1024);
                sampleCount += 1024;
            } else {
                // sleep for 100ms
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        if (buffer.empty() && sampleCount < maxSamples) {
            sf_write_float(sndfile, buffer.data(), buffer.size());
        }

        sf_close(sndfile);
    }

public:
    /*md **Values**: */
    ///*/md - `VOLUME` to set volume. */
    // Val& volume = val(100.0f, "VOLUME", { "Volume", .unit = "%" }, [&](auto p) { setVolumeWithGain(p.value, gain.get()); });

    TapeRecording(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();

        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        sfinfo.channels = props.channels;
        sfinfo.samplerate = props.sampleRate;
    }

    ~TapeRecording()
    {
        loopRunning = false;
        if (writerThread.joinable()) {
            writerThread.join();
        }
    }

    void sample(float* buf)
    {
        buffer.push_back(buf[track]);
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP || event == AudioEventType::PAUSE) {
            loopRunning = false;
        } else if (event == AudioEventType::START) {
            if (writerThread.joinable()) {
                writerThread.join();
            }
            buffer.clear();
            loopRunning = true;
            writerThread = std::thread(&TapeRecording::writerLoop, this);
        }
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
            maxFileSize = atoi(value) * 1024 * 1024;
            return true;
        }

        return AudioPlugin::config(key, value);
    }
};

#endif
