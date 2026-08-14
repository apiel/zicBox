#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include "audio/TrackRenderPool.h"
#include "helpers/clamp.h"
#include "zicGridV2/studio.h"
#include "zicGridV2/project.h"

#include <cstdlib>
#include <fstream>

inline static std::atomic<bool> keep_running { true };

struct AudioDeviceInfo {
    std::string name;        // e.g. "default", "hw:0,0"
    std::string displayName; // e.g. "Default Audio Device", "hw:0,0 (HDA Intel PCH)"
};

inline static std::string currentAudioDeviceName = "default";
inline static std::string requestedAudioDeviceName = "";
inline static std::atomic<bool> audioDeviceChangeRequested { false };
inline static std::mutex audioDeviceMutex;

inline std::string getAudioDeviceConfigPath()
{
#ifdef IS_RPI
    return "data/audio_device.txt";
#else
    return "../data/audio_device.txt";
#endif
}

inline void saveAudioDeviceConfig(const std::string& devName)
{
    try {
        std::ofstream file(getAudioDeviceConfigPath());
        if (file.is_open()) {
            file << devName << "\n";
        }
    } catch (...) {}
}

inline std::string loadAudioDeviceConfig()
{
    try {
        std::ifstream file(getAudioDeviceConfigPath());
        if (file.is_open()) {
            std::string line;
            if (std::getline(file, line)) {
                while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) {
                    line.pop_back();
                }
                if (!line.empty()) return line;
            }
        }
    } catch (...) {}
    return "";
}

inline std::vector<AudioDeviceInfo> getAudioOutputDevices()
{
    std::vector<AudioDeviceInfo> devices;
    devices.push_back({ "default", "Default Audio Device" });

    int cardNum = -1;
    while (snd_card_next(&cardNum) == 0 && cardNum >= 0) {
        char* cardName = nullptr;
        snd_card_get_name(cardNum, &cardName);
        std::string cardStr = cardName ? cardName : ("Card " + std::to_string(cardNum));
        if (cardName) free(cardName);

        std::string ctlName = "hw:" + std::to_string(cardNum);
        snd_ctl_t* ctl = nullptr;
        if (snd_ctl_open(&ctl, ctlName.c_str(), 0) == 0 && ctl != nullptr) {
            int devNum = -1;
            while (snd_ctl_pcm_next_device(ctl, &devNum) == 0 && devNum >= 0) {
                snd_pcm_info_t* pcmInfo = nullptr;
                snd_pcm_info_alloca(&pcmInfo);
                snd_pcm_info_set_device(pcmInfo, devNum);
                snd_pcm_info_set_subdevice(pcmInfo, 0);
                snd_pcm_info_set_stream(pcmInfo, SND_PCM_STREAM_PLAYBACK);

                if (snd_ctl_pcm_info(ctl, pcmInfo) == 0) {
                    const char* pcmName = snd_pcm_info_get_name(pcmInfo);
                    std::string hwId = "hw:" + std::to_string(cardNum) + "," + std::to_string(devNum);

                    std::string dispName = cardStr;
                    if (pcmName && pcmName[0] != '\0') {
                        dispName += " - " + std::string(pcmName);
                    }
                    dispName += " (" + hwId + ")";

                    bool exists = false;
                    for (const auto& d : devices) {
                        if (d.name == hwId) {
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        devices.push_back({ hwId, dispName });
                    }
                }
            }
            snd_ctl_close(ctl);
        }
    }

    return devices;
}

inline snd_pcm_t* audioInit(const char* devName = nullptr)
{
    snd_pcm_t* pcm_h = nullptr;
    std::string devStr;
    if (devName && devName[0] != '\0') {
        devStr = devName;
    } else {
        devStr = loadAudioDeviceConfig();
        if (devStr.empty()) {
            const char* envDev = std::getenv("ZIC_AUDIO_DEVICE");
            if (envDev && envDev[0] != '\0') {
                devStr = envDev;
            } else {
                devStr = "default";
            }
        }
    }

    const char* dev = devStr.c_str();

    int err = snd_pcm_open(&pcm_h, dev, SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        std::cerr << "Audio open error (ALSA) on '" << dev << "': " << snd_strerror(err) << std::endl;
        return nullptr;
    }

    std::cout << "ALSA audio initialized on device: " << dev << std::endl;
    currentAudioDeviceName = devStr;

    err = snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 20000);
    if (err < 0) {
        std::cerr << "snd_pcm_set_params failed (20ms latency): " << snd_strerror(err) << ", retrying 50ms..." << std::endl;
        err = snd_pcm_set_params(pcm_h, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, 2, SAMPLE_RATE, 1, 50000);
        if (err < 0) {
            std::cerr << "snd_pcm_set_params failed (50ms latency): " << snd_strerror(err) << std::endl;
            snd_pcm_close(pcm_h);
            return nullptr;
        }
    }

    return pcm_h;
}

