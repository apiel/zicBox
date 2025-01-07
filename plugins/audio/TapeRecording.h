#ifndef _TAPE_RECORDING_H_
#define _TAPE_RECORDING_H_

#include "audioPlugin.h"
#include "mapping.h"
#include <sndfile.h>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

/*md
## TapeRecording

TapeRecording plugin is used to record audio buffer for a given track.
*/
class TapeRecording : public Mapping {
protected:
    std::string folder = "tape";
    std::string filename = "track";
    SNDFILE* sndfile = nullptr;
    SF_INFO sfinfo{};
    std::thread writerThread;
    std::atomic<bool> running{false};

    // Buffer for thread-safe communication
    std::vector<float> writeBuffer;
    std::mutex bufferMutex;
    std::condition_variable bufferCv;

    void initFilepath()
    {
        std::string filepath = folder + "/" + filename + ".wav";
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        sfinfo.channels = props.channels;
        sfinfo.samplerate = props.sampleRate;

        sndfile = sf_open(filepath.c_str(), SFM_WRITE, &sfinfo);
        if (!sndfile) {
            throw std::runtime_error("Failed to open audio file for writing");
        }
    }

    void writerLoop()
    {
        while (running.load()) {
            std::unique_lock<std::mutex> lock(bufferMutex);
            bufferCv.wait(lock, [&] { return !writeBuffer.empty() || !running.load(); });

            if (!writeBuffer.empty()) {
                sf_write_float(sndfile, writeBuffer.data(), writeBuffer.size());
                writeBuffer.clear();
            }
        }

        // Flush remaining samples before exiting
        if (!writeBuffer.empty()) {
            sf_write_float(sndfile, writeBuffer.data(), writeBuffer.size());
        }
    }

public:
    /*md **Values**: */
    ///*/md - `VOLUME` to set volume. */
    // Val& volume = val(100.0f, "VOLUME", { "Volume", .unit = "%" }, [&](auto p) { setVolumeWithGain(p.value, gain.get()); });

    TapeRecording(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        initValues();
        running.store(true);
        writerThread = std::thread(&TapeRecording::writerLoop, this);
    }

    ~TapeRecording()
    {
        running.store(false);
        bufferCv.notify_all();
        if (writerThread.joinable()) {
            writerThread.join();
        }
        if (sndfile) {
            sf_close(sndfile);
        }
    }

    void sample(float* buf)
    {
        // Push the incoming sample to the write buffer
        {
            std::lock_guard<std::mutex> lock(bufferMutex);
            writeBuffer.push_back(*buf);
        }
        bufferCv.notify_one();
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

        return AudioPlugin::config(key, value);
    }
};

#endif

