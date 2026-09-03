#pragma once

#include <alsa/asoundlib.h>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include "studio.h"

extern std::atomic<bool> keep_running;

class MidiClockInput {
private:
    snd_seq_t* seq = nullptr;
    int inPort = -1;
    int connectedClient = -1;
    int connectedPort = -1;

    std::thread midiThread;

    std::atomic<bool> connected { false };
    std::atomic<bool> receivingClock { false };
    std::atomic<float> detectedBpm { 120.0f };
    char deviceNameBuf[64] { 0 };

    uint32_t tickCounter = 0;
    std::chrono::steady_clock::time_point lastClockTime;
    std::chrono::steady_clock::time_point lastEventTime;

    void updateDeviceName(const std::string& name)
    {
        std::strncpy(deviceNameBuf, name.c_str(), sizeof(deviceNameBuf) - 1);
        deviceNameBuf[sizeof(deviceNameBuf) - 1] = '\0';
        std::strncpy(studio.midiDeviceName, deviceNameBuf, sizeof(studio.midiDeviceName) - 1);
    }

    bool findAndConnectTargetDevice()
    {
        if (!seq) return false;

        snd_seq_client_info_t* cinfo;
        snd_seq_port_info_t* pinfo;
        snd_seq_client_info_alloca(&cinfo);
        snd_seq_port_info_alloca(&pinfo);

        int bestClient = -1;
        int bestPort = -1;
        std::string bestName = "";

        snd_seq_client_info_set_client(cinfo, -1);
        while (snd_seq_query_next_client(seq, cinfo) >= 0) {
            int client = snd_seq_client_info_get_client(cinfo);
            // Skip system and our own client
            if (client == 0 || client == snd_seq_client_id(seq)) continue;

            std::string cname = snd_seq_client_info_get_name(cinfo);

            snd_seq_port_info_set_client(pinfo, client);
            snd_seq_port_info_set_port(pinfo, -1);
            while (snd_seq_query_next_port(seq, pinfo) >= 0) {
                int p = snd_seq_port_info_get_port(pinfo);
                unsigned int caps = snd_seq_port_info_get_capability(pinfo);

                // Port must support READ / SUBS_READ
                if ((caps & (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ)) == (SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ)) {
                    std::string portName = snd_seq_port_info_get_name(pinfo);
                    std::string fullName = cname + " " + portName;

                    // Prefer "EDGE" device specifically
                    if (cname.find("EDGE") != std::string::npos || portName.find("EDGE") != std::string::npos) {
                        bestClient = client;
                        bestPort = p;
                        bestName = fullName;
                        break;
                    } else if (bestClient < 0 && cname.find("Midi Through") == std::string::npos && cname.find("System") == std::string::npos) {
                        // Fallback to any physical MIDI input device
                        bestClient = client;
                        bestPort = p;
                        bestName = fullName;
                    }
                }
            }
            if (bestName.find("EDGE") != std::string::npos) break;
        }

        if (bestClient >= 0 && bestPort >= 0) {
            int err = snd_seq_connect_from(seq, inPort, bestClient, bestPort);
            if (err >= 0) {
                connectedClient = bestClient;
                connectedPort = bestPort;
                connected.store(true);
                updateDeviceName(bestName);
                std::cout << "[MIDI] Connected to MIDI clock source: " << bestName << " (" << bestClient << ":" << bestPort << ")" << std::endl;
                studio.midiConnected.store(true);
                return true;
            }
        }

        return false;
    }

