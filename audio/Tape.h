#pragma once

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <sndfile.h>
#include <string>
#include <vector>

class Tape {
public:
    explicit Tape(std::string folder = "")
        : samplesFolder(std::move(folder))
    {
    }

    std::atomic<bool> armed { false };
    std::atomic<bool> recording { false };

    std::string filename = "zzz_tape_0.wav"; // Prefix with zzz to be at the end of the list

    ~Tape()
    {
        stopRecordingAndPrepareNext();
    }

    void process(const float* mono, size_t frames, uint32_t sampleRate)
    {
        if (!mono || frames == 0 || sampleRate == 0) {
            return;
        }

        if (!armed.load()) {
            if (recording.load()) {
                stopRecordingAndPrepareNext();
            }
            return;
        }

        float peak = 0.0f;
        for (size_t i = 0; i < frames; ++i) {
            peak = std::max(peak, std::abs(mono[i]));
        }
        const bool hasIncomingAudio = peak >= START_THRESHOLD;

        if (!recording.load() && hasIncomingAudio) {
            startRecording(sampleRate);
        }

        if (!recording.load()) {
            return;
        }

        const uint64_t maxSilentFrames = static_cast<uint64_t>(sampleRate) * STOP_SILENCE_SECONDS;

        if (hasIncomingAudio) {
            silenceFrames = 0;
            if (!pendingSilence.empty()) {
                sf_write_float(sndfile, pendingSilence.data(), static_cast<sf_count_t>(pendingSilence.size()));
                pendingSilence.clear();
            }
            sf_write_float(sndfile, mono, static_cast<sf_count_t>(frames));
            return;
        }

        silenceFrames += static_cast<uint64_t>(frames);
        pendingSilence.insert(pendingSilence.end(), mono, mono + frames);

        if (silenceFrames >= maxSilentFrames) {
            // Do not flush `pendingSilence`: this trims trailing silence.
            stopRecordingAndPrepareNext();
        }
    }

private:
    static constexpr float START_THRESHOLD = 0.008f;
    static constexpr uint32_t STOP_SILENCE_SECONDS = 2;

    SNDFILE* sndfile = nullptr;
    uint64_t silenceFrames = 0;
    std::vector<float> pendingSilence;
    std::string samplesFolder;

    void startRecording(uint32_t sampleRate)
    {
        if (recording.load()) {
            return;
        }

        if (!samplesFolder.empty()) {
            std::filesystem::create_directories(samplesFolder);
            refreshNextFileName(samplesFolder);
        }

        std::string path = samplesFolder.empty() ? filename : (samplesFolder + "/" + filename);

        SF_INFO sfinfo {};
        sfinfo.samplerate = static_cast<int>(sampleRate);
        sfinfo.channels = 1;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

        sndfile = sf_open(path.c_str(), SFM_WRITE, &sfinfo);
        if (!sndfile) {
            recording.store(false);
            return;
        }

        silenceFrames = 0;
        pendingSilence.clear();
        recording.store(true);
    }

    void stopRecordingAndPrepareNext()
    {
        const bool wasRecording = recording.load();

        if (sndfile) {
            sf_close(sndfile);
            sndfile = nullptr;
        }

        silenceFrames = 0;
        pendingSilence.clear();
        recording.store(false);

        if (wasRecording) {
            armed.store(false);
        }

        if (!samplesFolder.empty()) {
            refreshNextFileName(samplesFolder);
        }
    }

    void refreshNextFileName(const std::string& samplesFolder)
    {
        int maxIdx = -1;

        std::error_code ec;
        if (!std::filesystem::exists(samplesFolder, ec)) {
            filename = "zzz_tape_0.wav";
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(samplesFolder, ec)) {
            if (ec || !entry.is_regular_file()) {
                continue;
            }

            const std::string name = entry.path().filename().string();
            if (name.size() < 10 || name.rfind("zzz_tape_", 0) != 0 || name.substr(name.size() - 4) != ".wav") {
                continue;
            }

            const std::string suffix = name.substr(5, name.size() - 9);
            if (suffix.empty() || !std::all_of(suffix.begin(), suffix.end(), [](unsigned char c) { return std::isdigit(c) != 0; })) {
                continue;
            }

            int id = std::atoi(suffix.c_str());
            maxIdx = std::max(maxIdx, id);
        }

        filename = "zzz_tape_" + std::to_string(maxIdx + 1) + ".wav";
    }
};