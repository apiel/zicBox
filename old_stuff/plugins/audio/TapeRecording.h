/** Description:
This code defines an advanced audio component called `TapeRecording`. This plugin's core function is to capture and manage continuous audio input specifically for a single selected track within a larger audio system.

### How It Works

1.  **Recording Process:** When recording begins, the audio data from the chosen track is collected into an internal memory buffer. To ensure the main audio performance isn't disrupted, the plugin uses a dedicated, separate background operation (a "writer thread"). This background process constantly monitors the memory buffer, takes chunks of recorded audio, and safely writes them to a temporary standard WAV file on the disk. This writing process limits the recording size to a configurable maximum, typically 200MB.

2.  **Playback and Control:** The plugin monitors global audio events (like Start, Stop, or Pause). When instructed to play, it opens the temporary recording file and streams the captured audio data back into the system, routing it to a specified output track.

3.  **Configuration:** Users can configure various parameters, including the specific track number being recorded, the storage folder path for files, and the maximum allowable recording size.

4.  **Saving:** The plugin provides a utility to save segments of the temporary recording permanently. Users can specify a starting and ending point, and the system will copy only that portion into a new, final WAV file with a desired name.

In essence, `TapeRecording` acts as a seamless loop recorder and editor for a single track's audio history.

sha: 1d1d3364a9c096cea13fc833ca7ef7a2a09f5c6a2a905950573342c8b66f040d 
*/
#pragma once

#include "audioPlugin.h"
#include "log.h"
#include "mapping.h"
#include "audio/audioFile.h"

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

    TapeRecording(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        trackNum.props().max = props.maxTracks - 1;
        initValues();

        //md **Config**:
        auto& json = config.json;

        if (json.contains("track")) {
            trackNum.set(json["track"].get<float>());
            trackPlayback = json["track"].get<float>();
        }

        //md - `"folder": "tape"` to set samples folder path.
        folder = json.value("folder", folder);

        //md - `"filename": "track"` to set filename. By default it is `track`.
        filename = json.value("filename", filename);

        //md - `"maxFileSize": 200` to set max file size. By default it is `200MB`.
        if (json.contains("maxFileSize")) {
            maxSamples = json["maxFileSize"].get<float>() * 1024 * 1024 / sizeof(float);
        }

        //md - `"maxTrack": 32` to set the max track number. By default it is `32`.
        if (json.contains("maxTrack")) {
            trackNum.props().max = json["maxTrack"].get<float>();
        }
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
        if (loopRunning) { // we could even check for buffer size...
            buffer.push_back(buf[track]);
        }
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
