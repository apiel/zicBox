#ifndef _MULTI_TAPE_RECORDING_H_
#define _MULTI_TAPE_RECORDING_H_

#include "audioPlugin.h"
#include "mapping.h"

#include <filesystem>
#include <mutex>
#include <sndfile.h>
#include <stdexcept>
#include <thread>
#include <vector>

/*md
## MultiTapeRecording

MultiTapeRecording plugin is used to record audio buffer for a given track.
*/

// TODO provide a way to start recording at the next bar

class MultiTapeRecording : public Mapping {
protected:
    std::string folder = "tape";
    std::string filename = "track";
    SF_INFO sfinfo;
    std::thread writerThread;
    bool loopRunning = true;

    struct Tape {
        uint8_t track;
        SNDFILE* sndfile = nullptr;
        std::vector<float> buffer;
    };
    std::vector<Tape> tapes = { { 1 }, { 2 }, { 3 }, { 4 }, { 5 }, { 6 }, { 7 }, { 8 } };

    size_t maxFileSize = 1024 * 1024 * 1024; // 1GB

    void writerLoop()
    {

        std::filesystem::create_directories(folder + "/tmp");

        for (auto& tape : tapes) {
            std::string filepath = folder + "/tmp/" + filename + "_" + std::to_string(tape.track) + ".wav";
            tape.sndfile = sf_open(filepath.c_str(), SFM_WRITE, &sfinfo);
            if (!tape.sndfile) {
                throw std::runtime_error("Failed to open audio file for writing");
            }
        }

        size_t maxSamples = maxFileSize / sizeof(float);
        size_t sampleCount = 0;
        while (loopRunning && sampleCount < maxSamples) {
            bool updated = false;
            for (auto& tape : tapes) {
                if (tape.buffer.size() > 1024) {
                    sf_write_float(tape.sndfile, tape.buffer.data(), 1024);
                    tape.buffer.erase(tape.buffer.begin(), tape.buffer.begin() + 1024);
                    sampleCount += 1024;
                    updated = true;
                }
            }
            if (!updated) {
                // sleep for 100ms
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        // if (buffer.empty() && sampleCount < maxSamples) {
        //     sf_write_float(sndfile, buffer.data(), buffer.size());
        // }
        // sf_close(sndfile);

        for (auto& tape : tapes) {
            if (tape.buffer.empty() && sampleCount < maxSamples) {
                sf_write_float(tape.sndfile, tape.buffer.data(), tape.buffer.size());
            }
            sf_close(tape.sndfile);
        }
    }

public:
    /*/md **Values**: */

    MultiTapeRecording(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();

        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        sfinfo.channels = props.channels;
        sfinfo.samplerate = props.sampleRate;
    }

    ~MultiTapeRecording()
    {
        loopRunning = false;
        if (writerThread.joinable()) {
            writerThread.join();
        }
    }

    void sample(float* buf)
    {
        for (auto& tape : tapes) {
            tape.buffer.push_back(buf[tape.track]);
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP || event == AudioEventType::PAUSE) {
            loopRunning = false;
        } else if (event == AudioEventType::START) {
            if (writerThread.joinable()) {
                writerThread.join();
            }
            for (auto& tape : tapes) {
                tape.buffer.clear();
            }
            loopRunning = true;
            writerThread = std::thread(&MultiTapeRecording::writerLoop, this);
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

        /*md - `TOTAL_MAX_FILE_SIZE: 200` to set max file size. By default it is `1024MB`.*/
        if (strcmp(key, "TOTAL_MAX_FILE_SIZE") == 0) {
            maxFileSize = atoi(value) * 1024 * 1024;
            return true;
        }

        return AudioPlugin::config(key, value);
    }
};

#endif