inline bool changeAudioDevice(const std::string& devName)
{
    {
        std::lock_guard<std::mutex> lock(audioDeviceMutex);
        requestedAudioDeviceName = devName;
        audioDeviceChangeRequested.store(true);
    }
    saveAudioDeviceConfig(devName);
    return true;
}

inline void setAudioThreadRealtime(pthread_t thread, int priority, const char* name)
{
    sched_param sch {};
    sch.sched_priority = priority;
    int rc = pthread_setschedparam(thread, SCHED_FIFO, &sch);
    if (rc != 0) {
        std::cout << "Unable to set realtime priority for " << name << " (need CAP_SYS_NICE/root)" << std::endl;
    } else {
        std::cout << "Realtime priority set for " << name << std::endl;
    }
}

static Random rnd;

inline void audioWorker(snd_pcm_t* initialPcm)
{
    snd_pcm_t* pcm = initialPcm;

    setAudioThreadRealtime(pthread_self(), 30, "zicGrid_Audio");

    const size_t num_frames = 256;
    std::vector<int16_t> buf(num_frames * 2);
    std::vector<float> tapeBuf(num_frames, 0.f);
    std::vector<float> mixed(num_frames, 0.f);

    const size_t hw = std::thread::hardware_concurrency() == 0 ? 2 : std::thread::hardware_concurrency();
    const size_t maxWorkersByHw = (hw > 2) ? (hw - 2) : 1;
    const size_t workers = std::min<size_t>(4, std::max<size_t>(1, maxWorkersByHw));

    auto renderTrackFn = [](Track& trk, const TrackFrameEvent* trackEvents, size_t numFrames, std::vector<float>& localMix) {
        if (!trk.engine) return;

        float maxPeak = 0.0f;
        for (size_t f = 0; f < numFrames; ++f) {
            const TrackFrameEvent& ev = trackEvents[f];

            if (ev.loadClip) {
                loadClip(trk, ev.clipIdx);
                trk.pendingClipIdx = -1;
            }

            if (ev.noteOn) {
                trk.engine->noteOn(ev.note, ev.velocity);
                trk.noteSamplesRemaining = ev.noteLenSamples;
                trk.playingNote = ev.note;
            }

            if (trk.noteSamplesRemaining > 0 && --trk.noteSamplesRemaining == 0) {
                trk.engine->noteOff(trk.playingNote);
            }

            const float s = trk.engine->sample() * ((trk.isMuted || trk.chainMuted) ? 0.f : trk.volume);
            maxPeak = std::max(maxPeak, std::abs(s));
            localMix[f] += s;

            if (f % 4 == 0) {
                std::lock_guard<std::mutex> hl(trk.historyMtx);
                if (!trk.history.empty()) {
                    trk.history.push_back(s);
                    trk.history.pop_front();
                }
            }
        }

        trk.vumeter.store(maxPeak);
    };

    TrackRenderPool<Track> renderPool(workers, renderTrackFn);
    std::vector<Track*> trackPtrs;
    std::vector<TrackFrameEvent> events;

    while (keep_running) {
        if (audioDeviceChangeRequested.load()) {
            std::string newDev;
            {
                std::lock_guard<std::mutex> lock(audioDeviceMutex);
                newDev = requestedAudioDeviceName;
                audioDeviceChangeRequested.store(false);
            }
            std::cout << "[Audio Engine] Re-opening ALSA audio on device: " << newDev << std::endl;
            if (pcm) {
                snd_pcm_drain(pcm);
                snd_pcm_close(pcm);
                pcm = nullptr;
            }
            pcm = audioInit(newDev.c_str());
            if (!pcm && newDev != "default") {
                std::cerr << "[Audio Engine] Failed to open " << newDev << ", falling back to default" << std::endl;
                pcm = audioInit("default");
            }
        }

        {
            std::lock_guard<std::mutex> lock(studio.audioMutex);
            std::fill(buf.begin(), buf.end(), 0);

            trackPtrs.clear();
            trackPtrs.reserve(studio.tracks.size());
            for (auto& trk : studio.tracks) {
                trackPtrs.push_back(trk.get());
            }

            const size_t trackCount = trackPtrs.size();
            events.assign(trackCount * num_frames, TrackFrameEvent {});

            for (size_t f = 0; f < num_frames; ++f) {
                if (!studio.isPlaying) continue;

                studio.sampleCounter++;
                if (studio.sampleCounter >= studio.samplesPerStep) {
                    studio.sampleCounter = 0;
                    studio.currentStep = (studio.currentStep + 1) % SEQ_STEPS;
                    const bool wrapped = (studio.currentStep == 0);
                    const int curStep = studio.currentStep;

                    for (size_t t = 0; t < trackCount; ++t) {
                        Track* trk = trackPtrs[t];
                        auto& ev = events[t * num_frames + f];

                        if (wrapped) {
                            if (trk->pendingClipIdx >= 0) { // We always prioritize pending clips, also over chain
                                ev.loadClip = true;
                                ev.clipIdx = trk->pendingClipIdx;
                            } else if (trk->chainPlaying && !trk->chain.empty()) {
                                trk->chainActiveIdx++;
                                if (trk->chainActiveIdx >= (int)trk->chain.size()) {
                                    if (trk->chainLoopMode == 1) { // Hold mode
                                        trk->chainActiveIdx = (int)trk->chain.size() - 1;
                                    } else { // Loop mode
                                        trk->chainActiveIdx = 0;
                                    }
                                }
                                int nextItem = trk->chain[trk->chainActiveIdx];
                                if (nextItem == -1) {
                                    trk->chainMuted = true;
                                } else {
                                    trk->chainMuted = false;
                                    if (nextItem != trk->activeClipIdx) {
                                        ev.loadClip = true;
                                        ev.clipIdx = nextItem;
                                    }
                                }
                            }
                        }

                        if (trk->repeatActive && trk->noteRepeat > 0) {
                            int interval = 1 << (trk->noteRepeat - 1);
                            if (curStep % interval == 0) {
                                ev.noteOn = true;
                                ev.note = trk->repeatNote;
                                ev.velocity = 1.0f;
                                ev.noteLenSamples = (uint32_t)(0.5f * studio.samplesPerStep);
                            }
                        } else {
                            auto& step = trk->sequence[curStep];
                            if (step.active && !trk->isMuted && rnd.pct() <= step.condition) {
                                ev.noteOn = true;
                                ev.note = step.note;
                                ev.velocity = step.velocity;
                                ev.noteLenSamples = (uint32_t)(step.len * studio.samplesPerStep);
                            }
                        }
                    }
                }
            }

            renderPool.render(trackPtrs, events, num_frames, mixed);

            for (size_t f = 0; f < num_frames; ++f) {
                float sample = mixed[f];
                sample = studio.masterFx.scatter.process(sample, (double)studio.samplesPerStep);
                sample = studio.masterFx.compressor.process(sample);
                sample = sample * studio.masterFx.volume;
                tapeBuf[f] = sample;

                if (f % 4 == 0) {
                    std::lock_guard<std::mutex> hl(studio.masterHistoryMtx);
                    if (!studio.masterHistory.empty()) {
                        studio.masterHistory.push_back(sample);
                        studio.masterHistory.pop_front();
                    }
                }
            }

            // Process Master Tape audio recorder
            studio.masterFx.tape.process(tapeBuf.data(), num_frames, SAMPLE_RATE);

            // Apply Master FX chain
            float masterVol = studio.masterFx.volume;
            for (size_t f = 0; f < num_frames; ++f) {
                float sample = tapeBuf[f];
                sample = std::clamp(sample, -1.0f, 1.0f);
                int16_t pcmVal = (int16_t)(sample * 32767.0f);
                buf[f * 2] = pcmVal;     // L
                buf[f * 2 + 1] = pcmVal; // R
            }
        }

        if (pcm) {
            snd_pcm_sframes_t frames = snd_pcm_writei(pcm, buf.data(), num_frames);
            if (frames < 0) {
                frames = snd_pcm_recover(pcm, (int)frames, 0);
                if (frames < 0) {
                    std::cerr << "ALSA writei recovery failed: " << snd_strerror((int)frames) << std::endl;
                }
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }

    if (pcm) {
        snd_pcm_drain(pcm);
        snd_pcm_close(pcm);
        pcm = nullptr;
    }
}
