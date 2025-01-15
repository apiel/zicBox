#ifndef _TAPE_RECORDING_H_
#define _TAPE_RECORDING_H_

#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "plugins/audio/utils/audioFile.h"

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

// TODO provide a way to start recording at the next bar

class TapeRecording : public Mapping {
protected:
    std::string folder = "samples";
    std::string filename = "track";
    SNDFILE* sndfile = nullptr;
    SNDFILE* playSndfile = nullptr;
    std::thread writerThread;
    bool loopRunning = true;
    uint8_t trackPlayback = 0;
    int fileUpdateState = 0;

    std::vector<float> buffer;

    size_t maxSamples = (200 * 1024 * 1024) / sizeof(float); // 200MB

    std::string getTmpFolder()
    {
        return folder + "/tape_tmp/";
    }

    std::string getTmpFilePath()
    {
        return getTmpFolder() + filename + ".wav";
    }

    void writerLoop()
    {
        std::string filepath = getTmpFilePath();

        std::filesystem::create_directories(getTmpFolder());

        SF_INFO sfinfo;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        sfinfo.channels = props.channels;
        sfinfo.samplerate = props.sampleRate;
        sndfile = sf_open(filepath.c_str(), SFM_WRITE, &sfinfo);
        if (!sndfile) {
            throw std::runtime_error("Failed to open audio file for writing");
        }

        size_t sampleCount = 0;
        while (loopRunning && sampleCount < maxSamples) {
            if (buffer.size() > 1024) {
                sf_write_float(sndfile, buffer.data(), 1024);
                buffer.erase(buffer.begin(), buffer.begin() + 1024);
                sampleCount += 1024;
                fileUpdateState++;
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
    /*md - `TRACK` to set the track number. */
    Val& trackNum = val(0.0f, "TRACK", { "Track", .max = 32 }, [&](auto p) {
        p.val.setFloat(p.value);
        track = p.val.get();
    });

    TapeRecording(AudioPlugin::Props& props, char* _name)
        : Mapping(props, _name)
    {
        trackNum.props().max = props.maxTracks - 1;
        initValues();
    }

    ~TapeRecording()
    {
        loopRunning = false;
        if (writerThread.joinable()) {
            writerThread.join();
        }
    }

    sf_count_t playSampleCount = 0;
    void play(sf_count_t start, sf_count_t end)
    {
        std::string filepath = getTmpFilePath();
        SF_INFO sfinfo;
        if (playSndfile) {
            stop();
        }
        SNDFILE* file = sf_open(filepath.c_str(), SFM_READ, &sfinfo);
        if (!file) {
            return;
        }
        sf_seek(file, start, SEEK_SET);
        playSampleCount = (end ? end : sfinfo.frames) - start;
        playSndfile = file;
    }

    void stop()
    {
        sf_close(playSndfile);
        playSndfile = NULL;
    }

    static const size_t CHUNK_SIZE = 1024;
    float readBuffer[CHUNK_SIZE];
    size_t currentSampleIndex = CHUNK_SIZE;

    void sample(float* buf)
    {
        buffer.push_back(buf[track]);
        if (playSndfile) {
            if (playSampleCount) {
                if (currentSampleIndex >= CHUNK_SIZE) {
                    sf_count_t count = sf_read_float(playSndfile, readBuffer, CHUNK_SIZE);

                    if (count == 0) {
                        stop();
                        return;
                    }
                    currentSampleIndex = 0; // Reset index for the new chunk
                }
                float tmpBuf = readBuffer[currentSampleIndex++];
                playSampleCount--;

                buf[trackPlayback] = tmpBuf;
            } else {
                stop();
            }
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
            buffer.clear();
            loopRunning = true;
            writerThread = std::thread(&TapeRecording::writerLoop, this);
            pthread_setname_np(writerThread.native_handle(), "tapeWriter");
        }
    }

    /*md **Config**: */
    bool config(char* key, char* value) override
    {
        if (strcmp(key, "TRACK") == 0) {
            trackNum.set(atoi(value));
            trackPlayback = atoi(value);
            return true;
        }

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

        /*md - `MAX_TRACK` to set the max track number. */
        if (strcmp(key, "MAX_TRACK") == 0) {
            trackNum.props().max = atoi(value);
            return true;
        }

        return AudioPlugin::config(key, value);
    }

    enum DATA_ID {
        PLAY_STOP,
        SYNC,
        SAVE,
    };

    /*md **Data ID**: */
    uint8_t getDataId(std::string name) override
    {
        /*md - `PLAY_STOP` play or stop the recorded wavfile */
        if (name == "PLAY_STOP")
            return DATA_ID::PLAY_STOP;
        /*md - `SYNC` watch for file change and set play state */
        if (name == "SYNC")
            return DATA_ID::SYNC;
        /*md - `SAVE` save the recorded wavfile */
        if (name == "SAVE")
            return DATA_ID::SAVE;
        return atoi(name.c_str());
    }

    struct PlayData {
        sf_count_t start = 0;
        sf_count_t end = 0;
    } playData;

    void* data(int id, void* userdata = NULL)
    {
        switch (id) {
        case DATA_ID::PLAY_STOP: {
            if (playSndfile) {
                stop();
            } else {
                play(playData.start, playData.end);
                return playSndfile;
            }
            return NULL;
        }
        case DATA_ID::SYNC: {
            if (userdata != NULL) {
                playData.start = ((PlayData*)userdata)->start;
                playData.end = ((PlayData*)userdata)->end;
            }
            return (void*)&fileUpdateState;
        }
        case DATA_ID::SAVE: {
            if (userdata) {
                std::string name = *(std::string*)userdata;
                std::string dest = folder + "/" + name + ".wav";

                copyPartialAudioFile(getTmpFilePath(), dest, playData.start, playData.end);
            }
            return NULL;
        }
        }

        return NULL;
    }
};

#endif
