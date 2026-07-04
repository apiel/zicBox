/** Description:
This component, named `RamTapeRecording`, functions as a specialized utility for capturing audio within the main application. Its primary role is to record the live sound coming from a specific, user-selected audio track.

The process works by temporarily storing all the audio samples in the computer's extremely fast, temporary memory (RAM). This approach allows for very high-speed capture.

Users can specify several settings, including the output folder, the file name, and the maximum allowed size for the recording buffer (often measured in Megabytes). If the maximum size is reached, the system can operate in a "circular buffer" mode. In this mode, the recording continues, but new audio data automatically overwrites the oldest stored data, ensuring the memory footprint remains stable.

The component monitors the application’s state. When the main audio playback is stopped or paused, the system automatically triggers a background process to retrieve all the collected audio data from the RAM buffer and write it out as a standard, permanent `.wav` audio file onto the hard drive. This ensures the temporary recording is safely preserved only when the session ends.

sha: 44075be3ce2716c987b9bf8301c6e3d6fde8a3c2990fc72578bc5c5e0131ffd9 
*/
#pragma once

#include "audioPlugin.h"
#include "log.h"
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
    // size_t maxSamples = (20 * 1024 * 1024) / sizeof(float); // 20MB
    size_t maxSamples = (5 * 1024 * 1024) / sizeof(float); // 20MB

    std::vector<float> buffer;

    void save()
    {
        std::string filepath = folder + "/tmp/ram_" + filename + ".wav";
        logInfo("[%d] Saving to %s", track, filepath.c_str());
        std::filesystem::create_directories(folder + "/tmp");

        SF_INFO sfinfo;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
        sfinfo.channels = props.channels;
        sfinfo.samplerate = props.sampleRate;
        sndfile = sf_open(filepath.c_str(), SFM_WRITE, &sfinfo);
        if (!sndfile) {
            throw std::runtime_error("Failed to open audio file for writing");
        }

        // write the first part of the buffer
        for (size_t i = bufferIndex; i < buffer.size(); i += 1024) {
            size_t size = buffer.size() - i > 1024 ? 1024 : buffer.size() - i;
            sf_write_float(sndfile, buffer.data() + i, size);
        }

        // write the second part of the buffer
        for (size_t i = 0; i < bufferIndex; i += 1024) {
            size_t size = bufferIndex - i > 1024 ? 1024 : bufferIndex - i;
            sf_write_float(sndfile, buffer.data() + i, size);
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

    RamTapeRecording(AudioPlugin::Props& props, AudioPlugin::Config& config)
        : Mapping(props, config)
    {
        trackNum.props().max = props.maxTracks - 1;

        /*md **Config**: */
        auto& json = config.json;
        if (json.contains("track")) {
            trackNum.set(json["track"].get<float>());
        }

        //md - `"folder": "tape"` to set samples folder path.
        folder = json.value("folder", folder);

        //md - `"filename": "track"` to set filename. By default it is `track`.
        filename = json.value("filename", filename);

        //md - `"maxFileSize": 200` to set max file size. By default it is `200MB`.
        if (json.contains("maxFileSize")) {
            maxSamples = json["maxFileSize"].get<float>() * 1024 * 1024 / sizeof(float);
        }

        //md - `"circularBuffer": true` to enable circular buffer. By default it is `true`.
        circularBuffer = json.value("circularBuffer", circularBuffer);
    }

    bool circularBuffer = true;
    size_t bufferIndex = 0;
    void sample(float* buf)
    {
        if (isPlaying) {
            if (buffer.size() < maxSamples) {
                buffer.push_back(buf[track]);
            } else if (circularBuffer) {
                buffer[bufferIndex] = buf[track];
                bufferIndex = (bufferIndex + 1) % maxSamples;
            }
        }
    }

    void onEvent(AudioEventType event, bool playing) override
    {
        if (event == AudioEventType::STOP || event == AudioEventType::PAUSE) {
            save();
        } else if (event == AudioEventType::START) {
            buffer.clear();
            bufferIndex = 0;
        }
        isPlaying = playing;
    }
};