    void processMidiLoop()
    {
        lastClockTime = std::chrono::steady_clock::now();
        lastEventTime = std::chrono::steady_clock::now();

        while (keep_running) {
            if (!connected.load()) {
                if (findAndConnectTargetDevice()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                } else {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
            }

            // Read ALSA sequencer events
            snd_seq_event_t* ev = nullptr;
            int pending = snd_seq_event_input_pending(seq, 1);
            if (pending > 0 && snd_seq_event_input(seq, &ev) >= 0 && ev) {
                auto now = std::chrono::steady_clock::now();
                lastEventTime = now;

                if (ev->type == SND_SEQ_EVENT_CLOCK) {
                    receivingClock.store(true);
                    studio.midiSyncActive.store(true);

                    // Calculate BPM based on tick interval (24 ticks per quarter note)
                    auto durUs = std::chrono::duration_cast<std::chrono::microseconds>(now - lastClockTime).count();
                    lastClockTime = now;

                    if (durUs > 1000 && durUs < 200000) { // Reasonable tick duration (~12.5 BPM to 2500 BPM)
                        float instBpm = 60.0f * 1000000.0f / (24.0f * static_cast<float>(durUs));
                        float current = detectedBpm.load();
                        float smoothed = 0.80f * current + 0.20f * instBpm;
                        detectedBpm.store(smoothed);
                        studio.midiBpm.store(smoothed);
                    }

                    tickCounter++;
                    if (tickCounter >= 6) { // 6 ticks = 1/16th note step
                        tickCounter = 0;
                        studio.midiStepPending.fetch_add(1);
                    }
                } else if (ev->type == SND_SEQ_EVENT_START || ev->type == SND_SEQ_EVENT_CONTINUE) {
                    tickCounter = 5; // Next clock tick (tick 6) triggers step 0 immediately
                    studio.currentStep.store(SEQ_STEPS_MINI - 1);
                    studio.isPlaying.store(true);
                    receivingClock.store(true);
                    studio.midiSyncActive.store(true);
                    std::cout << "[MIDI] Received START/CONTINUE transport message" << std::endl;
                } else if (ev->type == SND_SEQ_EVENT_STOP) {
                    studio.isPlaying.store(false);
                    receivingClock.store(false);
                    studio.midiSyncActive.store(false);
                    std::cout << "[MIDI] Received STOP transport message" << std::endl;
                }

                snd_seq_free_event(ev);
            } else {
                // Check for clock timeout (no clock for > 800ms)
                auto now = std::chrono::steady_clock::now();
                if (receivingClock.load() && std::chrono::duration_cast<std::chrono::milliseconds>(now - lastEventTime).count() > 800) {
                    receivingClock.store(false);
                    studio.midiSyncActive.store(false);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        }
    }

public:
    MidiClockInput() = default;

    ~MidiClockInput()
    {
        stop();
    }

    bool init()
    {
        int err = snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK);
        if (err < 0) {
            std::cerr << "[MIDI] Warning: Unable to open ALSA Sequencer (" << snd_strerror(err) << "). MIDI sync disabled." << std::endl;
            return false;
        }

        snd_seq_set_client_name(seq, "zicPixMini");

        inPort = snd_seq_create_simple_port(seq, "MIDI Clock In",
            SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);

        if (inPort < 0) {
            std::cerr << "[MIDI] Warning: Failed to create ALSA input port." << std::endl;
            snd_seq_close(seq);
            seq = nullptr;
            return false;
        }

        std::cout << "[MIDI] ALSA Sequencer initialized successfully." << std::endl;
        return true;
    }

    void start()
    {
        if (seq) {
            midiThread = std::thread(&MidiClockInput::processMidiLoop, this);
            pthread_setname_np(midiThread.native_handle(), "zicPix_Midi");
        }
    }

    void stop()
    {
        if (midiThread.joinable()) {
            midiThread.join();
        }
        if (seq) {
            if (connected.load() && connectedClient >= 0 && connectedPort >= 0) {
                snd_seq_disconnect_from(seq, inPort, connectedClient, connectedPort);
            }
            snd_seq_close(seq);
            seq = nullptr;
        }
    }

    bool isConnected() const { return connected.load(); }
    bool isReceivingClock() const { return receivingClock.load(); }
    float getBpm() const { return detectedBpm.load(); }
    const char* getDeviceName() const { return deviceNameBuf; }
};
